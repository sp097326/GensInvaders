# Changelog - Gens Invaders

Registro de cambios del proyecto. Lo más reciente va arriba.
Detalle de la sesión de junio 2026 en `gensinvaders-session-log.html`.

## 2026-11-23 - Rampa de dificultad escalonada, un cambio por oleada (AMH + Bimo) - V0.4.82

### Cambiado
- Alberto sintió un salto muy grande de la oleada 4 a la 5. En NORMAL, en la
  5 subían juntas la velocidad (1 -> 2 px/frame, que además duplica el ritmo
  de bajada), la ráfaga y el tope de balas (2 -> 3) y la velocidad de bala
  (5 -> 6): ~60% más disparos por segundo. Todas compartían "cada 4
  oleadas"; en la 6 subía además la caída por rebote (8 -> 12).
- Ahora cada parámetro tiene su propia oleada de primer aumento
  (`speedFirst`, `shotsFirst`, `bulletsFirst`, `bulletSpeedFirst`,
  `stepFirst` en `waveTuneTable`) y luego sube cada 4 oleadas, así que
  ninguna oleada sube más de una cosa (el intervalo entre ráfagas sigue
  bajando poco a poco cada oleada):
  - NORMAL: velocidad 2 en la 5, ráfaga 3 en la 6, bala 6 en la 7, caída
    12 en la 8, velocidad 3 en la 9, ráfaga 4 en la 10, bala 7 en la 11,
    caída 16 en la 12.
  - EASY: velocidad en la 5 y 9, ráfaga/tope 2 en la 6, bala en la 7 y 11.
  - HARD: ráfaga 3 en la 4 y 4 en la 8, velocidad 3 en la 5, bala 7 en la
    6, caída 20 en la 7.
- Los máximos no cambian; se alcanzan un poco más tarde en los ciclos de
  10 oleadas (la oleada 9 en NORMAL queda 3 / 3 / 72 / 6 / 12 en vez de
  3 / 4 / 72 / 7 / 12). Solo cambia `src/difficulty.c`.

## 2026-11-23 - RapidFire y escudo activo se congelan durante el warp (AMH + Bimo) - V0.4.81

### Corregido
- Alberto notó que si atrapas una "R" justo al terminar la oleada, el
  tiempo de RapidFire seguía corriendo durante la animación de warp (~2.5 s,
  150 frames NTSC / 125 PAL), donde no se puede disparar, y se desperdiciaba.
  Ahora `powerups_update(frozen)` no descuenta el RapidFire mientras dura el
  warp (incluido el warp que lleva a una oleada de jefe); la barra RF se
  queda quieta y sigue al empezar la oleada. Una "R" atrapada durante el
  warp (por ejemplo la del jefe) empieza a contar al llegar la oleada.
- Por lo mismo, un escudo encendido al terminar la oleada también se
  congela: en el warp no hay balas enemigas, así que no sirve. La burbuja y
  el zumbido siguen mientras tanto.
- Las recargas (escudo e impulso) siguen corriendo durante el warp, como
  antes: ahí no se desperdicia nada y congelarlas sería un castigo. El
  impulso no se puede usar en el warp, así que no hay nada más que congelar.

## 2026-11-23 - Puntos extra por cápsula E/I con la barra llena (AMH + Bimo) - V0.4.80

### Cambiado
- Idea de Alberto: una "E" o una "I" atrapada con su barra ya llena
  (escudo listo sin usarse / impulso listo, sin relleno guardado) ya no se
  desperdicia: da `CAPSULE_FULL_BONUS` (5) naves de puntos según la
  dificultad: 50 en EASY, 100 en NORMAL, 150 en HARD (las naves valen
  10/20/30; una nave extra cada 1000).
- Aviso: suena el "ding" de disparo cargado listo (CH4) en vez del sonido
  de recoger, y aparece "+N" en la fila 27, en el lugar de "+1 NAVE", el
  mismo ~1 s (`hud_showBonus`). Si los puntos completan una nave extra se
  muestra "+1 NAVE".
- Con el escudo encendido o a media embestida sigue el relleno guardado de
  V0.4.79, sin puntos. La "R" y las cápsulas de jefe no cambian.

## 2026-11-23 - Cápsulas de escudo (E) e impulso (I) (AMH + Bimo) - V0.4.79

### Agregado
- Idea de Alberto: además de la cápsula "R" (RapidFire) pueden caer una
  "E" que termina al instante la recarga del escudo y una "I" que termina
  la del impulso. La probabilidad de que caiga cápsula al destruir una nave
  no cambia (10%, garantizada tras 10 derribos sin cápsula, una en
  pantalla); al soltarse, el tipo se sortea 1/3 cada uno
  (`CAPSULE_WEIGHT_RAPID/SHIELD/BOOST` en `inc/config.h`, 0 desactiva un
  tipo). Los jefes siempre sueltan "R".
- Colores: "R" naranja/blanca (PAL1, como antes), "E" azul/blanca (PAL1, el
  azul de la burbuja del escudo), "I" verde/blanca (PAL3, el verde de la
  barra IMP lista). Mismo sprite de 8x8 (1 sprite de hardware, 1 tile):
  `res/tiles/imgcapsule.bmp` pasa de 16x8 a 16x24, 3 filas de animación
  (R, E, I) con 2 cuadros de parpadeo; el tipo elige la fila y la paleta.
- Al atrapar "E": si el escudo recarga, queda listo (sonido de listo y
  parpadeo). Con el escudo encendido, el relleno se guarda y al apagarse
  queda listo sin recarga. Si ya estaba listo no pasa nada extra.
- Al atrapar "I": igual con el impulso (`player_refillBoost`): si recarga,
  queda listo y la barra parpadea; a media embestida se guarda y queda
  listo al terminar (también si te pegan). Las tres usan el sonido de
  recoger cápsula.

## 2026-11-23 - El jefe suelta la cápsula RF en vez de darla directo (AMH + Bimo) - V0.4.78

### Cambiado
- Alberto notó que al vencer a la tortuga el RapidFire se encendía solo,
  como si hubiera agarrado una cápsula. No era un error: desde la primera
  nave nodriza (d907f02) `boss_hitShot` llamaba `powerups_grantRapidFire()`
  al morir cualquier jefe (La Cookie igual). Ahora el jefe suelta una
  cápsula "R" en su centro y hay que atraparla.
- La cápsula del jefe no depende del 10% ni del límite de una en pantalla:
  siempre aparece y, si hubiera otra cayendo (en oleada de jefe no puede
  haber), la reemplaza. Reinicia el contador de lástima.
- Posición recortada a la pantalla y a Y 28-112 (`BOSS_CAPSULE_Y_MIN/MAX`):
  si la tortuga muere en plena picada (a la altura de la nave o fuera de
  pantalla), la cápsula sale en la mitad de arriba, con más de 1 s de caída
  antes de llegar a la nave.
- Sobrevive al warp hacia la siguiente oleada (`powerups_clearWaveCapsule`
  solo borra cápsulas de enemigos) y se puede atrapar durante el warp. Si
  pierdes una vida antes de atraparla se borra, igual que cualquier cápsula.
