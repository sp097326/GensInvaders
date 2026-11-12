# Changelog - Gens Invaders

Registro de cambios del proyecto. Lo más reciente va arriba.
Detalle de la sesión de junio 2026 en `gensinvaders-session-log.html`.

## 2026-11-12 (tarde 12) - Tiempos del escudo según la dificultad (AMH + Bimo)

### Cambiado
- La duración y la recarga del escudo ya no son fijas (5 s / 30 s). Pasan a la
  tabla de dificultad (`shieldDurationSec` y `shieldRechargeSec`): FÁCIL 5 s
  activo y 10 s de recarga, NORMAL 3 s / 20 s, DIFÍCIL 2 s / 30 s.
- `powerups` convierte esos segundos a cuadros con `IS_PAL_SYSTEM` (×60 en
  NTSC, ×50 en PAL), igual que antes, para que el tiempo real no cambie de
  un sistema a otro. El HUD sigue mostrando los segundos que quedan
  (`ESC: ON n` y `ESC: ns`).
- Los macros fijos `SHIELD_DURATION_*` / `SHIELD_RECHARGE_*` salen de
  `config.h`; el tamaño del sprite no cambia.

## 2026-11-12 (tarde 11) - Power-up de escudo (AMH + Bimo)

### Agregado
- Habilidad de escudo, botón C (B ya dispara y A sigue siendo RapidFire).
  Empieza cargado al inicio de la partida. Activo 5 s (300 cuadros en NTSC,
  250 en PAL) y luego recarga 30 s (1800 / 1500). Las duraciones se eligen
  con `IS_PAL_SYSTEM` para que el tiempo real sea el mismo en ambos sistemas;
  el proyecto no manejaba PAL antes.
- Mientras está activo, un aro azul/blanco de 32×32 (4 cuadros que pulsan,
  paleta de `space.bmp`, color 0 transparente) envuelve la nave. Las balas
  enemigas que tocan la caja de 32×32 se destruyen y no hacen daño. No hay
  colisión cuerpo a cuerpo nave-enemigo en este juego, así que el escudo
  solo tiene que detener balas.
- No se puede activar durante la explosión, la pausa, la recarga ni mientras
  la nave está fuera. El temporizador se congela en pausa (el update solo
  corre en PLAYING). Al perder una vida se conserva el estado de recarga
  (o LISTO si no se había usado); no se puede morir por una bala con el
  escudo puesto, así que en la práctica la recarga sigue su curso durante
  la explosión y el respawn. Una partida nueva lo deja otra vez cargado.
- Indicador a la derecha de la línea 2 del HUD, sin tocar SCORE ni DIFF:
  `ESC: LISTO`, `ESC: ON n` mientras dura y `ESC: ns` en la recarga.
- SFX cortos (~0,16 s) al activar y al terminar la recarga, en el canal PCM 1
  para no cortar el láser ni la explosión.
- Un solo sprite de hardware (32×32 = 1 sprite VDP, 16 teselas). El recuento
  real queda en ~27 de 80; el “presupuesto 70” anterior contaba cada nave
  de 16×16 como 4 sprites.

## 2026-11-12 (tarde 10) - Explosión del jugador (AMH + Bimo)

### Agregado
- Nueva hoja de sprites `res/tiles/imgexplosion.bmp` (96×16, 6 cuadros de 16×16,
  8 bits indexados, la misma paleta de `space.bmp`, el color 0 es transparente):
  destello blanco, bola de fuego que crece, restos y desvanecimiento en naranja,
  blanco y rojo oscuro.
- Recurso `SPRITE explosion` en `resources.res` (2×2 teselas, FAST, velocidad 0:
  los cuadros se avanzan a mano para que la animación no se ciclene).
- Al ser alcanzado, la nave se oculta y se reproduce la explosión en su posición
  durante 48 cuadros (~0,8 s, 8 ticks por cuadro). En ese lapso no hay mando ni
  disparos enemigos (las naves y las balas ya en vuelo siguen moviéndose). Un
  temblor breve del campo de estrellas (scroll horizontal de BG_B) y un destello
  de 4 cuadros del naranja de PAL1 a blanco.
- Si quedan vidas, el respawn espera el resto de `RESPAWN_DELAY` (42 cuadros)
  para que el tiempo total fuera siga siendo ~90 cuadros, y luego la
  invulnerabilidad parpadeante como antes. Si no quedan vidas, GAME OVER se
  muestra al terminar la explosión (ya no es inmediato).
