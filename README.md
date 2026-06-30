# OrderBox Sprint 4 - PlatformIO

OrderBox es un sistema local de pedidos para restaurantes sobre ESP32-S3. El modulo crea un WiFi AP offline, sirve la interfaz desde LittleFS y persiste datos operativos en JSON.

## Funcionalidades Sprint 4

- Cliente QR por mesa: `http://192.168.4.1/?table=1`.
- Cliente mostrador: `http://192.168.4.1/?mode=counter`.
- Admin protegido con HTTP Basic para productos, categorias, extras, mesas, negocio, backup e historial.
- Dashboard en tiempo real en `/dashboard` con pedidos activos, ventas del dia, promedio de preparacion y estados.
- Kitchen Display en `/kitchen` con pedidos `RECEIVED` y `PREPARING`, tarjetas grandes, sonido y avance a `READY`.
- Panel de meseros en `/waiter` para entregar pedidos `READY`.
- Panel de caja en `/cashier` con ventas del dia y exportacion CSV.
- Historial en `/history` con filtros, repeticion de pedidos y exportacion JSON/CSV.
- Productos agrupados por categoria, fotos desde LittleFS y busqueda instantanea.
- Configuracion persistente del negocio en `/business.json`.
- Cancelacion con motivo, permitida solo cuando el pedido esta `RECEIVED`.
- Servicio de impresion preparado: `PrinterService::printOrder(const Order& order)`.

## Comandos PlatformIO

```bash
pio run
pio run -t upload
pio run -t uploadfs
pio device monitor -b 115200
```

Ejecuta `pio run -t uploadfs` cada vez que cambies archivos en `data/`.

## Acceso inicial

- WiFi: `OrderBox-XXXXXX`, donde `XXXXXX` es el ID unico del ESP32-S3.
- Password WiFi: `12345678`
- Cliente: `http://192.168.4.1/`
- Admin: `http://192.168.4.1/admin`
- Dashboard: `http://192.168.4.1/dashboard`
- Cocina: `http://192.168.4.1/kitchen`
- Meseros: `http://192.168.4.1/waiter`
- Caja: `http://192.168.4.1/cashier`
- Historial: `http://192.168.4.1/history`

Credenciales operativas:

```text
Usuario: admin
Password: 12345678
```

## Endpoints

Publicos:

- `GET /api/public-settings`
- `GET /api/menu`
- `GET /api/public/categories`
- `GET /api/public/extras`
- `POST /api/order/create`

Protegidos:

- `GET /api/orders`, `POST /api/order/update`, `POST /api/order/delete`
- `GET /api/products`, `POST /api/product/save`, `POST /api/product/delete`
- `GET /api/categories`, `POST /api/category/save`, `POST /api/category/delete`
- `GET /api/extras`, `POST /api/extra/save`, `POST /api/extra/delete`
- `GET /api/tables`, `POST /api/table/save`, `POST /api/table/delete`
- `GET /api/settings`, `POST /api/settings/save`
- `GET /api/business`, `POST /api/business/save`
- `GET /api/orders/export.json`, `GET /api/orders/export.csv`
- `GET /api/backup/export`, `POST /api/backup/import`
- `POST /api/upload/image`, `POST /api/reset`

Todas las rutas son fijas; no se usan regex.

## Persistencia LittleFS

- `/settings.json`
- `/business.json`
- `/categories.json`
- `/products.json`
- `/extras.json`
- `/tables.json`
- `/orders.json`

Las imagenes subidas desde Admin se guardan en `/img/products/`. Formatos aceptados: JPG, PNG, WebP y SVG. Limite actual: 120 KB por imagen.

## Estructura

```text
src/
  App.h, Globals.cpp, Utils.cpp, Serializers.cpp, Storage.cpp
  Realtime.cpp, WiFiManager.cpp, Handlers.cpp, Routes.cpp, Printer.cpp
data/
  index.html, app.js
  admin.html, admin.js
  dashboard.html, dashboard.js
  kitchen.html, kitchen.js
  waiter.html, waiter.js
  cashier.html, cashier.js
  history.html, history.js
  style.css, img/
```

## Como probar Sprint 4

1. Compila con `pio run`.
2. Sube firmware con `pio run -t upload`.
3. Sube LittleFS con `pio run -t uploadfs`.
4. Conectate al WiFi `OrderBox-XXXXXX`.
5. En `/admin`, configura negocio, categorias, productos, extras y mesas.
6. Desde `/?table=1`, crea un pedido con extras, foto visible y observaciones.
7. En `/dashboard`, confirma actualizacion en tiempo real sin recargar.
8. En `/kitchen`, activa sonido y cambia `Recibido -> Preparando -> Listo`.
9. En `/waiter`, marca el pedido como entregado.
10. En `/cashier`, revisa ventas del dia y exporta CSV.
11. En `/history`, filtra, repite pedidos y exporta JSON/CSV.
