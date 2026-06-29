# OrderBox Sprint 1 - MVP actualizado

Proyecto para ESP32-S3 con Arduino Framework y PlatformIO.

## Funciones incluidas

- WiFi AP local.
- Servidor web local.
- Cliente por QR de mesa.
- Cliente por QR único de contador.
- Menú hardcodeado.
- Carrito.
- Creación de pedidos.
- Pedidos en memoria RAM.
- Panel de cocina en tiempo real.
- Panel de administración básico.
- Bloqueo/desbloqueo de mesas.
- Activación/desactivación de productos.
- WebSocket para cocina y administración.

## Red WiFi

```txt
SSID: OrderBox
Password: 12345678
```

## URLs principales

Cliente por mesa:

```txt
http://192.168.4.1/?table=1
```

Cliente por contador:

```txt
http://192.168.4.1/?mode=counter
```

Panel cocina:

```txt
http://192.168.4.1/kitchen.html
```

Panel administración:

```txt
http://192.168.4.1/admin.html
```

## Funcionamiento de QR

### QR por mesa

Cada mesa puede tener un QR diferente:

```txt
http://192.168.4.1/?table=1
http://192.168.4.1/?table=2
http://192.168.4.1/?table=3
```

El sistema asocia automáticamente el pedido a la mesa.

### QR único por contador

Cuando el negocio no utiliza mesas:

```txt
http://192.168.4.1/?mode=counter
```

El sistema asigna automáticamente un número de contador:

```txt
Contador #1
Contador #2
Contador #3
```

## Panel de administración

Permite:

- Desactivar productos agotados.
- Activar productos disponibles.
- Bloquear mesas.
- Desbloquear mesas.

Una mesa bloqueada no permite recibir nuevos pedidos desde su QR.

## API

### Obtener menú activo para cliente

```http
GET /api/menu
```

### Obtener todos los productos para administración

```http
GET /api/admin/menu
```

### Obtener mesas

```http
GET /api/tables
```

### Crear pedido

Pedido por mesa:

```http
POST /api/order
Content-Type: application/json

{
  "sourceType": "table",
  "table": "1",
  "items": [
    {
      "productId": 1,
      "qty": 2
    }
  ]
}
```

Pedido por contador:

```http
POST /api/order
Content-Type: application/json

{
  "sourceType": "counter",
  "items": [
    {
      "productId": 1,
      "qty": 2
    }
  ]
}
```

### Ver pedidos

```http
GET /api/orders
```

### Actualizar estado de pedido

```http
PUT /api/order/1
Content-Type: application/json

{
  "status": "READY"
}
```

Estados válidos:

- RECEIVED
- PREPARING
- READY
- DELIVERED

### Desactivar producto

```http
PUT /api/product/1
Content-Type: application/json

{
  "active": false
}
```

### Bloquear mesa

```http
PUT /api/table/1
Content-Type: application/json

{
  "locked": true
}
```

## Cargar firmware

```bash
pio run -t upload
```

## Cargar archivos web a LittleFS

```bash
pio run -t uploadfs
```

## Monitor serial

```bash
pio device monitor
```

## Limitaciones del Sprint 1

- Pedidos guardados en RAM.
- Productos y mesas hardcodeados.
- No hay login real en administración.
- No hay persistencia después de reinicio.
- No hay impresora térmica todavía.

## Siguiente mejora recomendada

Para Sprint 2:

- Persistencia en LittleFS.
- CRUD completo de productos.
- CRUD completo de mesas.
- Login admin.
- Configuración de nombre WiFi.
- Modo STA/AP configurable.
- Generador visual de QR.
- Historial de pedidos.