- La lógica vive en el módulo `player` (`player_isExploding`; `player_update`
  ahora puede devolver 1). El presupuesto de sprites de hardware se mantiene
  en 70: la explosión de 16×16 (4 celdas) sustituye a la nave oculta.

## 2026-11-12 (tarde 9) - Disparos enemigos más repartidos (AMH + Bimo)

### Cambiado
- Las naves que disparan ya no suelen ser siempre las mismas. Había tres
  causas: SGDK solo siembra el generador en el primer botón de la sesión
  (a menudo un toque del menú justo al arrancar, con un tick muy parecido);
  `random() % n` se apoyaba en los bits bajos del xorshift de 16 bits y
  podía agrupar columnas; y no se evitaba repetir las columnas de la
  andanada anterior.
- Al pulsar START para empezar la partida se vuelve a sembrar el generador
  con un contador de cuadros de la pantalla de título, `vtimer` y
  `getTick()`. En cada andanada se eligen columnas distintas al azar entre
  las que aún tienen un enemigo vivo (sigue disparando el más bajo de la
  columna) y, si hay alternativas, no se repiten las de la andanada previa.
  La tabla de dificultad no cambia; aplica a EASY, NORMAL y HARD.

## 2026-11-12 (tarde 8) - Normal dispara siempre en cada andanada (AMH + Bimo)

### Cambiado
- En NORMAL, `fireChance` pasa de 8 a 10: cada andanada (cada 120 cuadros,
  ~2 s) dispara de verdad, con 2 balas y tope 2, igual que antes. El 20% de
  fallos hacía que a veces pasaran 4 o 6 segundos sin disparos y se sintiera
  que casi no disparaban. No fue una regresión del refactor: la lógica y la
  tabla eran las mismas que en V0.1.25. EASY y HARD no se tocan.

## 2026-11-12 (tarde 7) - Refactor en módulos (AMH + Bimo)

### Cambiado
- `src/main.c` (1226 líneas en V0.1.25) se partió en módulos **sin cambiar
  el comportamiento** del juego. El `main` queda en inicialización, el
  campo de estrellas y el despacho del bucle; cada módulo tiene init/update/draw
  cuando aplica, y sus datos viven como `static` en su `.c`.
- Las cabeceras públicas están en `inc/`. El `makefile.gen` de SGDK ya toma
  todos los `src/*.c` (salvo `boot/rom_head.c`) e incluye `-Iinc -Isrc`.
- `GAME_VERSION` se incluye desde `game.h` (`#if __has_include("version.h")`,
  si no existe usa `V0.1.DEV`). `src/version.h` sigue siendo generado y
  git-ignorado. Tras un cambio de cabeceras hay que borrar `out/src/*.o`
  antes de recompilar, porque ahora varios módulos pueden incluir la versión.

### Agregado
- `inc/config.h`: constantes compartidas (formación, bordes, balas, vidas, etc.).
- `inc/game.h`: `GameEntity`, `GameState`, `killGameEntity` / `reviveGameEntity`.
- `difficulty.c/.h`: tabla de dificultad, nombres y getters (`difficulty_config()`, etc.).
- `bullets.c/.h`: pools de balas del jugador y de los enemigos.
- `hud.c/.h`: puntuación, texto centrado y limpieza de overlays.
- `enemies.c/.h`: formación, movimiento (incluido `rowsOpposite`), disparos y paletas.
- `player.c/.h`: nave, vidas, respawn e invulnerabilidad.
- `collision.c/.h`: choques bala-enemigo y bala-jugador.
- `title.c/.h`: logo de bloques, menú y pulso de color.
- `powerups.c/.h`: RapidFire (el pickup en pantalla sigue como reserva).
- `game_state.c/.h`: estados, mando, pausa, GAME OVER / YOU WIN y `reset`.

### Sin mover
- El campo de estrellas (`drawStarfield` y el scroll vertical de `offset`)
  se queda en `main.c`: no había un módulo de fondo en el plan.

## 2026-11-12 (tarde 6) - Filas en sentido contrario también en Normal (AMH + Bimo)

### Cambiado
- `rowsOpposite` queda en 1 también en NORMAL (el resto de valores de NORMAL
  no cambian: descenso 8, velocidad 1, intervalo 120, etc.). EASY sigue en 0
  con las dos filas en el mismo sentido. HARD no se toca.

## 2026-11-12 (tarde 5) - Filas en sentido contrario en Difícil (AMH + Bimo)

### Agregado
- Flag `rowsOpposite` en `DifficultyConfig` (EASY=0, NORMAL=0, HARD=1).
  Ponerlo en 1 también en NORMAL activa el mismo comportamiento ahí.
