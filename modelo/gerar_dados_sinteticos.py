"""
gerar_dados_sinteticos.py
=========================
Gera dataset sintético para treinar o Random Forest.
Os limiares clínicos são lidos do config.yaml com base na literatura

Uso (da raiz do projeto):
    python modelo/gerar_dados_sinteticos.py

Saída:
    modelo/data/pacientes.csv
"""

import numpy as np
import pandas as pd
import yaml
from pathlib import Path
from datetime import datetime, timedelta

# ── Caminhos ──────────────────────────────────────────────────
CAMINHO_CONFIG = Path('modelo/config.yaml')
CAMINHO_SAIDA  = Path('modelo/data/pacientes.csv')

# ── Parâmetros de geração (estes ficam aqui, não no config.yaml)
N_PACIENTES      = 400
PROPORCAO_RISCO  = 0.40   # 40% alto risco claro
PROPORCAO_AMBIG  = 0.20   # 20% zona cinzenta
LEITURAS_POR_PAC = 4
SEMENTE          = 42

# ── Carrega config.yaml ───────────────────────────────────────

def carregar_config():
    if not CAMINHO_CONFIG.exists():
        raise FileNotFoundError(
            f"config.yaml não encontrado em: {CAMINHO_CONFIG}\n"
            "Certifique-se de rodar da raiz do projeto."
        )
    with open(CAMINHO_CONFIG, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)

CFG = carregar_config()

# Atalhos para os limiares lidos do YAML
_S  = CFG['sensores']
_C  = CFG['clinicos']
_SC = CFG['score']

# Limiares extraídos — usados nos PARAMS e no calcular_risco_score()
PRESS_AMARELO   = _S['pressao_maxima_kpa']['amarelo'][0]    # 150
PRESS_VERMELHO  = _S['pressao_maxima_kpa']['vermelho'][0]   # 196
TEMP_VERMELHO   = _S['temperatura_max_c']['vermelho'][0]    # 33.5
TEMP_AMARELO    = _S['temperatura_max_c']['amarelo'][0]     # 33.0
UMID_SEG_MIN    = _S['umidade_pct']['seguro_min']           # 40
UMID_SEG_MAX    = _S['umidade_pct']['seguro_max']           # 60
UMID_ATE_MIN    = _S['umidade_pct']['atencao_min']          # 30
UMID_ATE_MAX    = _S['umidade_pct']['atencao_max']          # 75
HBA1C_AMARELO   = _C['hba1c_pct']['amarelo'][0]             # 7.0
HBA1C_VERMELHO  = _C['hba1c_pct']['vermelho'][0]            # 8.0
ANOS_AMARELO    = _C['anos_diabetes']['amarelo'][0]         # 10
ANOS_VERMELHO   = _C['anos_diabetes']['vermelho'][0]        # 15
IMC_VERMELHO    = _C['imc']['vermelho'][0]                  # 30.0
IDADE_AMARELO   = _C['idade_anos']['amarelo'][0]            # 60
LIMIAR_RISCO    = _SC['limiar_alto_risco']                  # 5

print(f"[config] Limiares carregados de {CAMINHO_CONFIG}")
print(f"  Pressão risco  : >= {PRESS_VERMELHO} kPa")
print(f"  Temperatura    : >= {TEMP_VERMELHO} °C")
print(f"  Umidade segura : {UMID_SEG_MIN}% – {UMID_SEG_MAX}%")
print(f"  HbA1c risco    : >= {HBA1C_VERMELHO}%")
print(f"  Limiar score   : >= {LIMIAR_RISCO} pontos")

# ── Parâmetros por classe ─────────────────────────────────────
# Os limites dos sensores usam os limiares do config como referência.
# Baixo risco: sempre abaixo do limiar amarelo
# Alto risco:  sempre acima do limiar vermelho
# Zona cinzenta: cruza os dois limiares

