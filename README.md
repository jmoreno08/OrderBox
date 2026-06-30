# OrderBox Sprint 2 - PlatformIO

Version refactorizada del Sprint 2 para ESP32/ESP32-S3 usando PlatformIO con Arduino Framework.

## Funciones incluidas

- Servidor web local en ESP32.
- Modo Access Point offline.
- LittleFS para archivos web y persistencia JSON.
- CRUD de productos y mesas.
- Bloqueo de mesas.
- Pedidos por mesa o contador.
- Panel cliente, cocina y administracion.
- WebSocket para actualizacion en tiempo real.
- Rutas sin regex para evitar el error `AsyncURIMatcher`.

## Requisitos

- PlatformIO IDE para VS Code o PlatformIO CLI.
- Cable USB de datos para flashear el ESP32-S3.
- Placa ESP32-S3 compatible, por ejemplo `ESP32S3 Dev Module`.

Las librerias se instalan desde `platformio.ini`:

- `ArduinoJson`
- `ESP32Async/AsyncTCP`
- `ESP32Async/ESPAsyncWebServer`

## Ejecutar con PlatformIO

Desde la raiz del proyecto, primero verifica que PlatformIO este disponible:

```bash
pio --version
```

Compila el firmware:

```bash
pio run
```

Conecta la placa ESP32-S3 por USB y sube el firmware:

```bash
pio run -t upload
```

Despues sube los archivos web de `data/` al filesystem LittleFS:

```bash
pio run -t uploadfs
```

Abre el monitor serial para ver la IP, mensajes de arranque y errores:

```bash
pio device monitor -b 115200
```

Si cambias solo HTML, CSS, JavaScript o imagenes, normalmente basta con ejecutar de nuevo:

```bash
pio run -t uploadfs
```

Si cambias codigo C++ en `src/`, ejecuta de nuevo:

```bash
pio run -t upload
```

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
|   `-- Routes.cpp
|-- data/
|   |-- index.html
|   |-- kitchen.html
|   |-- admin.html
|   |-- style.css
|   |-- app.js
|   |-- kitchen.js
|   |-- admin.js
|   `-- img/
`-- OrderBox.ino
```

`OrderBox.ino` queda como variante para Arduino IDE. El build de PlatformIO usa `src/main.cpp` y los modulos en `src/`.

## Acceso inicial

Despues de cargar el firmware y `data/`:

- Red WiFi: `OrderBox`
- Password: `12345678`
- Menu: `http://192.168.4.1/`
- Cocina: `http://192.168.4.1/kitchen.html`
- Admin: `http://192.168.4.1/admin.html`

## Seguridad de Admin y Cocina

Las vistas de cocina y administracion usan autenticacion HTTP Basic.

Credenciales por defecto:

```text
Usuario: admin
Password: 12345678
```

Rutas protegidas:

- `http://192.168.4.1/admin.html`
- `http://192.168.4.1/kitchen.html`
- APIs de productos, mesas, pedidos, configuracion, reset y carga de imagenes.

La vista cliente (`/?table=1` o `/?mode=counter`) no muestra accesos a Admin ni Cocina. Solo conserva las rutas publicas necesarias para leer el menu, leer configuracion publica y crear pedidos.

## QR por mesa

Cada QR puede apuntar a:

```text
http://192.168.4.1/?table=1
```

Tambien funciona con mDNS si luego se agrega:

```text
http://orderbox.local/?table=1
```

## Imagenes de productos

Para operacion offline, guarda las imagenes en LittleFS dentro de:

```text
data/img/
```

Luego en el producto usa una ruta como:

```text
/img/burger.svg
/img/pizza.svg
/img/producto.jpg
```

Recomendacion para ESP32: usar imagenes pequenas, idealmente WebP/JPG optimizadas de 30 KB a 80 KB por producto.

Tambien puedes cargarlas desde el navegador en el panel Admin:

1. Entra a `http://192.168.4.1/admin.html`.
2. En Productos, selecciona una imagen en el campo de archivo.
3. Completa nombre, precio, categoria y descripcion.
4. Pulsa `Guardar producto`.

El ESP32 guarda la imagen en LittleFS bajo `/img/products/` y asigna esa ruta al producto. Se aceptan archivos `.jpg`, `.jpeg`, `.png`, `.webp` y `.svg`, con limite de 120 KB por imagen.