- En HARD las filas se mueven en sentidos opuestos (la de arriba arranca a
  la derecha, la de abajo a la izquierda), rebotan cada una por su cuenta
  (solo con sus enemigos vivos) y solo baja la fila que rebota, con el
  `stepDown` y `enemySpeed` de la tabla. La fila inferior empieza recorrida
  a la derecha para que tenga espacio de ir a la izquierda.
- Para no juntar las filas en la misma línea de barrido, una fila no baja
  si quedaría a menos de 24 px (`ENEMY_ROW_SPACING`) de la siguiente fila
  viva debajo: en ese caso solo invierte el sentido. Si la fila de abajo
  ya no tiene naves, la de arriba puede bajar sin ese tope.
- EASY y NORMAL siguen con la formación unida de siempre.

## 2026-11-12 (tarde 4) - Segunda fila de enemigos (AMH + Bimo)

### Agregado
- **Segunda fila** de 7 enemigos debajo de la primera (14 en total). Se mueven,
  rebotan y bajan como una sola formación. GAME OVER usa el enemigo vivo más
  bajo; YOU WIN al limpiar las dos filas. Los disparos eligen, en cada columna,
  la nave más baja que siga viva.
- Infraestructura por fila (`EnemyRowType`): cada fila tiene su propio
  `SpriteDefinition`, paleta y puntaje. Por ahora las dos reutilizan `ship`;
  la de abajo usa PAL3 con un tinte verde/amarillo para distinguirla. Arriba
  vale 20 puntos, abajo 10.
- Un sprite distinto por fila no debería afectar el desempeño si se mantiene
  en 16x16: el costo lo marca el tamaño y la cantidad, no el dibujo en sí.

### Cambiado
- `MAX_ENEMIES` pasa a 14 (`ENEMY_ROWS * ENEMIES_PER_ROW`). El espaciado
  vertical es 24 px para que las filas no compartan línea de barrido.

## 2026-11-12 (tarde 3) - Pantalla de título (AMH + Bimo)

### Agregado
- **Pantalla de título** al arrancar (y al volver de GAME OVER / YOU WIN).
  El logo **GENSINVADERS** se dibuja grande en dos renglones (GENS / INVADERS)
  con letras de bloques sólidos 8x8 en el plano A, más una sombra. Se usaron
  tiles del fondo y no sprites para no rebasar el límite de 20 sprites por
  línea de barrido (INVADERS en una sola fila no cabría). El color del logo
  pulsa en tonos dorados. La versión `V0.1.xx` aparece bajo el título.
- Menú de 2 opciones: **START GAME (INICIAR)** y **DIFFICULTY: EASY/NORMAL/HARD**.
  ARRIBA/ABAJO mueve el cursor; IZQUIERDA/DERECHA (y START o A sobre
  Dificultad) cambian el nivel; START en INICIAR comienza la partida con
  la dificultad elegida (se conserva la última al volver al título).
  Los sprites del juego se ocultan en esta pantalla; el fondo de estrellas
  sigue en el plano B.

### Cambiado
- El menú suelto "SELECT DIFFICULTY" se reemplazó por esta pantalla
  (`STATE_TITLE`).

## 2026-11-12 (tarde 2) - Ajuste de cadencia y velocidad por dificultad (AMH + Bimo)

### Corregido
- HARD se sentía igual o más lento que NORMAL, pero `currentDiff` sí se
  aplicaba bien (el menú, el HUD y `resetGame()` no lo pisaban; la tabla
  estaba bien indexada). La causa era de diseño: HARD usaba el mismo
  `SHOT_INTERVAL` de 120 cuadros y la misma velocidad horizontal (1 px),
  así que las andanadas llegaban al mismo ritmo. El tope de 4 balas casi
  nunca se notaba: a velocidad 5 una bala sale de pantalla en ~38 cuadros,
  mucho antes de la siguiente andanada. El descenso de 16 px solo se ve
  en cada rebote, con el mismo intervalo entre rebotes que en NORMAL.
  No había indicios de caída de cuadros (pocos sprites, bucles chicos y
  el loop espera al vblank).
- En EASY casi no disparaban: cadencia de 120 cuadros y `fireChance` 6/10,
  o sea un disparo cada ~3 s en promedio.

