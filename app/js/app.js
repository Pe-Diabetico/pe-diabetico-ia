'use strict';

// ── UUIDs BLE ─────────────────────────────────────────────────
const BLE_SVC_UUID        = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const BLE_CHAR_NOTIFY_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'; // NOTIFY ← dados sensor
const BLE_CHAR_WRITE_UUID  = 'beb5483e-36e1-4688-b7f5-ea07361b26a9'; // WRITE  → dados clínicos

// ── Chave localStorage ────────────────────────────────────────
const LS_PACIENTE = 'monitor_pe_paciente';

// ── Estado ───────────────────────────────────────────────────
let bleDevice   = null;
let bleChar     = null;
let bleCharWrite = null;
let conectado   = false;
let demoTimer   = null;
let demoTick    = 0;
const alertLog  = [];

// Dados clínicos do paciente ativo (carregados do localStorage)
let paciente = {
  nome:        '',
  idade:       null,
  imc:         null,
  anos_dm:     null,
  hba1c:       null,
  has:         0,
  tabagismo:   0,
};

// ── Helper DOM ────────────────────────────────────────────────
function $(id) { return document.getElementById(id); }

let el = {};

document.addEventListener('DOMContentLoaded', () => {

  el = {
    dot:          $('dot'),
    connLabel:    $('conn-label'),
    btnConnect:   $('btn-connect'),
    btnLabel:     $('btn-label'),
    chkDemo:      $('chk-demo'),
    cardStatus:   $('card-status'),
    statusTitle:  $('status-title'),
    statusSub:    $('status-sub'),
    cardTemp:     $('card-temp'),
    valTemp:      $('val-temp'),
    barTemp:      $('bar-temp'),
    hintTemp:     $('hint-temp'),
    cardUmid:     $('card-umid'),
    valUmid:      $('val-umid'),
    barUmid:      $('bar-umid'),
    hintUmid:     $('hint-umid'),
    pvalMeta1:    $('pval-meta1'),   pfillMeta1:  $('pfill-meta1'),
    pvalMeta5:    $('pval-meta5'),   pfillMeta5:  $('pfill-meta5'),
    pvalCal:      $('pval-calcaneo'),pfillCal:    $('pfill-calcaneo'),
    zoneMeta1:    $('zone-meta1'),
    zoneMeta5:    $('zone-meta5'),
    zoneCal:      $('zone-calcaneo'),
    alertList:    $('alert-list'),
    lastUpdate:   $('last-update'),
    // Cadastro
    modalCadastro:   $('modal-cadastro'),
    pacienteBanner:  $('paciente-banner'),
    pacienteNome:    $('paciente-nome'),
  };

  // ── Eventos de conexão BLE
  $('btn-connect').addEventListener('click', () => {
    if (conectado) desconectarBLE();
    else           conectarBLE();
  });

  $('chk-demo').addEventListener('change', e => {
    if (e.target.checked) iniciarDemo();
    else                  pararDemo();
    atualizarCardConfig();
  });

  // Mostra card de configurações só no modo demo
  atualizarCardConfig();

  // ── Eventos do modal de cadastro
  $('btn-abrir-cadastro').addEventListener('click', abrirModal);
  $('btn-editar-paciente').addEventListener('click', abrirModal);
  $('btn-modal-fechar').addEventListener('click', fecharModal);
  $('btn-salvar-paciente').addEventListener('click', salvarPaciente);

  // Fecha modal ao clicar no overlay
  $('modal-cadastro').addEventListener('click', e => {
    if (e.target === $('modal-cadastro')) fecharModal();
  });

  // Toggles HAS e tabagismo
  $('tog-has').addEventListener('click', () => toggleBtn('tog-has'));
  $('tog-tabagismo').addEventListener('click', () => toggleBtn('tog-tabagismo'));

  // ── Carrega paciente salvo
  carregarPaciente();

  // ── Inicia demo
  if ($('chk-demo').checked) iniciarDemo();
});

// ── CADASTRO DO PACIENTE ──────────────────────────────────────

