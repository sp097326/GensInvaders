# Gens Invaders
**Español** | [English](README.en.md)

Segundo juego para la consola Sega Genesis de 16 bits. Esta vez con la intención de aprender
a trabajar mejor los sprites antes de pasar a algo más complejo. Igual que GensPong, este código
deriva directamente del tutorial de la página de Ohsat:
https://www.ohsat.com/tutorial/megalaga/megalaga-1/

Ahí se proporciona todo lo necesario para que el código funcione, con una solución muy sencilla,
paso a paso e intuitiva.

De nuevo voy a modificar y derivar código del tutorial, pero el crédito de la base es de
OHSAT. Esta vez las notas y la información van en este archivo README.MD en lugar de la
parte inicial del archivo main.c.

En resumen, Gens Invaders es un clon sencillo de la fórmula de Space Invaders con el estilo
gráfico de 16 bits de la Genesis.

## Cómo compilar
Requisitos (Windows):
* SGDK 2.11 instalado en `C:\tools\SGDK`, con la variable de entorno `GDK` apuntando ahí.
* Java 21 (Eclipse Temurin) en el PATH; lo necesitan `rescomp.jar` y `sizebnd.jar` de SGDK.

Desde la carpeta del proyecto:

    %GDK%\bin\make -f %GDK%\makefile.gen

La ROM queda en `out/rom.bin` (256 KB).

Para jugar, ejecuta `run.bat` desde la raíz del proyecto (o `run.bat build` para compilar antes).
`run.bat build` también escribe `src/version.h` con la versión `V0.4.xx`, donde xx es el
total de commits de git (`git rev-list --count HEAD`); el prefijo 0.4 marca esta
etapa del juego y el último número no se reinicia. Compila después de hacer commit para que la
versión coincida con GitHub. Si falta `version.h`, el juego muestra `V0.4.DEV`.
Abre la ROM en BlastEm 0.6.2, instalado en `C:\Tools\BlastEm\blastem-win32-0.6.2\blastem.exe`.
Teclas predeterminadas de BlastEm: flechas = D-pad, S = B (disparar), D = C (escudo), Enter = START.
Nota: la línea `Error 127 (ignored)` durante el enlazado es una peculiaridad conocida de SGDK/LTO en Windows y
no afecta la ROM.

### Compilar en macOS
Requisitos (macOS, Apple Silicon):
* Toolchain en `~/Desktop/retrodev/mars-sgdk211/m68k-elf`: GCC 14.2 m68k-elf nativo arm64 más
  SGDK 2.11 (headers, `libmd.a`, `makefile.gen`, `rescomp.jar`, `sjasm`...), compilado desde marsdev
  sin `sudo`. Se puede usar otra ubicación con `GENS_GDK=/path/to/m68k-elf`.
* Java (`openjdk` de Homebrew) en el PATH para `rescomp.jar` y `sizebnd.jar`.
* BlastEm 1.0 (`brew install blastem`); si no está, se usa `Genesis Plus.app`.

Desde la carpeta del proyecto:

    ./build.sh build        # writes src/version.h (same V0.4.xx as run.bat) and compiles
    ./build.sh run          # opens out/rom.bin in BlastEm (or Genesis Plus)
    ./build.sh clean        # removes out/
    ./build.sh build run    # commands can be chained; no argument = run

La ROM compila con GCC 14 con 0 warnings y sin cambios en el código del juego. No uses la
instalación anterior de `/opt/toolchains/mars` para este juego: trae SGDK 2.00 y es un binario Intel que
no corre en Silicón Apple.

