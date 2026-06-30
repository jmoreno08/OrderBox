let P=[],C=[],E=[],T=[],O=[],B={currency:'$'};
const money = n => (B.currency || '$') + Number(n || 0).toLocaleString('es-CO');
const statusLabel = s => ({RECEIVED:'Recibido',PREPARING:'Preparando',READY:'Listo',DELIVERED:'Entregado',CANCELLED:'Cancelado'}[s]||s);

async function api(url, body){
  const r = await fetch(url,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body||{})});
  if(!r.ok) throw new Error(await r.text());
  return r.json();
}

function orderTotal(o){return o.items.reduce((sum,i)=>sum+(i.price+(i.extras||[]).reduce((a,e)=>a+Number(e.price||0),0))*i.qty,0);}

async function load(){
  [P,C,E,T,O,B] = await Promise.all([
    fetch('/api/products').then(r=>r.json()),
    fetch('/api/categories').then(r=>r.json()),
    fetch('/api/extras').then(r=>r.json()),
    fetch('/api/tables').then(r=>r.json()),
    fetch('/api/orders').then(r=>r.json()),
    fetch('/api/business').then(r=>r.json()).catch(()=>({currency:'$'}))
  ]);
  renderDashboard(); renderCategories(); renderProducts(); renderExtras(); renderTables(); renderHistory(); fillSelects();
  const s = await fetch('/api/settings').then(r=>r.json());
  businessName.value=s.businessName||''; apSsid.value=s.apSsid||''; apPassword.value=s.apPassword||''; counterModeEnabled.checked=!!s.counterModeEnabled;
  businessName.value=B.businessName||s.businessName||''; businessLogo.value=B.logo||''; primaryColor.value=B.primaryColor||'#5b4bff'; address.value=B.address||''; phone.value=B.phone||''; currency.value=B.currency||'$'; taxEnabled.checked=!!B.taxEnabled; taxRate.value=B.taxRate||0; serviceTipEnabled.checked=!!B.serviceTipEnabled; serviceTipRate.value=B.serviceTipRate||0;
  if(B.primaryColor) document.documentElement.style.setProperty('--primary',B.primaryColor);
}

function fillSelects(){
  pcategoryId.innerHTML=C.map(c=>`<option value="${c.id}">${c.name}</option>`).join('');
  eproductId.innerHTML=P.map(p=>`<option value="${p.id}">${p.name}</option>`).join('');
}

function renderDashboard(){
  const active = O.filter(o=>!['DELIVERED','CANCELLED'].includes(o.status)).length;
  const sales = O.reduce((a,o)=>a+orderTotal(o),0);
  const blocked = T.filter(t=>t.locked).length;
  const sold = {};
  O.forEach(o=>o.items.forEach(i=>sold[i.name]=(sold[i.name]||0)+i.qty));
  const top = Object.entries(sold).sort((a,b)=>b[1]-a[1])[0];
  stats.innerHTML = `<div class="stat">Activos<b>${active}</b></div><div class="stat">Pedidos<b>${O.length}</b></div><div class="stat">Ventas<b>${money(sales)}</b></div><div class="stat">Mas vendido<b>${top?top[0]:'-'}</b></div><div class="stat">Mesas bloqueadas<b>${blocked}</b></div>`;
}

function renderCategories(){categoriesList.innerHTML=C.map(c=>`<div class="admin-row"><span class="logo">C</span><div><b>${c.name}</b><br><small>${c.active?'Activa':'Inactiva'}</small></div><button onclick="editCategory(${c.id})">Editar</button><button class="danger-btn" onclick="removeCategory(${c.id})">Eliminar</button></div>`).join('');}
function editCategory(id){const c=C.find(x=>x.id===id); cid.value=c.id; cname.value=c.name; cactive.checked=!!c.active;}
async function saveCategory(){await api('/api/category/save',{id:Number(cid.value||0),name:cname.value,active:cactive.checked}); cid.value=''; cname.value=''; cactive.checked=true; load();}
async function removeCategory(id){await api('/api/category/delete',{id}); load();}

function renderProducts(){const q=(productSearch.value||'').toLowerCase();const list=P.filter(p=>!q||`${p.name} ${p.category} ${p.description}`.toLowerCase().includes(q));productsList.innerHTML=list.map(p=>`<div class="admin-row"><img class="mini-img" src="${p.image||'/img/placeholder.svg'}" onerror="this.src='/img/placeholder.svg'"><div><b>${p.name}</b><br><small>${p.category||'General'} - ${money(p.price)} - ${p.active?'Activo':'Inactivo'}</small></div><button onclick="editProduct(${p.id})">Editar</button><button class="danger-btn" onclick="removeProduct(${p.id})">Eliminar</button></div>`).join('')||'<p class="muted">Sin productos.</p>';}
function editProduct(id){const p=P.find(x=>x.id===id); pname.dataset.id=p.id; pname.value=p.name; pprice.value=p.price; pcategoryId.value=p.categoryId; pdescription.value=p.description||''; pimage.value=p.image||''; pfile.value=''; pactive.checked=!!p.active;}
async function uploadProductImage(){const file=pfile.files[0]; if(!file) return pimage.value||'/img/placeholder.svg'; if(file.size>120000) throw new Error('Imagen maximo 120 KB'); const body=new FormData(); body.append('image',file); const r=await fetch('/api/upload/image',{method:'POST',body}); const j=await r.json(); if(!r.ok) throw new Error(j.error||'Upload error'); return j.path;}
async function saveProduct(){const image=await uploadProductImage(); await api('/api/product/save',{id:Number(pname.dataset.id||0),name:pname.value,price:Number(pprice.value),categoryId:Number(pcategoryId.value),description:pdescription.value,image,active:pactive.checked}); ['pname','pprice','pdescription','pimage'].forEach(id=>document.getElementById(id).value=''); pfile.value=''; pname.dataset.id=''; load();}
async function removeProduct(id){await api('/api/product/delete',{id}); load();}