function abrirModal() {
  // Preenche o modal com os dados atuais
  $('pac-nome').value     = paciente.nome       || '';
  $('pac-idade').value    = paciente.idade      ?? '';
  $('pac-imc').value      = paciente.imc        ?? '';
  $('pac-anos-dm').value  = paciente.anos_dm    ?? '';
  $('pac-hba1c').value    = paciente.hba1c      ?? '';

  setToggle('tog-has',      paciente.has);
  setToggle('tog-tabagismo', paciente.tabagismo);

  $('modal-cadastro').classList.add('aberto');
  $('pac-nome').focus();
}

function fecharModal() {
  $('modal-cadastro').classList.remove('aberto');
}

function toggleBtn(id) {
  const btn = $(id);
  const novoVal = btn.dataset.val === '0' ? '1' : '0';
  btn.dataset.val        = novoVal;
  btn.textContent        = novoVal === '1' ? 'Sim' : 'Não';
  btn.setAttribute('aria-pressed', novoVal === '1' ? 'true' : 'false');
}

function setToggle(id, val) {
  const btn = $(id);
  const v   = String(val) === '1' ? '1' : '0';
  btn.dataset.val  = v;
  btn.textContent  = v === '1' ? 'Sim' : 'Não';
  btn.setAttribute('aria-pressed', v === '1' ? 'true' : 'false');
}

function salvarPaciente() {
  paciente = {
    nome:      $('pac-nome').value.trim()             || '',
    idade:     parseFloat($('pac-idade').value)       || null,
    imc:       parseFloat($('pac-imc').value)         || null,
    anos_dm:   parseFloat($('pac-anos-dm').value)     || null,
    hba1c:     parseFloat($('pac-hba1c').value)       || null,
    has:       parseInt($('tog-has').dataset.val)     || 0,
    tabagismo: parseInt($('tog-tabagismo').dataset.val) || 0,
  };

  localStorage.setItem(LS_PACIENTE, JSON.stringify(paciente));
  atualizarBannerPaciente();
  fecharModal();

  // Envia dados clínicos ao ESP32 se conectado
  if (conectado && bleCharWrite) enviarDadosClinicos();
}

function carregarPaciente() {
  try {
    const salvo = JSON.parse(localStorage.getItem(LS_PACIENTE));
    if (salvo) { paciente = salvo; }
  } catch { /* sem dados salvos */ }
  atualizarBannerPaciente();
}

function atualizarBannerPaciente() {
  const temNome = paciente.nome && paciente.nome.trim().length > 0;
  el.pacienteBanner.hidden = !temNome;
  if (temNome) el.pacienteNome.textContent = paciente.nome;
}

// ── Envia dados clínicos ao ESP32 via BLE WRITE ───────────────
async function enviarDadosClinicos() {
  if (!bleCharWrite) return;
  const payload = {
    idade:         paciente.idade    ?? 65,
    imc:           paciente.imc      ?? 28,
    anos_diabetes: paciente.anos_dm  ?? 8,
    hba1c:         paciente.hba1c    ?? 7.0,
    has:           paciente.has,
    tabagismo:     paciente.tabagismo,
  };
  try {
    const enc = new TextEncoder().encode(JSON.stringify(payload));
    await bleCharWrite.writeValue(enc);
    console.info('[BLE] Dados clínicos enviados ao ESP32:', payload);
  } catch (e) {
    console.warn('[BLE] Falha ao enviar dados clínicos:', e.message);
  }
}

// ── CSV ───────────────────────────────────────────────────────
// Exporta todas as leituras acumuladas na sessão
const _leituras = [];

const CSV_CABECALHO =
  'timestamp,nome,idade,imc,anos_diabetes,hba1c,has,tabagismo,' +
  'kpa_calcaneo,kpa_meta1,kpa_meta5,temp,umid,risco_ulcera,risco_prob';

