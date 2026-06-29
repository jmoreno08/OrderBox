const productsContainer = document.getElementById("products");
const tablesContainer = document.getElementById("tables");
const wsStatus = document.getElementById("wsStatus");

let products = [];
let tables = [];

function formatCurrency(value) {
  return "$" + Number(value).toLocaleString("es-CO");
}

async function loadProducts() {
  const response = await fetch("/api/admin/menu");
  products = await response.json();
  renderProducts();
}

async function loadTables() {
  const response = await fetch("/api/tables");
  tables = await response.json();
  renderTables();
}

function renderProducts() {
  productsContainer.innerHTML = "";

  products.forEach(product => {
    const div = document.createElement("div");
    div.className = "admin-row";

    div.innerHTML = `
      <div>
        <strong>${product.name}</strong>
        <span>${formatCurrency(product.price)}</span>
        <small>${product.active ? "Activo" : "Inactivo"}</small>
      </div>
      <button class="${product.active ? "danger" : "success"}" onclick="toggleProduct(${product.id}, ${!product.active})">
        ${product.active ? "Desactivar" : "Activar"}
      </button>
    `;

    productsContainer.appendChild(div);
  });
}

function renderTables() {
  tablesContainer.innerHTML = "";

  tables.forEach(table => {
    const div = document.createElement("div");
    div.className = "admin-row";

    div.innerHTML = `
      <div>
        <strong>Mesa ${table.id}</strong>
        <span>QR: http://192.168.4.1/?table=${table.id}</span>
        <small>${table.locked ? "Bloqueada" : "Disponible"}</small>
      </div>
      <button class="${table.locked ? "success" : "danger"}" onclick="toggleTable('${table.id}', ${!table.locked})">
        ${table.locked ? "Desbloquear" : "Bloquear"}
      </button>
    `;

    tablesContainer.appendChild(div);
  });
}

async function toggleProduct(productId, active) {
  const response = await fetch(`/api/product/${productId}`, {
    method: "PUT",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({ active })
  });

  if (response.ok) {
    const updated = await response.json();
    products = products.map(product => product.id === updated.id ? updated : product);
    renderProducts();
  }
}

async function toggleTable(tableId, locked) {
  const response = await fetch(`/api/table/${tableId}`, {
    method: "PUT",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({ locked })
  });

  if (response.ok) {
    const updated = await response.json();
    tables = tables.map(table => table.id === updated.id ? updated : table);
    renderTables();
  }
}

function connectWebSocket() {
  const protocol = window.location.protocol === "https:" ? "wss" : "ws";
  const ws = new WebSocket(`${protocol}://${window.location.host}/ws`);

  ws.onopen = () => {
    wsStatus.textContent = "Conectado";
  };

  ws.onclose = () => {
    wsStatus.textContent = "Desconectado";
    setTimeout(connectWebSocket, 2000);
  };

  ws.onerror = () => {
    wsStatus.textContent = "Error WS";
  };

  ws.onmessage = (event) => {
    const message = JSON.parse(event.data);

    if (message.event === "productUpdated") {
      products = products.map(product => product.id === message.data.id ? message.data : product);
      renderProducts();
    }

    if (message.event === "tableUpdated") {
      tables = tables.map(table => table.id === message.data.id ? message.data : table);
      renderTables();
    }
  };
}

loadProducts();
loadTables();
connectWebSocket();
