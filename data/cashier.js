let orders=[],currency='$';
const money=n=>currency+Number(n||0).toLocaleString('es-CO');
const labels={RECEIVED:'Recibido',PREPARING:'Preparando',READY:'Listo',DELIVERED:'Entregado',CANCELLED:'Cancelado'};
function total(o){return o.items.reduce((a,i)=>a+(i.price+(i.extras||[]).reduce((x,e)=>x+Number(e.price||0),0))*i.qty,0);}
function today(o){return new Date(Number(o.createdAt||0)).toDateString()===new Date().toDateString();}
function src(o){return o.sourceType==='counter'?`Mostrador #${o.counterNumber}`:`Mesa ${o.table}`;}
async function load(){const [o,b]=await Promise.all([fetch('/api/orders').then(r=>r.json()),fetch('/api/business').then(r=>r.json()).catch(()=>({}))]);orders=o;currency=b.currency||currency;render();}
function render(){const day=orders.filter(today);stats.innerHTML=`<div class="stat">Pedidos del dia<b>${day.length}</b></div><div class="stat">Ventas estimadas<b>${money(day.reduce((a,o)=>a+total(o),0))}</b></div><div class="stat">Pendientes<b>${day.filter(o=>!['DELIVERED','CANCELLED'].includes(o.status)).length}</b></div>`;ordersList.innerHTML=day.reverse().map(o=>`<div class="admin-row"><span class="logo">#${o.id}</span><div><b>${src(o)} - ${labels[o.status]}</b><br><small>${o.items.map(i=>i.qty+'x '+i.name).join(', ')} - ${money(total(o))}</small></div></div>`).join('')||'<p class="muted">Sin pedidos del dia.</p>';}
async function download(url,name){const data=await fetch(url).then(r=>r.text());const a=document.createElement('a');a.href=URL.createObjectURL(new Blob([data]));a.download=name;a.click();}
let ws=new WebSocket(`ws://${location.host}/ws`);ws.onmessage=load;load();
