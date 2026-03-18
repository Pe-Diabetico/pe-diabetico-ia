"""
exportar_modelo_esp32.py
========================
Treina o Random Forest com todas as 11 features e exporta
como C++ (model.h) para rodar no ESP32.
Os hiperparâmetros são lidos do config.yaml.

Uso (da raiz do projeto):
    pip install micromlgen scikit-learn pandas numpy pyyaml
    python modelo/exportar_modelo_esp32.py

Saída:
    firmware/esp32/model.h
"""

import pandas as pd
import numpy as np
import yaml
from pathlib import Path
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split, cross_val_score
from sklearn.metrics import classification_report
from micromlgen import port

# ── Caminhos ──────────────────────────────────────────────────
CAMINHO_CONFIG = Path('modelo/config.yaml')
CAMINHO_CSV    = Path('modelo/data/pacientes.csv')
CAMINHO_SAIDA  = Path('firmware/esp32/model.h')
RAM_LIMITE_KB  = 300

# ── Features — ordem DEVE ser idêntica ao firmware ────────────
FEATURES = [
    'idade', 'imc', 'anos_diabetes', 'hba1c', 'has', 'tabagismo',
    'kpa_calcaneo', 'kpa_meta1', 'kpa_meta5', 'temp', 'umid',
]

# ── [1] Carrega config.yaml ───────────────────────────────────

print("[1] Carregando configuração...")

if not CAMINHO_CONFIG.exists():
    raise FileNotFoundError(f"config.yaml não encontrado: {CAMINHO_CONFIG}")

with open(CAMINHO_CONFIG, 'r', encoding='utf-8') as f:
    CFG = yaml.safe_load(f)

RF = CFG['random_forest']

RF_PARAMS = {
    'n_estimators':   RF['n_estimators'],
    'max_depth':      RF['max_depth'],
    'min_samples_leaf': RF.get('min_samples_leaf', 3),
    'max_features':   RF.get('max_features', 'sqrt'),
    'class_weight':   RF.get('class_weight', 'balanced'),
    'random_state':   RF.get('random_state', 42),
}

print(f"    n_estimators  : {RF_PARAMS['n_estimators']}")
print(f"    max_depth     : {RF_PARAMS['max_depth']}")
print(f"    min_samples_leaf: {RF_PARAMS['min_samples_leaf']}")
print(f"    class_weight  : {RF_PARAMS['class_weight']}")

# ── [2] Carrega dados ─────────────────────────────────────────

print("\n[2] Carregando dados...")

if not CAMINHO_CSV.exists():
    raise FileNotFoundError(
        f"CSV não encontrado: {CAMINHO_CSV}\n"
        "Execute primeiro: python modelo/gerar_dados_sinteticos.py"
    )

df = pd.read_csv(CAMINHO_CSV)
print(f"    {len(df)} amostras | {len(df.columns)} colunas")

ausentes = [f for f in FEATURES if f not in df.columns]
if ausentes:
    raise ValueError(f"Colunas ausentes no CSV: {ausentes}")

if 'risco_ulcera' not in df.columns:
    raise ValueError("Coluna 'risco_ulcera' não encontrada no CSV.")

# ── [3] Prepara X e y ─────────────────────────────────────────

print("\n[3] Preparando features...")

X = df[FEATURES].copy()
for col in FEATURES:
    if X[col].isna().any():
        mediana = X[col].median()
        X[col]  = X[col].fillna(mediana)
        print(f"    NaN em '{col}' preenchido com mediana ({mediana:.2f})")

X = X.values
y = df['risco_ulcera'].values

dist = pd.Series(y).value_counts()
print(f"    Baixo risco (0): {dist.get(0,0)} amostras")
print(f"    Alto risco  (1): {dist.get(1,0)} amostras")

# ── [4] Treina ────────────────────────────────────────────────

print(f"\n[4] Treinando Random Forest...")
print(f"    {RF_PARAMS['n_estimators']} árvores | depth={RF_PARAMS['max_depth']} | {len(FEATURES)} features")

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=RF_PARAMS['random_state'], stratify=y
)

clf = RandomForestClassifier(**RF_PARAMS)
clf.fit(X_train, y_train)

print("\n    Avaliação no conjunto de teste:")
print(classification_report(
    y_test, clf.predict(X_test),
    target_names=['Baixo Risco', 'Alto Risco'],
    zero_division=0
))

cv = cross_val_score(clf, X, y, cv=5, scoring='f1')
print(f"    Validação cruzada (5 folds) — F1: {cv.mean():.4f} ± {cv.std():.4f}")

# ── [5] Verifica RAM ──────────────────────────────────────────

n_nos      = sum(t.tree_.node_count for t in clf.estimators_)
ram_kb     = (n_nos * 20) / 1024
depth_real = max(t.get_depth() for t in clf.estimators_)

print(f"\n    Nós totais       : {n_nos}")
print(f"    Profundidade real: {depth_real}")
print(f"    RAM estimada     : {ram_kb:.1f} KB de 520 KB disponíveis")

if ram_kb > RAM_LIMITE_KB:
    print(f"    AVISO: modelo grande. Reduza n_estimators ou max_depth no config.yaml.")
else:
    print(f"    OK — sobram ~{520 - ram_kb:.0f} KB para firmware e stack.")

# ── [6] Importância das features ──────────────────────────────

print("\n    Importância das features:")
for feat, imp in sorted(zip(FEATURES, clf.feature_importances_),
                        key=lambda x: x[1], reverse=True):
    print(f"    {feat:20s} {imp:.4f}  {'█' * int(imp * 40)}")

# ── [7] Exporta model.h ───────────────────────────────────────

print(f"\n[5] Exportando model.h...")

CAMINHO_SAIDA.parent.mkdir(parents=True, exist_ok=True)

codigo_h = port(clf, classmap={0: 'BAIXO', 1: 'ALTO'})

cabecalho = f"""/*
 * model.h — Random Forest embarcado para ESP32
 * ==============================================
 * Gerado por: exportar_modelo_esp32.py
 * Configuração lida de: config.yaml
 * NÃO edite manualmente — re-execute o script para atualizar.
 *
 * Modelo:
 *   n_estimators   : {RF_PARAMS['n_estimators']}
 *   max_depth      : {RF_PARAMS['max_depth']}
 *   depth real     : {depth_real}
 *   n_features     : {len(FEATURES)}
 *   n_nos_total    : {n_nos}
 *   RAM estimada   : {ram_kb:.1f} KB
 *
 * Features — array features[] no firmware DEVE seguir esta ordem:
"""
for i, f in enumerate(FEATURES):
    cabecalho += f" *   [{i:2d}] {f}\n"
cabecalho += " *\n * Classes: 0 = BAIXO RISCO | 1 = ALTO RISCO\n */\n\n"

with open(CAMINHO_SAIDA, 'w') as f:
    f.write(cabecalho + codigo_h)

print(f"    Salvo em: {CAMINHO_SAIDA}")
print("\nAbra Arduino IDE e faça upload do firmware e model.h.")