function registrarLeitura(d) {
  // d.risco vem do ESP32 quando conectado ao hardware real (0 ou 1).
  // No modo demo esse campo não existe - fica em branco no CSV,
  // pois o model.h não está rodando no browser.
  _leituras.push({
    ts:          new Date().toISOString(),
    nome:        paciente.nome      || '',
    idade:       paciente.idade     ?? '',
    imc:         paciente.imc       ?? '',
    anos_dm:     paciente.anos_dm   ?? '',
    hba1c:       paciente.hba1c     ?? '',
    has:         paciente.has,
    tabagismo:   paciente.tabagismo,
    kpa_cal:     d.kpa_cal  ?? (d.adc_cal  ? adcParaKpa(d.adc_cal)  : 0),
    kpa_m1:      d.kpa_m1   ?? (d.adc_m1   ? adcParaKpa(d.adc_m1)   : 0),
    kpa_m5:      d.kpa_m5   ?? (d.adc_m5   ? adcParaKpa(d.adc_m5)   : 0),
    temp:        d.temp      ?? '',
    umid:        d.umid      ?? '',
    risco:       d.risco     ?? '',       // 0 ou 1 - resultado do model.h no ESP32
    risco_prob:  d.risco_prob ?? '',      // probabilidade em % (ex: 87.3)
  });
}

function adcParaKpa(adc) {
  return adcParaKpaNum(adc).toFixed(1);
}

function baixarCSV() {
  if (!_leituras.length) {
    alert('Nenhuma leitura registrada ainda.');
    return;
  }
  const linhas = _leituras.map(l =>
    [l.ts, `"${l.nome}"`, l.idade, l.imc, l.anos_dm, l.hba1c,
     l.has, l.tabagismo, l.kpa_cal, l.kpa_m1, l.kpa_m5,
     l.temp, l.umid, l.risco].join(',')
  );
  const blob = new Blob(
    [CSV_CABECALHO + '\n' + linhas.join('\n')],
    { type: 'text/csv' }
  );
  const url = URL.createObjectURL(blob);
  const a   = document.createElement('a');
  a.href     = url;
  a.download = `monitor_pe_${new Date().toISOString().slice(0, 10)}.csv`;
  a.click();
  URL.revokeObjectURL(url);
}

// Expõe para o botão do modal
window.baixarCSV = baixarCSV;

// ── VISIBILIDADE DO CARD DE CONFIGURAÇÕES ────────────────────
function atualizarCardConfig() {
  const cardConfig = $('card-config');
  if (!cardConfig) return;
  const emDemo = $('chk-demo') && $('chk-demo').checked;
  cardConfig.style.display = emDemo ? '' : 'none';
}

// ── LIMIARES ─────────────────────────────────────────────────
// Valores fixos do config.yaml - referência clínica (IWGDF / literatura)
// Usados sempre, exceto nos inputs do modo demo que sobrescrevem temp e umid.
const LIM = {
  // Pressão (kPa) - IWGDF / Borges (2023)
  presVermelho: 196,   // >= 196 kPa → alerta
  presAmarelo:  150,   // >= 150 kPa → atenção

  // Temperatura (°C) - IWGDF
  tempVermelho: 33.5,  // >= 33.5°C → alerta
  tempAmarelo:  33.0,  // >= 33.0°C → atenção

  // Umidade dentro do calçado (%) - Lourenço (2018) / Kosaji (2025)
  umidSegMin:   40,    // < 40% → atenção ressecamento
  umidSegMax:   60,    // > 60% → atenção maceração
  umidAteMin:   30,    // < 30% → alerta ressecamento grave
  umidAteMax:   75,    // > 75% → alerta maceração
};

// No modo demo os inputs de configuração ficam visíveis e ajustam
// apenas os limiares de demonstração - não alteram os valores clínicos reais.
function limDemo() {
  return {
    tempMax: parseFloat($('thresh-temp').value)     || LIM.tempVermelho,
    umidMax: parseFloat($('thresh-umid').value)     || LIM.umidAteMax,
    presMax: parseFloat($('thresh-pressure').value) || LIM.presVermelho, // kPa
  };
}

