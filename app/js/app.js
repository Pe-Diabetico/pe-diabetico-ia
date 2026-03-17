'use strict';

// ── UUIDs BLE (idênticos ao firmware) ────────────────────────
const BLE_SVC_UUID  = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const BLE_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';

// ── Estado ───────────────────────────────────────────────────
let bleDevice  = null;
let bleChar    = null;
let conectado  = false;
let demoTimer  = null;
let demoTick   = 0;
const alertLog = [];

// ── Refs DOM ─────────────────────────────────────────────────
const $  = id => document.getElementById(id);
const el = {
  dot:          $('dot'),
  connLabel:    $('conn-label'),
  btnConnect:   $('btn-connect'),
  btnIcon:      $('btn-icon'),
  btnLabel:     $('btn-label'),
  chkDemo:      $('chk-demo'),
  // Status
  cardStatus:   $('card-status'),
  statusTitle:  $('status-title'),
  statusSub:    $('status-sub'),
  // Temperatura
  cardTemp:     $('card-temp'),
  valTemp:      $('val-temp'),
  barTemp:      $('bar-temp'),
  hintTemp:     $('hint-temp'),
  // Umidade
  cardUmid:     $('card-umid'),
  valUmid:      $('val-umid'),
  barUmid:      $('bar-umid'),
  hintUmid:     $('hint-umid'),
  // Pressão
  pvalMeta1:    $('pval-meta1'),  pfillMeta1:  $('pfill-meta1'),  pzMeta1:  $('pz-meta1'),
  pvalMeta5:    $('pval-meta5'),  pfillMeta5:  $('pfill-meta5'),  pzMeta5:  $('pz-meta5'),
  pvalCal:      $('pval-calcaneo'),pfillCal:   $('pfill-calcaneo'),pzCal:   $('pz-calcaneo'),
  // Mapa
  zoneMeta1:    $('zone-meta1'),
  zoneMeta5:    $('zone-meta5'),
  zoneCal:      $('zone-calcaneo'),
  // Alertas
  alertList:    $('alert-list'),
  // Timestamp
  lastUpdate:   $('last-update'),
};

// ── Limiares dos inputs de configuração ──────────────────────
function lim() {
  return {
    tempMax: parseFloat($('thresh-temp').value)     || 34,
    umidMax: parseFloat($('thresh-umid').value)     || 80,
    presMax: parseFloat($('thresh-pressure').value) || 3000,
  };
}

// ── Classificação de zona ─────────────────────────────────────
function zonaLinear(val, max) {
  if (val >= max)         return 'danger';
  if (val >= max * 0.80)  return 'warn';
  return 'ok';
}

function zonaUmid(u) {
  const { umidMax } = lim();
  if (u > umidMax || u < 25)          return 'danger';
  if (u > umidMax * 0.85 || u < 35)   return 'warn';
  return 'ok';
}

function zonaPres(adc) {
  const { presMax } = lim();
  if (adc >= presMax)          return 'danger';
  if (adc >= presMax * 0.75)   return 'warn';
  return 'ok';
}

// ── Actualiza card de métrica ─────────────────────────────────
function setMetrica({ valEl, barEl, cardEl, hintEl, valor, maxBar, zonaFn, unidade }) {
  const z = zonaFn(valor);

  valEl.textContent = valor.toFixed(1);
  valEl.className   = `metric-value ${z}`;

  const pct = Math.min((valor / maxBar) * 100, 100);
  barEl.style.width = `${pct}%`;
  barEl.className   = `metric-bar-fill ${z}`;

  const msgs = {
    ok:     `Normal — ${valor.toFixed(1)} ${unidade}`,
    warn:   `Atenção — ${valor.toFixed(1)} ${unidade}`,
    danger: `Limite excedido — ${valor.toFixed(1)} ${unidade}`,
  };
  hintEl.textContent = msgs[z];
  return z;
}

