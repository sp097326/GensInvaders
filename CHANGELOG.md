# Changelog - Gens Invaders

Registro de cambios del proyecto. Lo más reciente va arriba.
Detalle de la sesión de junio 2026 en `gensinvaders-session-log.html`.

## 2026-11-10 (noche) - Vidas, fondo de estrellas y formación (AMH + Bimo)

### Agregado
- **3 vidas** (`MAX_LIVES`). Al recibir un disparo la nave desaparece ~1.5 s
  (`RESPAWN_DELAY`), reaparece en su lugar y parpadea ~2 s sin poder ser
  golpeada (`INVULN_FRAMES`). GAME OVER solo cuando se acaban las naves.
  El HUD muestra `SHIPS`. Nuevas funciones: `placePlayer()`, `updatePlayerLife()`, `playerHit()`.
- **Fondo de estrellas** (`drawStarfield()`): el plano B se llena UNA vez al
  iniciar con tiles aleatorios de `space.bmp` y solo se mueve el scroll vertical.
  El código viejo lo hacía dentro del loop (1,280 escrituras por cuadro), por eso
  era lento y estaba comentado.

### Corregido
- **Enemigos encimados:** cada enemigo rebotaba por su cuenta en la orilla y
  chocaba con los que venían detrás. Ahora se mueven en formación (`enemyDir`):
  si cualquiera toca una orilla, todo el grupo da la vuelta junto.
- Sin control del jugador mientras espera reaparecer; el powerup se apaga al morir.

## 2026-11-10 (tarde) - Sonido, encabezado y script para correr (AMH + Bimo)

Probado en BlastEm 0.6.2: la intro y el láser se escuchan; la explosión se cortaba.

### Agregado
- `run.bat` en la raíz: `run.bat` abre `out\rom.bin` en BlastEm
  (`C:\Tools\BlastEm\blastem-win32-0.6.2\blastem.exe`); `run.bat build` compila antes.
- `res/music/intro_short.wav`: la intro recortada a 5 s con desvanecido final
  (mono, 16 bits, 22 kHz). El `Intro.wav` original de 30 s se conserva en el repo.

### Cambiado
- La explosión del jugador suena en su propio canal (`SOUND_PCM_CH2`) con prioridad 15;
  el láser sigue en `SOUND_PCM_CH3`, así ya no se pisan.
- `resources.res` usa `intro_short.wav`. El sample bajó de 430,592 a 70,144 bytes
  y la ROM de 512 KB a 256 KB.
- Encabezado de la ROM (`src/boot/rom_head.c`): nombre "GENS INVADERS",
  copyright "(C)AMH  2026.NOV" (antes "SAMPLE PROGRAM").
- `.gitignore`: ahora solo ignora `src/boot/sega.s` y los `.bak`, para que
  `rom_head.c` (que tiene el nombre del juego) quede en el repo.

## 2026-11-10 - Fin de juego y limpieza (AMH + Bimo)

Toolchain: SGDK 2.11 en `C:\tools\SGDK`, Java 21 (Temurin) para rescomp.
Build: `make -f %GDK%\makefile.gen` compila sin warnings; ROM de 524,288 bytes.

### Agregado
- Estados de juego (`GameState`: `STATE_PLAYING`, `STATE_GAME_OVER`, `STATE_WIN`).
  El loop principal solo mueve jugador, balas y enemigos mientras se juega.
- Pantalla **GAME OVER** cuando una bala enemiga le pega al jugador.
- Pantalla **YOU WIN!** cuando cae el último enemigo.
- **START** reinicia la partida (`resetGame()`): reusa los sprites ya creados y
  solo reinicia posiciones, contadores, score y powerup.
- Helpers `drawCenteredText()`, `showEndScreen()`, `clearEndScreen()` y `placeEnemies()`.
- Constantes `PLAYER_START_X` / `PLAYER_START_Y`.

### Corregido
- La bala enemiga que impacta al jugador ahora se destruye y libera su lugar
  en el pool (antes seguía viva y ocupando slot).
- Ya no se revisan colisiones contra el jugador cuando está muerto.
- `SYS_enableInts()` estaba después del `while(1)` y nunca se ejecutaba; ahora se
  llama al terminar la inicialización del VDP.
- `player.name` era `"PLAYER"` (6 letras + `\0` = 7 bytes en un `char[6]`); ahora `"PLYR"`.
- Warning `-Wmissing-field-initializers` en `powerup` (se inicializa `name` con `""`).

### Limpieza
- Se eliminó la variable global `i` (y `thex`, `they`, `val`, que solo usaba el
  bloque comentado del starfield); cada función usa su propio contador local.
- `handleCollitions()` renombrada a `handleCollisions()`.
- Typos en comentarios y comentarios viejos de depuración ("aquí truena").

### Pendiente
- (Resuelto en la sesión de la tarde: sonido confirmado, canales separados e intro recortada.)
- Sprite de explosión para el jugador y animaciones.

## 2026-06-26 - Sesión de reparación (AMH + Oz/Warp)

- Prototipo de `shootBullet()`; arreglos del toggle y de RapidFire.
- `PAL_setColor` en lugar de `PAL_setPaletteColors` (el crash "aquí truena").
- `updatePowerup()`: el powerup expira solo.
- `sega.s` actualizado al stub de SGDK actual.
- WAVs con driver `XGM` en `resources.res`.