function estaEmDemo() {
  return $('chk-demo') && $('chk-demo').checked;
}

// ── CLASSIFICAÇÃO DE ZONA ─────────────────────────────────────

function zonaTemp(t) {
  if (estaEmDemo()) {
    // Demo: usa os inputs configuráveis da tela
    const max = limDemo().tempMax;
    if (t >= max)         return 'danger';
    if (t >= max * 0.95)  return 'warn';
    return 'ok';
  }
  // Real: limiares clínicos do config.yaml
  if (t >= LIM.tempVermelho) return 'danger';
  if (t >= LIM.tempAmarelo)  return 'warn';
  return 'ok';
}

function zonaUmid(u) {
  if (estaEmDemo()) {
    const { umidMax } = limDemo();
    if (u > umidMax || u < LIM.umidAteMin)          return 'danger';
    if (u > umidMax * 0.92 || u < LIM.umidSegMin)   return 'warn';
    return 'ok';
  }
  // Real: limiares clínicos do config.yaml (dois extremos)
  if (u > LIM.umidAteMax || u < LIM.umidAteMin) return 'danger';
  if (u > LIM.umidSegMax  || u < LIM.umidSegMin) return 'warn';
  return 'ok';
}

function zonaPres(kpa) {
  // No modo demo usa o limiar do input configurável (em kPa)
  if (estaEmDemo()) {
    const max = limDemo().presMax;
    if (kpa >= max)         return 'danger';
    if (kpa >= max * 0.80)  return 'warn';
    return 'ok';
  }
  // Modo real: limiares clínicos do config.yaml (IWGDF / Borges 2023)
  if (kpa >= LIM.presVermelho) return 'danger';
  if (kpa >= LIM.presAmarelo)  return 'warn';
  return 'ok';
}

// ── RENDER ────────────────────────────────────────────────────
function setMetrica({ valEl, barEl, hintEl, valor, maxBar, zonaFn, unidade }) {
  const z   = zonaFn(valor);
  valEl.textContent = valor.toFixed(1);
  valEl.className   = `metric-value ${z}`;
  const pct = Math.min((valor / maxBar) * 100, 100);
  barEl.style.width = `${pct}%`;
  barEl.className   = `metric-bar-fill ${z}`;
  hintEl.textContent = {
    ok:     `Normal - ${valor.toFixed(1)} ${unidade}`,
    warn:   `Atenção - ${valor.toFixed(1)} ${unidade}`,
    danger: `Limite excedido - ${valor.toFixed(1)} ${unidade}`,
  }[z];
  return z;
}

function setPressao(valEl, fillEl, kpa, adc) {
  const z   = zonaPres(kpa);
  const pct = Math.min((kpa / 600) * 100, 100);   // 600 kPa = máximo do sensor
  fillEl.style.width = `${pct}%`;
  fillEl.className   = `pz-bar-fill${z !== 'ok' ? ' ' + z : ''}`;
  valEl.textContent  = `${kpa.toFixed(0)} kPa`;
  return z;
}

function adcParaKpaNum(adc) {
  return (adc / 4095) * 600;
}

function setMapa(z1, z5, zC) {
  _setZona(el.zoneMeta1, z1);
  _setZona(el.zoneMeta5, z5);
  _setZona(el.zoneCal,   zC);
}

function _setZona(zoneEl, nivel) {
  zoneEl.classList.remove('ok', 'warn', 'danger');
  if (nivel !== 'ok') zoneEl.classList.add(nivel);
}

function setStatus(zonas) {
  const danger = zonas.includes('danger');
  const warn   = zonas.includes('warn');
  if (danger) {
    el.cardStatus.className    = 'card card-status danger';
    el.statusTitle.textContent = 'Alerta crítico';
    el.statusSub.textContent   = 'Um ou mais sensores acima do limite configurado.';
  } else if (warn) {
    el.cardStatus.className    = 'card card-status warn';
    el.statusTitle.textContent = 'Atenção';
    el.statusSub.textContent   = 'Valores próximos ao limite. Monitore de perto.';
  } else {
    el.cardStatus.className    = 'card card-status';
    el.statusTitle.textContent = 'Tudo normal';
    el.statusSub.textContent   = 'Nenhum alerta no momento.';
  }
}

