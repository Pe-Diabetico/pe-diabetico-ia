import pandas as pd


def load_data(path='data/pacientes.csv'):
    return pd.read_csv(path)


def preprocess(df):
    df = df.dropna()
    # custom transformations...
    return df