// ── Actualiza barra de pressão ────────────────────────────────
function setPressao(valEl, fillEl, pzEl, adc) {
  const z   = zonaPres(adc);
  const pct = Math.min((adc / 4095) * 100, 100);

  fillEl.style.width = `${pct}%`;
  fillEl.className   = `pz-bar-fill ${z === 'ok' ? '' : z}`;
  valEl.textContent  = `${adc} ADC`;
  pzEl.className     = `pres-zone`;   // sem classe extra por zona — barra já comunica
  return z;
}

// ── Actualiza mapa do pé ──────────────────────────────────────
function setMapa(z1, z5, zC) {
  el.zoneMeta1.className = `heat-zone ${z1 === 'ok' ? '' : z1}`;
  el.zoneMeta5.className = `heat-zone ${z5 === 'ok' ? '' : z5}`;
  el.zoneCal.className   = `heat-zone ${zC === 'ok' ? '' : zC}`;
}

// ── Actualiza status geral ────────────────────────────────────
function setStatus(zonas) {
  const danger = zonas.includes('danger');
  const warn   = zonas.includes('warn');

  if (danger) {
    el.cardStatus.className  = 'card card-status danger';
    el.statusTitle.textContent = 'Alerta crítico';
    el.statusSub.textContent   = 'Um ou mais sensores acima do limite configurado.';
  } else if (warn) {
    el.cardStatus.className  = 'card card-status warn';
    el.statusTitle.textContent = 'Atenção';
    el.statusSub.textContent   = 'Valores próximos ao limite. Monitore de perto.';
  } else {
    el.cardStatus.className  = 'card card-status';
    el.statusTitle.textContent = 'Tudo normal';
    el.statusSub.textContent   = 'Nenhum alerta no momento.';
  }
}

// ── Adiciona alerta ao painel ─────────────────────────────────
function addAlerta(msg, nivel) {
  const agora = Date.now();
  const ultimo = alertLog[alertLog.length - 1];
  if (ultimo && ultimo.msg === msg && agora - ultimo.ts < 10000) return;
  alertLog.push({ msg, ts: agora });

  const hora = new Date().toLocaleTimeString('pt-BR', {
    hour: '2-digit', minute: '2-digit', second: '2-digit',
  });
  const div = document.createElement('div');
  div.className  = `alert-item ${nivel}`;
  div.innerHTML  = `${msg}<span class="alert-time">${hora}</span>`;

  const lista = el.alertList;
  const vazio = lista.querySelector('.empty-msg');
  if (vazio) vazio.remove();

  lista.insertBefore(div, lista.firstChild);
  while (lista.children.length > 30) lista.lastChild.remove();
}

function clearAlerts() {
  el.alertList.innerHTML = '<div class="empty-msg">Nenhum alerta no momento.</div>';
  alertLog.length = 0;
}

// ── Processa pacote de dados ──────────────────────────────────
function processar(d) {
  const { tempMax } = lim();

  const zT = setMetrica({
    valEl: el.valTemp, barEl: el.barTemp, cardEl: el.cardTemp, hintEl: el.hintTemp,
    valor: d.temp ?? 0, maxBar: tempMax + 6, unidade: '°C',
    zonaFn: v => zonaLinear(v, tempMax),
  });

  const zU = setMetrica({
    valEl: el.valUmid, barEl: el.barUmid, cardEl: el.cardUmid, hintEl: el.hintUmid,
    valor: d.umid ?? 0, maxBar: 100, unidade: '%',
    zonaFn: zonaUmid,
  });

  const zM1 = setPressao(el.pvalMeta1, el.pfillMeta1, el.pzMeta1, d.adc_m1  ?? 0);
  const zM5 = setPressao(el.pvalMeta5, el.pfillMeta5, el.pzMeta5, d.adc_m5  ?? 0);
  const zCal = setPressao(el.pvalCal,  el.pfillCal,   el.pzCal,   d.adc_cal ?? 0);

  setMapa(zM1, zM5, zCal);
  setStatus([zT, zU, zM1, zM5, zCal]);

  // Alertas textuais — sem emojis
  if (zT !== 'ok')   addAlerta(`Temperatura: ${(d.temp ?? 0).toFixed(1)} °C`, zT);
  if (zU !== 'ok')   addAlerta(`Umidade: ${(d.umid ?? 0).toFixed(0)} %`, zU);
  if (zM1 === 'danger') addAlerta(`Pressão elevada — 1º Metatarso: ${d.adc_m1} ADC`, 'danger');
  if (zM5 === 'danger') addAlerta(`Pressão elevada — 5º Metatarso: ${d.adc_m5} ADC`, 'danger');
  if (zCal === 'danger') addAlerta(`Pressão elevada — Calcâneo: ${d.adc_cal} ADC`, 'danger');

  el.lastUpdate.textContent = `Atualizado às ${new Date().toLocaleTimeString('pt-BR')}`;
}