// ── ALERTAS ───────────────────────────────────────────────────
function addAlerta(msg, nivel) {
  const agora  = Date.now();
  const ultimo = alertLog[alertLog.length - 1];
  if (ultimo && ultimo.msg === msg && agora - ultimo.ts < 10000) return;
  alertLog.push({ msg, ts: agora });

  const hora = new Date().toLocaleTimeString('pt-BR', {
    hour: '2-digit', minute: '2-digit', second: '2-digit',
  });
  const div = document.createElement('div');
  div.className = `alert-item ${nivel}`;
  div.innerHTML = `<span class="alert-time">${hora}</span><span class="alert-msg">${msg}</span>`;

  const lista = el.alertList;
  lista.querySelector('.empty-msg')?.remove();
  lista.insertBefore(div, lista.firstChild);
  while (lista.children.length > 30) lista.lastChild.remove();
}

function clearAlerts() {
  el.alertList.innerHTML = '<div class="empty-msg">Nenhum alerta no momento.</div>';
  alertLog.length = 0;
}
window.clearAlerts = clearAlerts;

// ── PROCESSA PACOTE DE DADOS ──────────────────────────────────
function processar(d) {
  const zT = setMetrica({
    valEl: el.valTemp, barEl: el.barTemp, hintEl: el.hintTemp,
    valor: d.temp ?? 0, maxBar: 42, unidade: '°C',
    zonaFn: zonaTemp,
  });

  const zU = setMetrica({
    valEl: el.valUmid, barEl: el.barUmid, hintEl: el.hintUmid,
    valor: d.umid ?? 0, maxBar: 100, unidade: '%',
    zonaFn: zonaUmid,
  });

  // Converte ADC → kPa para exibição e classificação clínica
  const adcM1  = d.adc_m1  ?? d.meta1    ?? 0;
  const adcM5  = d.adc_m5  ?? d.meta5    ?? 0;
  const adcCal = d.adc_cal ?? d.calcaneo ?? 0;

  // Se o firmware já enviou kPa, usa direto; senão converte do ADC
  const kpaM1  = d.kpa_meta1    ? parseFloat(d.kpa_meta1)    : adcParaKpaNum(adcM1);
  const kpaM5  = d.kpa_meta5    ? parseFloat(d.kpa_meta5)    : adcParaKpaNum(adcM5);
  const kpaCal = d.kpa_calcaneo ? parseFloat(d.kpa_calcaneo) : adcParaKpaNum(adcCal);

  const zM1  = setPressao(el.pvalMeta1, el.pfillMeta1, kpaM1,  adcM1);
  const zM5  = setPressao(el.pvalMeta5, el.pfillMeta5, kpaM5,  adcM5);
  const zCal = setPressao(el.pvalCal,   el.pfillCal,   kpaCal, adcCal);

  setMapa(zM1, zM5, zCal);
  setStatus([zT, zU, zM1, zM5, zCal]);

  if (zT !== 'ok') addAlerta(`Temperatura: ${(d.temp ?? 0).toFixed(1)} °C`, zT);
  if (zU !== 'ok') addAlerta(`Umidade: ${(d.umid ?? 0).toFixed(0)} %`, zU);
  if (zM1  === 'danger') addAlerta(`Pressão elevada - 1º Metatarso: ${kpaM1.toFixed(0)} kPa`, 'danger');
  if (zM5  === 'danger') addAlerta(`Pressão elevada - 5º Metatarso: ${kpaM5.toFixed(0)} kPa`, 'danger');
  if (zCal === 'danger') addAlerta(`Pressão elevada - Calcâneo: ${kpaCal.toFixed(0)} kPa`, 'danger');

  el.lastUpdate.textContent = `Atualizado às ${new Date().toLocaleTimeString('pt-BR')}`;

  // Acumula leitura para exportação CSV
  registrarLeitura(d);
}

