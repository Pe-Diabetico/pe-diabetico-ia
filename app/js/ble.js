/**
 * ble.js — Web Bluetooth API
 * ==========================
 * Gerencia a conexão BLE com o ESP32.
 * Parseia o JSON recebido e calcula o PTI acumulado.
 * Emite eventos customizados consumidos pelo app.js.
 *
 * Eventos emitidos:
 *   'ble:leitura'      → { kpa_cal, kpa_m1, kpa_m5, kpa_max, temp, umid, pti, seq, ts }
 *   'ble:status'       → { estado: 'conectado'|'conectando'|'desconectado', nome? }
 *   'ble:erro'         → { mensagem }
 */

'use strict';

const BLE_SVC_UUID  = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const BLE_CHAR_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const JANELA_PTI_MS = 60 * 60 * 1000; // 1 hora

let _device    = null;
let _char      = null;
let _conectado = false;
const _bufPTI  = []; // { ts: ms, kpa: float }

// ─── API pública ──────────────────────────────────────────────

async function conectar() {
  if (!navigator.bluetooth) {
    _emitErro('Web Bluetooth não suportado. Use Chrome ou Edge.');
    return;
  }
  try {
    _emitStatus('conectando');
    _device = await navigator.bluetooth.requestDevice({
      filters: [{ name: 'MonitorPlantar' }],
      optionalServices: [BLE_SVC_UUID],
    });
    _device.addEventListener('gattserverdisconnected', _aoDesconectar);

    const server  = await _device.gatt.connect();
    const service = await server.getPrimaryService(BLE_SVC_UUID);
    _char         = await service.getCharacteristic(BLE_CHAR_UUID);

    await _char.startNotifications();
    _char.addEventListener('characteristicvaluechanged', _aoReceberDados);

    _conectado = true;
    _emitStatus('conectado', _device.name);
  } catch (e) {
    _conectado = false;
    if (e.name === 'NotFoundError') _emitStatus('desconectado'); // usuário cancelou
    else _emitErro(e.message);
  }
}

function desconectar() {
  if (_device?.gatt?.connected) _device.gatt.disconnect();
}

function estaConectado() { return _conectado; }

// ─── Interno ──────────────────────────────────────────────────

function _aoReceberDados(evt) {
  const txt = new TextDecoder().decode(evt.target.value);
  let d;
  try { d = JSON.parse(txt); } catch { return; }

  const ts     = Date.now();
  const kpaMax = Math.max(+d.kpa_calcaneo || 0, +d.kpa_meta1 || 0, +d.kpa_meta5 || 0);

  // Adiciona ao buffer e remove pontos fora da janela de 1h
  _bufPTI.push({ ts, kpa: kpaMax });
  while (_bufPTI.length > 1 && ts - _bufPTI[0].ts > JANELA_PTI_MS) _bufPTI.shift();

  // Integração trapezoidal: PTI = Σ ( (kpa_i + kpa_{i-1}) / 2 ) × dt_s
  let pti = 0;
  for (let i = 1; i < _bufPTI.length; i++) {
    const dt_s = (_bufPTI[i].ts - _bufPTI[i - 1].ts) / 1000;
    pti += ((_bufPTI[i].kpa + _bufPTI[i - 1].kpa) / 2) * dt_s;
  }

  document.dispatchEvent(new CustomEvent('ble:leitura', {
    detail: {
      kpa_cal: +d.kpa_calcaneo || 0,
      kpa_m1:  +d.kpa_meta1    || 0,
      kpa_m5:  +d.kpa_meta5    || 0,
      kpa_max: kpaMax,
      temp:    d.temp != null ? +d.temp : null,
      umid:    d.umid != null ? +d.umid : null,
      seq:     d.seq ?? null,
      pti,
      ts,
    },
  }));
}

function _aoDesconectar() {
  _conectado = false;
  _char      = null;
  _bufPTI.splice(0);
  _emitStatus('desconectado');
}

function _emitStatus(estado, nome = '') {
  document.dispatchEvent(new CustomEvent('ble:status', { detail: { estado, nome } }));
}

function _emitErro(mensagem) {
  document.dispatchEvent(new CustomEvent('ble:erro', { detail: { mensagem } }));
}

// Exporta para uso global (scripts sem module)
window.BLE = { conectar, desconectar, estaConectado };
