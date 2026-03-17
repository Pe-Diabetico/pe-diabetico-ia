"""
target_engine.py
================
Gera o target (risco_ulcera) lendo as zonas diretamente do config.yaml.

Cada feature contribui com pontos ao score conforme a zona em que se encontra:
  verde    → 0 pontos
  amarelo  → peso_amarelo pontos
  vermelho → peso_vermelho pontos

Se score_total >= score.limiar_alto_risco → risco_ulcera = 1
"""

import pandas as pd
import logging
from config_loader import CONFIG

logger = logging.getLogger(__name__)

_S = CONFIG['sensores']
_C = CONFIG['clinicos']
_SC = CONFIG['score']


# ─── Funções de zona ───────────────────────────────────────────────────────

def zona_linear(valor, cfg_feature):
    """Retorna 'verde', 'amarelo' ou 'vermelho' para features com zonas lineares."""
    v_min = cfg_feature['vermelho'][0]
    a_min = cfg_feature['amarelo'][0]
    if v_min is not None and valor >= v_min:
        return 'vermelho'
    if a_min is not None and valor >= a_min:
        return 'amarelo'
    return 'verde'


def zona_umidade(umid, cfg_umid):
    """
    Zona especial para umidade — ruim nos dois extremos.
    Lê os limiares seguro_min/max e atencao_min/max do config.yaml.
    """
    s_min = cfg_umid['seguro_min']
    s_max = cfg_umid['seguro_max']
    a_min = cfg_umid['atencao_min']
    a_max = cfg_umid['atencao_max']

    if umid < a_min or umid > a_max:
        return 'vermelho'
    if umid < s_min or umid > s_max:
        return 'amarelo'
    return 'verde'


def _pontos_zona(zona, peso_amarelo, peso_vermelho):
    if zona == 'vermelho':
        return peso_vermelho
    if zona == 'amarelo':
        return peso_amarelo
    return 0


# ─── Score por linha ───────────────────────────────────────────────────────

def calcular_score(df: pd.DataFrame) -> pd.Series:
    """Calcula o score de risco para cada linha do DataFrame."""
    score = pd.Series(0, index=df.index, dtype=float)

    # ── Sensores ──────────────────────────────────────────────

    # Pressão máxima
    z_press = df['pico_pressao_max_kpa'].apply(
        lambda v: zona_linear(v, _S['pressao_maxima_kpa'])
    )
    score += z_press.map(lambda z: _pontos_zona(
        z, _SC['pressao_amarelo'], _SC['pressao_vermelho']
    ))

    # PTI
    z_pti = df['pti_max_kpa_s'].apply(
        lambda v: zona_linear(v, _S['pti_kpa_s'])
    )
    score += z_pti.map(lambda z: _pontos_zona(
        z, _SC['pti_amarelo'], _SC['pti_vermelho']
    ))

    # Temperatura
    if 'temp_media_c' in df.columns:
        z_temp = df['temp_media_c'].apply(
            lambda v: zona_linear(v, _S['temperatura_max_c']) if pd.notna(v) else 'verde'
        )
        score += z_temp.map(lambda z: _pontos_zona(
            z, _SC['temperatura_amarelo'], _SC['temperatura_vermelho']
        ))

    # Umidade (lógica especial)
    if 'umidade_media_pct' in df.columns:
        z_umid = df['umidade_media_pct'].apply(
            lambda v: zona_umidade(v, _S['umidade_pct']) if pd.notna(v) else 'verde'
        )
        score += z_umid.map(
            lambda z: _SC['umidade_fora_faixa'] if z != 'verde' else 0
        )

    # ── Clínicos ──────────────────────────────────────────────

    # HbA1c
    if 'hba1c' in df.columns:
        z_hba = df['hba1c'].apply(lambda v: zona_linear(v, _C['hba1c_pct']))
        score += z_hba.map(lambda z: _pontos_zona(
            z,
            _C['hba1c_pct']['peso_risco_amarelo'],
            _C['hba1c_pct']['peso_risco_vermelho']
        ))

    # Anos de DM
    if 'anos_diabetes' in df.columns:
        z_anos = df['anos_diabetes'].apply(lambda v: zona_linear(v, _C['anos_diabetes']))
        score += z_anos.map(lambda z: _pontos_zona(
            z,
            _C['anos_diabetes']['peso_risco_amarelo'],
            _C['anos_diabetes']['peso_risco_vermelho']
        ))

    # IMC
    if 'imc' in df.columns:
        z_imc = df['imc'].apply(lambda v: zona_linear(v, _C['imc']))
        score += z_imc.map(lambda z: _pontos_zona(
            z,
            _C['imc']['peso_risco_amarelo'],
            _C['imc']['peso_risco_vermelho']
        ))

    # Variáveis binárias
    if 'tabagismo' in df.columns:
        score += df['tabagismo'].fillna(0) * _C['tabagismo']['peso_risco']
    if 'has' in df.columns:
        score += df['has'].fillna(0) * _C['pressao_arterial']['peso_risco']

    return score


def gerar_target(df: pd.DataFrame) -> pd.DataFrame:
    """
    Adiciona colunas 'score_risco' e 'risco_ulcera' ao DataFrame.

    Se 'risco_ulcera' já existir com valores preenchidos (dados reais),
    mantém os valores reais e apenas calcula o score para análise.
    """
    df = df.copy()
    df['score_risco'] = calcular_score(df)
    limiar = _SC['limiar_alto_risco']

    if 'risco_ulcera' in df.columns and df['risco_ulcera'].notna().all():
        logger.info("risco_ulcera encontrado no CSV — usando labels reais.")
    else:
        df['risco_ulcera'] = (df['score_risco'] >= limiar).astype(int)
        n_alto = df['risco_ulcera'].sum()
        logger.info(
            f"Target gerado via regras (limiar={limiar}). "
            f"Alto risco: {n_alto}/{len(df)} ({100*n_alto/len(df):.1f}%)"
        )

    return df