- `powerups_grantRapidFire` deja de ser pública. Las dos naves jefe
  (tortuga y La Cookie) usan la misma ruta. La tortuga no cambia en lo demás.

## 2026-11-23 - Estela del impulso más visible y salto a 0.4 (AMH + Bimo) - V0.4.77

### Cambiado
- Alberto notó que la estela del impulso casi no se veía (un solo sprite,
  cuadro sí cuadro no). Ahora son 2 naves fantasma visibles todos los
  cuadros, donde estaba la nave hace 2 y 4 frames: 12 y 24 px detrás a
  velocidad de impulso. Al terminar el impulso se quedan 6 frames
  (`BOOST_TRAIL_LINGER`) y se deslizan hacia la nave; una que queda encima
  de la nave se oculta. Un golpe a media embestida las borra al instante.
- Paleta fantasma: usan PAL0 colores 1-4 (la fuente de SGDK solo usa 0 y
  15) en tonos azules tipo escudo: azul 2060C0, azul claro 40A0FC, cian
  claro A0E0FC y contorno 002060, en vez de los colores normales de la nave.
- Sprites de hardware: 59 de 80 (uno más que V0.3.76).
- Constantes: `BOOST_GHOSTS`, `BOOST_GHOST_LAG1/2`, `BOOST_TRAIL_LINGER` y
  `BOOST_HIST` en `inc/config.h` (sale `BOOST_GHOST_LAG`).
- La tortuga no cambia.

### Versión
- El prefijo de versión pasa de 0.3 a 0.4 (jefes alternados tortuga/Cookie
  + impulso). `run.bat build` y `./build.sh build` ahora escriben
  `V0.4.xx`; el último número sigue siendo el total de commits (no se
  reinicia), así que esta versión es V0.4.77. Sin `version.h` el juego
  muestra `V0.4.DEV`. Tag `v0.4.77`.

## 2026-11-23 - Impulso con A + izquierda/derecha (AMH + Bimo) - V0.3.76

### Agregado
- Idea de Alberto para el botón A, que no tenía uso en el juego: A +
  IZQUIERDA o A + DERECHA hace un impulso (dash) para esquivar. La nave
  recorre 48 px (unas 3 naves) en 8 frames a 6 px/frame (~0.13 s; el
  movimiento normal es de 2), sin salirse de la pantalla. Funciona en
  cualquier orden (A con la dirección ya presionada o la dirección con A
  presionado). A solo no hace nada.
- Recarga: la misma duración que la recarga del escudo en cada dificultad
  (`shieldRechargeSec`): 10 s en EASY, 20 s en NORMAL y 30 s en HARD. Es un
  temporizador propio, independiente del escudo. Si te pegan a media
  embestida, la recarga empieza igual.
- Sin invulnerabilidad: el impulso solo te mueve; si una bala o la tortuga
  te tocan durante el impulso, pierdes la vida igual.
- Estela: un segundo sprite de la nave (1 sprite de hardware, 4 tiles)
  aparece 16 px detrás, cuadro sí cuadro no, mientras dura el impulso.
- HUD: barra "IMP" en la fila de abajo (columnas 18-28), con los mismos
  colores que la del escudo: verde lista, azul durante el impulso, roja
  mientras se recarga; parpadea al quedar lista.
- Bloqueado durante el warp entre oleadas, igual que los disparos y el
  escudo nuevo. El título, el menú, el código de oleada (que termina en B,
  A) y la captura de iniciales no cambian: el impulso solo se lee durante
  el juego.

### Cambiado
- El texto "+1 NAVE" ahora borra solo 8 columnas (10-17) para no tocar la
  barra IMP. Constantes `BOOST_*` en `inc/config.h`.

## 2026-11-23 - Abanico de la tortuga más lento (AMH + Bimo) - V0.3.75

### Cambiado
- Alberto: el abanico de 3 balas de la tortuga es por mucho el ataque que
  más mata. Ahora cae al 65% de la velocidad normal del abanico
  (`BOSS2_FAN_SPEED_PCT` en `inc/config.h`, mínimo 1 px/frame).
- En las oleadas de la tortuga (10, 30, 50...) la bala enemiga ya está en
  su tope de 7 px/frame en EASY, NORMAL y HARD, así que el abanico pasa de
  8 a 5 px/frame en las tres dificultades (37.5% más lento).
- La velocidad lateral del abanico no cambia (`BOSS_FAN_SPREAD` 2), así que
  se abre un poco más y deja huecos más anchos entre las balas.
- La Cookie no cambia (su abanico sigue a `bulletSpeed + 1`).

## 2026-11-23 - Tortuga más rápida (AMH + Bimo) - V0.3.74

### Cambiado
- Alberto: la tortuga sigue siendo mucho más fácil que La Cookie.
- Patrulla: `bossSpeed + 1` = 2/3/4 px/frame en EASY/NORMAL/HARD (antes
  1/2/3; `BOSS2_PATROL_EXTRA`). La Cookie no cambia.
- Picada: 5 + `bossSpeed` = 6/7/8 px/frame (antes 5/6/7).
- Intervalo al azar entre picadas ~20% más corto: EASY 192-336 frames
  (3.2-5.6 s), NORMAL 144-288 (2.4-4.8 s), HARD 120-216 (2-3.6 s); antes
  240-420, 180-360 y 150-270.
- Aviso sin cambios (20 frames). En NORMAL pasan ~36 frames (~0.6 s) desde
  que empieza el aviso hasta que el cuerpo llega a la nave (20 de aviso +
  110 px a 7 px/frame); la nave necesita ~11 frames para salir de la caja
  de 28 px, así que se sigue pudiendo esquivar.

## 2026-11-23 - Tortuga en la oleada 10, picada más rápida y regreso al azar (AMH + Bimo) - V0.3.73

### Cambiado
- Orden de jefes invertido (Alberto la sintió más fácil que La Cookie): la
  tortuga es el jefe de la oleada 10 y La Cookie el de la 20. Después se
  alternan: apariciones impares (10, 30, 50...) tortuga y pares (20, 40,
  60...) La Cookie (`BOSS2_ON_ODD`). La vida sigue el número de aparición:
  tortuga 350 en la 10, Cookie 700 en la 20, tortuga 1400 en la 30, Cookie
  2800 en la 40, etc.
- Aviso antes de la picada: 20 frames (~0.33 s) en lugar de 36.
- Picada más rápida: 4 + `bossSpeed` = 5/6/7 px/frame en EASY/NORMAL/HARD
  (antes 3/4/5). En NORMAL, desde que empieza el aviso hasta que el cuerpo
  llega a la altura de la nave pasan ~38 frames (~0.6 s); la nave se mueve
  2 px/frame y necesita ~11 frames para salir de la caja de 28 px.

### Agregado
- Al salir por abajo, la tortuga reaparece arriba en una columna al azar
  (0 a 256), no en la misma por donde salió; baja a su altura y sigue
  patrullando en la dirección que traía.