function renderExtras(){extrasList.innerHTML=E.map(e=>`<div class="admin-row"><span class="logo">+</span><div><b>${e.name}</b><br><small>${(P.find(p=>p.id===e.productId)||{}).name||'-'} - ${money(e.price)} - ${e.active?'Activo':'Inactivo'}</small></div><button onclick="editExtra(${e.id})">Editar</button><button class="danger-btn" onclick="removeExtra(${e.id})">Eliminar</button></div>`).join('');}
function editExtra(id){const e=E.find(x=>x.id===id); eid.value=e.id; eproductId.value=e.productId; ename.value=e.name; eprice.value=e.price; eactive.checked=!!e.active;}
async function saveExtra(){await api('/api/extra/save',{id:Number(eid.value||0),productId:Number(eproductId.value),name:ename.value,price:Number(eprice.value),active:eactive.checked}); eid.value=''; ename.value=''; eprice.value=''; eactive.checked=true; load();}
async function removeExtra(id){await api('/api/extra/delete',{id}); load();}

function renderTables(){tablesList.innerHTML=T.map(t=>`<div class="admin-row"><span class="logo">M</span><div><b>${t.name}</b><br><small>${t.locked?'Bloqueada':'Disponible'} - http://192.168.4.1/?table=${t.id}</small></div><button onclick="copyText('http://192.168.4.1/?table=${t.id}')">Copiar URL</button><button onclick="editTable('${t.id}')">Editar</button><button class="danger-btn" onclick="removeTable('${t.id}')">Eliminar</button></div>`).join('') + `<div class="admin-row"><span class="logo">#</span><div><b>Contador</b><br><small>http://192.168.4.1/?mode=counter</small></div><button onclick="copyText('http://192.168.4.1/?mode=counter')">Copiar URL</button></div>`;}
function copyText(v){navigator.clipboard&&navigator.clipboard.writeText(v);}
function editTable(id){const t=T.find(x=>x.id===id); tid.value=t.id; tname.value=t.name; tlocked.checked=!!t.locked;}
async function saveTable(){await api('/api/table/save',{id:tid.value,name:tname.value,locked:tlocked.checked}); tid.value=''; tname.value=''; tlocked.checked=false; load();}
async function removeTable(id){await api('/api/table/delete',{id}); load();}

function renderHistory(){
  const state=filterStatus.value, q=filterText.value.toLowerCase(), table=filterTable.value.toLowerCase();
  let list=O.slice().reverse().filter(o=>(!state||o.status===state)&&(!table||String(o.table).toLowerCase().includes(table))&&(!q||JSON.stringify(o).toLowerCase().includes(q)));
  historyList.innerHTML=list.map(o=>`<div class="admin-row"><span class="logo">#${o.id}</span><div><b>${statusLabel(o.status)} - ${o.sourceType==='counter'?'Mostrador '+o.counterNumber:'Mesa '+o.table}</b><br><small>${o.items.map(i=>i.qty+'x '+i.name).join(', ')} - ${money(orderTotal(o))}${o.cancelReason?' - '+o.cancelReason:''}</small></div>${o.status==='RECEIVED'?`<button class="danger-btn" onclick="cancelOrder(${o.id})">Cancelar</button>`:`<button disabled>Cancelar</button>`}<button class="danger-btn" onclick="deleteOrder(${o.id})">Eliminar</button></div>`).join('');
}
async function deleteOrder(id){await api('/api/order/delete',{id}); load();}
async function cancelOrder(id){const cancelReason=prompt('Motivo de cancelacion: cliente cancelo, sin ingredientes, error del pedido u otro','cliente cancelo');if(cancelReason){await api('/api/order/update',{id,status:'CANCELLED',cancelReason});load();}}

async function saveSettings(){await api('/api/settings/save',{businessName:businessName.value,apSsid:apSsid.value,apPassword:apPassword.value,counterModeEnabled:counterModeEnabled.checked});load();}
async function saveBusiness(){await api('/api/business/save',{businessName:businessName.value,logo:businessLogo.value,primaryColor:primaryColor.value,address:address.value,phone:phone.value,currency:currency.value,taxEnabled:taxEnabled.checked,taxRate:Number(taxRate.value),serviceTipEnabled:serviceTipEnabled.checked,serviceTipRate:Number(serviceTipRate.value)});load();}
async function resetData(){await fetch('/api/reset',{method:'POST'});load();}
async function exportBackup(){const data=await fetch('/api/backup/export').then(r=>r.text()); const a=document.createElement('a'); a.href=URL.createObjectURL(new Blob([data],{type:'application/json'})); a.download='orderbox-backup.json'; a.click();}
async function exportOrders(type){const url=type==='csv'?'/api/orders/export.csv':'/api/orders/export.json'; const data=await fetch(url).then(r=>r.text()); const a=document.createElement('a'); a.href=URL.createObjectURL(new Blob([data])); a.download=type==='csv'?'orderbox-orders.csv':'orderbox-orders.json'; a.click();}
async function importBackup(){const f=backupFile.files[0]; if(!f) return; await api('/api/backup/import', JSON.parse(await f.text())); backupFile.value=''; load();}

load();
