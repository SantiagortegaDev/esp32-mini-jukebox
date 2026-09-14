# ESP32 Minecraft Jukebox

Una jukebox de escritorio armada con un ESP32: reproduce las 54 canciones de
*Minecraft Volume Alpha & Beta* de C418 desde una microSD, muestra el título
y el autor (o una animación) en una pantallita OLED, y se maneja entera con
tres botones.

<p align="center">
  <img src="docs/images/photo.jpg" alt="La jukebox armada" width="360">
</p>

## Qué es esto

Arranca sola: se prende, hace una barra de carga en la pantalla y ya está
sonando. No hay que tocar nada para empezar a escuchar música — los botones
son para saltar canciones, ver la lista completa, subir/bajar volumen y
elegir qué animación se muestra mientras suena.

## Hardware

| Señal          | Pin ESP32     | Notas                              |
|----------------|---------------|-------------------------------------|
| OLED SDA       | GPIO21        | I2C por defecto                     |
| OLED SCL       | GPIO22        | I2C por defecto                     |
| DFPlayer RX    | GPIO17 (TX2)  | con resistencia de 1kΩ en serie     |
| DFPlayer TX    | GPIO16 (RX2)  |                                      |
| DFPlayer VCC   | 5V            | GND común con todo el circuito      |
| Botón Prev     | GPIO32        | `INPUT_PULLUP`, activo en bajo      |
| Botón Select   | GPIO25        | `INPUT_PULLUP`, activo en bajo      |
| Botón Next     | GPIO33        | `INPUT_PULLUP`, activo en bajo      |

Módulos: ESP32 DevKit, pantalla OLED SSD1306 128×64 I2C, DFPlayer Mini +
tarjeta microSD (FAT32), 3 pulsadores.

## Compilar y subir

Con [PlatformIO](https://platformio.org/) instalado:

```bash
pio run                 # compila
pio run -t upload       # sube al ESP32 (puerto autodetectado)
pio device monitor      # log por serie, útil si algo no arranca
```

En Linux, si no detecta el puerto USB, agregá tu usuario al grupo `dialout`
(`sudo usermod -aG dialout $USER`) y reiniciá sesión.

## Los botones

- **Reproduciendo:**
  - `Select` corto = pausa/reanuda.
  - `Select` mantenido = abre la lista de canciones.
  - `Next`/`Prev` corto = siguiente/anterior canción.
  - `Next`/`Prev` mantenido = subir/bajar volumen.
- **Lista de canciones:**
  - `Next`/`Prev` = navegar la lista.
  - `Select` corto = reproducir la canción marcada.
  - `Select` mantenido = pasa al selector de animaciones.
- **Selector de animaciones:**
  - `Next`/`Prev` = probar animaciones (vista previa en vivo, con contador tipo `3/11`).
  - `Select` corto = confirmar y volver a reproducir.
  - `Select` mantenido = volver a la lista de canciones.
- Si te quedás sin tocar nada en cualquiera de los dos menús, a los 6
  segundos vuelve solo a la pantalla de reproducción.

## Las canciones

Las 54 pistas son el álbum completo de C418 (Alpha + Beta), definidas en
[`include/tracks.h`](include/tracks.h). Los mp3 no están en este repo — hay
que ponerlos vos en la microSD (ver más abajo). Por defecto reproduce en
modo aleatorio con historial: `Prev` siempre te devuelve a la canción
anterior real, no a otra random.

## Animaciones

Hay 11 animaciones disponibles: 2 dibujadas por código (un disco girando y
una notita rebotando) y 9 en bitmap, generadas con
[OLED Bitmap Generator](https://www.oledanimationmaker.com/) y guardadas en
[`include/animation_bitmaps.h`](include/animation_bitmaps.h). Se eligen a
mano desde el menú de animaciones, y la elección se queda fija hasta que
cambies de canción o entres al menú de nuevo (no se resetea sola).

La pantalla de arranque también usa un bitmap real (una barra de carga que
va llenándose cuadro a cuadro y desaparece al llegar al final), en
[`include/boot_bitmap.h`](include/boot_bitmap.h).

## Modo preview

`PREVIEW_MODE` en `config.h` (apagado por defecto) arma una intro fija para
mostrar el proyecto: arranca directo en *Chirp* con la animación #11, la
siguiente canción es *Moog City 2*, y de ahí en más sigue en modo aleatorio
normal (cada canción nueva con una animación aleatoria distinta, guardando
también ese historial).

## Configuración

Todo lo ajustable (pines, tiempos, modo aleatorio, modo animación, modo
preview) está centralizado en [`include/config.h`](include/config.h) — no
hace falta tocar nada más para cambiar ese comportamiento.

## Agregar tus propias canciones

El DFPlayer Mini espera los archivos numerados en la raíz de la microSD:
`0001.mp3`, `0002.mp3`, `0003.mp3`, etc., sin saltos.

1. Copiá tus mp3 a la microSD renombrados en ese formato.
2. Editá [`include/tracks.h`](include/tracks.h) para que la lista de
   `TRACKS` coincida en orden y cantidad con lo que pusiste en la tarjeta
   (título, autor, y el número de archivo).
3. Recompilá y subí el firmware (`pio run -t upload`).
4. Commiteá el cambio en `tracks.h`:
   ```bash
   git add include/tracks.h
   git commit -m "Actualizar tracklist"
   git push
   ```
   (los mp3 en sí no se suben al repo, solo queda el registro de qué
   canción es cada número).

## Estructura del proyecto

```
include/
  config.h            configuración centralizada
  tracks.h            lista de canciones (título, autor, número de archivo)
  animations.h        API y modos del sistema de animaciones
  animation_bitmaps.h frames de las 9 animaciones bitmap
  boot_bitmap.h        frames de la barra de carga del arranque
  buttons.h / player.h / display.h
src/
  main.cpp            máquina de estados y lógica de botones
  animations.cpp / display.cpp / buttons.cpp / player.cpp
```

## Créditos

La música es de **C418** (*Minecraft Volume Alpha & Beta*) — este proyecto
no la incluye ni la redistribuye, es solo para uso personal. Gracias a
Adafruit por las librerías de OLED/GFX y a DFRobot por el DFPlayer Mini.

---
*SantiagortegaDev* with *Claude*