PARAMS = {

    0: {  # Baixo risco claro
        'idade':         (35,  58),
        'imc':           (19,  26),
        'anos_diabetes': (1,    8),
        'hba1c':         (5.0, HBA1C_AMARELO - 0.1),
        'prob_has':       0.30,
        'prob_tabagismo': 0.12,
        'kpa_min': 70,  'kpa_max': PRESS_AMARELO - 5,
        'temp_min': 27.0, 'temp_max': TEMP_AMARELO - 0.5,
        'umid_min': UMID_SEG_MIN + 2, 'umid_max': UMID_SEG_MAX,
    },

    1: {  # Alto risco claro
        'idade':         (62,  80),
        'imc':           (IMC_VERMELHO, 44),
        'anos_diabetes': (ANOS_AMARELO + 1, 32),
        'hba1c':         (HBA1C_VERMELHO, 11.5),
        'prob_has':       0.815,
        'prob_tabagismo': 0.25,
        'kpa_min': PRESS_VERMELHO + 4, 'kpa_max': 320,
        'temp_min': TEMP_VERMELHO + 0.3, 'temp_max': 38.0,
        'umid_min': None, 'umid_max': None,  # lógica especial
    },

    'ambiguo': {  # Zona cinzenta — cruza os limiares
        'idade':         (50,  70),
        'imc':           (25,  34),
        'anos_diabetes': (7,   15),
        'hba1c':         (HBA1C_AMARELO - 0.5, HBA1C_VERMELHO + 0.5),
        'prob_has':       0.55,
        'prob_tabagismo': 0.20,
        'kpa_min': PRESS_AMARELO - 20, 'kpa_max': PRESS_VERMELHO + 30,
        'temp_min': TEMP_AMARELO - 0.5, 'temp_max': TEMP_VERMELHO + 1.0,
        'umid_min': UMID_ATE_MIN - 2, 'umid_max': UMID_ATE_MAX + 2,
    },
}

NOMES = [f'paciente_{i+1:03d}' for i in range(N_PACIENTES)]

# ── Cálculo de risco por score (lê pesos do config.yaml) ─────

def calcular_risco_score(row):
    """
    Calcula o score de risco usando os pesos e limiares do config.yaml.
    Usado para determinar o target dos casos da zona cinzenta.
    """
    score = 0

    # Sensores — pressão
    kpa_max = max(row['kpa_calcaneo'], row['kpa_meta1'], row['kpa_meta5'])
    if kpa_max >= PRESS_VERMELHO:
        score += _SC['pressao_vermelho']
    elif kpa_max >= PRESS_AMARELO:
        score += _SC['pressao_amarelo']

    # Sensores — temperatura
    if row['temp'] >= TEMP_VERMELHO:
        score += _SC['temperatura_vermelho']
    elif row['temp'] >= TEMP_AMARELO:
        score += _SC['temperatura_amarelo']

    # Sensores — umidade (lógica dos dois extremos)
    if row['umid'] < UMID_ATE_MIN or row['umid'] > UMID_ATE_MAX:
        score += _SC['umidade_fora_faixa']
    elif row['umid'] < UMID_SEG_MIN or row['umid'] > UMID_SEG_MAX:
        score += _SC['umidade_fora_faixa']

    # Clínicos — HbA1c
    if row['hba1c'] >= HBA1C_VERMELHO:
        score += _C['hba1c_pct']['peso_risco_vermelho']
    elif row['hba1c'] >= HBA1C_AMARELO:
        score += _C['hba1c_pct']['peso_risco_amarelo']

    # Clínicos — anos de diabetes
    if row['anos_diabetes'] >= ANOS_VERMELHO:
        score += _C['anos_diabetes']['peso_risco_vermelho']
    elif row['anos_diabetes'] >= ANOS_AMARELO:
        score += _C['anos_diabetes']['peso_risco_amarelo']

    # Clínicos — IMC
    if row['imc'] >= IMC_VERMELHO:
        score += _C['imc']['peso_risco_vermelho']

    # Clínicos — binários
    score += row['tabagismo'] * _C['tabagismo']['peso_risco']
    score += row['has']       * _C['pressao_arterial']['peso_risco']

    # Clínicos — idade
    if row['idade'] >= IDADE_AMARELO:
        score += _C['idade_anos']['peso_risco_amarelo']

    return 1 if score >= LIMIAR_RISCO else 0

# ── Gerador por paciente ──────────────────────────────────────