### Cambiado
- La tabla ahora también controla `shotInterval`, `enemySpeed` y
  `bulletSpeed`.
  - **EASY:** intervalo 90, `fireChance` 10 (disparo regular, 1 bala);
    sin descenso; velocidad igual que NORMAL.
  - **NORMAL:** sin cambios de sensación (intervalo 120, 2 por andanada,
    tope 2, descenso 8, velocidad 1, bala 5).
  - **HARD:** intervalo 60, 2 por andanada, tope 4, descenso 16,
    velocidad horizontal 2 y balas a 6 (un poco más rápidas).
- `enemiesTryShoot()` usa `cfg->shotInterval`. `resetGame()` inicia
  `enemyDir` con `enemySpeed`. `shootEnemyBullet()` usa `bulletSpeed`.

## 2026-11-12 (tarde) - Dificultad y disparos simultáneos (AMH + Bimo)

### Agregado
- **Tabla de dificultad** (`DifficultyConfig difficultyTable`): los valores se
  ajustan en datos, no en el código de movimiento o disparo.
  - **EASY:** la formación no baja; como máximo 1 bala enemiga en pantalla.
  - **NORMAL:** baja 8 píxeles; hasta 2 balas enemigas; 2 enemigos distintos
    disparan a la vez.
  - **HARD:** baja 16 píxeles (el doble); 2 enemigos distintos disparan juntos
    y pueden acumularse hasta 4 balas en pantalla.
  También incluye `fireChance` (0-10) para afinar más adelante.
- **Pool propio de balas enemigas** (`enemyBullets[MAX_ENEMY_BULLETS]`, 4
  sprites). Cada una colisiona por su lado con el jugador; se liberan al
  impactar, al salir de pantalla y en `resetGame()`. Las balas del jugador
  siguen en su propio pool. Vidas, reaparición e invulnerabilidad no cambian.
- **Menú de dificultad:** después de la intro aparece "SELECT DIFFICULTY"
  (EASY / NORMAL / HARD). Se elige con ARRIBA/ABAJO y se confirma con START
  (por defecto NORMAL). Tras GAME OVER o YOU WIN, START vuelve a ese menú
  con la última elección marcada. La dificultad actual se ve en el HUD
  (`DIFF:`) y en la pantalla de pausa.

### Cambiado
- `positionEnemies()` usa `stepDown` de la tabla (0 en EASY).
- El disparo enemigo pasó a `enemiesTryShoot()`: una andanada de enemigos
  distintos, limitada por `shotsPerVolley` y `maxEnemyBullets`.
- START en las pantallas finales ya no reinicia la partida al instante;
  primero abre el menú de dificultad.

### Pendiente
- Más adelante, una pantalla de título con un menú de 2 opciones:
  START GAME y DIFFICULTY (EASY / NORMAL / HARD). Por ahora se mantiene
  el menú simple de selección de dificultad.

## 2026-11-12 - La formación baja en cada vuelta (AMH + Bimo)

### Agregado
- **Descenso de la formación:** cuando un enemigo vivo toca una orilla, todo
  el grupo da la vuelta y baja 8 píxeles (`ENEMY_STEP_DOWN`). Si cualquier
  enemigo vivo llega a la fila del jugador (la base del sprite alcanza
  `PLAYER_START_Y`), es GAME OVER con la maquinaria de estados existente.
- Al reiniciar con START, `placeEnemies()` vuelve a poner la formación en
  Y = 32 y `resetGame()` restaura `enemyDir`.

### Cambiado
- `positionEnemies()` ahora baja la formación en cada rebote, además de
  invertir la dirección.

## 2026-11-10 (noche 3) - Corrige versión DEV en la pausa (AMH + Bimo)

### Corregido
- La pausa mostraba `V0.1.DEV` aunque `src/version.h` existía: make no sabe que
  `main.c` depende de `version.h` y reutilizaba un `main.o` viejo. Ahora
  `run.bat build` borra `out\src\main.o` antes de compilar.

## 2026-11-10 (noche 2) - Pausa con versión (AMH + Bimo)

### Agregado
- **Pausa:** START durante el juego congela la acción y muestra "PAUSED",
  "GENS INVADERS V0.1.xx" y "PRESS START"; START de nuevo continúa.
  Nuevo estado `STATE_PAUSED` y funciones `pauseGame()` / `resumeGame()`.
- **Versión del juego:** `V0.1.xx`, donde xx es el número de commits del repo
  (`git rev-list --count HEAD`). `run.bat build` genera `src/version.h` antes
  de compilar; ese archivo está en `.gitignore`. Si no existe, el juego muestra `V0.1.DEV`.
  Para que la versión coincida con lo que está en GitHub, compilar después de hacer commit.

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