## 2026-11-23 - La tortuga patrulla de lado a lado y se lanza en picada (AMH + Bimo) - V0.3.72

### Cambiado
- Movimiento de la tortuga (oleada 20): ya no usa el recorrido Lissajous
  ni la flotación de ±2 px. Patrulla de izquierda a derecha a velocidad
  constante (`bossSpeed`: 1/2/3 px/frame en EASY/NORMAL/HARD) a una altura
  fija (`BOSS2_HOME_Y` = 32) y rebota en las orillas.
- La Cookie (oleadas 10, 30, 40...) no cambia.

### Agregado
- Picada: cada cierto tiempo al azar (EASY 4-7 s, NORMAL 3-6 s, HARD
  2.5-4.5 s; `BOSS2_DIVE_MIN_*` / `BOSS2_DIVE_MAX_*`) la tortuga avisa,
  se lanza recto hacia abajo desde su columna, atraviesa la pantalla, sale
  por abajo, reaparece por arriba y baja a 2 px/frame hasta su altura,
  donde retoma la patrulla.
- Aviso antes de lanzarse: se detiene 36 frames (~0.6 s), tiembla 1 px y
  el caparazón parpadea en amarillo (colores 1-3 de PAL2; un golpe al
  núcleo sigue mostrando el rojo).
- Velocidad de la picada: 2 + `bossSpeed` = 3/4/5 px/frame.
- Contacto: si el cuerpo de la tortuga (caparazón y cabeza, caja de 28x42
  en el desplazamiento 18,8) toca la nave durante la picada o el regreso,
  cuesta una vida (`player_hit`, igual que una bala). El escudo NO la
  detiene, igual que el hueso y la estrella (`BOSS2_DIVE_SHIELD_BLOCKS` 0;
  con 1 el escudo la haría inofensiva). La invulnerabilidad tras reaparecer
  sí protege. La tortuga sigue su camino después del golpe.
- Mientras avisa, baja o regresa no dispara estrellas ni abanico; el
  temporizador de disparo sigue solo durante la patrulla. Sigue recibiendo
  daño en el núcleo durante la picada (con la misma zona de 24x36).

### Notas de juego
- No empieza una picada mientras la nave está explotando o fuera de
  pantalla.
- La picada dura ~1 s (de y 32 a 224 a 4 px/frame en NORMAL) y el regreso
  ~0.8 s. Al regresar pasa un momento por encima de la barra JEFE.

## 2026-11-23 - Segundo jefe: la tortuga hechicera en la oleada 20 (AMH + Bimo) - V0.3.71

### Agregado
- Jefe de la oleada 20: la tortuga hechicera (sprite de Alberto,
  `res/tiles/imgboss2.png`, 64x64, 1 cuadro). Se convirtió de RGBA a PNG
  indexado de 16 colores con el color 0 transparente, sin tocar el dibujo
  (8 colores: 1-3 verdes del caparazón y las patas, 4 verde medio, 5 azul
  de la túnica, 6-7 amarillos, 8 ojos). Usa su propia paleta en PAL2, igual
  que La Cookie.
- Disparo principal: una estrella de 16x16 que sale de la punta de la
  varita (`res/tiles/imgstar.png`, amarillos de la tortuga, PAL1 espacios
  10-11, justo después de los del hueso). Misma mecánica que el hueso: cae
  recto `bulletSpeed - 2` px/frame (mínimo 1), máximo 2 en pantalla, el
  escudo NO la detiene y los disparos del jugador no la afectan. Zona de
  golpe de 10x10 centrada (desplazamiento 3,3). Cada estrella usa 1 sprite
  de hardware y 4 tiles de VRAM.
- Animación de flotar: la tortuga sube y baja ±2 px (ciclo de 64 frames)
  encima del recorrido Lissajous. Si después se agrega un segundo cuadro al
  PNG (8x8 tiles por cuadro), `placeSprite` lo anima solo, cada 16 frames.

### Cambiado
- `boss.c`: cada jefe se describe con una estructura `BossKind` (sprite,
  tamaño, zona de daño, límite inferior del recorrido, bocas del abanico y
  del disparo principal, cuadro del destello, flotación). La Cookie usa
  exactamente los valores de antes (`BOSS_*`); la tortuga usa `BOSS2_*` en
  `inc/config.h`. Al empezar una oleada de jefe se cambia la definición del
  mismo sprite con `SPR_setDefinition` (si fallara, se queda La Cookie).
- Tortuga: misma lógica de HP y dificultad que cualquier segunda aparición
  (700 HP, mismo daño, velocidad, intervalo y puntos). Zona de daño de
  24x36 en el centro del caparazón (desplazamiento 20,8); el resto del
  sprite (varita, patas, cabeza) solo saca chispas. Recorre y = 28-48
  (64 px de alto, sigue en la mitad superior). El abanico de 3 balas sale
  de la cabeza (x + 28, y + 48) con las mismas reglas que La Cookie. El
  destello rojo tiñe los colores 1-3 (verdes del caparazón y las patas);
  la túnica, la varita y las estrellas no cambian.
- Las oleadas 10, 30, 40... siguen con La Cookie sin cambios.

### Notas de juego
- El sprite toca el borde de abajo solo por un píxel suelto en (27,63);
  no afecta el juego.
- Presupuesto: 57 de 80 sprites de hardware (+2 estrellas) y unos 233 de
  420 tiles al arrancar; durante la oleada 20 la tortuga usa hasta 8 tiles
  más que La Cookie.

## 2026-11-19 - Tercera fila intercalada en las oleadas 11-19 (AMH + Bimo) - V0.3.70

### Agregado
- Oleadas 11 a 19: una tercera fila intercalada de 4 naves, alineada con
  las columnas 0, 2, 4 y 6 de las filas de arriba (18 naves en total en
  lugar de 14). Usa la misma nave y la paleta original (PAL2) por ahora;
  la fila de en medio sigue teñida (PAL3). Las oleadas 1-9, las de jefe y
  la 21 en adelante siguen con 2 filas (la fila completa de la 21 queda
  para después).
- `enemies_rowMaskForWave(wave, row)`: regresa la máscara de columnas de
  cada fila por oleada (`ENEMY_ROW_FULL` 0x7F, `ENEMY_ROW_STAGGER` 0x55).
  Para dar a la 21 una tercera fila completa basta con regresar 0x7F para
  la fila 2 en esas oleadas.

### Cambiado
- `ENEMY_ROWS` 2 -> 3 (`MAX_ENEMIES` 21). Los 21 sprites se crean al
  arrancar (55 de 80 sprites de hardware, 225 de 420 tiles de VRAM); solo
  se reviven y cuentan en el HUD las naves del acomodo de la oleada, y las
  demás quedan ocultas con vida 0, así que disparos, colisiones, invasión
  y fin de oleada las ignoran.
- Entrada de la formación durante el warp: `ENEMY_WARP_ENTER` = 104
  (32 + 3 x 24) en lugar de 80, para que la tercera fila también arranque
  fuera de la pantalla. El jefe sigue usando `WAVE_WARP_ENTER` (80).
