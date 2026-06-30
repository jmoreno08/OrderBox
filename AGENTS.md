# Repository Guidelines

## Project Structure & Module Organization

This repository contains an ESP32/ESP32-S3 PlatformIO project for OrderBox using the Arduino framework.

- `platformio.ini` defines the ESP32-S3 environment, Arduino framework, LittleFS, and library dependencies.
- `src/` contains the modular firmware, Wi-Fi access point setup, LittleFS persistence, HTTP API routes, and WebSocket events.
- `data/` contains static files uploaded to LittleFS: customer, kitchen, and admin pages plus shared CSS.
- `OrderBox.ino` is the Arduino IDE sketch variant; keep shared behavior aligned with `src/` when changing firmware.
- `README.md` documents setup, upload, and local device URLs.
- `*.jpg` files are hardware/reference images; avoid renaming them unless documentation is updated.

Runtime JSON files such as `/products.json`, `/tables.json`, `/orders.json`, and `/settings.json` are created on the ESP32 LittleFS filesystem, not in this repository.

## Build, Test, and Development Commands

Use PlatformIO from VS Code or the CLI.

- `pio run` builds the firmware.
- `pio run -t upload` flashes the ESP32-S3.
- `pio run -t uploadfs` uploads `data/` to LittleFS.
- `pio device monitor -b 115200` opens the serial monitor.
- After flashing, connect to `SSID: OrderBox`, password `12345678`, then visit `http://192.168.4.1/`, `/kitchen.html`, or `/admin.html`.

## Coding Style & Naming Conventions

Use the existing Arduino C++ style in `src/`: two-space indentation, same-line opening braces, descriptive camelCase functions, and uppercase constants/macros. Keep route handlers small and move business logic into helper functions. JavaScript in `data/` uses plain browser APIs, `const`/`let`, camelCase names, and two-space indentation. Keep shared styling in `data/style.css`.

## Testing Guidelines

No automated tests are configured. Validate firmware changes with `pio run` and manually exercise the customer, kitchen, and admin pages on the ESP32 access point. For API changes, test affected endpoints and confirm WebSocket updates still refresh connected pages. When editing `data/`, run `pio run -t uploadfs` before device testing.

## Commit & Pull Request Guidelines

Recent commits use short imperative summaries, for example `Add OrderBox Sprint 1 MVP implementation`. Keep commits focused and describe the user-visible or firmware behavior changed. Pull requests should include a concise description, affected pages or API endpoints, manual test steps, and screenshots or notes for UI changes. Link related issues when available and call out Arduino library, board, or partition changes.

## Security & Configuration Tips

Do not commit real deployment Wi-Fi credentials. Defaults in the sketch are for local demo access only. Treat LittleFS reset and settings changes carefully because they can erase device data.
