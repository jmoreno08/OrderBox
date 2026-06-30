let products = [];
let cart = [];
const cartBox = () => document.getElementById('cart');
let activeCategory = 'Todas';
const qs = new URLSearchParams(location.search);
const table = qs.get('table') || '';
const money = n => '$' + Number(n || 0).toLocaleString('es-CO');
const img = p => p.image || '/img/placeholder.svg';

async function load(){
  const s = await fetch('/api/public-settings').then(r=>r.json());
  document.getElementById('title').textContent = s.businessName || 'OrderBox';
  document.getElementById('tableBadge').textContent = table ? `Mesa ${table}` : 'Pedido mostrador';
  products = await fetch('/api/menu').then(r=>r.json());
  renderCategories();
  renderMenu();
  renderCart();
}
function renderCategories(){
  const cats = ['Todas', ...new Set(products.map(p=>p.category || 'General'))];
  categories.innerHTML = cats.map(c=>`<button class="${c===activeCategory?'active':''}" onclick="setCategory('${c}')">${icon(c)} ${c}</button>`).join('');
}
function icon(c){return ({Todas:'✨',Hamburguesas:'🍔',Pizzas:'🍕',Entradas:'🍟',Bebidas:'🥤',Postres:'🧁',Ensaladas:'🥗'}[c]||'🍽️')}
function setCategory(c){activeCategory=c;renderCategories();renderMenu();}
function renderMenu(){
  const list = activeCategory === 'Todas' ? products : products.filter(p => (p.category || 'General') === activeCategory);
  menu.innerHTML = list.map(p=>`<article class="product-card"><div class="product-img"><img src="${img(p)}" onerror="this.src='/img/placeholder.svg'"></div><div class="product-body"><h3>${p.name}</h3><p>${p.description || 'Producto disponible'}</p><div class="price-row"><span class="price">${money(p.price)}</span><button class="primary" onclick="add(${p.id})">Agregar</button></div></div></article>`).join('');
}
function add(id){
  const p = products.find(x=>x.id===id);
  const i = cart.find(x=>x.productId===id);
  if(i) i.qty++; else cart.push({productId:p.id,name:p.name,qty:1,price:p.price,image:img(p)});
  renderCart();
}
function dec(id){const i=cart.find(x=>x.productId===id); if(!i)return; i.qty--; if(i.qty<=0) del(id); else renderCart();}
function del(id){cart=cart.filter(i=>i.productId!==id); renderCart();}
function clearCart(){cart=[];renderCart();msg.textContent='';}
function renderCart(){
  if(!cart.length){cartBox().innerHTML='<p class="muted">Aún no agregas productos.</p>'; total.textContent=money(0); return;}
  cartBox().innerHTML = cart.map(i=>`<div class="cart-item"><img src="${i.image}" onerror="this.src='/img/placeholder.svg'"><div><b>${i.name}</b><small class="muted">${money(i.price)}</small><div class="qty"><button onclick="dec(${i.productId})">−</button><span>${i.qty}</span><button onclick="add(${i.productId})">+</button></div></div><strong>${money(i.qty*i.price)}</strong></div>`).join('');
  total.textContent = money(cart.reduce((a,i)=>a+i.qty*i.price,0));
}
async function sendOrder(){
  if(!cart.length){msg.textContent='Agrega al menos un producto.';return;}
  const body = {sourceType:table?'table':'counter',table,items:cart,notes:notes.value || ''};
  const r = await fetch('/api/order/create',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
  msg.textContent = r.ok ? '✅ Pedido enviado a cocina' : 'No fue posible enviar el pedido';
  if(r.ok){cart=[]; notes.value=''; renderCart();}
}
load();
