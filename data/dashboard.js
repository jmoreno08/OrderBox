let orders = [], soundEnabled = false, seen = new Set(), currency = '$';
const labels = {RECEIVED:'Recibido',PREPARING:'Preparando',READY:'Listo',DELIVERED:'Entregado',CANCELLED:'Cancelado'};
const activeStates = ['RECEIVED','PREPARING','READY'];
const money = n => currency + Number(n || 0).toLocaleString('es-CO');
const age = ms => { const m=Math.max(0,Math.floor((Date.now()-Number(ms||0))/60000)); return m<60 ? `${m} min` : `${Math.floor(m/60)} h ${m%60} min`; };
function total(o){return (o.items||[]).reduce((a,i)=>a+(Number(i.price||0)+(i.extras||[]).reduce((x,e)=>x+Number(e.price||0),0))*Number(i.qty||0),0);}
function source(o){return o.sourceType==='counter'?`Mostrador #${o.counterNumber}`:`Mesa ${o.table}`;}
function today(o){return new Date(Number(o.createdAt||0)).toDateString()===new Date().toDateString();}
function beep(freq=660){if(!soundEnabled)return;const c=new (window.AudioContext||window.webkitAudioContext)(),o=c.createOscillator(),g=c.createGain();o.frequency.value=freq;g.gain.value=.08;o.connect(g);g.connect(c.destination);o.start();setTimeout(()=>{o.stop();c.close();},160);}
function enableSound(){soundEnabled=true;soundBtn.textContent='Sonido activo';beep();}
async function load(){
  const [next,biz]=await Promise.all([fetch('/api/orders').then(r=>r.json()),fetch('/api/business').then(r=>r.json()).catch(()=>({}))]);
  currency=biz.currency||currency; if(biz.primaryColor) document.documentElement.style.setProperty('--primary',biz.primaryColor);
  next.forEach(o=>{if(!seen.has(o.id)&&seen.size)beep(880);seen.add(o.id);}); orders=next; render();
}
function render(){
  const day=orders.filter(today), active=orders.filter(o=>activeStates.includes(o.status));
  const prep=orders.filter(o=>o.readyAt&&o.createdAt).map(o=>(o.readyAt-o.createdAt)/60000).filter(v=>v>0);
  const avg=prep.length?Math.round(prep.reduce((a,b)=>a+b,0)/prep.length):0;
  const sold={}; day.forEach(o=>(o.items||[]).forEach(i=>sold[i.name]=(sold[i.name]||0)+Number(i.qty||0)));
  const top=Object.entries(sold).sort((a,b)=>b[1]-a[1])[0];
  stats.innerHTML=`<div class="stat">Activos<b>${active.length}</b></div><div class="stat">Pedidos del dia<b>${day.length}</b></div><div class="stat">Ventas del dia<b>${money(day.reduce((a,o)=>a+total(o),0))}</b></div><div class="stat">Prom. preparacion<b>${avg} min</b></div><div class="stat">Mas vendido<b>${top?top[0]:'-'}</b></div>`;
  stateStats.innerHTML=['RECEIVED','PREPARING','READY','DELIVERED','CANCELLED'].map(s=>`<div class="status-chip ${s}"><b>${labels[s]}</b><span>${orders.filter(o=>o.status===s).length}</span></div>`).join('');
  activeOrders.innerHTML=active.sort((a,b)=>b.id-a.id).map(card).join('')||'<p class="muted">Sin pedidos activos.</p>';
}
function card(o){return `<article class="order-card ${o.status}"><div class="order-head"><div><h3>#${o.id} ${source(o)}</h3><small>${labels[o.status]} - ${age(o.createdAt)}</small></div><span class="pill">${money(total(o))}</span></div><div class="order-items">${o.items.map(i=>`<div class="order-item"><span><b>${i.qty} x ${i.name}</b>${(i.extras||[]).map(e=>`<br><small>+ ${e.name}</small>`).join('')}${i.notes?`<br><small>Nota: ${i.notes}</small>`:''}</span></div>`).join('')}</div>${o.notes?`<p class="muted">Obs: ${o.notes}</p>`:''}<div class="order-actions">${o.status==='RECEIVED'?`<button class="btn-red" onclick="cancelOrder(${o.id})">Cancelar</button>`:`<button class="btn-state" disabled>Cancelar</button>`}</div></article>`;}
async function updateOrder(id,status,extra={}){const r=await fetch('/api/order/update',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,status,...extra})});if(!r.ok)alert(await r.text());load();}
function cancelOrder(id){const reason=prompt('Motivo: cliente cancelo, sin ingredientes, error del pedido u otro','cliente cancelo');if(reason)updateOrder(id,'CANCELLED',{cancelReason:reason});}
let ws=new WebSocket(`ws://${location.host}/ws`); ws.onmessage=load; load(); setInterval(render,30000);