- Sin cambios en el disparo: cada columna dispara desde su nave viva más
  baja, así que en las columnas 0, 2, 4 y 6 dispara primero la tercera
  fila. Con `rowsOpposite` la fila 2 se mueve como la fila 0 (empieza a la
  izquierda hacia la derecha) y rebota sola. Una fila no baja si quedaría
  a menos de 24 px de la fila viva de abajo, igual que antes.

### Notas de juego
- La fila de abajo empieza 24 px más cerca del jugador (y 80 contra 56),
  así que la invasión llega un par de pasos antes; en la 11 la formación
  va a velocidad 1 (oleada efectiva 4).
- El tope de balas enemigas no cambia; solo cambia de dónde salen.

## 2026-11-19 - Revisión de dificultad: rampa más suave, jefe con 350 HP y ciclos de 10 oleadas (AMH + Bimo) - V0.3.69

### Cambiado
- Rampa de las oleadas 1-9 más suave en las tres dificultades
  (`waveTuneTable` en `src/difficulty.c`): velocidad, tamaño de ráfaga,
  límite de balas y velocidad de bala suben cada 4 oleadas en vez de 3
  (en HARD la ráfaga sube cada 3 en vez de 2). El intervalo entre ráfagas
  baja más despacio: EASY 10 -> 5, NORMAL 10 -> 6 y HARD 10 -> 3 frames
  por oleada (mismos mínimos: 45 / 40 / 30). El descenso sube un paso
  más tarde: NORMAL cada 5 oleadas (antes 4) y HARD cada 6 (antes 5).
  En NORMAL, la oleada 9 queda como la 6-7 de antes (velocidad 3, 4
  disparos, intervalo 72, bala 7, paso 12; antes todo estaba al máximo:
  intervalo 40 y paso 16).
- La Cookie: 350 HP en su primera aparición (antes 500), unos 18 impactos
  al núcleo en NORMAL. Se sigue duplicando en cada regreso: 350, 700,
  1400, 2800, 5600 y 8000 desde la oleada 60 (`BOSS_HP_BASE`).
- Ciclos de 10 oleadas: después de cada jefe la dificultad de la
  formación baja y vuelve a subir. Se calcula una oleada efectiva
  `pos + 3 * ciclo` (ciclo = (oleada-1)/10, pos = (oleada-1)%10 + 1,
  tope 40): la 11 se juega como la 4, la 21 como la 7, la 31 como la 10.
  El número de aparición del jefe, sus HP, el texto OLEADA y el TOP 5 usan
  la oleada real; los datos del jefe (daño, velocidad, intervalo, puntos)
  no cambian. No hay tercera fila ni cambios en el movimiento (las
  variantes de la oleada 11 quedan para después).

### Tabla NORMAL (oleadas 1-12)

| Oleada | Efectiva | Velocidad | Disparos | Intervalo | Bala | Paso |
|---|---|---|---|---|---|---|
| 1 | 1 | 1 | 2 | 120 | 5 | 8 |
| 2 | 2 | 1 | 2 | 114 | 5 | 8 |
| 3 | 3 | 1 | 2 | 108 | 5 | 8 |
| 4 | 4 | 1 | 2 | 102 | 5 | 8 |
| 5 | 5 | 2 | 3 | 96 | 6 | 8 |
| 6 | 6 | 2 | 3 | 90 | 6 | 12 |
| 7 | 7 | 2 | 3 | 84 | 6 | 12 |
| 8 | 8 | 2 | 3 | 78 | 6 | 12 |
| 9 | 9 | 3 | 4 | 72 | 7 | 12 |
| 10 (jefe) | 10 | 3 | 4 | 66 | 7 | 12 |
| 11 | 4 | 1 | 2 | 102 | 5 | 8 |
| 12 | 5 | 2 | 3 | 96 | 6 | 8 |

## 2026-11-19 - Hueso del jefe más lento (AMH + Bimo) - V0.3.68

### Cambiado
- Tras probarlo, Alberto pidió bajar la velocidad del hueso:
  `BOSS_BONE_SLOWER` pasa de 1 a 2 en `inc/config.h`. Ahora cae a
  `bulletSpeed - 2` px/frame (mínimo 1), es decir 3-5 px/frame con balas
  de 5-7 (antes 4-6). Lo demás no cambia: zona de golpe de 40x8, máximo
  2 en pantalla y el escudo no lo detiene.

## 2026-11-19 - El jefe dispara un hueso en lugar de la ráfaga (AMH + Bimo) - V0.3.67

### Cambiado
- Ataque principal del jefe ("La Cookie"): la ráfaga de balas de 8x8
  repartidas bajo la panza (`fireVolley`) se reemplaza por un solo hueso
  de 48x16 (`fireBone`) que cae en línea recta, centrado bajo el jefe
  (x = boss.x + 8, el mismo ancho que cubría la ráfaga). El abanico de 3
  balas diagonales no cambia, ni la lógica que los alterna (al azar con
  más del 50% de vida, alternado con 50% o menos). El campo
  `bossShotsPerVolley` de la tabla de dificultad ya no lo usa el jefe.
- Zona de golpe del hueso más chica que el dibujo: 40x8 centrada dentro
  de los 48x16 (desplazamiento 4,4), para que rozar la orilla no destruya
  la nave.
- Cae 1 px/frame más lento que las balas enemigas de la oleada
  (`bulletSpeed - 1`, mínimo 1), porque una barra sólida de 48 px no deja
  huecos para pasar. Máximo 2 huesos en pantalla (`MAX_BOSS_BONES`); si
  ya hay 2, ese ataque no sale.
- El escudo (C) NO detiene el hueso: lo atraviesa y, si la zona de 40x8
  toca la nave, cuesta una vida igual que una bala (decisión de Alberto).
  Las balas normales de 8x8 se siguen deteniendo con el escudo. Los
  disparos del jugador no interactúan con el hueso (igual que con las
  balas enemigas). La invulnerabilidad tras reaparecer sí protege.
- Los huesos se borran junto con las balas enemigas: al cambiar de
  oleada (incluida la muerte del jefe), en GAME OVER / regreso al título
  y al reiniciar partida (`bullets_clearEnemy`, `bullets_hideAll`,
  `bullets_reset`).

### Agregado
- `res/tiles/imgbone.png`: sprite del hueso de Alberto (48x16, indexado,
  2 sprites de hardware y 12 tiles de VRAM por hueso). En
  `res/resources.res`: `SPRITE bossbone "tiles/imgbone.png" 6 2 FAST 0`.
- Paleta: el hueso usa PAL1 con sus 3 colores en los espacios libres 7-9
  (`space.bmp` solo usa 0-6 y ningún otro sprite usa 7-15). El PNG está
  indexado en 7-9 y `loadBonePal` copia esos colores a PAL1 al iniciar y
  en cada disparo. Así se ve con sus colores reales y no se pinta de rojo
  con el destello del jefe, que solo toca los colores 1-3 de PAL2.
