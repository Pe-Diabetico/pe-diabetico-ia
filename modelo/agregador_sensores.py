import pandas as pd


def agregate(raw_data):
    # Exemplo simplificado de agregação de sinal plantar
    df = pd.DataFrame(raw_data)
    return df.groupby('segmento').mean().reset_index()
