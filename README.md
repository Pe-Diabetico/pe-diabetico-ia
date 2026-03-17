# 🦶 pe-diabetico-ia

Monitor BLE de pressão plantar + predição de risco de úlcera em pé diabético.

```
pe-diabetico-ia/
├── firmware/esp32/
│   ├── monitor_plantar.ino   ← Upload para o ESP32
│   └── README.md             ← Pinagem e bibliotecas
│
├── app/
│   ├── index.html            ← Abra no Chrome — funciona offline
│   ├── manifest.json         ← PWA instalável no celular
│   ├── css/style.css
│   └── js/
│       ├── ble.js            ← Web Bluetooth + cálculo PTI
│       ├── storage.js        ← localStorage + CSV automático em disco
│       ├── ui.js             ← Atualização de tela
│       └── app.js            ← Lê config.yaml, calcula zonas, orquestra
│
└── modelo/
    ├── config.yaml           ← ⚙️  ÚNICO LUGAR PARA MUDAR LIMIARES
    ├── modelo_rf.py          ← python modelo_rf.py --treinar
    ├── target_engine.py      ← Gera target lendo as zonas do YAML
    ├── data_pipeline.py
    ├── config_loader.py
    └── data/pacientes.csv    ← CSV exportado pelo app vai aqui
```

---

## Fluxo completo

```
ESP32 (BLE, 500ms) → app/index.html → CSV em disco (automático)
                                     → localStorage (backup)
                          ↓
                  modelo/data/pacientes.csv
                          ↓
             python modelo_rf.py --treinar
```

---

## Passos para rodar o protótipo

### 1. ESP32
```
Arduino IDE → abrir firmware/esp32/monitor_plantar.ino → Upload
```

### 2. App
```bash
# Opção A — servidor local (recomendado para BLE funcionar):
cd app
python -m http.server 8080
# Abra http://localhost:8080 no Chrome

# Opção B — hospedar no Vercel/Netlify (HTTPS automático):
# arraste a pasta app/ para netlify.com/drop
```

### 3. Usar o app
1. Preencha os dados do paciente
2. Clique **"Abrir / Criar CSV"** → escolha onde salvar o arquivo
3. Clique **"Conectar ao MonitorPlantar"**
4. Os dados aparecem ao vivo e são gravados no CSV a cada 2 segundos
5. Ao final, o arquivo CSV está pronto para o modelo

### 4. Treinar o modelo
```bash
cd modelo
pip install scikit-learn pandas numpy pyyaml

# Copie o CSV exportado pelo app para modelo/data/pacientes.csv
python modelo_rf.py --treinar
```

---

## Ajustar limiares

Edite **apenas** `modelo/config.yaml`. As mudanças valem para o app E para o Python:

```yaml
sensores:
  umidade_pct:
    seguro_min: 40    # abaixo disso → amarelo (ressecamento)
    seguro_max: 60    # acima disso  → amarelo (maceração)
    atencao_min: 35   # abaixo disso → vermelho
    atencao_max: 65   # acima disso  → vermelho

  pressao_maxima_kpa:
    amarelo:  [150, 195]   # faixa de atenção
    vermelho: [196, null]  # faixa de risco (null = sem limite superior)
```

---

## Requisitos

- Chrome ou Edge (Web Bluetooth)
- Python 3.9+ com scikit-learn, pandas, numpy, pyyaml
- iOS: use o app **Bluefy** (App Store)
