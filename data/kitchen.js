const ordersContainer = document.getElementById("orders");
const wsStatus = document.getElementById("wsStatus");

let orders = [];

function formatCurrency(value) {
  return "$" + Number(value).toLocaleString("es-CO");
}

function statusLabel(status) {
  const map = {
    RECEIVED: "Recibido",
    PREPARING: "Preparando",
    READY: "Listo",
    DELIVERED: "Entregado"
  };

  return map[status] || status;
}

function sourceText(order) {
  if (order.sourceType === "counter") {
    return `Contador #${order.counterNumber}`;
  }

  return `Mesa ${order.table}`;
}

async function loadOrders() {
  const response = await fetch("/api/orders");
  orders = await response.json();
  renderOrders();
}

function renderOrders() {
  ordersContainer.innerHTML = "";

  if (orders.length === 0) {
    ordersContainer.innerHTML = `<section class="card"><p>No hay pedidos todavía.</p></section>`;
    return;
  }

  orders
    .slice()
    .reverse()
    .forEach(order => {
      const total = order.items.reduce((sum, item) => sum + item.price * item.qty, 0);

      const div = document.createElement("section");
      div.className = "card order-card";

      div.innerHTML = `
        <div class="order-header">
          <h2>Pedido #${order.id}</h2>
          <span class="badge">${statusLabel(order.status)}</span>
        </div>

        <p><strong>Origen:</strong> ${sourceText(order)}</p>

        <ul>
          ${order.items.map(item => `
            <li>${item.qty} x ${item.name} - ${formatCurrency(item.price * item.qty)}</li>
          `).join("")}
        </ul>

        <p class="total">Total: <strong>${formatCurrency(total)}</strong></p>

        <div class="actions">
          <button onclick="updateOrder(${order.id}, 'PREPARING')">Preparando</button>
          <button onclick="updateOrder(${order.id}, 'READY')">Listo</button>
          <button onclick="updateOrder(${order.id}, 'DELIVERED')">Entregado</button>
          <button class="danger" onclick="deleteOrder(${order.id})">Eliminar</button>
        </div>
      `;

      ordersContainer.appendChild(div);
    });
}

async function updateOrder(orderId, status) {
  const response = await fetch(`/api/order/${orderId}`, {
    method: "PUT",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({ status })
  });

  if (response.ok) {
    const updated = await response.json();
    orders = orders.map(order => order.id === updated.id ? updated : order);
    renderOrders();
  }
}

async function deleteOrder(orderId) {
  const response = await fetch(`/api/order/${orderId}`, {
    method: "DELETE"
  });

  if (response.ok) {
    orders = orders.filter(order => order.id !== orderId);
    renderOrders();
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

    if (message.event === "newOrder") {
      orders.push(message.data);
      renderOrders();
    }

    if (message.event === "orderUpdated") {
      orders = orders.map(order => order.id === message.data.id ? message.data : order);
      renderOrders();
    }

    if (message.event === "orderDeleted") {
      orders = orders.filter(order => order.id !== message.data.id);
      renderOrders();
    }
  };
}

loadOrders();
connectWebSocket();
