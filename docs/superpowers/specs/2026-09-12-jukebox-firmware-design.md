# Diseño de firmware — ESP32 Mini Jukebox

Fecha: 2026-09-12

## Objetivo

Firmware para un reproductor tipo jukebox de Minecraft: ESP32 + DFPlayer Mini
(reproductor MP3 por microSD) + pantalla OLED SSD1306 128x64 (I2C) + 3 botones
(Prev, Select, Next). Arranca solo, sin menú previo: al terminar el splash de
bienvenida empieza a reproducir de inmediato.

## Hardware / pinout

| Señal | Pin ESP32 | Notas |
|---|---|---|
| OLED SDA | GPIO21 | I2C por defecto |
| OLED SCL | GPIO22 | I2C por defecto |
| DFPlayer RX | GPIO17 (TX2) | con resistencia de 1kΩ en serie |
| DFPlayer TX | GPIO16 (RX2) | |
| DFPlayer VCC | 5V | GND común con todo el circuito |
| Botón Prev | GPIO32 | `INPUT_PULLUP`, activo en bajo |
| Botón Select | GPIO25 | `INPUT_PULLUP`, activo en bajo |
| Botón Next | GPIO33 | `INPUT_PULLUP`, activo en bajo |

## Máquina de estados

```
BOOT ──(splash termina, elige pista inicial)──► PLAYING
                                                    │  ▲
                                    SELECT largo ───┘  │ SELECT corto (confirma)
                                                    ▼  │
                                                 LIST_VIEW
```

- **BOOT**: animación de bienvenida (frames, no bloqueante, ~2-3s). Al terminar,
  elige la pista inicial (random si `RANDOM_MODE`, si no la pista 0) y pasa a
  `PLAYING`.
- **PLAYING** (con sub-estado `paused: bool`): pantalla con disco animado +
  título + autor (+ ícono de pausa si `paused`).
  - `SELECT` corto → alterna `paused`.
  - `SELECT` mantenido (~600ms) → entra a `LIST_VIEW` (cursor arranca en la
    pista actual).
  - `NEXT` corto → siguiente pista (ver "Selección de pista" abajo), siempre
    queda reproduciendo (`paused = false`).
  - `NEXT` mantenido → sube volumen en pasos, repitiendo cada ~150ms mientras
    se mantiene presionado, hasta el máximo del DFPlayer (30).
  - `PREV` corto → pista anterior (ver abajo), `paused = false`.
  - `PREV` mantenido → baja volumen igual que `NEXT` mantenido, hasta 0.
  - Evento `player.justFinished()` (la pista terminó sola) → se comporta como
    un `NEXT` automático (misma lógica de selección de siguiente pista),
    permanece en `PLAYING`.
- **LIST_VIEW**: lista vertical con scroll de todos los títulos (sin autor),
  fila del cursor resaltada/invertida.
  - `NEXT` corto → cursor baja (circular).
  - `PREV` corto → cursor sube (circular).
  - `SELECT` corto → reproduce la pista en el cursor, vuelve a `PLAYING`.
  - `NEXT`/`PREV` mantenidos no hacen nada especial aquí (sin repetición de
    cursor ni control de volumen en esta pantalla — YAGNI).

## Selección de pista (secuencial vs. random)

Constante de configuración en `tracks.h`:

```cpp
constexpr bool RANDOM_MODE = true; // false = modo secuencial circular
```

**Modo secuencial (`RANDOM_MODE = false`)**: `NEXT`/`PREV`/auto-avance mueven
un índice `% totalTracks`, circular en ambos extremos (después de la última
vuelve a la primera, y viceversa).

**Modo random (`RANDOM_MODE = true`, por defecto)**:
- Se mantiene un historial en memoria: `int history[HISTORY_SIZE]` (tamaño
  20) + `historyLen` + `historyPos`.
- "Avanzar" (`NEXT` corto o auto-avance) — si `historyPos` está al final del
  historial, se genera un índice random nuevo (reintentando si coincide con
  la pista actual, para no repetirla de inmediato), se agrega al historial y
  `historyPos` avanza; si `historyPos` no está al final (el usuario había
  retrocedido), simplemente avanza al siguiente valor ya existente en el
  historial en vez de generar uno nuevo.
