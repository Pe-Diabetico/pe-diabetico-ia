/**
 * storage.js — Persistência e exportação de dados
 * =================================================
 * Duas estratégias combinadas:
 *
 * A) localStorage (sempre disponível)
 *    Toda leitura BLE é acumulada em memória e salva no localStorage.
 *    Garante que dados não se percam em caso de refresh acidental.
 *
 * B) File System Access API (Chrome Desktop/Android — opcional)
 *    Se o browser suportar, o usuário escolhe um arquivo CSV uma vez.
 *    Cada leitura BLE é APPENDADA automaticamente ao arquivo em disco.
 *    Fallback gracioso: se não suportar, usa só o localStorage.
 *
 * Eventos emitidos:
 *   'storage:csv-ativo'    → arquivo CSV aberto para escrita contínua
 *   'storage:csv-inativo'  → arquivo fechado ou não suportado
 */

'use strict';

const CHAVE_LEITURAS = 'pedia_leituras';
const CHAVE_PACIENTE = 'pedia_paciente';

let _leituras    = _carregarLocal();
let _fileHandle  = null;   // FileSystemFileHandle (File System Access API)
let _writable    = null;   // FileSystemWritableFileStream
let _csvAtivo    = false;
let _cabecalhoEscrito = false;

// Cabeçalho CSV — compatível com data_pipeline.py do modelo RF
const CSV_CABECALHO = [
  'timestamp', 'nome', 'idade', 'imc', 'anos_diabetes', 'hba1c', 'has', 'tabagismo',
  'kpa_calcaneo', 'kpa_meta1', 'kpa_meta5', 'kpa_max',
  'pti_kpa_s', 'temp_c', 'umid_pct',
  'zona_pressao', 'zona_pti', 'zona_temp', 'zona_umid',
  'risco_geral',
].join(',');

// ─── API pública ──────────────────────────────────────────────

/**
 * Tenta abrir (ou criar) um arquivo CSV para escrita contínua.
 * Mostra o seletor de arquivo nativo do SO.
 * @returns {Promise<boolean>} true se o arquivo foi aberto
 */
async function abrirCSV() {
  if (!window.showSaveFilePicker) {
    console.info('[Storage] File System Access API não disponível. Usando download manual.');
    return false;
  }
  try {
    const dataHoje = new Date().toISOString().slice(0, 10);
    _fileHandle = await window.showSaveFilePicker({
      suggestedName: `pedia_${dataHoje}.csv`,
      types: [{ description: 'CSV', accept: { 'text/csv': ['.csv'] } }],
    });
    _writable = await _fileHandle.createWritable({ keepExistingData: false });
    await _writable.write(CSV_CABECALHO + '\n');
    _cabecalhoEscrito = true;
    _csvAtivo = true;
    document.dispatchEvent(new CustomEvent('storage:csv-ativo'));
    console.info('[Storage] CSV aberto para escrita contínua.');
    return true;
  } catch (e) {
    if (e.name !== 'AbortError') console.error('[Storage] Erro ao abrir CSV:', e);
    return false;
  }
}

/**
 * Fecha o stream do CSV (chame ao desconectar o BLE).
 */
async function fecharCSV() {
  if (_writable) {
    await _writable.close();
    _writable    = null;
    _fileHandle  = null;
    _csvAtivo    = false;
    _cabecalhoEscrito = false;
    document.dispatchEvent(new CustomEvent('storage:csv-inativo'));
    console.info('[Storage] CSV fechado.');
  }
}

/**
 * Salva uma leitura:
 *   1. No array em memória + localStorage (sempre)
 *   2. Appenda no arquivo CSV aberto (se File System API ativa)
 *
 * @param {Object} leitura — vinda do app.js após enriquecimento com zonas
 * @param {Object} paciente — dados clínicos preenchidos pelo usuário
 */
async function salvarLeitura(leitura, paciente) {
  const linha = _montarLinhaCSV(leitura, paciente);

  // 1. Memória + localStorage
  _leituras.push(linha);
  try {
    localStorage.setItem(CHAVE_LEITURAS, JSON.stringify(_leituras.slice(-5000)));
  } catch { /* quota exceeded — mantém só em memória */ }

  // 2. Arquivo CSV (se aberto)
  if (_csvAtivo && _writable) {
    try {
      await _writable.write(linha.csv + '\n');
    } catch (e) {
      console.error('[Storage] Erro ao escrever no CSV:', e);
      _csvAtivo = false;
    }
  }
}

/**
 * Salva os dados do paciente no localStorage.
 * @param {Object} paciente
 */
function salvarPaciente(paciente) {
  localStorage.setItem(CHAVE_PACIENTE, JSON.stringify(paciente));
}

/**
 * Recupera dados do paciente salvos.
 * @returns {Object|null}
 */
function carregarPaciente() {
  try { return JSON.parse(localStorage.getItem(CHAVE_PACIENTE)); }
  catch { return null; }
}

/**
 * Baixa todas as leituras da sessão como CSV (fallback sem File System API).
 */
function baixarCSV() {
  if (!_leituras.length) return false;
  const conteudo = [CSV_CABECALHO, ..._leituras.map(l => l.csv)].join('\n');
  const blob  = new Blob([conteudo], { type: 'text/csv' });
  const url   = URL.createObjectURL(blob);
  const a     = document.createElement('a');
  a.href      = url;
  a.download  = `pedia_${new Date().toISOString().slice(0, 10)}.csv`;
  a.click();
  URL.revokeObjectURL(url);
  return true;
}

/**
 * Retorna o número de leituras em memória.
 */
function totalLeituras() { return _leituras.length; }

/**
 * Limpa todas as leituras em memória e no localStorage.
 */
function limpar() {
  _leituras = [];
  localStorage.removeItem(CHAVE_LEITURAS);
}

/**
 * Retorna as últimas N leituras (para a tabela da UI).
 */
function ultimasLeituras(n = 50) { return _leituras.slice(-n).reverse(); }

/**
 * Retorna true se o File System API está ativa e escrevendo.
 */
function csvAtivoEmDisco() { return _csvAtivo; }

// ─── Interno ──────────────────────────────────────────────────

function _montarLinhaCSV(l, p) {
  const ts = new Date(l.ts).toISOString();
  const campos = [
    ts,
    `"${(p.nome || '').replace(/"/g, '')}"`,
    p.idade     || '',
    p.imc       || '',
    p.anosDM    || '',
    p.hba1c     || '',
    p.has       ? 1 : 0,
    p.tabagismo ? 1 : 0,
    l.kpa_cal.toFixed(1),
    l.kpa_m1.toFixed(1),
    l.kpa_m5.toFixed(1),
    l.kpa_max.toFixed(1),
    l.pti.toFixed(0),
    l.temp !== null ? l.temp.toFixed(1) : '',
    l.umid !== null ? l.umid.toFixed(0) : '',
    l.zona_pressao || '',
    l.zona_pti     || '',
    l.zona_temp    || '',
    l.zona_umid    || '',
    l.risco_geral  || '',
  ];
  return { ts, csv: campos.join(',') };
}

function _carregarLocal() {
  try { return JSON.parse(localStorage.getItem(CHAVE_LEITURAS) || '[]'); }
  catch { return []; }
}

window.Storage = {
  abrirCSV, fecharCSV, salvarLeitura,
  salvarPaciente, carregarPaciente,
  baixarCSV, totalLeituras, limpar,
  ultimasLeituras, csvAtivoEmDisco,
};
