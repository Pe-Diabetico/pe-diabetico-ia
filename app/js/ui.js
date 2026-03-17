/**
 * ui.js — Atualização do DOM
 * ==========================
 * Só renderiza. Não tem lógica de negócio nem acessa dados diretamente.
 * Toda decisão de risco/zona vem do app.js.
 */

'use strict';

// Mapa de zona → classe CSS e emoji
const ZONA_ESTILO = {
  verde:    { cls: 'verde',    emoji: '🟢' },
  amarelo:  { cls: 'amarelo',  emoji: '🟡' },
  vermelho: { cls: 'vermelho', emoji: '🔴' },
};

// ─── Status BLE ───────────────────────────────────────────────

function setStatusBLE(estado, nome = '') {
  const dot  = document.getElementById('dot');
  const txt  = document.getElementById('txtStatus');
  const btn  = document.getElementById('btnBLE');
  const btxt = document.getElementById('btnBLETxt');

  dot.className = 'dot';

  if (estado === 'conectado') {
    dot.classList.add('on');
    txt.textContent  = nome || 'Conectado';
    btn.className    = 'btn btn-principal conectado';
    btxt.textContent = '🔌 Desconectar';
  } else if (estado === 'conectando') {
    dot.classList.add('mid');
    txt.textContent  = 'Conectando...';
    btn.className    = 'btn btn-principal conectando';
    btxt.textContent = '⏳ Aguarde...';
    btn.disabled     = true;
  } else if (estado === 'erro') {
    dot.classList.add('off');
    txt.textContent  = 'Erro';
    btn.className    = 'btn btn-principal';
    btxt.textContent = '📡 Tentar novamente';
    btn.disabled     = false;
  } else {
    txt.textContent  = 'Desconectado';
    btn.className    = 'btn btn-principal';
    btxt.textContent = '📡 Conectar ao MonitorPlantar';
    btn.disabled     = false;
  }
}

// ─── Status CSV ───────────────────────────────────────────────

function setStatusCSV(ativo, total = 0) {
  const el = document.getElementById('statusCSV');
  if (!el) return;
  if (ativo) {
    el.className   = 'csv-status csv-ativo';
    el.textContent = `📄 CSV aberto — ${total} leitura(s) gravada(s)`;
  } else {
    el.className   = 'csv-status';
    el.textContent = total > 0
      ? `💾 ${total} leitura(s) em memória (sem arquivo aberto)`
      : '📂 Nenhum arquivo CSV aberto';
  }
}

// ─── Cards de leitura ─────────────────────────────────────────

/**
 * Atualiza todos os cards com as leituras enriquecidas pelo app.js.
 * @param {Object} l — leitura com campos zona_* adicionados
 */
function atualizarCards(l) {
  _setCard('vCal',  'bCal',  l.kpa_cal,  600, l.zona_pressao);
  _setCard('vM1',   'bM1',   l.kpa_m1,   600, l.zona_pressao_m1 || l.zona_pressao);
  _setCard('vM5',   'bM5',   l.kpa_m5,   600, l.zona_pressao_m5 || l.zona_pressao);

  if (l.temp !== null)
    _setCard('vTemp', 'bTemp', l.temp, 45,  l.zona_temp, 1);
  if (l.umid !== null)
    _setCard('vUmid', 'bUmid', l.umid, 100, l.zona_umid, 0);

  // PTI
  const ptiEl  = document.getElementById('vPTI');
  const ptiK   = l.pti / 1000;
  ptiEl.textContent = ptiK >= 1000
    ? `${(ptiK / 1000).toFixed(1)}M`
    : `${ptiK.toFixed(0)}k`;
  ptiEl.className = `card-valor ${l.zona_pti || 'verde'}`;

  const badge = document.getElementById('ptiNivel');
  const z = ZONA_ESTILO[l.zona_pti] || ZONA_ESTILO.verde;
  badge.textContent = `${z.emoji} PTI ${l.zona_pti || '—'}`;
  badge.className   = `pti-badge ${l.zona_pti || 'verde'}`;
}

function _setCard(idVal, idBar, valor, maximo, zona, casas = 0) {
  const el = document.getElementById(idVal);
  if (!el) return;
  el.textContent = valor.toFixed(casas);
  el.className   = `card-valor ${zona || 'verde'}`;
  const fill = document.getElementById(idBar);
  if (fill) fill.style.width = `${Math.min((valor / maximo) * 100, 100)}%`;
}

// ─── Painel de alerta ─────────────────────────────────────────

function atualizarAlerta(nivel, fatores) {
  const box = document.getElementById('boxAlerta');
  if (!box) return;

  if (!nivel || nivel === 'verde') { box.hidden = true; return; }

  box.hidden = false;
  const emoji = nivel === 'vermelho' ? '🔴' : '🟡';
  const titulo = nivel === 'vermelho'
    ? '<strong>Risco elevado detectado.</strong>'
    : 'Atenção — monitorar de perto.';

  box.innerHTML = `
    <div class="alerta-box ${nivel === 'amarelo' ? 'aviso' : ''}">
      <div class="alerta-icone">${emoji}</div>
      <div class="alerta-texto">${titulo}<br/>${fatores.join(' &bull; ')}</div>
    </div>`;
}

// ─── Tabela de leituras ───────────────────────────────────────

function renderizarTabela(leituras) {
  const corpo = document.getElementById('corpoTabela');
  if (!corpo) return;

  if (!leituras.length) {
    corpo.innerHTML = '<tr class="vazio"><td colspan="9">Aguardando leituras do sensor...</td></tr>';
    return;
  }

  corpo.innerHTML = leituras.map(l => {
    const hora = new Date(l.ts).toLocaleTimeString('pt-BR', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
    const campos = l.csv.split(',');
    // índices: 0=ts,1=nome,2=idade,3=imc,4=anosDM,5=hba1c,6=has,7=tab,
    //          8=cal,9=m1,10=m5,11=max,12=pti,13=temp,14=umid,
    //          15=z_press,16=z_pti,17=z_temp,18=z_umid,19=risco
    const zRisco = campos[19] || 'verde';
    const cls    = `b-risco b-${zRisco}`;
    return `<tr>
      <td>${hora}</td>
      <td>${campos[8]}</td>
      <td>${campos[9]}</td>
      <td>${campos[10]}</td>
      <td>${campos[13] || '—'}</td>
      <td>${campos[14] || '—'}</td>
      <td>${(+campos[12] / 1000).toFixed(0)}k</td>
      <td><span class="${cls}">${zRisco}</span></td>
    </tr>`;
  }).join('');
}

// ─── Toast ────────────────────────────────────────────────────

let _toastTimer;
function toast(msg, ms = 3500) {
  const el = document.getElementById('toast');
  if (!el) return;
  el.textContent = msg;
  el.hidden = false;
  clearTimeout(_toastTimer);
  _toastTimer = setTimeout(() => { el.hidden = true; }, ms);
}

window.UI = {
  setStatusBLE, setStatusCSV,
  atualizarCards, atualizarAlerta,
  renderizarTabela, toast,
};