## Estructura del código
Los headers públicos viven en inc/; la implementación, en src/. El makefile.gen de SGDK ya compila todos los src/*.c (excepto boot/rom_head.c) y agrega -Iinc -Isrc. Los datos de cada módulo son static dentro de su .c; solo se exportan los tipos compartidos y las funciones de acceso. src/main.c contiene la inicialización, el fondo de estrellas y el despacho del ciclo principal.

* inc/config.h — constantes de juego compartidas (tamaño de la formación, bordes, límites de balas, vidas, ids de SFX). Los valores ajustables que cambian según la dificultad se quedan en la tabla de dificultad.
* inc/game.h — GameEntity, GameState, killGameEntity / reviveGameEntity y GAME_VERSION (incluye src/version.h, ignorado por git, cuando existe; si no, V0.3.DEV).
* difficulty.c/.h — tabla DifficultyConfig, nombres, dificultad actual, difficulty_config() / difficulty_cycle().
* bullets.c/.h — pools de balas del jugador y de los enemigos, disparo, movimiento y eliminación.
* hud.c/.h — líneas de puntaje / naves / dificultad, barra del escudo (ESC + 8 tiles), texto centrado, limpieza de overlays.
* enemies.c/.h — formación de dos filas, movimiento rowsOpposite, ráfagas, paletas, bandera de descenso para ganar/perder.
* player.c/.h — sprite de la nave, límites de movimiento, vidas, reaparición y parpadeo de invulnerabilidad.
* collision.c/.h — revisiones AABB; balas del jugador contra enemigos, balas enemigas contra el jugador y contacto fatal de cuerpo enemigo-jugador (o una nave que llega al fondo visible).
* title.c/.h — logo de bloques, menú, pulso dorado; GAME_VERSION bajo el logo.
* powerups.c/.h — cápsula RapidFire y la habilidad de escudo (C; la duración y la recarga vienen de la tabla de dificultad).
* boss.c/.h — nave nodriza en las oleadas 10, 20, 30... (350 HP, se duplica en cada regreso hasta 8000; daño en el tercio central, a toda la altura del núcleo; recorrido Lissajous en la mitad superior; ataques: hueso de 48x16 y abanico de 3 balas; desde la V0.3.73 la tortuga hechicera es el jefe de las oleadas 10, 30, 50... y La Cookie el de las 20, 40, 60...; la tortuga dispara una estrella de 16x16).
* game_state.c/.h — título / juego / pausa / GAME OVER / YOU WIN, manejador del control, reinicio y texto de las pantallas finales.
* src/main.c — inicialización del toolchain, fondo de estrellas (drawStarfield + scroll vertical de offset) y el ciclo que llama a game_state_update(). El fondo de estrellas se quedó en main.c porque no había un módulo de fondo en el plan.

## Controles
* Pantalla de título: UP/DOWN mueven el cursor entre START GAME (INICIAR) y DIFFICULTY.
        LEFT/RIGHT (o START / A sobre DIFFICULTY) alternan EASY / NORMAL / HARD.
        START sobre START GAME inicia la partida con la dificultad elegida (NORMAL por defecto).
* D-pad izquierda/derecha: mueve la nave (durante el juego).
* B: disparar (mantén ~1 s para el disparo cargado perforante; un toque corto es el disparo normal de 8x8).
* RapidFire: recoge una cápsula "R" que cae de los enemigos destruidos (10% de probabilidad, garantizada tras 10 derribos sin cápsula; cada jefe vencido suelta una siempre; 10 s en EASY, 8 s en NORMAL/HARD). La nave se pone roja y una barra RF en la esquina inferior derecha muestra el tiempo restante. Recoger otra reinicia el temporizador. El botón A ya no lo activa.
* Cápsulas (desde V0.4.79): cuando un enemigo suelta cápsula, el tipo sale al azar, 1/3 cada uno: "R" naranja = RapidFire, "E" azul = recarga completa del escudo (ESC) y "I" verde = recarga completa del impulso (IMP). Si la atrapas mientras el escudo o el impulso están en uso, quedan listos en cuanto terminan, sin recarga. Si la barra ya está llena, la "E" o la "I" dan puntos extra (5 naves: 50/100/150 en EASY/NORMAL/HARD, `CAPSULE_FULL_BONUS`). Los jefes siempre sueltan "R". Pesos en `inc/config.h` (`CAPSULE_WEIGHT_*`).
* Jefe: las oleadas 10, 20, 30... reemplazan las dos filas con una nave nodriza. 350 HP en la primera visita, el doble en cada regreso hasta 8000 (oleada 60+). Después de cada jefe la dificultad de la formación baja y vuelve a subir (ciclos de 10 oleadas: la 11 se juega como la 4). En las oleadas 11 a 19 hay una tercera fila intercalada de 4 naves. El tercio central brillante (a toda la altura del cuerpo) recibe daño (30/20/10 en EASY/NORMAL/HARD; el disparo cargado hace 3x); un golpe al núcleo tiñe la nave de rojo y suena un tic, un golpe en el ala solo saca chispas. Ataques: un hueso de 48x16 que cae recto desde la panza (zona de golpe de 40x8, para que rozarlo no te destruya; cae más lento que las balas, 2 px/frame menos; máximo 2 en pantalla; el escudo NO lo detiene, hay que esquivarlo) y un abanico de 3 balas diagonales dirigido hacia ti (este sí lo detiene el escudo). Recorre la mitad superior de la pantalla (Lissajous, la velocidad escala con la dificultad) y nunca entra a la mitad del jugador. Una barra JEFE en la fila 2 muestra los HP restantes. El sprite es provisional. Desde la V0.3.73 los jefes se alternan: la tortuga hechicera en las oleadas 10, 30, 50... y La Cookie en las 20, 40, 60... (la vida sigue el número de aparición: tortuga 350 en la 10, Cookie 700 en la 20). La tortuga (64x64): en lugar del hueso lanza una estrella de 16x16 desde la varita (zona de golpe de 10x10, mismas reglas que el hueso: cae 2 px/frame más lento que las balas, máximo 2, el escudo no la detiene), el abanico sale de su cabeza (más lento que el de La Cookie: 5 px/frame) y solo el centro del caparazón (24x36) recibe daño. Patrulla de izquierda a derecha a velocidad constante (2/3/4 px/frame) a una altura fija y, cada 2.4-4.8 s al azar (NORMAL), avisa (tiembla y parpadea en amarillo ~0.33 s) y se lanza en picada recto hacia abajo a 6/7/8 px/frame: si te toca pierdes una vida (el escudo no la detiene), sale por abajo, reaparece por arriba en una columna al azar y vuelve a su altura. No dispara mientras se lanza. 
* Easter egg (pantalla de título): UP, UP, DOWN, DOWN, LEFT, RIGHT, B, A desbloquea OLEADA INICIAL (1-99). Ingrésalo con el cursor en INICIAR para que LEFT/RIGHT no cambien la dificultad. Empezar en la oleada 2+ es práctica y no se guarda en el TOP 5. El desbloqueo dura hasta reiniciar (no se guarda en SRAM). En BlastEm el botón B es S y A es A.
* A + IZQUIERDA / DERECHA: impulso (dash) para esquivar: la nave recorre unas 3 naves (48 px) en ~0.13 s hacia ese lado. Se recarga en el mismo tiempo que el escudo (10 / 20 / 30 s en EASY / NORMAL / HARD); la barra IMP abajo al centro muestra la recarga (verde = lista). No da invulnerabilidad. A solo no hace nada durante el juego; en el menú de título y en la captura de iniciales A sigue funcionando como antes.
* C: activa el escudo si está cargado. La duración y la recarga dependen de la
        dificultad: EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
        Empieza listo al inicio de la partida. No se puede usar mientras explotas, estás muerto,
        en pausa o recargando. En BlastEm la tecla predeterminada para C es D.
        El HUD muestra ESC y una barra a la derecha de la línea 2: azul que se vacía mientras
        está activo, rojo oscuro que se llena mientras recarga, verde (con un breve parpadeo azul) cuando está listo.
* START: durante el juego, pausa / reanuda (la pantalla de pausa muestra la versión del juego y la
        dificultad actual). Después de GAME OVER o YOU WIN, regresa a la pantalla de título
        con la última dificultad preseleccionada.
* Tienes 3 naves; tras un impacto de bala la nave reaparece y parpadea ~2 s (las balas no la dañan).
* Reglas: un impacto de bala cuesta una vida. Una nave enemiga que toca al jugador siempre es
        GAME OVER después de la explosión, aunque queden vidas o escudo (el escudo
        sigue bloqueando balas, salvo el hueso del jefe). Si los invasores pasan sin tocarlo y llegan al
        fondo de la pantalla, también es GAME OVER, para que la partida no se estanque.

## Registro de cambios
Los cambios detallados están en `CHANGELOG.md`. La sesión de reparación de junio de 2026 está documentada en
`gensinvaders-session-log.html`.

SECCIÓN DE NOTAS
-- OK, de verdad abandoné esto por un buen rato. Intentaré tener algo listo antes del
    fin de mes, no puedo prometer mucho; creo que abarqué más de lo que podía apretar...
-- DESPUÉS DE UN BUEN RATO por fin encontré tiempo para hacer algo. Pequeña sorpresa, eso sí...
    desde la versión 1.80 de SGDK varias cosas no funcionan o de plano rompen la
    compilación. Voy a ver si puedo resolverlo, pero mi poco conocimiento de C, sumado
    a mi poco conocimiento de la Genesis (CPU 68000, VDP y sonido), puede convertir esto en un
    ejercicio de futilidad. Al menos arreglé un problema...
-- Avanzando con más código; por alguna razón va muy lento. No sé si es mi código
    o algo de la configuración de la computadora.
-- Una vez más mi tiempo no es proporcional a la velocidad del desarrollo de SGDK y, en fin, aquí
    estamos en la versión 2.0 de SGDK. El código parece funcionar tal como está; todavía no he resuelto
    el problema del sonido ni los problemas de animación de sprites.
-- Nov 2026: de vuelta con ayuda de Bimo. Actualizado a SGDK 2.11, el juego vuelve a compilar
    sin problemas y ahora tiene un final real (GAME OVER / YOU WIN) con reinicio.

PROGRAMADOR: 
*   AMH - MANZANAS
FECHA_INICIAL: 
*   07/16/2021
FECHA_ÚLTIMA_MODIFICACIÓN:
*   11/18/2026

HISTORIAL:
*       16/07/21 - Se creó la plantilla y un código tipo helloworld.
*   	21/06/23 - Se agregó el fondo espacial y el sprite inicial del jugador.
*       22/06/23 - Se agregaron enemigos, sus sprites y su animación inicial; se implementó un manejador básico
                del control y el posicionamiento del jugador; se implementaron las balas y su animación.
*       23/06/23 - Se agregaron rutinas de detección de colisiones y el resultado de cada colisión.
*       28/06/23 - Se agregó el disparo enemigo y la colisión tanto para enemigos como para el jugador.
*       30/06/23 - Se agregaron sonidos -- por cierto, no funcionan *; también se cambió de emulador.
*       06/07/23 - Se agregó código para activar el power-up; sigue en proceso, revisando la funcionalidad y
                la traducción de código del VDP a PAL.
*       21/08/23 - Se agregó código para una canción de intro; sigo depurando por qué no suenan los sfx.
*       22/08/23 - Se cambió la información del encabezado de la ROM para mostrar el nombre del juego.
*       26/06/26 - Sesión de reparación: se corrigió el crash del toggle de power-up/RapidFire/paleta, el power-up
                expira solo, se actualizó el stub de arranque sega.s y el driver WAV cambió a XGM. Vuelve a compilar.
*       10/11/26 - Toolchain SGDK 2.11 + Java 21. Se agregaron estados de juego (GAME OVER / YOU WIN) y reinicio
                con START; se corrigió la bala enemiga que no se liberaba al golpear al jugador, las interrupciones que nunca
                se reactivaban y el desbordamiento del nombre del jugador; se quitó la global i y se renombró handleCollisions. Se agregó CHANGELOG.md.
                Después: intro recortada a 5 s, explosión en su propio canal PCM, encabezado de la ROM renombrado a
                GENS INVADERS, run.bat para lanzar BlastEm. Luego: 3 vidas con reaparición e invulnerabilidad
                con parpadeo, fondo de estrellas dibujado una sola vez, los enemigos se mueven en formación.
                Pantalla de pausa con START que muestra la versión del juego (V0.1.xx = número de commits de git).
*       12/11/26 - La formación baja 8 px en cada rebote contra un borde; GAME OVER si llega
                a la fila del jugador. START sigue regresando la formación a Y = 32.
*       12/11/26 - Menú de dificultad (EASY / NORMAL / HARD) y un pool exclusivo de balas enemigas
                para que dos enemigos distintos puedan disparar a la vez. El ajuste se hace por datos
                (difficultyTable). Después de GAME OVER / YOU WIN, START regresa al menú.
*       12/11/26 - Reajuste de dificultades: la tabla ahora también tiene shotInterval, enemySpeed
                y bulletSpeed. EASY dispara con regularidad cada 90 frames; NORMAL no cambia;
                HARD se mueve y dispara el doble de rápido (paso 16, intervalo 60, velocidad 2).
*       12/11/26 - Pantalla de título con un logo grande de bloques GENS / INVADERS (tiles sólidos en
                el plano A, con sombra y un pulso de color dorado) y un menú de 2 opciones:
                START GAME (INICIAR) y DIFFICULTY. La versión se muestra bajo el logo.
*       12/11/26 - Segunda fila de 7 enemigos (14 en total), una sola formación; la fila inferior usa una
                paleta teñida y vale 10 puntos, la superior 20. El disparo prefiere la
                nave viva más baja de cada columna.
*       12/11/26 - En HARD, las filas se mueven en direcciones opuestas y rebotan de forma independiente
                (bandera rowsOpposite; ponla en 1 en NORMAL para activarlo ahí también). Una fila
                no baja si quedaría a menos de 24 px de la fila de abajo.
*       12/11/26 - Movimiento de filas opuestas activado también en NORMAL (rowsOpposite=1);
                EASY sigue moviéndose como una sola formación.
*       12/11/26 - Refactorización de src/main.c en módulos (headers config/game,
                difficulty, bullets, hud, enemies, player, collision, title,
                powerups, game_state). Sin cambios de jugabilidad. main.c queda con la inicialización
                y el despacho; el fondo de estrellas se queda ahí.
*       12/11/26 - fireChance de NORMAL en 10 para que cada ráfaga de 120 frames
                realmente dispare (2 disparos, límite 2). EASY y HARD sin cambios.
*       12/11/26 - Las ráfagas enemigas eligen columnas distintas al azar y evitan
                repetir las columnas de la ráfaga anterior cuando quedan otras.
                El RNG se vuelve a sembrar cuando START inicia una partida (conteo de
                frames del título + vtimer + getTick()). Tabla de dificultad sin cambios.
*       12/11/26 - Explosión del jugador al recibir un impacto: hoja de 6 frames de 16x16
                (destello, bola de fuego, escombros, desvanecido) en la posición de la nave
                durante 48 frames (~0.8 s). El control y el disparo enemigo se congelan
                mientras dura; luego reaparición o GAME OVER como antes. Breve
                sacudida del fondo de estrellas y destello de PAL1.
*       12/11/26 - Habilidad de escudo en el botón C (B dispara, A sigue siendo
                RapidFire). Burbuja azul/blanca de 5 s alrededor de la nave
                (bloquea balas enemigas), luego 30 s de recarga. Empieza
                cargado. El HUD muestra ESC: LISTO / ON n / ns. El estado de recarga
                se conserva al morir; una partida nueva empieza lista.
*       12/11/26 - La duración y la recarga del escudo ahora siguen la tabla
                de dificultad: EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
                Los segundos se convierten a frames con IS_PAL_SYSTEM
                (x60 NTSC, x50 PAL). El HUD sigue mostrando los segundos restantes.
*       12/11/26 - El prefijo de versión sube a 0.2. El último número sigue siendo
                el total de commits de git (esta versión es V0.2.42).
                La etiqueta de respaldo es V0.2.DEV. Tag v0.2.42.
*       12/11/26 - El HUD del escudo ahora es una etiqueta corta ESC más una barra de 8 tiles
                (64 pasos) a la derecha de la línea 2: azul que se vacía mientras
                está activo, rojo oscuro que se llena mientras recarga, verde cuando
                está listo (breve parpadeo azul al terminar la recarga).
*       12/11/26 - El GAME OVER por invasión ahora es una colisión de cuerpo con el
                jugador (explosión y luego GAME OVER, aunque queden vidas).
                Ya no basta con llegar a la fila del jugador sin tocarlo.
                Si una nave se cuela y llega al fondo visible,
                se aplica la misma explosión fatal.
*       12/11/26 - Las naves enemigas muestran una explosión propia de 6 frames de 16x16
                 (destello blanco/amarillo compacto, chispas verdes que salen volando)
                 durante 24 frames (~0.4 s) al ser destruidas. Se dibuja con PAL3
                 para distinguirla de la bola de fuego naranja/roja
                 del jugador. Un pool de 6 explosiones simultáneas; la nave
                 desaparece de inmediato y la explosión ocupa su lugar.
                 Si una choca con el jugador, ambos explotan antes del GAME OVER.
*       13/11/26 - Oleadas infinitas: limpiar ambas filas ya no termina la
                 partida. Aparece un breve letrero OLEADA N (~1.5 s) y dos
                 filas nuevas surgen arriba. Se conservan el puntaje, las vidas y el estado
                 del escudo. Cada oleada escala el movimiento y la cadencia de disparo
                 desde los valores base de la dificultad, con topes. El prefijo
                 de versión sube a 0.3; el último número sigue siendo el
                 total de commits (esta versión es V0.3.46). La de respaldo
                 es V0.3.DEV. Tag v0.3.46.
*       13/11/26 - Tabla de récords TOP 5 guardada en la SRAM del cartucho.
                 Después de GAME OVER, un puntaje que califica (supera el 5.º lugar)
                 pide 3 iniciales estilo arcade y luego se muestra la tabla
                 bajo el texto de GAME OVER (lugar, iniciales, puntaje, oleada,
                 letra de dificultad F/N/D). Una sola tabla compartida. También
                 accesible desde el menú de título (PUNTAJES). Tag v0.3.47.
*       13/11/26 - Vida extra cada 1000 puntos (1000, 2000, 3000...),
                 tope de 9 naves para que el HUD siga en un dígito. Aparece un
                 "+1 NAVE" parpadeante por ~1 s y suena una breve melodía 1UP
                 de dos notas en el canal PCM 4. Mismo umbral en todas las
                 dificultades; se reinicia en cada partida. Tag v0.3.48.
*       13/11/26 - Los puntos por nave destruida ahora siguen la tabla de
                 dificultad: EASY 10, NORMAL 20, HARD 30 (ambas filas). Una oleada
                 completa vale 140 / 280 / 420. Las vidas extra siguen cada
                 1000 puntos. Tag v0.3.49.
*       13/11/26 - Las vidas ahora son hasta 9 naves en miniatura a la derecha del
                 HUD (tiles del plano, no sprites). El letrero "+1 NAVE"
                 se queda fijo y legible ~1 s; el ícono nuevo parpadea.
                 Tag v0.3.50.
*       13/11/26 - Los íconos de vida se movieron a la esquina inferior izquierda (fila 27,
                 justo debajo del escudo). "+1 NAVE" va junto a ellos;
                 la esquina superior derecha del HUD vuelve a quedar libre. Tag v0.3.51.
*       13/11/26 - Breve warp entre oleadas: estrellas más rápidas, flama del motor,
                 la nave sube un poco y luego las dos filas siguientes entran deslizándose
                 desde arriba bajo el letrero OLEADA N (~2.5 s).
                 Tag v0.3.52.
*       13/11/26 - El zumbido del escudo dura todo el tiempo que está activo
                 (muestra de 2 s en CH1, se vuelve a disparar en EASY/NORMAL). El
                 blip anterior de 0.16 s era demasiado corto. Tag v0.3.53.
*       13/11/26 - Disparo cargado: mantén B ~1 s y suelta un rayo perforante
                 de 24x24. Un toque corto sigue disparando la bala normal de 8x8.
                 Tag v0.3.54.
*       13/11/26 - El sonido al soltar el disparo cargado es más fuerte y más grave
                 (un estruendo de bajos, distinto del láser normal).
                 Tag v0.3.55.
*       13/11/26 - RapidFire ahora es una cápsula "R" que cae, no un toggle de
                 depuración en el botón A. 10% de probabilidad (garantizada tras 10 derribos), 10 s
                 en EASY y 8 s en NORMAL/HARD, barra RF abajo a la
                 derecha. Tag v0.3.56.
*       13/11/26 - Jefe nave nodriza cada 10 oleadas. 500 HP en la oleada 10,
                 el doble en cada regreso hasta 8000 (oleada 50+). Solo el
                 núcleo central recibe daño. Tag v0.3.57.
*       13/11/26 - Un código en la pantalla de título (UUDDLRLRBA) desbloquea un selector
                 de oleada inicial (1-99). Las partidas de práctica desde la oleada 2+ no
                 se guardan en el TOP 5. Tag v0.3.58.
*       13/11/26 - El hitbox del núcleo del jefe se amplió al tercio central y
                 a toda la altura del cuerpo, con destello blanco y tic al golpear
                 el núcleo y una chispa al golpear un ala. La nave nodriza
                 ahora recorre la mitad superior de la pantalla (Lissajous).
                 Tag v0.3.59.
*       13/11/26 - El disparo cargado ya no se salta la pelea del jefe.
                 En una oleada de jefe la formación oculta ya tiene 0
                 naves restantes; ese conteo no debe dar por limpia la oleada.
                 La nave nodriza ahora solo se va cuando sus HP llegan a 0
                 (explosión, bono y luego warp). Tag v0.3.60.
*       16/11/26 - Compilación en macOS: build.sh (build / clean / run, mismo
                 sello de versión que run.bat) con un toolchain nativo arm64 m68k-elf
                 GCC 14.2 + SGDK 2.11 en
                 ~/Desktop/retrodev/mars-sgdk211 y BlastEm 1.0 de
                 Homebrew. Sin cambios en el código del juego. Tag v0.3.61.
*       18/11/26 - README unificado: README.MD queda completo en español
                 y README.en.md es la versión en inglés, con un
                 selector de idioma al inicio de ambos.
*       18/11/26 - Nuevo sprite temporal del jefe (64x56, 2 cuadros, paleta
                 propia en PAL2, tinte rojo al golpear, núcleo de 24x40) y
                 segundo ataque: abanico de 3 balas dirigido al jugador, al
                 azar con más del 50% de vida y alternado con la ráfaga normal
                 con 50% o menos. V0.3.66.
*       19/11/26 - El jefe dispara un hueso de 48x16 (sprite de Alberto,
                 PAL1 colores 7-9) en lugar de la ráfaga de balas: zona de
                 golpe de 40x8, cae 1 px/frame más lento que las balas,
                 máximo 2 en pantalla y el escudo no lo detiene. El
                 abanico no cambia. V0.3.67.
*       19/11/26 - Hueso más lento tras la prueba de Alberto: cae 2 px/frame
                 más lento que las balas (antes 1), 3-5 px/frame. V0.3.68.
*       19/11/26 - Revisión de dificultad: rampa más suave (sube cada 4
                 oleadas, intervalo baja más lento), La Cookie con 350 HP
                 y ciclos de 10 oleadas (la 11 se juega como la 4, la 21
                 como la 7). V0.3.69.
*       19/11/26 - Oleadas 11-19: tercera fila intercalada de 4 naves
                 (columnas 0, 2, 4, 6; 18 naves en total). ENEMY_ROWS 3,
                 acomodo por oleada con enemies_rowMaskForWave. V0.3.70.
*       23/11/26 - Segundo jefe en la oleada 20: la tortuga hechicera
                 (64x64, paleta propia en PAL2, núcleo de 24x36 en el
                 caparazón, flota ±2 px). Dispara una estrella de 16x16
                 desde la varita (PAL1 colores 10-11, zona de 10x10) en
                 lugar del hueso, y el abanico desde la cabeza. Las
                 oleadas 10 y 30 no cambian. V0.3.71.
*       23/11/26 - La tortuga patrulla de izquierda a derecha a altura
                 fija (sin Lissajous ni flotación) y cada 3-6 s al azar
                 (NORMAL) se lanza en picada tras un aviso de 0.6 s:
                 cuesta una vida al tocarte, el escudo no la detiene,
                 sale por abajo y regresa desde arriba. V0.3.72.
*       23/11/26 - Jefes invertidos: tortuga en la 10 (350 HP), Cookie
                 en la 20, y se alternan (impares tortuga, pares Cookie).
                 Aviso de 20 frames (antes 36), picada a 5/6/7 px/frame
                 (antes 3/4/5) y regreso por arriba en columna al azar.
                 V0.3.73.
*       23/11/26 - Tortuga más rápida: patrulla 2/3/4 px/frame (antes
                 1/2/3), picada 6/7/8 (antes 5/6/7) e intervalo entre
                 picadas ~20% más corto (NORMAL 2.4-4.8 s). V0.3.74.
*       23/11/26 - Abanico de la tortuga al 65% de velocidad: 8 -> 5
                 px/frame en las tres dificultades (BOSS2_FAN_SPEED_PCT).
                 La Cookie no cambia. V0.3.75.
*       23/11/26 - Impulso con A + izquierda/derecha: 48 px en 8 frames,
                 recarga igual que el escudo (10/20/30 s), estela y barra
                 IMP en la fila de abajo. Sin invulnerabilidad. V0.3.76.
*       23/11/26 - Estela del impulso más visible: 2 naves fantasma en
                 azul (12 y 24 px detrás) todos los cuadros, se quedan 6
                 frames al terminar. Versión sube a 0.4. V0.4.77.
*       23/11/26 - El jefe (tortuga y La Cookie) suelta una cápsula RF
                 en su centro en vez de dar RapidFire directo; siempre
                 aparece y sobrevive al warp. V0.4.78.
*       23/11/26 - Cápsulas nuevas: "E" (rellena el escudo) e "I"
                 (rellena el impulso), junto a la "R"; 1/3 cada una al
                 soltarse una cápsula. Jefes: siempre "R". V0.4.79.
*       23/11/26 - "E" o "I" con la barra llena dan puntos extra
                 (5 naves: 50/100/150) con "+N" abajo. V0.4.80.

PENDIENTES:
*       Componer una intro real (pista XGM o un jingle corto bien hecho); la actual es un placeholder recortado
*       Empezar una lista de hacia dónde llevar el "juego" base para hacer algo mejor; aquí van algunas ideas:
        - [Hecho 12/11/26] Agregar más enemigos, controlar una segunda fila.
        - Cambiar sprites, usar uno distinto para los enemigos, quizá 2.
          El EnemyRowType por fila ya está listo: pon una hoja de 16x16 en res/tiles/ y
          apunta el spriteDef de la fila a ella en resources.res / enemies.c. La fila
          inferior ya usa una paleta distinta como sustituto.
        - [Hecho 12/11/26] Agregar un sprite de explosión para el jugador cuando lo alcanza el fuego enemigo.
        - [Hecho 12/11/26] ¿Pueden disparar dos enemigos a la vez?
        - Los valores de dificultad se manejan por datos (difficultyTable en difficulty.c): descenso, máximo
          de balas enemigas, disparos por ráfaga, probabilidad de disparo, intervalo de disparo, velocidad enemiga,
          velocidad de bala, rowsOpposite y duración / recarga del escudo (segundos)
          se pueden ajustar ahí sin cambiar el código de movimiento o disparo. Activo en
          NORMAL y HARD; pon el rowsOpposite de EASY en 1 para activarlo ahí también.
          Tiempos del escudo: EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
        - [Hecho 12/11/26] Pantalla de título con menú de 2 opciones: START GAME y DIFFICULTY (EASY / NORMAL / HARD).
          Logo grande de bloques GENS / INVADERS dibujado con tiles sólidos en el plano A.
        - [Hecho 12/11/26] Power-up de escudo (botón C). La duración y la recarga siguen
          la tabla de dificultad (EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s).
          Listo al inicio de la partida. El HUD muestra ESC más una barra (azul / rojo oscuro / verde).
        - [Hecho 13/11/26] RapidFire es una cápsula que se recoge (no un toggle de
          depuración en el botón A). 10% de probabilidad, garantizada tras 10 derribos, 10 s en EASY
          y 8 s en NORMAL/HARD, con una barra de tiempo RF.
        - Más power-ups: ¿más velocidad para la nave? ¿una bomba?
        - [Hecho 12/11/26] La formación baja en cada giro (invaders clásico). GAME OVER si una nave enemiga choca con el jugador, o si llega al fondo visible después de colarse.
        - [Hecho 12/11/26] Animación de explosión propia cuando se destruye una nave
          enemiga (y cuando una choca con el jugador). Más pequeña y
          rápida que la del jugador (~0.4 s, chispas verdes/amarillas, pool de 6).
        - [Hecho 13/11/26] Oleadas infinitas: limpiar un par de filas genera
          la siguiente oleada un poco más rápida (movimiento y cadencia de disparo), con topes
          por dificultad. La partida ya no termina con YOU WIN.
        - [Hecho 13/11/26] Tabla de récords TOP 5 en SRAM, captura de iniciales
          después de GAME OVER y una opción PUNTAJES en la pantalla de título.
        - [Hecho 13/11/26] Vida extra cada 1000 puntos, tope 9, con un
          jingle 1UP corto y un letrero "+1 NAVE".
        - [Hecho 13/11/26] Los puntos por enemigo dependen de la dificultad
          (EASY 10, NORMAL 20, HARD 30); ambas filas usan el mismo valor.
        - [Hecho 13/11/26] Las vidas del HUD son naves en miniatura (hasta 9) y
          el letrero "+1 NAVE" es un mensaje sólido y legible en el plano A.
        - [Hecho 13/11/26] Los íconos de vida están en la esquina inferior izquierda,
          con el letrero "+1 NAVE" a un lado.
        - [Hecho 13/11/26] Transición entre oleadas: warp del fondo de estrellas,
          flama del motor y la siguiente formación entrando deslizándose desde arriba.
        - [Hecho 13/11/26] El sonido del escudo dura todo el tiempo
          que está activo (zumbido de 2 s en CH1, no un blip de 0.16 s).
        - [Hecho 13/11/26] El disparo cargado (mantener B ~1 s) lanza un rayo
          perforante de 24x24; los toques siguen disparando la bala normal de 8x8.
        - [Hecho 13/11/26] El SFX al soltar el disparo cargado es un estruendo más fuerte y grave.
        - [Hecho 13/11/26] RapidFire es una cápsula que se recoge (no un toggle de
          depuración en el botón A). 10% de probabilidad, garantizada tras 10 derribos, 10 s en EASY
          y 8 s en NORMAL/HARD, con una barra de tiempo RF.
        - [Hecho 13/11/26] Jefe nave nodriza en las oleadas 10, 20, 30... con
          500 HP en la primera visita, el doble en cada regreso hasta 8000.
          Daño solo en el núcleo central.
        - [Hecho 13/11/26] Easter egg en la pantalla de título: UUDDLRLRBA desbloquea un
          selector de oleada inicial (1-99). Las partidas que empiezan en la oleada 2+ son
          de práctica y no se escriben en el TOP 5.
        - [Hecho 13/11/26] Retroalimentación de daño del jefe y recorrido 2D en la mitad superior:
          hitbox del núcleo más grande, destello blanco + tic al golpear el núcleo,
          chispa al golpear un ala, movimiento Lissajous que nunca entra
          a la mitad inferior.
        - [Hecho 13/11/26] El disparo cargado ya no termina una oleada de jefe:
          enemiesLeft ya vale 0 con las filas ocultas, así que ese
          conteo se ignora hasta que los HP de la nave nodriza llegan a 0.
        - [Hecho 16/11/26] Compilar también en macOS: ./build.sh build|clean|run
          con SGDK 2.11 y un GCC 14.2 m68k-elf nativo para Apple Silicon.

## Bitácora de trabajo - 10/11/2026 (con Bimo)
Todo lo siguiente se hizo el 10 de noviembre de 2026 (hasta V0.1.16). El detalle de cada cambio está en `CHANGELOG.md`.

**Toolchain y compilación**
* Se migró a SGDK 2.11 y se instaló Java 21 (Eclipse Temurin), que necesita `rescomp`; el proyecto compila sin warnings.
* `run.bat` lanza la ROM en BlastEm 0.6.2; `run.bat build` escribe `src/version.h` (V0.1.xx, xx = número de commits de git), compila y lanza.
* Se corrigió que `run.bat build` reutilizara un `main.o` viejo, lo que hacía que la pantalla de pausa mostrara `V0.1.DEV`.
* `.gitignore` ahora ignora `out/`, `src/boot/sega.s`, `src/boot/*.bak` y `src/version.h`; `src/boot/rom_head.c` sí se versiona.

**Jugabilidad**
* Estados de juego: jugando, en pausa, GAME OVER y YOU WIN; START reinicia cuando termina el juego.
* 3 naves: tras un impacto la nave reaparece y parpadea unos 2 segundos, sin recibir daño.
* Los enemigos se mueven en formación: cuando uno llega a un borde, todos dan la vuelta juntos (ya no se enciman).
* Pantalla de pausa con START que muestra "PAUSED", "GENS INVADERS V0.1.xx" y "PRESS START".
* El HUD muestra el puntaje, los enemigos restantes y las naves restantes.

**Gráficos y sonido**
* El fondo de estrellas (el código que estaba comentado) ya funciona; se dibuja una sola vez en lugar de en cada frame, lo que corrigió la lentitud.
* Intro recortada a un placeholder de 5 segundos con fade (`music/intro_short.wav`); se conserva el `Intro.wav` original.
* La explosión del jugador suena en su propio canal PCM para que el láser ya no la corte.
* El encabezado de la ROM muestra "GENS INVADERS" en lugar del "SAMPLE PROGRAM" de SGDK.

**Corrección de errores y limpieza**
* Bala enemiga que no se liberaba al golpear al jugador; interrupciones que nunca se reactivaban; desbordamiento del buffer del nombre del jugador.
* Se quitaron globales sin uso (`i`, `thex`, `they`, `val`) y se renombró la rutina de colisiones a `handleCollisions`.

## Bitácora de trabajo - 12/11/2026 (con Bimo)
Todo lo siguiente se hizo el 12 de noviembre de 2026. El detalle de cada cambio está en `CHANGELOG.md`.

**Jugabilidad**
* Descenso de la formación (TODO #7 del README): cuando cualquier enemigo vivo llega a un borde, todo el grupo da la vuelta y baja 8 píxeles (`ENEMY_STEP_DOWN`).
* GAME OVER si el sprite de cualquier enemigo vivo llega a la fila del jugador (la parte inferior del sprite alcanza `PLAYER_START_Y`). Usa la ruta existente `showEndScreen(STATE_GAME_OVER)`.
* Reiniciar con START sigue regresando la formación a Y = 32 mediante `placeEnemies()` y restaura `enemyDir`.
* Las balas enemigas y las colisiones no cambian; siguen usando las posiciones actualizadas.

**Dificultad y disparo enemigo (más tarde el 12 de noviembre)**
* Tabla de dificultad basada en datos (`DifficultyConfig difficultyTable`): EASY (sin descenso, 1 bala enemiga), NORMAL (paso de 8 px, hasta 2 balas, dos enemigos disparan juntos), HARD (paso de 16 px, dos disparan juntos, hasta 4 balas en pantalla). `fireChance` está en la tabla para ajustarlo después.
* Pool exclusivo de balas enemigas (`enemyBullets`, 4 sprites). Cada bala colisiona con el jugador por su cuenta y se libera al impactar, al salir de pantalla o al reiniciar. Las balas del jugador se quedan en su propio pool. Vidas, reaparición e invulnerabilidad sin cambios.
* Menú previo al juego "SELECT DIFFICULTY": UP/DOWN para elegir, START para confirmar (NORMAL por defecto). Después de GAME OVER o YOU WIN, START regresa a ese menú con la última elección preseleccionada. La dificultad actual se muestra en el HUD (`DIFF:`) y en la pantalla de pausa.
* El menú independiente SELECT DIFFICULTY después se reemplazó por la pantalla de título (ver abajo).

**Reajuste de dificultad (todavía más tarde el 12 de noviembre)**
* Causa raíz de que HARD se sintiera como NORMAL: `currentDiff` se aplicaba bien, pero HARD compartía el intervalo de disparo de 120 frames y la velocidad horizontal de 1 px, y el límite de 4 balas casi nunca se llenaba porque las balas salían de la pantalla en ~38 frames. No hubo evidencia de frames perdidos.
* La tabla ahora también tiene `shotInterval`, `enemySpeed` y `bulletSpeed`.
* EASY: dispara cada 90 frames con `fireChance` 10 (1 bala). NORMAL: sin cambios (120, 2 disparos, límite 2, paso 8, velocidad 1, bala 5). HARD: intervalo 60, límite 4, paso 16, velocidad horizontal 2, velocidad de bala 6.

**Pantalla de título (todavía más tarde el 12 de noviembre)**
* Se reemplazó el menú independiente SELECT DIFFICULTY por una pantalla de título (`STATE_TITLE`).
* Logo grande de dos renglones, GENS (bloques de 5x5) sobre INVADERS (bloques de 4x5), dibujado con tiles sólidos de 8x8 en el plano A con sombra, para no rebasar el límite de 20 sprites por línea de barrido. El color de relleno pulsa entre dorado y naranja. `V0.1.xx` se muestra bajo el logo. El fondo de estrellas sigue desplazándose en el plano B; los sprites del juego se ocultan.
* Menú: START GAME (INICIAR) y DIFFICULTY: EASY/NORMAL/HARD. UP/DOWN mueven el cursor; LEFT/RIGHT (y START o A sobre DIFFICULTY) alternan el nivel; START sobre START GAME inicia la partida. Después de GAME OVER / YOU WIN, START regresa aquí con la última dificultad preseleccionada.

**Segunda fila de enemigos (todavía más tarde el 12 de noviembre)**
* Dos filas de 7 (14 naves) se mueven, rebotan y bajan como una sola formación. GAME OVER usa el enemigo vivo más bajo; YOU WIN cuando se limpian ambas filas. Las ráfagas eligen la nave viva más baja de cada columna.
* `EnemyRowType` por fila (sprite, paleta, puntos). Por ahora ambas filas reutilizan `ship`; la fila inferior va teñida de verde/amarillo en PAL3 y vale 10 puntos, la superior 20. Un gráfico distinto de 16x16 más adelante es solo un nuevo `SPRITE` en `resources.res` y un cambio de puntero — del mismo tamaño no tiene costo extra de rendimiento.
* Presupuesto de sprites: 14 enemigos + 1 jugador + 6 balas del jugador + 4 balas enemigas = 25 sprites de software. En sprites de hardware (16x16 = 4 celdas, 8x8 = 1): 14*4 + 4 + 6 + 4 = 70 de 80. Por línea de barrido, una fila completa usa 14 de 20 (las filas están separadas 24 px, así que no comparten línea).

**Filas opuestas en HARD (todavía más tarde el 12 de noviembre)**
* Nueva bandera `rowsOpposite` en la tabla de dificultad: 0 en EASY y NORMAL (ambas filas siguen moviéndose como una formación), 1 en HARD. Pon la bandera de NORMAL en 1 para activar el mismo comportamiento ahí.
* En HARD, la fila superior arranca hacia la derecha y la inferior hacia la izquierda (desplazada a la derecha para que no empiece ya pegada al borde izquierdo). Cada fila rebota según sus propias naves vivas y solo esa fila baja. Una fila no baja si quedaría a menos de 24 px de la siguiente fila viva de abajo; solo se invierte. Si la fila inferior ya no existe, la superior puede bajar libremente. GAME OVER sigue usando el enemigo vivo más bajo.

**Filas opuestas también en NORMAL (todavía más tarde el 12 de noviembre)**
* NORMAL ahora también tiene `rowsOpposite=1`. Sus demás valores no cambian (paso 8, velocidad 1, intervalo 120, límite 2). EASY se queda en 0 (ambas filas se mueven juntas). HARD sin cambios.

**Refactorización modular (todavía más tarde el 12 de noviembre)**
* Se dividió src/main.c (1226 líneas en V0.1.25) en módulos sin cambios de comportamiento. Headers públicos en inc/; SGDK ya compila todos los src/*.c e incluye -Iinc.
* Módulos nuevos: `config.h` / `game.h`, `difficulty`, `bullets`, `hud`, `enemies`, `player`, `collision`, `title`, `powerups`, `game_state`. `main.c` ahora es inicialización + fondo de estrellas + despacho del ciclo (~91 líneas).
* GAME_VERSION se incluye desde game.h, así que el título y la pausa siguen mostrando V0.1.xx. Después de cambiar headers, borra out/src/*.o antes de recompilar.
* No se movió: el fondo de estrellas (drawStarfield y el scroll de offset) se queda en main.c (no se planeó un módulo de fondo).

**NORMAL siempre dispara su ráfaga (todavía más tarde el 12 de noviembre)**
* Después de la refactorización, Alberto notó que en NORMAL las naves casi no parecían disparar. Un diff contra V0.1.25 no mostró regresión: mismo ticker, pool, getters y tabla. NORMAL tenía `fireChance` 8, así que aproximadamente 1 de cada 5 ráfagas se saltaba y dos fallas seguidas significaban 4-6 segundos sin balas (EASY y HARD ya disparaban en cada intento).
* `fireChance` en NORMAL ahora es 10: cada ráfaga de 120 frames (~2 s) dispara dos balas, todavía con límite de 2. Intervalo, velocidad, paso, límite y `rowsOpposite` sin cambios. EASY y HARD sin cambios.

**Tiradores enemigos más variados (todavía más tarde el 12 de noviembre)**
* En NORMAL la cadencia de ráfagas era correcta (2 naves cada ~2 s), pero casi siempre eran las mismas naves.
* Causas raíz: SGDK solo siembra `random()` automáticamente con el primer botón de la sesión (a menudo un toque en el menú justo después de arrancar, con un tick parecido); `random() % n` usaba los bits bajos del xorshift de 16 bits y podía agrupar columnas; el selector no evitaba las columnas de la ráfaga anterior.
* El START que inicia una partida ahora llama a `setRandomSeed` con un contador de frames de la pantalla de título, `vtimer` y `getTick()`. Cada ráfaga sigue disparando con la nave viva más baja de una columna, pero las columnas se eligen al azar, son distintas dentro de la ráfaga y se saltan las columnas de la ráfaga anterior cuando hay otras elegibles. Usa los bits altos de `random()` (`randRange`). Aplica a todas las dificultades; los valores de la tabla no cambian.

**Explosión del jugador (todavía más tarde el 12 de noviembre)**
* Nueva hoja indexada de 6 frames de 16x16 `res/tiles/imgexplosion.bmp` (96x16, color 0 transparente, la misma PAL1 que `space.bmp`: destello blanco, bola de fuego naranja/roja que se expande, escombros, desvanecido). `SPRITE explosion` en `resources.res` (2x2, FAST, velocidad de animación 0; los frames se avanzan a mano para que no se repita en ciclo).
* Al recibir un impacto, la nave del jugador se oculta y la explosión se reproduce en esa posición durante 48 frames (~0.8 s, 8 ticks por frame visual). El control ya está bloqueado mientras la salud es 0; la pausa también se bloquea. Se omiten el disparo enemigo y el ticker de disparo (`player_isExploding`); los enemigos y las balas que ya están en pantalla siguen moviéndose. Una breve sacudida horizontal que se amortigua en el fondo de estrellas (BG_B) y un destello de 4 frames del color 2 de PAL1 a blanco, que luego se restaura.
* Si quedan vidas, el resto de `RESPAWN_DELAY` (42 frames) corre después de la explosión para que el tiempo total muerto siga en ~90 frames, y luego viene la invulnerabilidad con parpadeo de siempre. Si no quedan vidas, el GAME OVER espera a que termine la explosión (`player_update` regresa 1). La lógica vive en el módulo player. El presupuesto de sprites de hardware se queda en 70: la explosión de 16x16 (4 celdas) reemplaza a la nave oculta.

**Power-up de escudo (todavía más tarde el 12 de noviembre)**
* El botón C activa un escudo (B ya es disparar, A sigue siendo el toggle de RapidFire). Empieza cargado al inicio de la partida, dura 5 s, luego recarga durante 30 s y no se puede volver a activar hasta entonces. NTSC usa 300 / 1800 frames; en PAL, `IS_PAL_SYSTEM` selecciona 250 / 1500 para que el tiempo real siga siendo 5 s / 30 s. El proyecto no tenía tiempos para PAL antes de esto.
* Mientras está activo, una burbuja azul/blanca de 32x32 que pulsa en 4 frames (`res/tiles/imgshield.bmp`, misma PAL1, color 0 transparente) sigue a la nave. Las balas enemigas que tocan la caja de 32x32 se destruyen y no hacen daño. Este juego no tiene colisión de cuerpo nave contra enemigo, así que el escudo solo tiene que detener balas. Suenan blips PCM cortos en el canal 1 cuando el escudo se activa y cuando termina la recarga.
* No se puede usar durante la explosión del jugador, mientras está muerto, en pausa ni durante la recarga. El temporizador solo avanza en STATE_PLAYING, así que la pausa lo congela. Al perder una vida se conserva el estado de recarga (o LISTO); una bala no puede matar al jugador mientras el escudo está activo, así que en la práctica la recarga simplemente sigue corriendo durante la explosión y la reaparición. `powerups_reset` en una partida nueva lo regresa a listo. Campo del HUD a la derecha de la línea 2: `ESC: LISTO`, `ESC: ON n` o `ESC: ns`, que se redibuja con la línea del puntaje para que el diseño se mantenga intacto.
* Un sprite de hardware extra (32x32 = 1 sprite del VDP, 16 tiles). El conteo real de sprites es de unos 27 de 80. El presupuesto anterior de "70" contaba cada nave de 16x16 como 4 sprites; esta habilidad agrega un sprite real del VDP, no 16.

**Tiempos del escudo por dificultad (todavía más tarde el 12 de noviembre)**
* Después de probarlo, 5 s de escudo se sentían demasiado. La duración y la recarga se movieron a la tabla de dificultad (`shieldDurationSec` / `shieldRechargeSec`): EASY 5 s / 10 s, NORMAL 3 s / 20 s, HARD 2 s / 30 s.
* `powerups` sigue convirtiendo esos segundos a frames con `IS_PAL_SYSTEM` (x60 NTSC, x50 PAL) al reiniciar, así que el tiempo real es el mismo en ambos sistemas. El HUD sigue mostrando los segundos restantes (`ESC: ON n`, `ESC: ns`). Se quitaron de `config.h` las macros fijas `SHIELD_DURATION_*` / `SHIELD_RECHARGE_*`.

**Versión 0.2 (todavía más tarde el 12 de noviembre)**
* El prefijo de versión ahora es 0.2. El último número sigue siendo el total de commits (`git rev-list --count HEAD`), no un conteo desde un tag base 0.2, así que esta versión es V0.2.42 (el commit siguiente a V0.1.41). El respaldo sin `src/version.h` es `V0.2.DEV`. Tag anotado `v0.2.42`.
* `run.bat build` escribe `V0.2.xx` en `src/version.h` con Python para que `echo` ya no deje un espacio al final, y borra todos los `out/src/*.o` antes de compilar porque varios módulos incluyen la versión a través de `game.h`.

**Barra del escudo en el HUD (todavía más tarde el 12 de noviembre)**
* Se reemplazó el HUD numérico del escudo (`ESC: LISTO` / `ESC: ON n` / `ESC: ns`) por una etiqueta corta `ESC` y una barra de 8 tiles (64 niveles, tiles de octavos de bloque) a la derecha de la línea 2, sin tocar SCORE ni DIFF.
* Mientras el escudo está activo la barra es azul (PAL1) y se vacía con el tiempo restante. Durante la recarga se llena en rojo oscuro (PAL1). Cuando está listo queda llena y verde (color 1 de PAL3, el mismo verde de la fila inferior de enemigos) y parpadea en azul durante medio segundo al terminar la recarga, junto con el SFX de listo que ya existía.
* El plano A solo se reescribe cuando cambia el nivel, el estado o la fase del parpadeo. Los tiles se generan en código y se cargan en `TILE_USER_INDEX + 2` (el título sigue usando los dos primeros tiles de usuario). Sin paleta ni BMP nuevos; los colores de relleno ya existen en PAL1 / PAL3. La parte vacía de la barra es un riel negro (índice de paleta 4).

**GAME OVER por invasión al contacto de cuerpos (todavía más tarde el 12 de noviembre)**
* La partida ya no termina solo porque un enemigo vivo llegue a la fila del jugador (`PLAYER_START_Y`) sin tocar la nave. `collision` ahora revisa si el cuerpo de un enemigo se encima con el del jugador: eso siempre es GAME OVER después de la explosión del jugador (`player_hitFatal`), aunque queden vidas. Invasión clásica: no solo pierdes una nave.
* El escudo y la invulnerabilidad después de reaparecer no protegen contra ese contacto (siguen deteniendo las balas enemigas).
* Si una nave baja más allá de la altura del jugador sin encimarse en X, la formación sigue marchando. Cuando cualquier enemigo vivo llega al fondo visible (`y + h >= BOTTOM_EDGE`, 224), se aplican la misma explosión fatal y el GAME OVER, para que el juego no se estanque con invasores quietos debajo del cañón. No rebotan ni se congelan.

**Explosiones enemigas (todavía más tarde el 12 de noviembre)**
* Nueva hoja indexada de 6 frames de 16x16 `res/tiles/imgenemyexp.bmp` (96x16, color 0 transparente, la misma paleta que `space.bmp`). La explosión es más pequeña que la bola de fuego del jugador: un destello blanco/amarillo compacto y luego chispas verdes que salen hacia fuera y se desvanecen. `SPRITE enemyexp` en `resources.res` (2x2, FAST, velocidad de animación 0; los frames se avanzan a mano). Se dibuja con PAL3 durante el juego (verde/amarillo/blanco) para que se distinga claramente de la explosión naranja/roja/blanca del jugador en PAL1.
* Dura 24 frames (~0.4 s a 60 Hz, 4 ticks por frame visual). La nave enemiga se oculta de inmediato (`killGameEntity`) y la explosión se reproduce en ese `x,y`. Las explosiones son sprites independientes: no colisionan ni afectan el movimiento de la formación.
* Pool de 6 (`MAX_ENEMY_EXPLOSIONS` en `inc/explosions.h` / `src/explosions.c`). Si todos los espacios están ocupados, se reutiliza el más antiguo (el de menor tiempo restante). Cada explosión es 1 sprite del VDP (16x16). El pico de sprites de hardware es de unos 33 de 80 (14 enemigos + jugador + explosión del jugador + escudo + 6 balas del jugador + 4 balas enemigas + 6 explosiones enemigas). Las naves muertas ya están ocultas, así que una línea de barrido contiene explosiones en lugar de una fila viva completa y se mantiene bajo el límite de 20 sprites.
* Se engancha desde `collision`: una bala del jugador que mata una nave genera una explosión y luego sigue la ruta existente de puntaje / YOU WIN. Un choque de cuerpo contra el jugador (o un aterrizaje en `BOTTOM_EDGE`) genera una explosión en cada nave que se encima (o que aterrizó), las mata sin dar puntaje ni YOU WIN, y luego llama a `player_hitFatal` para que ambas explosiones se vean antes del GAME OVER. La explosión enemiga puede terminar debajo del texto de YOU WIN / GAME OVER; la pausa la congela igual que los demás temporizadores.
* SFX opcional de un pop agudo y corto (`res/sounds/enemyexp.wav`, PCM id 69, ~0.10 s) en el canal 3 con prioridad moderada, para que pueda sonar al mismo tiempo que la explosión del jugador en el canal 2 durante un choque por invasión.

**Oleadas infinitas y versión 0.3 (13/11/2026)**
* Limpiar ambas filas ya no muestra YOU WIN. El juego se queda en STATE_PLAYING, muestra un letrero centrado `OLEADA N` durante unos 1.5 s (90 frames NTSC / 75 PAL) y luego vuelve a generar dos filas en `ENEMY_START_Y`. Se conservan el puntaje, las vidas, RapidFire y el escudo (incluso a mitad de la recarga). Se limpian las balas enemigas y el ticker de ráfagas; las balas del jugador y las explosiones enemigas que ya se estén reproduciendo continúan. El jugador puede seguir moviéndose, disparando y usando el escudo durante el letrero; la pausa congela el temporizador y redibuja el letrero al reanudar.
* La oleada 1 es una copia exacta de la tabla de dificultad existente. Las oleadas siguientes escalan un `DifficultyConfig` en tiempo de ejecución (`difficulty_beginMatch` / `difficulty_nextWave` en `difficulty.c`). Incrementos y topes por dificultad:
  * EASY: intervalo de 90 baja de 10 en 10 hasta un mínimo de 45; velocidad de 1 a 3 cada 3 oleadas; ráfaga/límite se quedan en 1 hasta la oleada 4, luego 2 (nunca más); velocidad de bala de 5 a 7 cada 3 oleadas; sin descenso (stepDown se queda en 0).
  * NORMAL: intervalo de 120 a 40; velocidad de 1 a 3 cada 3 oleadas; la ráfaga y el límite de balas crecen de 2 a 4 cada 3 oleadas; velocidad de bala de 5 a 7; stepDown de 8 a 16 (+4 cada 4 oleadas). Las filas siguen moviéndose en direcciones opuestas; la selección de columnas no cambia.
  * HARD: intervalo de 60 a 30; velocidad de 2 a 3 cada 3 oleadas; ráfaga de 2 a 4 cada 2 oleadas (el límite de balas ya es 4); velocidad de bala de 6 a 7; stepDown de 16 a 20 (+4 cada 5 oleadas).
  El pool de balas enemigas se queda en 4, así que el presupuesto de sprites de hardware no crece con el número de oleada.
* El HUD muestra `OLEADA n` en la línea 2, columna 16, después de `DIFF:` y antes de la barra `ESC` (cabe incluso con oleadas de 3 dígitos). La pantalla de pausa también muestra la oleada.
* El prefijo de versión ahora es 0.3. El último número sigue siendo el total de commits (`git rev-list --count HEAD`), no un conteo desde un tag base 0.3, así que esta versión es V0.3.46. El respaldo sin `src/version.h` es `V0.3.DEV`. `run.bat build` escribe `V0.3.xx`. Tag anotado `v0.3.46`. Las reglas de GAME OVER no cambian; el escudo sigue deteniendo solo balas.

**Récords TOP 5 (13/11/2026)**
* Una sola tabla compartida de 5 entradas se guarda en la SRAM del cartucho (magic `GIHS`, formato 1, checksum de 16 bits). Cada renglón guarda 3 iniciales, el puntaje, la oleada alcanzada y una letra de dificultad (F/N/D). Se prefirió una sola tabla con columna de dificultad en lugar de tres listas separadas, para poder comparar de un vistazo los puntajes de Fácil, Normal y Difícil. Si el bloque de SRAM no existe o falla el checksum, se escriben valores predeterminados: GEN 1000, INV 800, UDL 600, AMH 400, BIM 200 (oleada 1).
* Después de GAME OVER, un puntaje que supera el 5.º lugar actual (los empates se quedan fuera) abre la captura de 3 iniciales: arriba/abajo recorren A-Z, espacio y `.`; izquierda/derecha mueven el cursor; A o START confirman. Luego la tabla se dibuja bajo `GAME OVER` (renglones 12-16) con un `>` parpadeante en el renglón nuevo, y `PRESS START` sigue regresando al título. Un puntaje que no califica se salta la captura y solo muestra la tabla. La explosión del jugador sigue terminando antes de esta pantalla.
* El menú de título tiene una tercera opción, `PUNTAJES (TOP 5)`. START o A abren la tabla bajo el logo; START o B regresan al menú.
* El encabezado de la ROM ya declaraba SRAM (`RA`, 0xF820, 0x200000-0x20FFFF) en `rom_head.c`; se agregó un comentario. En BlastEm el archivo es `%LOCALAPPDATA%\blastem\rom\save.sram` porque la ROM se lanza como `out\rom.bin`. Borrar ese archivo y volver a lanzar restaura los valores predeterminados.

**Vida extra cada 1000 puntos (13/11/2026)**
* Se otorga una nave en 1000, 2000, 3000... (`EXTRA_LIFE_SCORE` en `config.h`, igual en todas las dificultades). Si un solo derribo cruza más de un umbral, se cuentan todas las marcas cruzadas. Las vidas tienen tope de 9 (`LIVES_CAP`) para que `SHIPS: n` en el HUD siga en un dígito; pasado el tope no se agrega nave, pero el siguiente umbral sí avanza (no se acumulan 1UP pendientes). Las vidas iniciales siguen en 3 (`MAX_LIVES`). El contador se reinicia en `player_reset` al inicio de la partida.
* Al otorgarla, `SHIPS` se actualiza de inmediato y aparece un `+1 NAVE` (o `+n NAVE`) parpadeante durante 1 s aproximadamente (60 frames NTSC / 50 PAL) en la fila 2, bajo el HUD y sin estorbar al letrero de oleada. Un ding corto de dos notas (`res/sounds/oneup.wav`, PCM id 70, ~0.22 s) suena en el canal 4 para no cortar el láser (CH3) ni la explosión del jugador (CH2).
* No se otorgan vidas en las pantallas de GAME OVER / iniciales (ahí el puntaje ya no sube). Un 1UP ganado durante la explosión de la última nave por una bala cancela el GAME OVER pendiente y el jugador reaparece. Un golpe fatal por invasión o por el borde inferior sigue terminando la partida aunque se cruce un umbral mientras se reproduce la explosión; el umbral avanza pero no se agrega nave.

**Puntos por nave según la dificultad (13/11/2026)**
* La única fuente de puntaje sigue siendo que una bala del jugador destruya a un enemigo (sin bono por oleada; un choque por invasión sigue sin dar puntos). Antes ambas dificultades usaban un reparto fijo por fila: fila superior 20, inferior 10, así que una oleada completa de 14 siempre valía 210 puntos.
* Los puntos ahora vienen de `enemyPoints` en la tabla de dificultad, iguales para todas las naves: EASY 10, NORMAL 20, HARD 30. Una oleada limpia vale 140 / 280 / 420. Las vidas extra siguen cada 1000 puntos en todas las dificultades, así que el primer 1UP llega alrededor de la oleada 8 en EASY (7 oleadas = 980), la oleada 4 en NORMAL (3 oleadas = 840) y la oleada 3 en HARD (2 oleadas = 840).
* La tabla compartida TOP 5 no cambia, incluidas las entradas de ejemplo. Los puntajes de Difícil subirán más rápido que los de Fácil; la letra de dificultad (F/N/D) sigue indicando de qué partida se trata.

**Íconos de naves en el HUD y letrero 1UP legible (13/11/2026)**
* La falla de "+1 NAVE" no era el límite de 20 sprites por línea de barrido. El letrero ya era texto del plano A en la fila 2 (arriba de los enemigos, que empiezan en Y=32). Dos cosas lo hacían difícil de leer: `hud_updateExtraLife` borraba y redibujaba toda la fila en cada frame durante la zona visible, lo que producía líneas horizontales parpadeantes en el VDP, y el texto usaba prioridad baja, así que las balas del jugador que salían por arriba le hacían hoyos. Además, el parpadeo de 8 encendido/8 apagado dejaba el mensaje invisible la mitad de la ventana de ~1 s.
* Ahora el letrero se dibuja una sola vez, se queda fijo durante el mismo ~1 s (60 frames NTSC / 50 PAL) y usa tiles del plano A de alta prioridad (`VDP_setTextPriority`), así que los sprites pasan por detrás. Los tiles solo se tocan cuando el letrero aparece o desaparece.
* Ya no existe `SHIPS: n`. Hasta 9 naves en miniatura de 8x8 (una silueta reducida de la nave del jugador, PAL1, plano A, no sprites) quedan en la esquina superior derecha, columnas 31-39, alineadas a la izquierda para que una vida nueva aparezca a la derecha de las que ya están. Caben las 9, así que no hay forma de desborde "x9". Se actualizan cuando se pierde o se gana una vida. Durante el letrero 1UP el ícono recién otorgado parpadea; los demás se quedan fijos. `SCORE` y `LEFT` siguen a la izquierda de la fila 0, con un espacio antes de los íconos.

**Íconos de vida movidos abajo a la izquierda (13/11/2026)**
* Los nueve íconos de nave de 8x8 dejaron la esquina superior derecha de la fila 0 (el HUD superior ya estaba ocupado con SCORE, LEFT, DIFF, OLEADA y la barra ESC) y ahora están abajo a la izquierda del plano visible: fila 27, píxeles 216-223, columnas 0-8, todavía alineados a la izquierda para que una vida nueva aparezca a la derecha. Siguen siendo tiles del plano A de alta prioridad, no sprites.
* Esa fila queda justo debajo del escudo de 32 px (que termina en Y=216) y no se encima con la nave del jugador (Y=192-207, solo se mueve en horizontal), así que `PLAYER_START_Y` se quedó en 192. Las balas enemigas y una invasión tardía en esa esquina pasan por detrás de los íconos. Las explosiones se quedan a la altura Y de la nave y no cubren la fila.
* El letrero fijo `+1 NAVE` se movió a la misma fila, a partir de la columna 10, justo después de los nueve espacios de íconos, así que ya no ocupa la parte de arriba de la pantalla. Borrar el letrero no borra los íconos. El ícono recién otorgado sigue parpadeando mientras el letrero está visible. `SCORE` / `LEFT` vuelven a usar todo el ancho de la fila 0.

**Animación de warp entre oleadas (13/11/2026)**
* Limpiar una oleada ya no salta directo a la siguiente formación después de un letrero de 1.5 s. Ahora hay una transición de ~2.5 s (150 frames NTSC / 125 PAL) con `OLEADA N` en pantalla todo el tiempo. Se limpian las balas enemigas; se conservan el puntaje, las vidas, el escudo (incluidos sus temporizadores) y RapidFire. Las balas del jugador que ya van en el aire pueden terminar; se bloquean los disparos nuevos, los toggles de RapidFire y activar el escudo. El movimiento horizontal y la pausa siguen funcionando. El letrero 1UP y los íconos de vida no cambian (la flama de 8x8 queda en Y=208-215, justo arriba de los íconos de la fila 27).
* La nave sube 8 píxeles y una flama de escape de 8x8 con dos frames (`res/tiles/imgflame.bmp`, PAL1, un sprite de hardware) parpadea bajo el casco. El paso del fondo de estrellas sube de 2 a 14 y vuelve a bajar. Durante el primer ~1.25 s las dos filas siguientes esperan completamente arriba de la pantalla; en el último 1.25 s entran deslizándose a sus posiciones (80 px) mientras las estrellas frenan y la flama se apaga. La oleada 1 sigue empezando en su lugar, sin warp.
* Presupuesto de sprites: un sprite extra de 8x8 (35 asignados, todavía muy por debajo de 80). La flama no comparte línea de barrido con la nave de 16x16 y solo se encima con la parte baja del escudo si ya está activo, así que el límite de 20 sprites por línea no es problema.

**Zumbido del escudo mientras está activo (13/11/2026)**
* El sonido de activación parecía cortarse, pero nada le estaba robando el canal. `shield_on.wav` dura solo 0.16 s; los disparos (CH3), las explosiones (CH2) y el jingle 1UP (CH4) ya usan otros canales PCM. Un nuevo zumbido de 2 s (`res/sounds/shield_hum.wav`, PCM id 71) suena en CH1 (el canal que ya usaba el escudo) durante todo el tiempo que el escudo está activo: 2 s en HARD (una reproducción), 3 s en NORMAL y 5 s en EASY (la muestra se vuelve a disparar cuando termina). Empieza con el ataque de la activación original para que el botón siga teniendo golpe, y luego queda debajo de la acción como un drone más bajo.
* El zumbido se detiene cuando termina el temporizador del escudo, en pausa, al morir, en GAME OVER y en la pantalla de título. Si el escudo sigue activo al quitar la pausa o después de reaparecer, vuelve a empezar. Durante el warp entre oleadas el escudo sigue activo, así que el zumbido también. El ding de "escudo listo" sigue en CH1 y solo suena después de que el zumbido se detuvo. La muestra corta `SFX_SHIELD_ON`, que ya no se usa, sigue en la ROM pero ya no se reproduce por sí sola.

**Disparo cargado (13/11/2026)**
* Ahora el disparo ocurre al soltar el botón, para que mantenerlo pueda cargar. Un toque corto (menos de ~1 s, 60 frames NTSC / 50 PAL) sigue disparando la bala normal de 8x8 y usa el límite de RapidFire (3 o 6). Mantener B alrededor de un segundo arma un disparo cargado: aparece un aura naranja de 16x16 alrededor de la nave, que parpadea en blanco cuando está lista, y suena un ding corto de fijación de blanco en CH4 (prioridad 10, por debajo del jingle 1UP; nunca usa CH1, así que no puede cortar el zumbido del escudo). Al soltar se dispara un rayo de 24x24 (3x el disparo normal, un sprite de hardware de 3x3, un poco más rápido a -4 px/frame) que perfora a todos los enemigos que toca, dando los puntos normales por nave, hasta que sale de la pantalla.
* Solo puede haber un rayo cargado en pantalla; tiene su propio espacio y no cuenta contra el límite de RapidFire. Si todavía hay uno volando, soltar una carga dispara una bala normal para que el botón nunca quede sin respuesta. La carga se cancela (sin disparo) cuando empieza un warp de oleada, al morir y en las pantallas de título / GAME OVER. Después de la pausa, si B se soltó mientras estaba en pausa, la carga se descarta sin disparar. RapidFire no cambia: solo sube el límite de balas pequeñas; la carga funciona igual con él activo o no.
**Estruendo del disparo cargado (13/11/2026)**
* La muestra al soltar el disparo cargado era más delgada y aguda que el láser normal (pico 16249, RMS 6733, ataque alrededor de 1.8 kHz). Ahora es un estruendo de bajos de ~0.32 s (barrido de unos 280 Hz a 70 Hz) con un clic corto para que se siga sintiendo como disparo. Pico 20000, justo debajo de la explosión del jugador (20814) y más fuerte que el láser (17972); RMS de unos 9600, cercano al del láser, así que se mantiene presente sin saturar. Sigue sonando en CH3, así que no puede cortar el zumbido del escudo en CH1. El ding de carga lista es un poco más lleno (se agregó un armónico más grave), pero sigue siendo más bajo que el estruendo y que el jingle 1UP.

**Cápsula RapidFire (13/11/2026)**
* El toggle de RapidFire en el botón A era un modo de depuración que había quedado (6 balas, tinte rojo, 100 frames, uso ilimitado). Ahora es un objeto que se recoge de verdad. Los enemigos destruidos tienen 10% de probabilidad (1 de 10) de soltar una cápsula "R" parpadeante de 8x8 que cae a 1 px/frame. Como máximo hay una cápsula en pantalla; después de 10 derribos sin cápsula, la siguiente es segura. La última nave de una oleada nunca suelta una, porque el warp entre oleadas la borraría de inmediato. Tocar la cápsula otorga RapidFire; si sale por la parte de abajo de la pantalla, desaparece.
* Dura 10 s en EASY y 8 s en NORMAL y HARD (el mismo tiempo real en PAL, desde la tabla de dificultad). Recoger otra cápsula mientras está activo reinicia el temporizador. Una etiqueta "RF" abajo a la derecha y una barra roja de 8 tiles (plano A, alta prioridad, los mismos tiles de la barra de recarga del escudo) muestran el tiempo restante. El cuerpo de la nave (color 2 de PAL1) se pone rojo durante el efecto. El botón A queda libre durante el juego (se sigue usando en el menú de título y en la captura del nombre).
* El efecto se mantiene entre oleadas y sigue corriendo durante el warp, así que el tiempo que sobre te espera cuando vuelves a disparar. Una cápsula que todavía va cayendo se borra en el warp, al morir, en GAME OVER y en la pantalla de título. El efecto en sí termina al morir (tienes que recoger otra después de reaparecer). El blip al recogerla suena en CH4 (prioridad 11, por debajo del jingle 1UP) y no puede cortar el zumbido del escudo en CH1. El disparo cargado no cambia y sigue ignorando el límite de RapidFire. La cápsula es un solo sprite de hardware de 8x8 de baja prioridad, así que los íconos de vida y la barra RF siguen legibles si cae por esa esquina.
**Jefe nave nodriza (13/11/2026)**
* Las oleadas 10, 20, 30... reemplazan las dos filas de enemigos con una nave nodriza de 64x24 (arte provisional en `res/tiles/imgboss.bmp`, dos frames, dos sprites de hardware). Entra deslizándose durante el warp de siempre, bajo un letrero "OLEADA N" + "JEFE", y patrulla en horizontal en Y=40. Solo el núcleo brillante de 16x16 recibe daño; un disparo normal que pega en un ala se bloquea sin daño, y el rayo cargado atraviesa el casco y daña el núcleo una vez por pasada (3x un golpe normal). Al morir la nave explota, se otorgan puntos, se otorga o renueva RapidFire y empieza la siguiente oleada normal (desde V0.4.78 suelta una cápsula RF que hay que atrapar, ver abajo).
* Los HP empiezan en 500 en la oleada 10 y se duplican en cada regreso (1000, 2000, 4000) hasta un tope de 8000 a partir de la oleada 50. El tope cabe en un u16 y la barra de 16 tiles usa aritmética de 32 bits (`hp * 128 / max`), así que no hay desbordamiento. El daño al núcleo es 30 / 20 / 10 en EASY / NORMAL / HARD; el movimiento y el disparo también escalan (velocidad 1/2/3, 1/2/4 disparos cada 90/60/40 frames). Los puntos base son 500 / 1000 / 1500 por el número de aparición. Una etiqueta "JEFE" y una barra quedan en la fila 2 del HUD (azul, luego roja por debajo del 25%). El sprite es provisional para que Alberto lo reemplace después. Los tics de impacto suenan en CH4 (no pueden cortar el zumbido del escudo); el estruendo de la muerte va en CH2.
**Easter egg de oleada inicial (13/11/2026)**
* En la pantalla de título (no en la vista del TOP 5), ingresar UP, UP, DOWN, DOWN, LEFT, RIGHT, B, A desbloquea un cuarto renglón del menú, `OLEADA INICIAL: NN` (1-99, da la vuelta). El menú se sigue moviendo mientras se ingresa el código; la secuencia se rastrea en paralelo. Un botón equivocado o una pausa de ~2 s (120 frames NTSC / 100 PAL) la reinicia; un UP suelto después de un error puede empezar un intento nuevo. Completar el código con A no alterna también la dificultad ni abre PUNTAJES. Suena un jingle 1UP y aparece brevemente "TRUCO ACTIVADO" bajo el logo. El desbloqueo vive solo en RAM y dura hasta apagar o reiniciar; no se guarda en SRAM. Consejo: ingrésalo con el cursor en INICIAR para que LEFT/RIGHT no cambien la dificultad.
* START sobre INICIAR o sobre el renglón de oleada inicia la partida en esa oleada, con el escalado normal por oleada y, en 10/20/30..., la nave nodriza ya en su lugar (sin warp, como en la oleada 1) y con los HP que le tocarían en esa visita. Empezar en la oleada 1, aunque se haya desbloqueado, es una partida normal y puede entrar al TOP 5. Empezar en la oleada 2 o posterior es una partida de práctica: no califica para la tabla (no pide iniciales), la pantalla de pausa muestra PRACTICA y el GAME OVER dice que el puntaje no se guarda. Así la tabla oficial se mantiene limpia sin castigar a quien solo desbloqueó el selector y luego jugó desde la oleada 1.


**Hitbox del jefe, retroalimentación y recorrido en la mitad superior (13/11/2026)**
* La primera versión de la nave nodriza solo recibía daño en un núcleo de 16x16 a 4 px del borde superior, mientras que el brillo dibujado era más ancho, así que los disparos al punto débil evidente muchas veces contaban como golpe en el ala: la bala desaparecía, los HP no bajaban, la barra JEFE no se movía y no había destello ni tic. Por eso parecía que el jefe no recibía daño. La zona débil ahora es el tercio central del casco y toda la altura de 24 px del cuerpo (24x24 en el offset 20,0). Un golpe al núcleo pone en blanco la nave nodriza durante unos frames (solo PAL2, para que la nave del jugador no se decolore) y siempre suena el tic de impacto, también con el rayo cargado. Un golpe en el ala genera una chispa y sigue sin hacer daño. La barra JEFE se fuerza a redibujarse en cada cambio de HP.
* Se eliminó la patrulla solo horizontal en Y=40. Después de la animación de entrada, la nave nodriza sigue una trayectoria Lissajous suave por la mitad superior de la pantalla (Y 28-88, así que el casco nunca cruza la mitad hacia la zona del jugador). La velocidad escala con la dificultad (`bossSpeed` 1/2/3). Las ráfagas se siguen disparando desde la parte inferior y la barra de HP se queda en la fila 2 del HUD.

**El disparo cargado ya no se salta al jefe (13/11/2026)**
* En una oleada de jefe las dos filas están ocultas y `enemiesLeft` vale 0 durante toda la pelea. La ruta del rayo cargado trataba "rayo en vuelo y sin naves restantes" como oleada limpia, así que soltar un disparo cargado (aunque fallara) regresaba `COLLISION_WIN`, ocultaba la nave nodriza sin la explosión de derrota ni el bono y saltaba a la siguiente oleada. Los disparos normales no lo activaban porque solo revisaban el conteo después de un derribo real. La revisión de formación limpia ahora se ignora mientras el jefe está activo o la oleada actual es de jefe; `game_state` también se niega a iniciar la siguiente oleada si la nave nodriza sigue en pie. La pelea solo termina cuando los HP del jefe llegan a 0, con la explosión, los puntos, el RapidFire otorgado (desde V0.4.78, la cápsula RF que suelta) y el warp de siempre. Las oleadas normales no cambian.

## Bitácora de trabajo - 16/11/2026 (con Bimo)
Todo lo siguiente se hizo el 16 de noviembre de 2026 (V0.3.61). Los detalles están en `CHANGELOG.md`.

**Compilación en macOS (16/11/2026)**
* Nuevo `build.sh` en la raíz del repo, el equivalente de `run.bat` para macOS/Linux. `./build.sh build` escribe `src/version.h` byte por byte igual que `run.bat` (`#define GAME_VERSION "V0.3.xx"` + CRLF, xx = `git rev-list --count HEAD`, mínimo 2 dígitos), borra `out/src/*.o` porque varios módulos incluyen la versión a través de `game.h` y ejecuta el `makefile.gen` de SGDK 2.11. `./build.sh clean` ejecuta el clean de makefile.gen y `./build.sh run` abre `out/rom.bin`. Sin argumentos solo corre la ROM, como `run.bat`; los comandos se pueden encadenar (`./build.sh clean build run`).
* Toolchain: la MacBook Pro de Alberto (Apple Silicon) tenía marsdev instalado en `/opt/toolchains/mars`, pero esa instalación es un binario Intel (GCC 13.1, SGDK 2.00) enlazado contra bibliotecas de Homebrew en `/usr/local`, así que su compilador no corre ahí. La recompilación arm64 propia de marsdev de m68k-elf GCC 14.2.0 + binutils sí funcionó, así que se copió a `~/Desktop/retrodev/mars-sgdk211/m68k-elf` junto con SGDK 2.11 (tag v2.11). Ahí se compilaron `libmd.a` y `libmd_debug.a` con 0 warnings; `bintos`, `xgmtool`, `sjasm` y `convsym` se compilaron de forma nativa y se copiaron los jars de SGDK. `common.mk` es el de SGDK 2.11 con rutas absolutas a las herramientas `m68k-elf-*` y al plugin LTO / libgcc de GCC 14.2. Todo vive en carpetas del usuario; no se necesitó `sudo` y la instalación vieja de `/opt` quedó intacta. `GENS_GDK` cambia la ruta.
* Emulador: BlastEm 1.0.0 de Homebrew (`brew install blastem`, nativo arm64). `run` recurre a `open -a "Genesis Plus"` si BlastEm no está instalado.
* Resultado: GCC 14 compila el juego con 0 warnings y sin cambios en el código del juego; `out/rom.bin` pesa 384 KB y muestra V0.3.61.

## Bitácora de trabajo - 18/11/2026 (con Bimo)
Todo lo siguiente se hizo el 18 de noviembre de 2026. Los detalles están en `CHANGELOG.md`.

**Documentación (18/11/2026)**
* Este `README.MD` mezclaba inglés y español. Ahora está completo en español, y `README.en.md` es la versión completa en inglés, con la misma estructura. GitHub no tiene un selector de idioma integrado para los README, así que ambos archivos empiezan con una línea (`Español | English`) que enlaza al otro.
* El contenido, los comandos, las rutas, los identificadores, las versiones y las fechas no cambian. Se restauraron algunos caracteres que una edición anterior había convertido en caracteres de control en la lista de la estructura del código (`bullets.c`, `enemies.c`, `title.c`, `boot/rom_head.c`, `reviveGameEntity`, `rowsOpposite`).

**Jefe: nuevo sprite y segundo disparo (18/11/2026)**
* Sprite temporal de Alberto: `res/tiles/imgboss.png`, 64x56 con 2 cuadros (el fuego de los cohetes parpadea). Usa su propia paleta en PAL2 durante la oleada del jefe y al terminar se restaura la del fondo. Al golpear el núcleo el cuerpo se tiñe de rojo, porque un destello blanco no se notaba sobre el color crema.
* Hitbox: solo el cuerpo central (24x40, desplazado 20,8) recibe daño; los cohetes bloquean los disparos. El recorrido vertical se redujo (Y 28 a 56) porque el sprite mide más del doble de alto.
* Segundo ataque: abanico de 3 balas diagonales dirigido hacia el jugador. Con más del 50% de vida sale al azar (1 de cada 4 disparos); con 50% o menos alterna siempre con la ráfaga normal. Para esto las balas enemigas ahora tienen velocidad horizontal (`bullets_shootEnemyXYV`). Los ajustes están en BOSS_FAN_* en inc/config.h.
* Pendiente: sprites propios para los disparos del jefe.

## Bitácora de trabajo - 19/11/2026 (con Bimo)
Todo lo siguiente se hizo el 19 de noviembre de 2026 (V0.3.67). Los detalles están en `CHANGELOG.md`.

**Jefe: el hueso reemplaza a la ráfaga (19/11/2026)**
* La ráfaga de balas de 8x8 que el jefe repartía bajo la panza ahora es un solo hueso de 48x16 (`res/tiles/imgbone.png`, arte de Alberto) que cae recto, centrado bajo el jefe (x = boss.x + 8, el mismo ancho que cubría la ráfaga). El abanico de 3 balas y la forma de alternarlos no cambian.
* Zona de golpe de 40x8 centrada en el dibujo (desplazamiento 4,4): rozar la orilla del hueso no destruye la nave.
* Cae 1 px/frame más lento que las balas enemigas de la oleada (mínimo 1), porque una barra sólida no deja huecos. Hay máximo 2 huesos en pantalla a la vez.
* El escudo NO detiene el hueso (decisión de Alberto): lo atraviesa y, si la zona de 40x8 toca la nave, cuesta una vida. Las balas normales y el abanico se siguen deteniendo con el escudo. Los disparos del jugador no lo afectan.
* Paleta: el hueso usa PAL1 con sus colores en los espacios libres 7-9 (el fondo solo usa 0-6), así que se ve con sus colores reales y el destello rojo del jefe (colores 1-3 de PAL2) no lo pinta.
* Se borra junto con las balas enemigas al cambiar de oleada, al morir el jefe, en GAME OVER y al reiniciar. Cada hueso usa 2 sprites de hardware y 12 tiles de VRAM.
* Pendiente: el sprite todavía dice "BONE"; se cambia por el definitivo con las mismas medidas.

**Hueso más lento (19/11/2026, V0.3.68)**
* Alberto lo probó y pidió bajarle la velocidad. `BOSS_BONE_SLOWER` pasa de 1 a 2: el hueso cae a `bulletSpeed - 2` px/frame (mínimo 1), o sea 3-5 px/frame con balas de 5-7 (antes 4-6). Zona de golpe, límite de 2 y escudo sin cambios.

**Revisión de dificultad (19/11/2026, V0.3.69)**
* A Alberto le costaba llegar a la oleada 10 y no le había ganado a La Cookie: en NORMAL todo subía cada 3 oleadas y para la 9 ya estaba al máximo (velocidad 3, 4 disparos cada 40 frames, bala 7, paso 16).
* Rampa más suave en las tres dificultades: velocidad, ráfaga, límite y velocidad de bala suben cada 4 oleadas (HARD: ráfaga cada 3 en vez de 2); el intervalo baja 5 / 6 / 3 frames por oleada en EASY / NORMAL / HARD (antes 10); el descenso sube cada 5 oleadas en NORMAL y cada 6 en HARD. En NORMAL la oleada 9 queda como la 6-7 de antes (3 / 4 / 72 / 7 / 12).
* La Cookie arranca con 350 HP (antes 500) y se sigue duplicando: 350, 700, 1400, 2800, 5600 y 8000 desde la oleada 60.
* Ciclos de 10 oleadas: la formación usa una oleada efectiva `pos + 3 * ciclo` (tope 40), así que después de cada jefe baja la dificultad: la 11 se juega como la 4, la 21 como la 7, la 31 como la 10. El jefe, sus HP, el texto OLEADA y el TOP 5 usan la oleada real. La tabla de NORMAL para las oleadas 1-12 está en `CHANGELOG.md` y en un comentario de `src/difficulty.c`.
* Pendiente: variantes de la oleada 11 (filas parejas o tercera fila).

**Tercera fila intercalada en las oleadas 11-19 (19/11/2026, V0.3.70)**
* Se revisó el presupuesto de sprites: hoy se reservan 48 de 80 sprites de hardware y 197 de 420 tiles; cada fila va en sus propias líneas (separación de 24 px), así que una fila más no empeora el límite de 20 sprites por línea. Con la tercera fila: 55 de 80 y 225 tiles.
* Alberto eligió para la 11 una fila intercalada de 4 naves alineada con las columnas 0, 2, 4 y 6, de modo que la lógica de disparo por columna no cambia. Va de la oleada 11 a la 19; las oleadas 1-9, las de jefe y la 21+ siguen con 2 filas (la fila completa de la 21 queda para después).
* `ENEMY_ROWS` pasa a 3 y `enemies_rowMaskForWave(wave, row)` define qué columnas tiene cada fila por oleada. Las naves fuera del acomodo quedan ocultas con vida 0 y no cuentan en el HUD ni para terminar la oleada.
* La formación entra durante el warp desde 104 px arriba (`ENEMY_WARP_ENTER`) para que la tercera fila no aparezca ya visible; el jefe sigue usando 80.
* Impacto: 18 naves en lugar de 14; la fila de abajo arranca 24 px más cerca, así que la invasión llega un par de pasos antes (en la 11 la velocidad es 1). El tope de balas no cambia.

**Segundo jefe: la tortuga hechicera en la oleada 20 (23/11/2026, V0.3.71)**
* Sprite de Alberto: una tortuga hechicera vista desde arriba (caparazón verde, túnica azul con estrellas, varita). Llegó como PNG RGBA de 64x64 con 8 colores y se convirtió a `res/tiles/imgboss2.png` indexado de 16 colores (color 0 transparente) sin tocar el dibujo. Es el jefe de la oleada 20, equivalente a La Cookie: misma lógica de HP y dificultad que la segunda aparición (700 HP, mismo daño, velocidad, intervalo y puntos).
* `boss.c` ahora describe cada jefe con una estructura `BossKind` (sprite, tamaño, zona de daño, límite del recorrido, bocas de los ataques, destello y flotación). La Cookie conserva sus valores de antes; la tortuga usa las constantes `BOSS2_*` de `inc/config.h`. Al iniciar la oleada se cambia la definición del mismo sprite (`SPR_setDefinition`), así que no se reserva VRAM extra fuera de la pelea.
* Zona de daño: 24x36 en el centro del caparazón (desplazamiento 20,8). Varita, patas y cabeza solo sacan chispas. El destello rojo tiñe los colores 1-3 de su paleta (verdes del caparazón y las patas).
* Disparo principal: una estrella de 16x16 (`res/tiles/imgstar.png`, con los dos amarillos de la tortuga) que sale de la punta de la varita (x + 50, y + 20). Mismas reglas que el hueso: cae `bulletSpeed - 2` px/frame (mínimo 1), máximo 2 en pantalla, el escudo no la detiene. Zona de golpe de 10x10 centrada. Usa PAL1 colores 10-11, después de los del hueso (7-9). El abanico de 3 balas sale de la cabeza (x + 28, y + 48).
* Animación: por ahora un solo cuadro, con una flotación de ±2 px (ciclo de 64 frames) encima del recorrido Lissajous, que baja hasta y = 48 para que el sprite de 64 px siga en la mitad superior. Si se agrega un segundo cuadro al PNG (128x64, dos cuadros de 8x8 tiles), se anima solo cada 16 frames.
* Para probarla: código del título (UP, UP, DOWN, DOWN, LEFT, RIGHT, B, A) y OLEADA INICIAL 20.

**La tortuga patrulla y se lanza en picada (23/11/2026, V0.3.72)**
* Alberto cambió el movimiento: sin recorrido Lissajous ni flotación. Patrulla de izquierda a derecha a `bossSpeed` px/frame (1/2/3) en y = 32 (`BOSS2_HOME_Y`) y rebota en las orillas.
* Picada: un temporizador al azar (EASY 240-420 frames, NORMAL 180-360, HARD 150-270) dispara un aviso de 36 frames (se detiene, tiembla 1 px y el caparazón parpadea en amarillo). Luego baja recto a 2 + `bossSpeed` px/frame (3/4/5), sale por abajo, reaparece justo arriba de la pantalla y baja a 2 px/frame hasta y = 32. El sprite está completo fuera de la pantalla en ambos extremos, así que el salto no se ve.
* Contacto: caja de 28x42 (desplazamiento 18,8; caparazón y cabeza) contra la nave durante la picada y el regreso. Cuesta una vida con `player_hit`, como una bala; el escudo no la detiene (`BOSS2_DIVE_SHIELD_BLOCKS` 0) y la invulnerabilidad tras reaparecer sí protege.
* No dispara mientras avisa, baja o regresa. El núcleo sigue recibiendo daño durante la picada. La Cookie no cambia.

**Tortuga en la oleada 10, picada más rápida y regreso al azar (23/11/2026, V0.3.73)**
* Alberto la probó: funciona bien pero se siente más fácil que La Cookie, así que se invierte el orden. Apariciones impares (oleadas 10, 30, 50...) son la tortuga y pares (20, 40, 60...) La Cookie (`BOSS2_ON_ODD` en `inc/config.h`). La vida sigue el número de aparición: tortuga 350 en la 10, Cookie 700 en la 20, tortuga 1400 en la 30.
* La picada era fácil de esquivar: el aviso baja de 36 a 20 frames (~0.33 s) y la picada sube de 3/4/5 a 5/6/7 px/frame (EASY/NORMAL/HARD). En NORMAL quedan ~38 frames desde el aviso hasta que el cuerpo llega a la nave; la nave necesita ~11 para salir de la caja de 28 px, así que se puede esquivar si reaccionas al parpadeo.
* Al salir por abajo reaparece arriba en una columna al azar (x 0-256) en lugar de la misma por donde salió, baja a y = 32 y sigue patrullando.

**Tortuga más rápida (23/11/2026, V0.3.74)**
* Alberto la sigue sintiendo mucho más fácil que La Cookie. Patrulla a `bossSpeed + 1` = 2/3/4 px/frame (`BOSS2_PATROL_EXTRA`), picada a 6/7/8 px/frame (`BOSS2_DIVE_SPEED_BASE` 5) e intervalos ~20% más cortos: EASY 192-336 frames, NORMAL 144-288, HARD 120-216.
* En NORMAL quedan ~36 frames desde el inicio del aviso hasta que el cuerpo llega a la nave (antes ~38); la nave necesita ~11 para salir de la caja de 28 px.

**Abanico de la tortuga más lento (23/11/2026, V0.3.75)**
* Alberto notó que el abanico de 3 balas era por mucho el ataque que más lo mataba. El de la tortuga ahora cae al 65% de `bulletSpeed + 1` (`BOSS2_FAN_SPEED_PCT`, mínimo 1): en sus oleadas la bala ya está en el tope de 7, así que pasa de 8 a 5 px/frame en EASY, NORMAL y HARD.
* La velocidad lateral no cambia, así que el abanico se abre un poco más y los huecos entre balas son más anchos. La Cookie sigue igual.

**Impulso con A + izquierda/derecha (23/11/2026, V0.3.76)**
* Idea de Alberto para el botón A, que no tenía uso en el juego. A + IZQUIERDA o A + DERECHA (en cualquier orden) lanza un impulso: 8 frames a 6 px/frame = 48 px, unas 3 naves, recortado a la orilla de la pantalla. Durante el impulso el control lateral no cuenta.
* Recarga: `shieldRechargeSec` de la dificultad (10 / 20 / 30 s), con su propio temporizador; usar el escudo no la gasta ni al revés. Si te pegan a media embestida la recarga empieza igual.
* Sin invulnerabilidad (decisión de diseño): sirve para salir del camino de una bala, del abanico o de la picada de la tortuga, no para atravesarlos.
* Estela: un segundo sprite de la nave 16 px detrás, visible cuadro sí cuadro no mientras dura el impulso (58 de 80 sprites de hardware). Reemplazada en V0.4.77 (ver abajo).
* HUD: "IMP" + barra de 8 tiles en la fila 27, columnas 18-28, entre las vidas/"+1 NAVE" y la barra RF; mismos colores que la del escudo (verde lista, azul en uso, roja recargando, parpadeo al quedar lista). El texto "+1 NAVE" ahora solo borra las columnas 10-17.
* No se puede usar durante el warp entre oleadas. Título, menú, código de oleada y captura de iniciales no cambian.

**Estela del impulso más visible y versión 0.4 (23/11/2026, V0.4.77)**
* Alberto notó que la estela casi no se veía. Ahora son 2 naves fantasma visibles todos los cuadros, en las posiciones de la nave de hace 2 y 4 frames (12 y 24 px detrás a velocidad de impulso). Al terminar se quedan 6 frames (`BOOST_TRAIL_LINGER`) y se deslizan hacia la nave; un golpe las borra al instante.
* Paleta fantasma en PAL0 colores 1-4 (la fuente solo usa 0 y 15): azules tipo escudo en vez del azul/naranja/blanco/negro de la nave. 59 de 80 sprites de hardware.
* El prefijo de versión sube a 0.4 (`run.bat` y `build.sh` escriben `V0.4.xx`); el último número sigue siendo el total de commits, así que esta es V0.4.77 (tag `v0.4.77`). La tortuga no cambia.

**El jefe suelta la cápsula RF (23/11/2026, V0.4.78)**
* Alberto notó que al vencer a la tortuga el RapidFire se encendía solo. Era a propósito desde la primera nave nodriza (también con La Cookie), pero se sentía como agarrar una cápsula sin haberla tocado. Ahora cualquier jefe suelta una cápsula "R" en su centro y hay que atraparla.
* Sin el 10% ni el límite de una en pantalla: siempre aparece (reemplaza a otra si la hubiera). Recortada a la pantalla y a Y 28-112, así que si la tortuga muere en plena picada la cápsula sale en la mitad de arriba.
* Sobrevive al warp hacia la siguiente oleada y se puede atrapar durante el warp; perder una vida antes la borra, como a cualquier cápsula.

**Cápsulas de escudo e impulso (23/11/2026, V0.4.79)**
* Idea de Alberto: además de la "R", pueden caer cápsulas que rellenan de golpe las barras del escudo y del impulso. La probabilidad de que caiga una cápsula no cambia (10%, garantizada tras 10 derribos sin cápsula, una en pantalla); solo se sortea el tipo, 1/3 cada uno (`CAPSULE_WEIGHT_RAPID/SHIELD/BOOST` en `inc/config.h`; 0 desactiva un tipo).
* "R" naranja/blanca: RapidFire, igual que antes. "E" azul/blanca (como la burbuja del escudo): la recarga del escudo termina al instante, con el sonido y el parpadeo de "listo". "I" verde/blanca (el verde de la barra IMP lista): la recarga del impulso termina al instante y la barra parpadea.
* Si la atrapas con el escudo encendido o a media embestida, el relleno queda guardado: al terminar, la barra queda lista sin recarga. Si la barra ya estaba lista, no pasa nada extra.
* Mismo sprite de 8x8 (1 sprite de hardware, 1 tile en VRAM): `imgcapsule.bmp` ahora tiene 3 filas de animación (R, E, I) de 2 cuadros; la "I" usa PAL3 para su verde. Los jefes siguen soltando siempre una "R".

**Puntos extra por cápsula E/I con la barra llena (23/11/2026, V0.4.80)**
* Idea de Alberto: en vez de desperdiciar una "E" o una "I" cuando su barra ya está llena (escudo o impulso listos, sin usarse), atraparla da puntos extra: lo que valen 5 naves en la dificultad actual (`CAPSULE_FULL_BONUS` 5 en `inc/config.h`): 50 en EASY, 100 en NORMAL y 150 en HARD.
* Aviso: suena el "ding" de disparo cargado listo en lugar del sonido de recoger, y aparece "+N" en la fila de abajo, en el lugar de "+1 NAVE", por ~1 s. Los puntos cuentan para la nave extra cada 1000; si completan una, se muestra "+1 NAVE".
* Si el escudo está encendido o vas a media embestida, se mantiene el relleno guardado (sin puntos), como en V0.4.79.
