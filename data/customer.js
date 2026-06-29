const menuContainer = document.getElementById("menu");
const cartContainer = document.getElementById("cart");
const totalLabel = document.getElementById("total");
const sourceLabel = document.getElementById("sourceLabel");
const sendOrderBtn = document.getElementById("sendOrderBtn");
const message = document.getElementById("message");

const params = new URLSearchParams(window.location.search);

const table = params.get("table");
const mode = params.get("mode");

const sourceType = mode === "counter" || !table ? "counter" : "table";

if (sourceType === "table") {
  sourceLabel.innerHTML = `Pedido para mesa: <strong>${table}</strong>`;
} else {
  sourceLabel.innerHTML = `Pedido por: <strong>contador</strong>`;
}

let menu = [];
let cart = {};

function formatCurrency(value) {
  return "$" + Number(value).toLocaleString("es-CO");
}

async function loadMenu() {
  const response = await fetch("/api/menu");
  menu = await response.json();
  renderMenu();
}

function renderMenu() {
  menuContainer.innerHTML = "";

  if (menu.length === 0) {
    menuContainer.innerHTML = "<p>No hay productos activos en este momento.</p>";
    return;
  }

  menu.forEach(product => {
    const div = document.createElement("div");
    div.className = "menu-item";

    div.innerHTML = `
      <div>
        <strong>${product.name}</strong>
        <span>${formatCurrency(product.price)}</span>
      </div>
      <button onclick="addToCart(${product.id})">Agregar</button>
    `;

    menuContainer.appendChild(div);
  });
}

function addToCart(productId) {
  const product = menu.find(p => p.id === productId);
  if (!product) return;

  if (!cart[productId]) {
    cart[productId] = {
      productId: product.id,
      name: product.name,
      price: product.price,
      qty: 0
    };
  }

  cart[productId].qty++;
  renderCart();
}

function removeFromCart(productId) {
  if (!cart[productId]) return;

  cart[productId].qty--;

  if (cart[productId].qty <= 0) {
    delete cart[productId];
  }

  renderCart();
}

function renderCart() {
  const items = Object.values(cart);
  cartContainer.innerHTML = "";

  if (items.length === 0) {
    cartContainer.innerHTML = "<p>El carrito está vacío.</p>";
    totalLabel.textContent = formatCurrency(0);
    return;
  }

  let total = 0;

  items.forEach(item => {
    total += item.price * item.qty;

    const div = document.createElement("div");
    div.className = "cart-item";

    div.innerHTML = `
      <div>
        <strong>${item.name}</strong>
        <span>${item.qty} x ${formatCurrency(item.price)}</span>
      </div>
      <button onclick="removeFromCart(${item.productId})">-</button>
    `;

    cartContainer.appendChild(div);
  });

  totalLabel.textContent = formatCurrency(total);
}

async function sendOrder() {
  const items = Object.values(cart).map(item => ({
    productId: item.productId,
    qty: item.qty
  }));

  if (items.length === 0) {
    message.textContent = "Agrega productos antes de enviar.";
    return;
  }

  sendOrderBtn.disabled = true;
  message.textContent = "Enviando pedido...";

  const payload = {
    sourceType,
    table: sourceType === "table" ? table : "",
    items
  };

  try {
    const response = await fetch("/api/order", {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify(payload)
    });

    const result = await response.json();

    if (!response.ok) {
      if (response.status === 423) {
        message.textContent = "Esta mesa está bloqueada temporalmente.";
      } else {
        message.textContent = result.error || "No se pudo enviar el pedido.";
      }
      return;
    }

    cart = {};
    renderCart();

    if (result.sourceType === "counter") {
      message.textContent = `Pedido de contador #${result.counterNumber} enviado correctamente.`;
    } else {
      message.textContent = `Pedido #${result.id} enviado correctamente.`;
    }
  } catch (error) {
    message.textContent = "Error enviando el pedido.";
  } finally {
    sendOrderBtn.disabled = false;
  }
}

sendOrderBtn.addEventListener("click", sendOrder);

loadMenu();
renderCart();
