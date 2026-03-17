from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score


def train(df, config):
    X = df.drop(columns=['target'])
    y = df['target']
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=config['random_forest']['random_state'])
    model = RandomForestClassifier(
        n_estimators=config['random_forest']['n_estimators'],
        max_depth=config['random_forest']['max_depth'],
        random_state=config['random_forest']['random_state'])
    model.fit(X_train, y_train)
    pred = model.predict(X_test)
    print('Acurácia:', accuracy_score(y_test, pred))
    return model


def predict(model, X):
    return model.predict_proba(X)[:, 1]