- `inc/config.h`: `BOSS_BONE_W/H`, `BOSS_BONE_HIT_OX/OY/W/H`,
  `BOSS_BONE_SLOWER`, `MAX_BOSS_BONES`, `BOSS_BONE_PAL_FIRST/COUNT`.
- `bullets.c/.h`: `bullets_shootBone`, `bullets_boneAt`,
  `bullets_boneHitbox`, `bullets_despawnBone`.

### Corregido
- En la entrada del 18/11 de este CHANGELOG y en la bitácora de ambos
  README había letras convertidas en caracteres de control (`res/...`,
  `applyBossPal`, `boss_hide`, `bullets_shootEnemyXYV`, `velx`); se
  restauraron.

### Pendiente
- El sprite todavía dice "BONE": se verá así en el juego hasta que llegue
  el arte definitivo (mismas medidas, 48x16).

## 2026-11-18 - Nuevo sprite del jefe y segundo disparo en abanico (AMH + Bimo) - V0.3.66

### Cambiado
- Sprite del jefe (nave nodriza): nuevo arte temporal de Alberto,
  `res/tiles/imgboss.png` (indexado, 16 colores, 2 cuadros de 64x56; el
  segundo intercambia los tonos del fuego para que las llamas parpadeen).
  Se eliminó `res/tiles/imgboss.bmp.` En `res/resources.res`:
  SPRITE mothership "tiles/imgboss.png" 8 7 FAST 0.
- El jefe carga su propia paleta en PAL2 durante su oleada
  (`applyBossPal`); `boss_hide` regresa la paleta del fondo para la
  formación.
- Al recibir un golpe en el núcleo, el cuerpo se tiñe de rojo (colores
  33-35 a 0xFC6C6C) en lugar de blanco, porque el cuerpo crema hacía
  invisible el destello blanco.
- inc/config.h: BOSS_H 24 a 56; núcleo de 24x40 con desplazamiento
  (20, 8) (BOSS_CORE_H 40, BOSS_CORE_OY 8). Los cohetes bloquean los
  disparos sin hacer daño. Con el sprite más alto, el recorrido vertical
  queda entre Y 28 y 56 (centro 42).

### Agregado
- Segundo ataque del jefe: abanico de 3 balas en diagonal desde la parte
  inferior, un poco más rápidas que la ráfaga normal e inclinadas hacia
  el jugador (izquierda, centro o derecha). Con más del 50% de vida se
  elige al azar (1 de cada BOSS_FAN_CHANCE = 4 disparos); con 50% o
  menos alterna siempre con la ráfaga normal. Ajustes en config.h:
  BOSS_FAN_CHANCE, BOSS_FAN_SPREAD, BOSS_FAN_AIM_ZONE.
- `bullets_shootEnemyXYV`(x, y, vx, vy): bala enemiga con velocidad
  horizontal. Las balas enemigas ahora suman `velx` y desaparecen al
  salir por los bordes laterales; los disparos existentes usan `velx` = 0.

### Pendiente
- Sprites propios para los disparos del jefe (los está haciendo el
  equipo de Alberto); por ahora usan el sprite de bala enemiga actual.

## 2026-11-18 - README unificado en español y README.en.md en inglés (AMH + Bimo)

### Cambiado
- `README.MD`: todo el documento queda en español (antes mezclaba
  encabezados y texto en inglés, como "How to build", "Controls",
  "Change log" y "Work log"). Se conservan la estructura, el orden, los
  bloques de código, los comandos, las rutas, los identificadores, las
  versiones y las fechas.

### Agregado
- `README.en.md`: versión completa en inglés del mismo documento, con la
  misma estructura.
- Selector de idioma al inicio de ambos archivos: en `README.MD`
  `**Español** | [English](README.en.md)` y en `README.en.md`
  `[Español](README.MD) | **English**`. GitHub no tiene un selector de
  idioma integrado para los README, por eso se usan enlaces entre archivos.
- Entrada del 18/11/2026 en HISTORY y en la bitácora de trabajo de ambos
  README, y en `gensinvaders-session-log.html`.

### Corregido
- En la sección de estructura del código, una edición anterior había
  convertido algunas letras en caracteres de control (`\b`, `\e`, `\r`,
  `\t`), lo que dejaba `ullets.c`, `nemies.c`, `itle.c`, `oot/rom_head.c`
  y cortaba `reviveGameEntity` y `rowsOpposite` en dos líneas. Se
  restauraron en ambos README.

## 2026-11-17 - Comentario de inicialización en main.c (AMH + Bimo)

### Cambiado
- `src/main.c`: el comentario antes de `SYS_disableInts()` ahora explica
  que es el bloque de inicialización y que las interrupciones se apagan
  para evitar corrupción de los datos hacia la cola de DMA.

## 2026-11-16 - Compilación en macOS con build.sh (AMH + Bimo)

### Agregado
- `build.sh` en la raíz: equivalente de `run.bat` para macOS y Linux.
  `./build.sh build` escribe `src/version.h` igual que `run.bat`
  (`V0.3.<commits>`, mínimo 2 dígitos, terminado en CRLF), borra
  `out/src/*.o` y compila con el `makefile.gen` de SGDK 2.11.
  `./build.sh clean` limpia `out/` y `./build.sh run` abre
  `out/rom.bin`. Sin argumentos solo corre la ROM, como `run.bat`, y
  los comandos se pueden encadenar (`./build.sh clean build run`).
- Toolchain para Mac con Apple Silicon en
  `~/Desktop/retrodev/mars-sgdk211`: GCC 14.2 m68k-elf nativo arm64
  (compilado desde el marsdev de Alberto) más SGDK 2.11 (`libmd.a`,
  `libmd_debug.a`, `rescomp`, `xgmtool`, `sjasm`, `bintos`, `convsym`).
  Todo en carpetas del usuario, sin `sudo`. La ruta se puede cambiar
  con la variable `GENS_GDK`.
- Emulador: `run` usa BlastEm 1.0 (`brew install blastem`) y, si no
  está, Genesis Plus (`open -a "Genesis Plus"`).

### Notas
- Con GCC 14 el juego compila con 0 warnings y sin cambios en el código
  del juego. La ROM queda en `out/rom.bin` (384 KB) con V0.3.61.
- El Mars instalado en `/opt/toolchains/mars` (GCC 13.1, SGDK 2.00) es
  un binario Intel que depende de Homebrew en `/usr/local`, y no corre
  en esta Mac. No se modificó.

## 2026-11-13 - El disparo cargado ya no salta la pelea del jefe (AMH + Bimo)

### Corregido
- En la oleada del jefe las filas enemigas ya están ocultas, así que el
  contador de naves vale 0 durante toda la pelea. Al soltar un disparo
  cargado, una comprobación trataba ese 0 como "oleada limpia" y saltaba
  a la siguiente (el jefe desaparecía sin explosión ni bonus, y la barra
  no tenía que llegar a cero). Ahora esa oleada solo termina cuando la
  vida del jefe llega a 0, con la explosión, el bonus y luego el warp.
  Las oleadas normales no cambian: el disparo cargado sigue limpiando
  la formación y pasando de oleada al matar al último.