- "Retroceder" (`PREV` corto) — si `historyPos > 0`, retrocede el puntero y
  reproduce esa pista (nunca genera una random nueva).
- La pista inicial tras `BOOT` también es random y se agrega como primera
  entrada del historial.

## Componentes

- **`buttons.h/.cpp`**: por botón, antirrebote ~25ms (`millis()`). Expone:
  - `shortPress()` — edge, se dispara al soltar antes del umbral de largo.
  - `longPressEdge()` — edge, se dispara una sola vez al cruzar ~600ms
    (solo se usa para `SELECT`).
  - `isHeld()` — estado continuo, usado por `NEXT`/`PREV` para repetir el
    paso de volumen cada ~150ms mientras siguen presionados.
- **`display.h/.cpp`** (sobre `Adafruit_SSD1306`):
  - `showBoot(frame)` — dibuja el frame de splash correspondiente.
  - `showPlaying(track, paused)` — disco animado + título + autor + ícono
    de pausa opcional.
  - `showList(tracks[], cursorIndex)` — lista con scroll, ventana que sigue
    al cursor, fila resaltada.
  - `showError(message)` — pantalla fija de error.
- **`player.h/.cpp`** (envuelve `DFRobotDFPlayerMini` sobre `Serial2`):
  - `bool begin()` — inicializa, retorna `false` si el módulo no responde.
  - `play(index)`, `pause()`, `resume()`.
  - `volumeUp()`, `volumeDown()` — con límites 0-30.
  - `justFinished()` — sondea el evento de fin de pista de la librería.
- **`tracks.h`**: `struct Track { const char* title; const char* author;
  uint16_t fileIndex; }` + tabla placeholder (~10 entradas genéricas, el
  usuario las reemplaza después) + `constexpr bool RANDOM_MODE`.
- **`main.cpp`**: `enum class State { BOOT, PLAYING, LIST_VIEW }` + estado de
  reproducción (pista actual, `paused`, historial) + cursor de lista. Conecta
  los tres módulos en `setup()`/`loop()`, sin lógica de bajo nivel propia.

## Flujo de datos (cada vuelta del `loop()`, no bloqueante)

1. Sondear los 3 botones → evento(s).
2. Según `state` actual + evento, mutar estado / llamar a `player`
   (incluye la lógica de selección de pista secuencial/random descrita
   arriba).
3. Sondear `player.justFinished()` → si `state == PLAYING` y no
   `paused`, avanzar automáticamente.
4. Redibujar `display` según el `state` actual, throttled a ~30ms para
   que la animación del disco se vea fluida sin saturar el bus I2C.

## Manejo de errores

- Si `player.begin()` falla al iniciar → `display.showError("DFPlayer no
  detectado")`, no se intenta reproducir nada (evita acceder a un módulo
  ausente).
- Volumen siempre acotado a 0-30 (rango real del DFPlayer), sin desbordarse
  aunque se mantenga presionado el botón.
- Índices de pista/cursor siempre `% totalTracks` para el wraparound
  circular en modo secuencial; en modo random, el historial nunca se deja
  vacío (se siembra con la pista inicial en `BOOT`).

## Pruebas

Firmware embebido — no hay tests automatizados razonables. Checklist manual
de bring-up:

- El boot muestra el splash y arranca reproduciendo (pista random si
  `RANDOM_MODE = true`, pista 0 si es `false`).
- Cada botón (corto/largo/mantenido) hace lo esperado en cada estado
  (`PLAYING` y `LIST_VIEW`).
- El wraparound funciona en ambos extremos en modo secuencial.
- En modo random: `NEXT` nunca repite la pista actual de inmediato, `PREV`
  siempre regresa a la pista realmente anterior (no a otra random), y esto
  se sostiene incluso después de una secuencia de PREV seguida de NEXT.
- Desconectar el DFPlayer muestra el mensaje de error en vez de colgarse.
- Cambiar `RANDOM_MODE` a `false` recompila y vuelve al comportamiento
  secuencial sin tocar nada más del código.
