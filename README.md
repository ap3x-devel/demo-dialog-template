# Demo setup window (NASM, Win32)

A small, reusable settings window for 32-bit Windows demos written in NASM.
It is built in memory as a dialog template (no `.rc` file, no resource compiler) and needs no Python:
only `nasm` and `ld` (MinGW / binutils) are used to build it.

Coder: **Ivandoe**

## Features

- Resolution combo box (native, 960x540, 1280x720, 1600x900, 1920x1080, 2560x1440)
- Frame rate: 30 / 60 fps
- Quality: Low / Medium / High (point step and size compensation, x264 CRF for export)
- Music on/off, shader effects on/off
- Windowed / fullscreen (real display-mode change, with borderless fallback if refused)
- Run / Quit buttons
- Vector-icon button **Export MP4** (sets a flag so the demo can render frame by frame)
- Logo bitmap at the top

## Layout

```
src/setup.inc      the whole window (dialog template, handlers, display mode)
src/exemple.asm    minimal program using it
res/               logo.bin, icon_1..7.bin, icon.o (prebuilt COFF .rsrc), export.svg
tools/             mkrsrc.c, mkicon.c, mkexp.c, kernel32.def, user32.def
libs/imp32/        import libraries (kernel32, user32, gdi32)
build.sh           builds the example
```

## Build the example

```sh
sh build.sh        # nasm -f win32 + ld -m i386pe  ->  bin/exemple.exe
```

## Use it in your own demo

1. Copy `src/setup.inc` and `res/icon.o`, and link `icon.o`.
2. In your `.asm`:

```nasm
section .bss
scr_w resd 1            ; native screen size, filled by setup_mode
scr_h resd 1
%include "setup.inc"

section .text
_start:
 call setup_show        ; shows the window, exits the process on "Quit"
 call setup_mode        ; computes cfg_w/cfg_h, switches display mode if fullscreen
 ; ... create your window / GL context with cfg_w, cfg_h
```

3. Read the results:

| Variable | Meaning |
|---|---|
| `cfg_w`, `cfg_h` | window size |
| `cfg_fps` | 30 or 60 |
| `cfg_q` | 0 Low, 1 Medium, 2 High |
| `q_sh`, `q_ps` | point step (1 of 2^q_sh) and size compensation (float), derived from `cfg_q` |
| `cfg_mus` | music enabled |
| `cfg_fs` | fullscreen |
| `cfg_fx` | effects enabled |
| `cfg_exp` | 1 if "Export MP4" was clicked (window forced to 0,0 borderless, music on) |
| `cfg_x`, `cfg_y` | fallback position when the fullscreen mode is refused |
| `crf_tab` | x264 CRF per quality level (for an ffmpeg export) |

4. Link against `kernel32`, `user32` (and what your demo needs). If an import is missing from a `.lib`,
   add it to `tools/*.def` and regenerate:

```sh
llvm-dlltool -m i386 -k -d user32.def -l user32.lib
```

`mute_buf` (silences an audio buffer) is disabled by default; define `AVEC_MUTE_BUF` and provide
`BUFB` and `pcm` to use it.

## Customize

- Texts and lists (`res_names`, `fps_names`, `qua_names`, `res_tab`) are at the top of `setup.inc`.
- Logo: replace `res/logo.bin` (320x175 bitmap, no file header), then regenerate the resource object:

```sh
gcc tools/mkrsrc.c -o mkrsrc && ./mkrsrc res     # rewrites res/icon.o
```

- Exe icon sizes: `res/icon_N.bin` (see `tools/mkicon.c`); export button icon: `res/icon_7.bin` (`tools/mkexp.c`, `res/export.svg`).

## Status

Verified by compilation only (`nasm` + `ld`). The window has not been run on a real Windows machine yet.
