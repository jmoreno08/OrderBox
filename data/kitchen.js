let allOrders = [];
let soundEnabled = false;
let knownOrders = new Set();
const statuses = ['RECEIVED','PREPARING'];
const labels = {RECEIVED:'Nuevos',PREPARING:'Preparando',READY:'Listo',DELIVERED:'Entregado',CANCELLED:'Cancelado'};
const money = n => '$' + Number(n || 0).toLocaleString('es-CO');
const elapsed = ms => { const m = Math.max(0, Math.floor((Date.now() - Number(ms || 0)) / 60000)); return m < 60 ? `${m} min` : `${Math.floor(m / 60)} h ${m % 60} min`; };

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
  const next = o.status === 'RECEIVED' ? `<button class="btn-orange" onclick="upd(${o.id},'PREPARING')">Preparar</button>` : `<button class="btn-green" onclick="upd(${o.id},'READY')">Listo</button>`;
  const cancel = o.status === 'RECEIVED'
    ? `<button class="btn-red" onclick="cancelOrder(${o.id})">Cancelar</button>`
    : `<button class="btn-state" disabled title="Solo se puede cancelar un pedido recibido">Cancelar</button>`;

  return `<article class="order-card ${o.status}">
    <div class="order-head"><div><h3>#${o.id}</h3><small>${source(o)} - ${elapsed(o.createdAt)}</small></div><span class="pill">${money(orderTotal(o))}</span></div>
    <div class="order-items">${o.items.map(i=>`<div class="order-item"><span><b>${i.qty} x ${i.name}</b>${(i.extras||[]).map(e=>`<br><small>+ ${e.name} ${money(e.price)}</small>`).join('')}${i.notes?`<br><small>Nota: ${i.notes}</small>`:''}</span></div>`).join('')}</div>
    ${o.notes?`<p class="muted">Obs: ${o.notes}</p>`:''}
    <div class="order-actions">${next}${cancel}</div>
  </article>`;
}

async function upd(id,status,extra={}){const r=await fetch('/api/order/update',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,status,...extra})});if(!r.ok)alert(await r.text());load();}
function cancelOrder(id){const cancelReason=prompt('Motivo de cancelacion: cliente cancelo, sin ingredientes, error del pedido u otro','cliente cancelo');if(cancelReason)upd(id,'CANCELLED',{cancelReason});}

let ws = new WebSocket(`ws://${location.host}/ws`);
ws.onmessage = load;
load();
setInterval(render,30000);