// ── MODO DEMONSTRAÇÃO ─────────────────────────────────────────
function demoStep() {
  demoTick++;
  const t = demoTick / 10;
  const temp    = 31 + Math.sin(t * 0.7) * 2.5 + (demoTick % 40 === 0 ? 3.8 : 0);
  const umid    = 58 + Math.sin(t * 0.4) * 18  + (demoTick % 55 === 0 ? 24  : 0);
  const base    = 1800 + Math.sin(t * 1.1) * 600;
  const adc_m1  = Math.round(Math.min(base + Math.random() * 200, 4095));
  const adc_m5  = Math.round(Math.min(base * 0.8 + Math.random() * 150, 4095));
  const adc_cal = Math.round(Math.min(
    base * 1.1 + (demoTick % 30 === 0 ? 1300 : 0) + Math.random() * 100, 4095
  ));
  processar({ temp, umid, adc_m1, adc_m5, adc_cal });
}

function iniciarDemo() { if (!demoTimer) demoTimer = setInterval(demoStep, 1000); }
function pararDemo()   { clearInterval(demoTimer); demoTimer = null; }

// ── BLE ───────────────────────────────────────────────────────
async function conectarBLE() {
  if (!navigator.bluetooth) {
    alert('Web Bluetooth não disponível. Use Chrome ou Edge.');
    return;
  }
  try {
    setBLEStatus('conectando');
    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ name: 'MonitorPlantar' }],
      optionalServices: [BLE_SVC_UUID],
    });
    bleDevice.addEventListener('gattserverdisconnected', aoDesconectar);

    const server  = await bleDevice.gatt.connect();
    const service = await server.getPrimaryService(BLE_SVC_UUID);

    // Característica NOTIFY - recebe dados do sensor
    bleChar = await service.getCharacteristic(BLE_CHAR_NOTIFY_UUID);
    await bleChar.startNotifications();
    bleChar.addEventListener('characteristicvaluechanged', aoReceberBLE);

    // Característica WRITE - envia dados clínicos (opcional, não bloqueia)
    try {
      bleCharWrite = await service.getCharacteristic(BLE_CHAR_WRITE_UUID);
    } catch {
      console.info('[BLE] Característica WRITE não encontrada - firmware antigo?');
      bleCharWrite = null;
    }

    conectado = true;
    setBLEStatus('conectado', bleDevice.name || 'MonitorPlantar');

    $('chk-demo').checked = false;
    pararDemo();

    // Envia dados clínicos ao conectar
    if (bleCharWrite) enviarDadosClinicos();

  } catch (e) {
    if (e.name !== 'NotFoundError') alert(`Erro na conexão: ${e.message}`);
    setBLEStatus('desconectado');
  }
}

function desconectarBLE() {
  if (bleDevice?.gatt?.connected) bleDevice.gatt.disconnect();
}

function aoDesconectar() {
  conectado    = false;
  bleChar      = null;
  bleCharWrite = null;
  setBLEStatus('desconectado');
}

function aoReceberBLE(evt) {
  try {
    processar(JSON.parse(new TextDecoder().decode(evt.target.value)));
  } catch { /* pacote malformado */ }
}

function setBLEStatus(estado, nome = '') {
  el.dot.className       = 'dot';
  el.btnConnect.disabled = false;

  if (estado === 'conectado') {
    el.dot.classList.add('on');
    el.connLabel.textContent = nome;
    el.btnConnect.className  = 'btn-connect conectado';
    el.btnLabel.textContent  = 'Desconectar';
  } else if (estado === 'conectando') {
    el.dot.classList.add('mid');
    el.connLabel.textContent = 'Conectando...';
    el.btnConnect.className  = 'btn-connect conectando';
    el.btnLabel.textContent  = 'Aguarde';
    el.btnConnect.disabled   = true;
  } else {
    el.connLabel.textContent = 'Desconectado';
    el.btnConnect.className  = 'btn-connect';
    el.btnLabel.textContent  = 'Conectar';
  }
}