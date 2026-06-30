# OrderBox Sprint 3 - PlatformIO

OrderBox es un sistema local de pedidos para restaurantes en ESP32-S3. Funciona offline como Access Point, sirve la UI desde LittleFS y mantiene los datos en JSON.

## Funciones principales

- Cliente por QR de mesa: `http://192.168.4.1/?table=1`.
- Cliente por contador: `http://192.168.4.1/?mode=counter`.
- Admin protegido con HTTP Basic.
- Cocina protegida con HTTP Basic y tablero Kanban.
- CRUD de productos, categorias, extras y mesas.
- Extras y observaciones por item y por pedido.
- Pedidos persistidos con estados `RECEIVED`, `PREPARING`, `READY`, `DELIVERED`, `CANCELLED`.
- WebSocket para actualizacion en tiempo real.
- Sonido opcional en cocina para nuevos pedidos.
- Dashboard, historial con filtros, backup export/import y helper de URLs.
- Base preparada para impresion ESC/POS mediante `printOrderTicket(Order& order)`.

## Comandos PlatformIO

```bash
pio run
pio run -t upload
pio run -t uploadfs
pio device monitor -b 115200
```

Ejecuta `uploadfs` cada vez que cambies archivos en `data/`.

## Acceso inicial

- WiFi: `OrderBox-XXXXXX`, donde `XXXXXX` es el ID unico del modulo ESP32-S3.
- Password: `12345678`
- Cliente: `http://192.168.4.1/`
- Cocina: `http://192.168.4.1/kitchen.html`
- Admin: `http://192.168.4.1/admin.html`

Credenciales Admin/Cocina:

```text
Usuario: admin
Password: 12345678
```

## Endpoints principales

Publicos:

- `GET /api/public-settings`
- `GET /api/menu`
- `GET /api/public/categories`
- `GET /api/public/extras`
- `POST /api/order/create`

Protegidos:

- `GET /api/products`, `POST /api/product/save`, `POST /api/product/delete`
- `GET /api/categories`, `POST /api/category/save`, `POST /api/category/delete`
- `GET /api/extras`, `POST /api/extra/save`, `POST /api/extra/delete`
- `GET /api/tables`, `POST /api/table/save`, `POST /api/table/delete`
- `GET /api/orders`, `POST /api/order/update`, `POST /api/order/delete`
- `GET /api/settings`, `POST /api/settings/save`
- `GET /api/backup/export`, `POST /api/backup/import`
- `POST /api/upload/image`, `POST /api/reset`

Todas las rutas son fijas; no se usan regex.

## Persistencia

LittleFS guarda:

- `/settings.json`
- `/categories.json`
- `/products.json`
- `/extras.json`
- `/tables.json`
- `/orders.json`

Las imagenes subidas desde Admin se guardan en `/img/products/`. Formatos aceptados: JPG, PNG, WebP y SVG. Limite actual: 120 KB por imagen.

## Estructura

```text
OrderBox/
|-- platformio.ini
|-- src/
|   |-- main.cpp
|   |-- App.h
|   |-- Globals.cpp
|   |-- Utils.cpp
|   |-- Serializers.cpp
|   |-- Storage.cpp
|   |-- Realtime.cpp
|   |-- WiFiManager.cpp
|   |-- Handlers.cpp
|   |-- Routes.cpp
|   `-- Printer.cpp
`-- data/
    |-- index.html
    |-- app.js
    |-- kitchen.html
    |-- kitchen.js
    |-- admin.html
    |-- admin.js
    |-- style.css
    `-- img/
```

## Como probar Sprint 3

1. Compila con `pio run`.
2. Sube firmware con `pio run -t upload`.
3. Sube filesystem con `pio run -t uploadfs`.
4. Conectate a la red `OrderBox-XXXXXX` que muestra el monitor serial.
5. Entra a Admin y crea categorias, productos y extras.
6. Abre `/?table=1`, agrega productos con extras y observaciones, y envia un pedido.
7. Abre Cocina, pulsa `Activar sonido` y mueve pedidos entre columnas.
8. En Admin revisa Dashboard, Historial, URLs de mesas y Backup.

## Nota tecnica

Sprint 3 mantiene arreglos estaticos para limitar RAM y evitar dependencias pesadas. El ticket se imprime por `Serial` como preparacion para UART de impresora termica en Sprint 4.
