; exemple minimal : affiche la petite fenetre de reglages puis montre le choix fait.
; coder : Ivandoe
bits 32
extern _ExitProcess@4
extern _GetSystemMetrics@4
extern _MessageBoxA@16
global _start
section .data
msg db "Reglages recuperes (cfg_w, cfg_h, cfg_fps, cfg_q, cfg_mus, cfg_fs, cfg_fx, cfg_exp).",0
ttl db "exemple",0
section .bss
scr_w resd 1                     ; requis par setup_mode (ecran natif)
scr_h resd 1
%include "setup.inc"
section .text
_start:
 call setup_show                 ; fenetre ; quitte le process si Avslutt
 call setup_mode                 ; remplit cfg_w/cfg_h (+ changement de mode si plein ecran)
 push 0
 push ttl
 push msg
 push 0
 call _MessageBoxA@16
 push 0
 call _ExitProcess@4
