# pe-diabetico-ia

Monitor BLE de pressão plantar com predição de risco de úlcera em pé diabético.
O modelo Random Forest roda diretamente no ESP32 — sem servidor, sem nuvem.

Demo: [pe-diabetico-ia.netlify.app](https://pe-diabetico-ia.netlify.app)

---

## Estrutura

```
pe-diabetico-ia/
├── firmware/
│   └── esp32/
│       ├── monitor_plantar.ino   ← firmware principal
│       ├── model.h               ← modelo RF gerado pelo Python
│       └── README.md             ← pinagem e bibliotecas
│
├── app/
│   ├── index.html
│   ├── manifest.json
│   ├── css/
│   │   └── style.css
│   ├── img/
│   │   └── desenho_pe.png
│   └── js/
│       └── app.js                ← BLE, zonas clínicas, CSV, cadastro
│
└── modelo/
    ├── config.yaml               ← ÚNICO LUGAR PARA MUDAR LIMIARES
    ├── gerar_dados_sinteticos.py ← gera pacientes.csv sintético
    ├── exportar_modelo_esp32.py  ← treina RF e exporta model.h
    └── data/
        └── pacientes.csv
```

---

## Como funciona

```
ESP32
  Lê FSR (pressão) + DHT22 (temperatura, umidade)
  Roda predict() do model.h com 11 features
  Exibe risco no OLED
  Envia JSON via BLE a cada 500ms

App (Chrome / Edge)
  Recebe dados via Web Bluetooth
  Exibe leituras e alertas em tempo real
  Profissional cadastra dados clínicos do paciente (uma vez)
  Exporta CSV ao final da sessão

Modelo (computador)
  python modelo/gerar_dados_sinteticos.py
  python modelo/exportar_modelo_esp32.py
  → gera firmware/esp32/model.h
  → compila e sobe no ESP32 via Arduino IDE
```

---

## Passos para rodar

### 1. Gerar o modelo

```bash
pip install scikit-learn pandas numpy pyyaml micromlgen

python modelo/gerar_dados_sinteticos.py
python modelo/exportar_modelo_esp32.py
```

O arquivo `firmware/esp32/model.h` será gerado automaticamente.

### 2. Subir o firmware

```
Arduino IDE → abrir firmware/esp32/monitor_plantar.ino → Upload
```

Bibliotecas necessárias (Library Manager):
- Adafruit SSD1306
- Adafruit GFX Library
- DHT sensor library (Adafruit)
- ArduinoJson 6.x

### 3. Usar o app

Acesse [pe-diabetico-ia.netlify.app](https://pe-diabetico-ia.netlify.app) no Chrome ou Edge.

Ou rode localmente:

```bash
cd app
python -m http.server 8080
# Abra http://localhost:8080 no Chrome
```

Fluxo de uso:
1. Clique em **Area do Profissional** (rodape) e cadastre os dados do paciente
2. Clique em **Conectar** e selecione o dispositivo MonitorPlantar
3. Os dados aparecem em tempo real com alertas clinicos
4. Ao final, clique em **Baixar CSV** para exportar as leituras

### 4. Re-treinar com dados reais

```bash
# Copie o CSV exportado pelo app para:
# modelo/data/pacientes.csv

python modelo/exportar_modelo_esp32.py
# → gera novo model.h
# → suba o firmware novamente no Arduino IDE
```

---

## Ajustar limiares clinicos

Edite apenas `modelo/config.yaml`. As mudancas propagam para o gerador de dados e para o modelo:

```yaml
sensores:
  pressao_maxima_kpa:
    amarelo:  [150, 195]   # >= 150 kPa: atencao
    vermelho: [196, null]  # >= 196 kPa: alerta (IWGDF)

  temperatura_max_c:
    amarelo:  [33.0, 33.4]
    vermelho: [33.5, null]  # >= 33.5 graus C: alerta (IWGDF)

  umidade_pct:
    seguro_min: 40   # abaixo: atencao ressecamento
    seguro_max: 60   # acima:  atencao maceracao
    atencao_min: 30  # abaixo: alerta ressecamento grave
    atencao_max: 75  # acima:  alerta maceracao (Kosaji, 2025)

random_forest:
  n_estimators: 20   # arvores — manter baixo para caber no ESP32
  max_depth:    10
```

---

## Requisitos

| Componente | Requisito |
|---|---|
| Browser | Chrome ou Edge (Web Bluetooth) |
| Python | 3.9+ |
| Arduino IDE | 2.x com core ESP32 |
| iOS | App Bluefy (App Store) — Safari nao suporta Web Bluetooth |

---

## Referencias

- IWGDF Guidelines (2023) — limiares de pressao plantar e temperatura
- Chauhan (2023) — Random Forest para classificacao de risco plantar
- Castillo-Morquecho (2024) — HbA1c e IMC como preditores
- Tavares et al. (2016) — prevalencias clinicas (HAS, tabagismo, anos de DM)
- Lourenco (2018) — microclima do calcado
- Kosaji et al. (2025) — maceracao e ressecamento plantar