// ── Modo demonstração ─────────────────────────────────────────
function demoStep() {
  demoTick++;
  const t = demoTick / 10;

  const temp    = 31 + Math.sin(t * 0.7) * 2.5 + (demoTick % 40 === 0 ? 3.8 : 0);
  const umid    = 58 + Math.sin(t * 0.4) * 18  + (demoTick % 55 === 0 ? 24  : 0);
  const base    = 1800 + Math.sin(t * 1.1) * 600;
  const adc_m1  = Math.round(Math.min(base + Math.random() * 200, 4095));
  const adc_m5  = Math.round(Math.min(base * 0.8 + Math.random() * 150, 4095));
  const adc_cal = Math.round(Math.min(base * 1.1 + (demoTick % 30 === 0 ? 1300 : 0) + Math.random() * 100, 4095));

  processar({ temp, umid, adc_m1, adc_m5, adc_cal });
}

function iniciarDemo() { if (!demoTimer) demoTimer = setInterval(demoStep, 1000); }
function pararDemo()   { clearInterval(demoTimer); demoTimer = null; }

// ── BLE ───────────────────────────────────────────────────────
async function conectarBLE() {
  if (!navigator.bluetooth) {
    alert('Web Bluetooth não disponível neste navegador. Use Chrome ou Edge.');
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
    bleChar       = await service.getCharacteristic(BLE_CHAR_UUID);

    await bleChar.startNotifications();
    bleChar.addEventListener('characteristicvaluechanged', aoReceberBLE);

    conectado = true;
    setBLEStatus('conectado', bleDevice.name || 'MonitorPlantar');

    el.chkDemo.checked = false;
    pararDemo();

  } catch (e) {
    if (e.name !== 'NotFoundError') alert(`Erro na conexão: ${e.message}`);
    setBLEStatus('desconectado');
  }
}

function desconectarBLE() {
  if (bleDevice?.gatt?.connected) bleDevice.gatt.disconnect();
}

function aoDesconectar() {
  conectado = false;
  bleChar   = null;
  setBLEStatus('desconectado');
}

function aoReceberBLE(evt) {
  try { processar(JSON.parse(new TextDecoder().decode(evt.target.value))); }
  catch { /* pacote malformado */ }
}

// ── Status visual da conexão ──────────────────────────────────
function setBLEStatus(estado, nome = '') {
  el.dot.className = 'dot';
  el.btnConnect.disabled = false;

  if (estado === 'conectado') {
    el.dot.classList.add('on');
    el.connLabel.textContent    = nome;
    el.btnConnect.className     = 'btn-connect conectado';
    el.btnLabel.textContent     = 'Desconectar';
  } else if (estado === 'conectando') {
    el.dot.classList.add('mid');
    el.connLabel.textContent    = 'Conectando...';
    el.btnConnect.className     = 'btn-connect conectando';
    el.btnLabel.textContent     = 'Aguarde';
    el.btnConnect.disabled      = true;
  } else {
    el.connLabel.textContent    = 'Desconectado';
    el.btnConnect.className     = 'btn-connect';
    el.btnLabel.textContent     = 'Conectar';
  }
}

// ── Eventos ───────────────────────────────────────────────────
$('btn-connect').addEventListener('click', () => {
  if (conectado) desconectarBLE();
  else           conectarBLE();
});

$('chk-demo').addEventListener('change', e => {
  if (e.target.checked) iniciarDemo();
  else                  pararDemo();
});

// ── Init ──────────────────────────────────────────────────────
if (el.chkDemo.checked) iniciarDemo();
