let allOrders = [];
let filter = 'ALL';
const money = n => '$' + Number(n || 0).toLocaleString('es-CO');
const img = i => i.image || '/img/placeholder.svg';
function statusLabel(s){return {RECEIVED:'Recibido',PREPARING:'Preparando',READY:'Listo',DELIVERED:'Entregado',CANCELLED:'Cancelado'}[s]||s;}
async function load(){allOrders = await fetch('/api/orders').then(r=>r.json()); render();}
function setFilter(s){filter=s; render();}
function render(){
  ['ALL','RECEIVED','PREPARING','READY'].forEach(s=>{const el=document.getElementById('tab'+s); if(el) el.classList.toggle('active',s===filter)});
  let list = allOrders.slice().reverse();
  if(filter !== 'ALL') list = list.filter(o=>o.status===filter);
  if(!list.length){orders.innerHTML='<p class="muted">No hay pedidos para mostrar.</p>';return;}
  orders.innerHTML = list.map(o=>`<article class="order-card ${o.status}"><div class="order-head"><div><h3>#${o.id} ${o.table?'· Mesa '+o.table:'· Mostrador '+(o.counterNumber||'')}</h3><small>${statusLabel(o.status)}</small></div><span class="pill">${o.items.length} items</span></div><div class="order-items">${o.items.map(i=>`<div class="order-item"><img class="mini-img" src="${img(i)}" onerror="this.src='/img/placeholder.svg'"><span><b>${i.qty} x ${i.name}</b><br><small>${money(i.price)}</small></span></div>`).join('')}</div><div class="order-actions"><button class="btn-state" onclick="upd(${o.id},'RECEIVED')">Recibido</button><button class="btn-state btn-orange" onclick="upd(${o.id},'PREPARING')">Preparando</button><button class="btn-state btn-green" onclick="upd(${o.id},'READY')">Listo</button><button class="btn-state" onclick="upd(${o.id},'DELIVERED')">Entregado</button></div></article>`).join('');
}
async function upd(id,status){await fetch('/api/order/update',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,status})});load();}
let ws = new WebSocket(`ws://${location.host}/ws`); ws.onmessage = load; load();
