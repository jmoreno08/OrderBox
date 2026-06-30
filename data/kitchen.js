let allOrders = [];
let soundEnabled = false;
let knownOrders = new Set();
const statuses = ['RECEIVED','PREPARING','READY','DELIVERED','CANCELLED'];
const labels = {RECEIVED:'Recibidos',PREPARING:'Preparando',READY:'Listos',DELIVERED:'Entregados',CANCELLED:'Cancelados'};
const money = n => '$' + Number(n || 0).toLocaleString('es-CO');

function orderTotal(o){
  return o.items.reduce((sum,i)=>sum+(i.price + (i.extras||[]).reduce((a,e)=>a+Number(e.price||0),0))*i.qty,0);
}

function source(o){return o.sourceType === 'counter' ? `Mostrador #${o.counterNumber}` : `Mesa ${o.table}`;}

function beep(){
  if(!soundEnabled) return;
  const ctx = new (window.AudioContext || window.webkitAudioContext)();
  const osc = ctx.createOscillator();
  const gain = ctx.createGain();
  osc.frequency.value = 880;
  gain.gain.value = 0.08;
  osc.connect(gain);
  gain.connect(ctx.destination);
  osc.start();
  setTimeout(()=>{osc.stop();ctx.close();},180);
}

function enableSound(){soundEnabled=true;soundBtn.textContent='Sonido activo';beep();}

async function load(){
  const next = await fetch('/api/orders').then(r=>r.json());
  next.forEach(o=>{if(!knownOrders.has(o.id) && knownOrders.size>0) beep(); knownOrders.add(o.id);});
  allOrders = next;
  render();
}

function render(){
  kanban.innerHTML = statuses.map(status=>{
    const list = allOrders.filter(o=>o.status===status).sort((a,b)=>b.id-a.id);
    return `<section class="kanban-col"><h2>${labels[status]} <span>${list.length}</span></h2>${list.map(card).join('') || '<p class="muted">Sin pedidos</p>'}</section>`;
  }).join('');
}

function card(o){
  const buttons = statuses.map(s=>{
    const disabled = s === 'CANCELLED' && o.status !== 'RECEIVED';
    return `<button class="btn-state" ${disabled?'disabled title="Solo se puede cancelar un pedido recibido"':''} onclick="upd(${o.id},'${s}')">${labels[s]}</button>`;
  }).join('');

  return `<article class="order-card ${o.status}">
    <div class="order-head"><div><h3>#${o.id}</h3><small>${source(o)}</small></div><span class="pill">${money(orderTotal(o))}</span></div>
    <div class="order-items">${o.items.map(i=>`<div class="order-item"><span><b>${i.qty} x ${i.name}</b>${(i.extras||[]).map(e=>`<br><small>+ ${e.name} ${money(e.price)}</small>`).join('')}${i.notes?`<br><small>Nota: ${i.notes}</small>`:''}</span></div>`).join('')}</div>
    ${o.notes?`<p class="muted">Obs: ${o.notes}</p>`:''}
    <div class="order-actions">${buttons}</div>
  </article>`;
}

async function upd(id,status){await fetch('/api/order/update',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,status})});load();}

let ws = new WebSocket(`ws://${location.host}/ws`);
ws.onmessage = load;
load();
