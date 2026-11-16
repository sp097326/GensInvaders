#!/usr/bin/env bash
# Compila y ejecuta Gens Invaders en macOS/Linux (equivalente a run.bat).
# Uso: ./build.sh          -> solo corre out/rom.bin
#      ./build.sh build    -> genera src/version.h y compila con SGDK 2.11
#      ./build.sh run      -> abre out/rom.bin en el emulador
#      ./build.sh clean    -> borra out/ (limpieza completa de makefile.gen)
#      Se pueden encadenar: ./build.sh clean build run
# Toolchain: m68k-elf GCC + SGDK 2.11 en estilo Marsdev. Por defecto usa
# ~/Desktop/retrodev/mars-sgdk211/m68k-elf; se puede cambiar con GENS_GDK=/ruta/m68k-elf.
set -euo pipefail

cd "$(dirname "$0")"
GDK="${GENS_GDK:-$HOME/Desktop/retrodev/mars-sgdk211/m68k-elf}"
export GDK
export PATH="$GDK/bin:$PATH"

check_gdk() {
    if [ ! -f "$GDK/makefile.gen" ] || [ ! -f "$GDK/lib/libmd.a" ]; then
        echo "No encuentro SGDK en $GDK (falta makefile.gen o lib/libmd.a)." >&2
        echo "Define GENS_GDK con la ruta correcta." >&2
        exit 1
    fi
}

write_version() {
    # Version = V0.3.xx, xx = numero de commits del repo (2 digitos minimo), igual que run.bat.
    local build
    build="$(git rev-list --count HEAD 2>/dev/null || echo 0)"
    build="$(printf '%02d' "$build")"
    # Mismo contenido exacto que run.bat (terminado en CRLF).
    printf '#define GAME_VERSION "V0.3.%s"\r\n' "$build" > src/version.h
    echo "Version: V0.3.$build"
}

do_build() {
    check_gdk
    write_version
    # Varios modulos incluyen GAME_VERSION via game.h; hay que borrar todos los .o.
    rm -f out/src/*.o
    make -f "$GDK/makefile.gen"
}

do_clean() {
    check_gdk
    make -f "$GDK/makefile.gen" clean
}

do_run() {
    if [ ! -f out/rom.bin ]; then
        echo "No existe out/rom.bin. Corre: ./build.sh build" >&2
        exit 1
    fi
    local rom="$PWD/out/rom.bin"
    if command -v blastem >/dev/null 2>&1; then
        nohup blastem "$rom" >/dev/null 2>&1 &
    elif [ "$(uname -s)" = "Darwin" ] && open -Ra "Genesis Plus" 2>/dev/null; then
        open -a "Genesis Plus" "$rom"
    else
        echo "No encontre BlastEm ni Genesis Plus. Instala BlastEm (brew install blastem)." >&2
        exit 1
    fi
}

[ $# -eq 0 ] && set -- run
for cmd in "$@"; do
    case "$cmd" in
        build) do_build ;;
        clean) do_clean ;;
        run)   do_run ;;
        *) echo "Uso: $0 [build|clean|run] ..." >&2; exit 1 ;;
    esac
done