## 2026-11-13 - Jefe: daño visible y movimiento en la mitad superior (AMH + Bimo)

### Corregido
- El núcleo débil medía solo 16x16 (un cuarto del ancho de la nave) y
  estaba un poco recortado arriba, mientras el brillo visible era más
  ancho. Disparar al resplandor a menudo contaba como ala: la bala
  desaparecía, la vida no bajaba y no había destello ni sonido, así
  que parecía que el jefe no recibía daño o que la barra no se
  actualizaba. El área débil ahora cubre el tercio central y toda
  la altura del cuerpo (24x24).

### Cambiado
- Un golpe al núcleo hace parpadear la nave en blanco (solo la paleta
  del jefe, no la del jugador) y suena el tic de impacto, también con
  el disparo cargado. Un golpe en el ala suelta una chispa y no quita
  vida. La barra JEFE se redibuja en cada cambio de vida.
- El jefe ya no patrulla solo en horizontal a Y=40: se mueve en un
  patrón suave (Lissajous) por toda la mitad superior, más rápido
  según la dificultad, sin bajar de la mitad de la pantalla. Los
  disparos y la barra lo siguen.

## 2026-11-13 - Truco de oleada inicial en el título (AMH + Bimo)

### Añadido
- En la pantalla de título, la secuencia arriba, arriba, abajo, abajo,
  izquierda, derecha, B, A desbloquea un selector "OLEADA INICIAL"
  (1-99). El menú sigue funcionando mientras se introduce el código;
  si te equivocas o pasan unos 2 s entre teclas, se reinicia. Al
  acertarlo suena el jingle de 1UP y aparece "TRUCO ACTIVADO". El
  desbloqueo dura hasta apagar o resetear, no se guarda en la SRAM.
- Empezar en la oleada N aplica la dificultad de esa oleada (velocidad
  y disparo) y, si es 10, 20, 30..., el jefe con la vida que le
  corresponde. Si arrancas en la oleada 2 o posterior, el puntaje es
  de práctica y no entra al TOP 5 (en pausa dice PRACTICA y al terminar
  se avisa que no se guarda). Arrancar en la 1, aunque el truco esté
  desbloqueado, sí puede entrar a la tabla.

## 2026-11-13 - Jefe nodriza cada 10 oleadas (AMH + Bimo)

### Añadido
- En las oleadas 10, 20, 30... aparece una nave nodriza en lugar de las
  dos filas. Solo el núcleo central recibe daño (las alas detienen las
  balas chicas; el disparo cargado las atraviesa y golpea el núcleo una
  vez por pase). La vida empieza en 500 y se duplica en cada visita
  (500, 1000, 2000, 4000) hasta un tope de 8000 a partir de la oleada
  50, para que la barra y los cálculos en 16 bits no se desborden.
- El daño por bala al núcleo es 30 en FÁCIL, 20 en NORMAL y 10 en
  DIFÍCIL; el disparo cargado hace el triple. En dificultades más altas
  se mueve y dispara más (1, 2 o 4 disparos por andanada). Hay una
  barra "JEFE" de 16 teselas en la fila 2 (pasa a rojo bajo el 25%).
  Al derrotarla se otorgan 500/1000/1500 puntos por aparición (x2, x3...)
  y se recarga RapidFire. El gráfico es un placeholder: se puede
  cambiar después.

## 2026-11-13 - RapidFire como power-up de cápsula (AMH + Bimo)

### Añadido
- Al destruir un enemigo puede caer una cápsula "R" de 8x8 que parpadea
  y baja despacio. Al tocarla se activa RapidFire (hasta 6 balas chicas
  en pantalla). Si se sale de la pantalla, desaparece.

### Cambiado
- El botón A ya no activa RapidFire (queda libre). La nave se pinta de
  rojo mientras el efecto está activo. La duración es de 10 s en FÁCIL
  y 8 s en NORMAL y DIFÍCIL; recoger otra cápsula reinicia el tiempo.
  En la esquina inferior derecha aparece "RF" y una barra roja con el
  tiempo restante. El disparo cargado no cambia y sigue funcionando
  junto con RapidFire.
- Probabilidad de soltar la cápsula: 1 de cada 10 (10%), como máximo
  una en pantalla. Si pasan 10 bajas sin soltar ninguna, la siguiente
  está garantizada. No cae en la última nave de la oleada (la
  aceleración la borraría). El efecto se mantiene entre oleadas (el
  reloj sigue corriendo en la aceleración); una cápsula que aún caía
  sí se borra. En muerte, GAME OVER o título se pierden el efecto y
  la cápsula. El sonido de recoger suena en el canal 4 y no corta el
  zumbido del escudo.

## 2026-11-13 - Disparo cargado más fuerte y grave (AMH + Bimo)

### Cambiado
- El sonido al soltar la bala cargada es ahora un impacto más fuerte y
  grave (~0,32 s, alrededor de 70-280 Hz), claramente distinto del
  láser de la bala chica. El pico queda por debajo de la explosión del
  jugador para no recortar. El aviso de carga lista se volvió un poco
  más lleno (con un armónico grave), sin subir de más. Sigue en el
  canal 3, así que no corta el zumbido del escudo.

## 2026-11-13 - Disparo cargado (AMH + Bimo)

### Añadido
- Mantener oprimido el botón B unos 1 s carga un disparo. Al soltar,
  sale una bala de 24×24 (el triple de la normal) que atraviesa a todos
  los enemigos en su camino y suma los puntos de cada uno, hasta salir
  de la pantalla. Un toque corto sigue disparando la bala de 8×8 de
  siempre (el disparo ocurre al soltar).
- Mientras carga, un aura naranja rodea la nave; al estar lista parpadea
  en blanco y suena un aviso corto en el canal 4 (prioridad menor que el
  1UP, sin tocar el zumbido del escudo en el canal 1). El disparo
  cargado tiene su propio sonido, más grave, en el canal 3.
- Solo hay una bala cargada a la vez y no usa el tope de RapidFire
  (3 o 6 balas chicas). Si todavía hay una en pantalla, al soltar se
  dispara una bala normal. RapidFire sigue igual: más balas chicas,
  la carga funciona igual. En el warp entre oleadas no se puede
  cargar ni disparar (si había una carga, se cancela).

## 2026-11-13 - Zumbido del escudo mientras está activo (AMH + Bimo)

### Corregido
- El sonido al activar el escudo duraba solo 0,16 s (`shield_on.wav`),
  por eso parecía que se cortaba. Los disparos (canal 3), las explosiones
  (canal 2) y el 1UP (canal 4) no lo interrumpían: el corte era la
  propia muestra, demasiado corta.

