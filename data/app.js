let products = [];
let extras = [];
let cart = [];
let activeCategory = 'Todas';
const qs = new URLSearchParams(location.search);
const table = qs.get('table') || '';
const mode = qs.get('mode') || '';
const money = n => '$' + Number(n || 0).toLocaleString('es-CO');
const img = p => p.image || '/img/placeholder.svg';

async function load(){
  const s = await fetch('/api/public-settings').then(r=>r.json());
  title.textContent = s.businessName || 'OrderBox';
  tableBadge.textContent = table ? `Mesa ${table}` : 'Pedido mostrador';
  products = await fetch('/api/menu').then(r=>r.json());
  extras = await fetch('/api/public/extras').then(r=>r.json());
  renderCategories();
  renderMenu();
  renderCart();
}

function productExtras(productId){
  return extras.filter(e => e.productId === productId && e.active);
}

function renderCategories(){
  const cats = ['Todas', ...new Set(products.map(p=>p.category || 'General'))];
  categories.innerHTML = cats.map(c=>`<button class="${c===activeCategory?'active':''}" onclick="setCategory('${c}')">${c}</button>`).join('');
}

function setCategory(c){activeCategory=c;renderCategories();renderMenu();}

function renderMenu(){
  const list = activeCategory === 'Todas' ? products : products.filter(p => (p.category || 'General') === activeCategory);
  menu.innerHTML = list.map(p=>{
    const pe = productExtras(p.id);
    return `<article class="product-card">
      <div class="product-img"><img src="${img(p)}" onerror="this.src='/img/placeholder.svg'"></div>
      <div class="product-body">
        <span class="pill">${p.category || 'General'}</span>
        <h3>${p.name}</h3>
        <p>${p.description || 'Producto disponible'}</p>
        <div class="extras-box">${pe.map(e=>`<label><input type="checkbox" data-extra="${e.id}" data-product="${p.id}"> ${e.name} ${e.price?money(e.price):''}</label>`).join('') || '<small class="muted">Sin extras</small>'}</div>
        <input id="note-${p.id}" placeholder="Nota para este item">
        <div class="price-row"><span class="price">${money(p.price)}</span><button class="primary" onclick="add(${p.id})">Agregar</button></div>
      </div>
    </article>`;
  }).join('');
}

function selectedExtras(productId){
  return Array.from(document.querySelectorAll(`[data-product="${productId}"][data-extra]:checked`))
    .map(el => extras.find(e => e.id === Number(el.dataset.extra)))
    .filter(Boolean);
}

function sameItem(a,b){
  const ax = a.extras.map(e=>e.id).sort().join(',');
  const bx = b.extras.map(e=>e.id).sort().join(',');
  return a.productId === b.productId && ax === bx && (a.notes || '') === (b.notes || '');
}

function add(id){
  const p = products.find(x=>x.id===id);
  const item = {productId:p.id,name:p.name,qty:1,price:p.price,image:img(p),extras:selectedExtras(id),notes:document.getElementById(`note-${id}`).value || ''};
  const existing = cart.find(x=>sameItem(x,item));
  if(existing) existing.qty++; else cart.push(item);
  document.querySelectorAll(`[data-product="${id}"][data-extra]`).forEach(el=>el.checked=false);
  document.getElementById(`note-${id}`).value='';
  renderCart();
}

function dec(index){cart[index].qty--; if(cart[index].qty<=0) cart.splice(index,1); renderCart();}
function inc(index){cart[index].qty++; renderCart();}
function del(index){cart.splice(index,1); renderCart();}
function clearCart(){cart=[];renderCart();msg.textContent='';}

function itemTotal(i){
  return (i.price + i.extras.reduce((a,e)=>a+Number(e.price||0),0)) * i.qty;
}

function renderCart(){
  if(!cart.length){cartBox.innerHTML='<p class="muted">Aun no agregas productos.</p>'; total.textContent=money(0); return;}
  cartBox.innerHTML = cart.map((i,idx)=>`<div class="cart-item">
    <img src="${i.image}" onerror="this.src='/img/placeholder.svg'">
    <div><b>${i.name}</b><small class="muted">${i.extras.map(e=>'+ '+e.name).join(', ') || money(i.price)}</small>${i.notes?`<small class="muted">Nota: ${i.notes}</small>`:''}<div class="qty"><button onclick="dec(${idx})">-</button><span>${i.qty}</span><button onclick="inc(${idx})">+</button><button onclick="del(${idx})">x</button></div></div>
    <strong>${money(itemTotal(i))}</strong>
  </div>`).join('');
  total.textContent = money(cart.reduce((a,i)=>a+itemTotal(i),0));
}

async function sendOrder(){
  if(!cart.length){msg.textContent='Agrega al menos un producto.';return;}
  const sourceType = table ? 'table' : 'counter';
  const body = {sourceType,table,items:cart.map(i=>({productId:i.productId,qty:i.qty,extras:i.extras.map(e=>e.id),notes:i.notes || ''})),notes:notes.value || ''};
  const r = await fetch('/api/order/create',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
  msg.textContent = r.ok ? 'Pedido enviado correctamente.' : 'No fue posible enviar el pedido';
  if(r.ok){cart=[]; notes.value=''; renderCart();}
}

load();
