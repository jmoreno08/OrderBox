const money = n => '$' + Number(n || 0).toLocaleString('es-CO');

async function load(){
  const p = await fetch('/api/products').then(r=>r.json());
  const t = await fetch('/api/tables').then(r=>r.json());
  const o = await fetch('/api/orders').then(r=>r.json());
  stats.innerHTML = `<div class="stat">Productos<b>${p.length}</b></div><div class="stat">Mesas<b>${t.length}</b></div><div class="stat">Pedidos<b>${o.length}</b></div><div class="stat">Ventas demo<b>${money(o.reduce((a,x)=>a+x.items.reduce((s,i)=>s+i.qty*i.price,0),0))}</b></div>`;
  productsList.innerHTML = p.map(x=>`<div class="admin-row"><img class="mini-img" src="${x.image||'/img/placeholder.svg'}" onerror="this.src='/img/placeholder.svg'"><div><b>${x.name}</b><br><small>${x.category||'General'} - ${money(x.price)} - ${x.active?'Activo':'Inactivo'}</small></div><button onclick="editProduct(${x.id})">Editar</button><button class="danger-btn" onclick="removeProduct(${x.id})">Eliminar</button></div>`).join('');
  window._products = p;
  tablesList.innerHTML = t.map(x=>`<div class="admin-row"><span class="logo">M</span><div><b>${x.name}</b><br><small>${x.locked?'Bloqueada':'Disponible'} - QR: /?table=${x.id}</small></div><button onclick="editTable('${x.id}')">Editar</button><button class="danger-btn" onclick="removeTable('${x.id}')">Eliminar</button></div>`).join('');
  window._tables = t;
  const s = await fetch('/api/settings').then(r=>r.json());
  businessName.value=s.businessName||'';
  apSsid.value=s.apSsid||'';
  apPassword.value=s.apPassword||'';
  counterModeEnabled.checked=!!s.counterModeEnabled;
}

function editProduct(id){
  const p=window._products.find(x=>x.id===id);
  if(!p)return;
  pname.dataset.id=p.id;
  pname.value=p.name;
  pprice.value=p.price;
  pcategory.value=p.category||'';
  pdescription.value=p.description||'';
  pimage.value=p.image||'';
  pfile.value='';
  uploadMsg.textContent='';
  pactive.checked=!!p.active;
  scrollTo({top:products.offsetTop,behavior:'smooth'});
}

async function uploadProductImage(){
  const file = pfile.files[0];
  if(!file) return pimage.value || '/img/placeholder.svg';

  if(file.size > 120000){
    uploadMsg.textContent='La imagen debe pesar maximo 120 KB.';
    throw new Error('Image too large');
  }

  uploadMsg.textContent='Subiendo imagen...';

  const body = new FormData();
  body.append('image', file);

  const response = await fetch('/api/upload/image',{method:'POST',body});
  const result = await response.json();

  if(!response.ok){
    uploadMsg.textContent=result.error || 'No se pudo subir la imagen.';
    throw new Error(uploadMsg.textContent);
  }

  uploadMsg.textContent='Imagen cargada.';
  pimage.value = result.path;
  return result.path;
}

async function saveProduct(){
  const id=Number(pname.dataset.id||0);
  const image=await uploadProductImage();
  await fetch('/api/product/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id,name:pname.value,price:Number(pprice.value),category:pcategory.value||'General',description:pdescription.value,image,active:pactive.checked})});
  ['pname','pprice','pcategory','pdescription','pimage'].forEach(id=>document.getElementById(id).value='');
  pfile.value='';
  uploadMsg.textContent='';
  pname.dataset.id='';
  pactive.checked=true;
  load();
}

async function removeProduct(id){await fetch('/api/product/delete',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id})});load();}
function editTable(id){const t=window._tables.find(x=>x.id===id); if(!t)return; tid.value=t.id;tname.value=t.name;tlocked.checked=!!t.locked;}
async function saveTable(){await fetch('/api/table/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id:tid.value,name:tname.value,locked:tlocked.checked})});tid.value='';tname.value='';tlocked.checked=false;load();}
async function removeTable(id){await fetch('/api/table/delete',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id})});load();}
async function saveSettings(){await fetch('/api/settings/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({businessName:businessName.value,apSsid:apSsid.value,apPassword:apPassword.value,counterModeEnabled:counterModeEnabled.checked})});load();}
async function resetData(){await fetch('/api/reset',{method:'POST'});load();}

load();