def gerar_paciente(rng, tipo, idx, data_base):
    p    = PARAMS[tipo]
    nome = NOMES[idx % len(NOMES)]

    idade         = int(rng.integers(p['idade'][0],         p['idade'][1] + 1))
    imc           = round(float(rng.uniform(p['imc'][0],    p['imc'][1])), 1)
    anos_diabetes = int(rng.integers(p['anos_diabetes'][0], p['anos_diabetes'][1] + 1))
    hba1c         = round(float(rng.uniform(p['hba1c'][0],  p['hba1c'][1])), 1)
    has           = int(rng.random() < p['prob_has'])
    tabagismo     = int(rng.random() < p['prob_tabagismo'])

    linhas = []

    for i in range(LEITURAS_POR_PAC):
        ts = data_base - timedelta(
            days=LEITURAS_POR_PAC - i,
            hours=int(rng.integers(0, 8))
        )

        kpa_base     = rng.uniform(p['kpa_min'], p['kpa_max'])
        kpa_calcaneo = round(kpa_base * rng.uniform(1.0, 1.25), 1)
        kpa_meta1    = round(kpa_base * rng.uniform(0.85, 1.10), 1)
        kpa_meta5    = round(kpa_base * rng.uniform(0.65, 0.90), 1)

        temp = round(rng.uniform(p['temp_min'], p['temp_max']), 1)

        if tipo == 1:
            # Alto risco: extremos de umidade
            umid = round(rng.uniform(UMID_ATE_MAX, 92), 1) if rng.random() < 0.5 \
                   else round(rng.uniform(18, UMID_ATE_MIN), 1)
        else:
            umid = round(rng.uniform(p['umid_min'], p['umid_max']), 1)

        row = {
            'timestamp':     ts.strftime('%Y-%m-%dT%H:%M:%S'),
            'nome':          nome,
            'idade':         idade,
            'imc':           imc,
            'anos_diabetes': anos_diabetes,
            'hba1c':         hba1c,
            'has':           has,
            'tabagismo':     tabagismo,
            'kpa_calcaneo':  kpa_calcaneo,
            'kpa_meta1':     kpa_meta1,
            'kpa_meta5':     kpa_meta5,
            'temp':          temp,
            'umid':          umid,
        }

        row['risco_ulcera'] = calcular_risco_score(row) if tipo == 'ambiguo' else tipo
        linhas.append(row)

    return linhas

# ── Dataset ───────────────────────────────────────────────────

def gerar_dataset():
    rng     = np.random.default_rng(SEMENTE)
    n_ambig = int(N_PACIENTES * PROPORCAO_AMBIG)
    n_resto = N_PACIENTES - n_ambig
    n_alto  = int(n_resto * PROPORCAO_RISCO)
    n_baixo = n_resto - n_alto

    print(f"\nGerando {N_PACIENTES} pacientes:")
    print(f"  Alto risco claro  (1): {n_alto}")
    print(f"  Baixo risco claro (0): {n_baixo}")
    print(f"  Zona cinzenta   (mix): {n_ambig}")
    print(f"  Leituras por paciente: {LEITURAS_POR_PAC}")
    print(f"  Total de linhas: {N_PACIENTES * LEITURAS_POR_PAC}")

    data_base = datetime.now()
    linhas    = []

    for i in range(n_alto):
        linhas.extend(gerar_paciente(rng, 1, i, data_base))
    for i in range(n_baixo):
        linhas.extend(gerar_paciente(rng, 0, i + n_alto, data_base))
    for i in range(n_ambig):
        linhas.extend(gerar_paciente(rng, 'ambiguo', i + n_alto + n_baixo, data_base))

    df = pd.DataFrame(linhas)
    return df.sample(frac=1, random_state=SEMENTE).reset_index(drop=True)

# ── Validação ─────────────────────────────────────────────────

def validar(df):
    print("\n── VALIDAÇÃO DO DATASET ─────────────────────────────────")
    print(f"Total de linhas : {len(df)}")

    dist = df['risco_ulcera'].value_counts()
    print(f"\n── DISTRIBUIÇÃO DO TARGET")
    print(f"  Baixo risco (0): {dist.get(0,0)} ({100*dist.get(0,0)/len(df):.1f}%)")
    print(f"  Alto risco  (1): {dist.get(1,0)} ({100*dist.get(1,0)/len(df):.1f}%)")

    cols  = ['idade','imc','anos_diabetes','hba1c',
             'kpa_calcaneo','kpa_meta1','kpa_meta5','temp','umid']
    alto  = df[df['risco_ulcera'] == 1]
    baixo = df[df['risco_ulcera'] == 0]

    print(f"\n── VERIFICAÇÕES (limiares do config.yaml)")
    print(f"  Pressão >= {PRESS_VERMELHO} kPa em alto risco : {(alto['kpa_meta1']>=PRESS_VERMELHO).mean()*100:.1f}%")
    print(f"  Pressão <  {PRESS_VERMELHO} kPa em baixo risco: {(baixo['kpa_meta1']<PRESS_VERMELHO).mean()*100:.1f}%")
    print(f"  HAS em alto risco  : {alto['has'].mean()*100:.1f}%  (ref: 81.5%)")
    print(f"  HAS em baixo risco : {baixo['has'].mean()*100:.1f}%")

    print("\n── PRIMEIRAS 3 LINHAS")
    print(df.head(3).to_string(index=False))

# ── Main ──────────────────────────────────────────────────────

if __name__ == '__main__':
    CAMINHO_SAIDA.parent.mkdir(parents=True, exist_ok=True)
    df = gerar_dataset()
    validar(df)
    df.to_csv(CAMINHO_SAIDA, index=False)
    print(f"\nSalvo em: {CAMINHO_SAIDA}")
    print("Próximo passo: python modelo/exportar_modelo_esp32.py")