### Añadido
- Un zumbido de 2 s (`res/sounds/shield_hum.wav`, id 71) en el canal 1,
  el mismo del escudo, que dura todo el tiempo que el escudo está
  activo (2 s en Difícil, 3 s en Normal, 5 s en Fácil; si dura más de
  2 s se relanza). Se detiene al acabarse el escudo, al pausar, al
  morir, en GAME OVER y al volver al título. Si el escudo sigue activo
  al reanudar o al reaparecer, el zumbido vuelve. Durante el warp entre
  oleadas el escudo sigue en pie, así que el sonido también. El aviso
  de escudo listo sigue en el canal 1, pero solo suena cuando el zumbido
  ya terminó.

## 2026-11-13 - Animación de aceleración entre oleadas (AMH + Bimo)

### Añadido
- Al limpiar una oleada hay una transición de unos 2,5 s (150 cuadros
  NTSC / 125 PAL). Las balas enemigas se quitan; el puntaje, las vidas,
  el escudo y el RapidFire se conservan. La nave sube un poco (8 px),
  enciende un motor de 8×8 (sprite nuevo de 2 fotogramas) y las
  estrellas se aceleran (de 2 a 14) y luego frenan. En la última parte
  las dos filas entran desde arriba, como si la nave se topara con ellas.
  El letrero `OLEADA N` permanece visible durante toda la transición.
- Durante el warp se puede mover la nave a los lados, pero no disparar
  ni activar el escudo (un escudo ya activo sigue y acompaña a la nave).
  Los temporizadores del escudo, el letrero de 1UP y los iconos de vidas
  siguen funcionando. La oleada 1 arranca sin esta animación.

## 2026-11-13 - Miniaturas de vidas abajo a la izquierda (AMH + Bimo)

### Cambiado
- Las miniaturas de naves pasan de la esquina superior derecha a la
  inferior izquierda (fila 27, píxeles 216-223, columnas 0-8). Esa fila
  queda justo debajo del escudo (que llega a Y=216) y no se solapa con
  la nave (Y=192-207), así que no hace falta subir al jugador. Siguen
  en el plano A, con prioridad alta: las balas enemigas y una invasión
  tardía en esa esquina pasan por detrás. El letrero `+1 NAVE` se mueve
  a la misma fila, a la derecha de los iconos, para no cargar más la
  parte de arriba. La esquina superior derecha del HUD queda libre.
  El icono nuevo sigue parpadeando al ganar una vida.

## 2026-11-13 - Iconos de naves en el HUD y letrero 1UP legible (AMH + Bimo)

### Corregido
- El letrero `+1 NAVE` se veía cortado y con líneas horizontales
  parpadeantes. La causa no era el límite de 20 sprites por línea: el
  texto ya estaba en el plano A, pero cada frame se borraba y
  redibujaba la fila 2 en plena imagen (escrituras a la VRAM fuera del
  vblank), y además tenía prioridad baja, así que las balas del jugador
  lo tapaban al subir. Ahora se dibuja una sola vez, se queda sólido
  unos 1 s (prioridad alta) y se borra al terminar.

### Cambiado
- El contador `SHIPS: n` se sustituye por hasta 9 miniaturas de la nave
  (tile 8×8 en el plano A, no sprites) en la esquina superior derecha
  (columnas 31-39). Empiezan a la izquierda de esa zona y crecen hacia
  la derecha; caben las 9 del tope, sin un «x9». Se actualizan al
  perder o ganar una vida. Con el 1UP, el icono nuevo parpadea mientras
  el letrero está visible.

## 2026-11-13 - Puntos por nave según la dificultad (AMH + Bimo)

### Cambiado
- El valor de cada nave destruida pasa a la tabla de dificultad
  (`enemyPoints`): Fácil 10, Normal 20, Difícil 30. Las dos filas valen
  lo mismo. Antes la fila de atrás daba 20 y la de adelante 10 en los
  tres niveles (210 puntos por oleada). No hay bonus de oleada ni otras
  fuentes de puntaje: solo las naves derribadas con bala (el choque de
  invasión sigue sin sumar).
- Una oleada completa (14 naves) vale 140 / 280 / 420. La vida extra
  sigue siendo cada 1000 puntos en las tres dificultades, así que el
  primer 1UP llega hacia la oleada 8 en Fácil (7 oleadas = 980), la 4
  en Normal (3 = 840) y la 3 en Difícil (2 = 840).
- El TOP 5 sigue siendo una sola tabla: los puntajes de Difícil
  crecerán más rápido. Los valores de ejemplo (GEN 1000, etc.) no se
  tocan.

## 2026-11-13 - Nave extra cada 1000 puntos (AMH + Bimo)

### Añadido
- Al llegar a 1000, 2000, 3000… puntos se otorga una nave extra (el umbral
  avanza aunque un solo disparo cruce varios múltiplos). El tope es 9
  (`LIVES_CAP`), para que `SHIPS: n` del HUD quepa en un dígito. Si ya se
  tiene el máximo, no se suma la vida pero el umbral sigue contando, así
  que no se “acumulan” 1UP pendientes.
- El umbral (`EXTRA_LIFE_SCORE` 1000) y el tope son los mismos en las tres
  dificultades y se reinician al empezar una partida.
- Un letrero `+1 NAVE` (o `+n NAVE` si se cruzan varios umbrales) parpadea
  unos 1 s en la fila 3, bajo el HUD, y se actualiza `SHIPS`. SFX corto de
  dos notas (`res/sounds/oneup.wav`, id 70) en el canal 4, para no cortar
  el láser ni las explosiones.
- No se otorga en GAME OVER, en la captura de iniciales ni si la muerte
  fue por invasión (choque o borde inferior). Si un 1UP llega durante la
  explosión de la última vida por una bala, se cancela el GAME OVER y la
  nave reaparece.

## 2026-11-13 - Tabla TOP 5 de puntuaciones (AMH + Bimo)

### Añadido
- Tabla de récords de 5 entradas, persistida en la SRAM del cartucho
  (firma `GIHS`, formato 1 y suma de comprobación). Si el bloque no es
  válido se cargan valores de ejemplo: GEN 1000, INV 800, UDL 600, AMH 400
  y BIM 200.
- Una sola tabla para todas las dificultades, con una columna F/N/D
  (Fácil / Normal / Difícil) en cada fila, más las iniciales, el puntaje
  y la oleada alcanzada. Así se comparan partidas de distinto nivel
  sin mantener tres listas.
- Tras GAME OVER, si el puntaje supera al 5.º (los empates no entran),
  se piden 3 iniciales al estilo arcade: arriba/abajo cambian la letra
  (A-Z, espacio y punto), izquierda/derecha mueven el cursor, A o START
  confirman. Después se muestra la tabla bajo el GAME OVER, con un `>`
  que parpadea en la nueva entrada. Si no clasifica, solo se ve la tabla.
  START sigue volviendo al título.
- En el título hay una tercera opción, `PUNTAJES (TOP 5)`. START o A la
  abre (el logo se queda); START o B regresan al menú.

### Notas
- La cabecera de la ROM ya declaraba SRAM (`RA`, 0xF820, 0x200000-0x20FFFF);
  solo se documentó. En BlastEm el archivo queda en
  `%LOCALAPPDATA%\blastem\rom\save.sram` (el ROM se lanza como `rom.bin`).
  Borrar ese archivo y volver a abrir el juego restaura los valores de
  ejemplo.

