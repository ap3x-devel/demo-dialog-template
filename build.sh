#!/bin/sh
# compile l'exemple (nasm + ld MinGW/binutils, sans Python)
set -e
cd "$(dirname "$0")"
nasm -f win32 -Ox -I src/ src/exemple.asm -o build/exemple.obj
L=libs/imp32
ld -m i386pe -e _start --subsystem windows -s build/exemple.obj res/icon.o $L/kernel32.lib $L/user32.lib -o bin/exemple.exe
ls -la bin/exemple.exe
