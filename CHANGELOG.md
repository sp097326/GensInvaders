# Changelog - Gens Invaders

Registro de cambios del proyecto. Lo más reciente va arriba.
Detalle de la sesión de junio 2026 en `gensinvaders-session-log.html`.

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