## 2026-11-13 - Oleadas infinitas y prefijo 0.3 (AMH + Bimo)



### Añadido

- Al eliminar las dos filas de una oleada el juego ya no termina con YOU WIN:

  aparece `OLEADA N` centrado unos 1,5 s (90 fotogramas en NTSC, 75 en PAL)

  y luego entran otras dos filas arriba. Se conservan puntuación, vidas y el

  estado del escudo / RapidFire. Las balas enemigas se limpian; las del

  jugador y las explosiones en curso siguen.

- El HUD muestra `OLEADA n` en la línea 2, a la derecha de `DIFF:` y antes

  de la barra `ESC`. También aparece en la pausa.

- Cada oleada escala una copia en tiempo de ejecución de la tabla de

  dificultad (la oleada 1 es idéntica a V0.2.45). Incrementos y topes:



  | | Intervalo (suelo) | Vel. horizontal (cada 3 oleadas) | Disparos / tope de balas | Vel. bala (cada 3) | Paso abajo |

  |---|---|---|---|---|---|

  | Fácil | -10 (mín. 45) | 1→3 | 1/1, 2/2 desde la oleada 4 (tope 2) | 5→7 | sigue en 0 |

  | Normal | -10 (mín. 40) | 1→3 | 2/2 → 4/4 cada 3 oleadas | 5→7 | 8→16 (+4 cada 4) |

  | Difícil | -10 (mín. 30) | 2→3 | 2→4 cada 2 oleadas (tope ya 4) | 6→7 | 16→20 (+4 cada 5) |



  En Fácil y Normal las filas siguen el mismo comportamiento de siempre

  (Fácil en bloque, Normal/Difícil en sentidos opuestos). El tope de 4

  balas enemigas es el del pool, para no pasar de 80 sprites ni de 20 por

  línea.



### Cambiado

- El prefijo de la versión pasa de `V0.2` a `V0.3`. El último número sigue

  siendo el conteo total de commits (`git rev-list --count HEAD`), no un

  conteo desde una etiqueta 0.3. Si no existe `src/version.h`, se muestra

  `V0.3.DEV`. `run.bat build` genera `V0.3.xx`.

- Las reglas de GAME OVER no cambian: choque con el jugador (después de las

  explosiones), llegar al borde inferior, o perder todas las vidas. El

  escudo sigue deteniendo solo las balas.



## 2026-11-12 (tarde 16) - Explosiones de naves enemigas (AMH + Bimo)

### Añadido
- Cada nave enemiga eliminada reproduce una explosión distinta a la del jugador:
  hoja de 6 fotogramas 16x16 (`res/tiles/imgenemyexp.bmp`, paleta de
  `space.bmp`), un destello compacto blanco/amarillo y chispas verdes que se
  dispersan. Se dibuja con PAL3 (verde/amarillo/blanco en partida) frente al
  fuego naranja/rojo de PAL1 del jugador. Dura 24 fotogramas (~0,4 s a 60 Hz),
  más corta y pequeña que la explosión del jugador (~0,8 s).
- La nave desaparece al instante y la explosión ocupa su lugar. No interviene
  en las colisiones ni en el movimiento de la formación.
- Hasta 6 explosiones simultáneas (`MAX_ENEMY_EXPLOSIONS`); si el pool está
  lleno se reutiliza la más antigua. Cada una es 1 sprite VDP (16x16). El pico
  queda en torno a 33 de 80 sprites; en una línea de barrido las naves muertas
  ya están ocultas, así que las explosiones no se suman a una fila completa y
  se mantienen bajo el límite de 20.
- En el choque con el jugador (o al alcanzar el borde inferior visible)
  explotan el enemigo (animación nueva) y el jugador (la suya); después GAME
  OVER, como antes. Si varios enemigos solapan al jugador en el mismo
  fotograma, todos explotan. No se otorga puntuación ni YOU WIN si el último
  enemigo cae de ese modo.
- SFX corto y agudo distinto (`res/sounds/enemyexp.wav`, id 69) en el canal 3,
  para que pueda sonar junto a la explosión del jugador en el canal 2.

## 2026-11-12 (tarde 15) - GAME OVER al chocar con el jugador (AMH + Bimo)

### Cambiado
- Ya no termina la partida solo porque una nave enemiga llega a la fila del
  jugador (`PLAYER_START_Y`), aunque no lo toque. El GAME OVER de invasión
  pasa a `collision`: si un enemigo vivo se solapa con la nave del jugador,
  se reproduce la explosión y después GAME OVER, aunque queden vidas. Es el
  comportamiento clásico de invasión, no se pierde solo una vida.
- El escudo y la invulnerabilidad del respawn no protegen de ese choque
  (sí siguen deteniendo las balas enemigas).
- Si las naves bajan más allá de la altura del jugador sin tocarlo, siguen
  marchando; cuando alguna alcanza el borde inferior visible (`BOTTOM_EDGE`,
  224), se aplica el mismo GAME OVER con explosión. Así no queda una partida
  atascada con enemigos debajo del cañón. No rebotan ni se detienen.

## 2026-11-12 (tarde 14) - Barra de escudo en el HUD (AMH + Bimo)

### Cambiado
- El indicador numérico del escudo (`ESC: LISTO` / `ESC: ON n` / `ESC: ns`)
  se sustituye por la etiqueta `ESC` y una barra de 8 teselas (64 niveles,
  octavos de bloque) a la derecha de la línea 2, sin tocar SCORE ni DIFF.
- En uso la barra azul (PAL1) se vacía con el tiempo que queda; en recarga
  se llena en rojo oscuro (PAL1); cuando está lista se ve llena y verde
  (color 1 de PAL3, el mismo verde de la fila inferior) y parpadea en azul
  un instante al terminar la recarga, junto con el SFX de siempre.
- El plano A solo se actualiza cuando cambia el nivel, el estado o el
  parpadeo. Las teselas se generan en código y se cargan a partir de
  `TILE_USER_INDEX + 2` (las dos primeras siguen siendo las del título).
  Usa colores ya presentes en PAL1/PAL3, sin paleta ni BMP nuevos.

## 2026-11-12 (tarde 13) - Salto a la versión 0.2 (AMH + Bimo)

### Cambiado
- El prefijo de la versión pasa de `V0.1` a `V0.2`. El último número sigue
  siendo el conteo total de commits (`git rev-list --count HEAD`), no se
  reinicia ni se cuenta desde una etiqueta, así que esta entrega es
  `V0.2.42`. Si no existe `src/version.h`, el juego muestra `V0.2.DEV`.
- `run.bat build` genera `V0.2.xx` sin el espacio de más que dejaba `echo`,
  y borra todos los `out\src\*.o` antes de compilar (varios módulos
  incluyen la versión a través de `game.h`). El título y la pausa la
  muestran igual que antes.
- Etiqueta anotada `v0.2.42` en este commit.

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
