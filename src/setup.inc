; ============================================================
;  setup.inc - petite fenetre de reglages avant la demo
;  resolution (liste deroulante), images/s (30/60), qualite, musique on/off,
;  fenetre/plein ecran, effets, boutons Run / Quit et bouton d'export MP4
;  (icone vectorielle).  Boite de dialogue construite en memoire.
;  coder : Ivandoe
; ============================================================
extern _DialogBoxIndirectParamA@20
extern _EndDialog@8
extern _SendDlgItemMessageA@20
extern _CheckDlgButton@12
extern _IsDlgButtonChecked@8
extern _AdjustWindowRect@12
extern _ChangeDisplaySettingsA@8
extern _GetClientRect@8
extern _IsWindow@4
extern _SetProcessDPIAware@0
extern _GetModuleHandleA@4
extern _LoadImageA@24
extern _MessageBoxA@16

%define ID_RES 101
%define ID_MUS 102
%define ID_WIN 103
%define ID_FUL 104
%define ID_FX  105
%define ID_EXP 106
%define ID_FPS 107
%define ID_QUA 108
%define NRES   6

; element de dialogue : style x y cx cy id classe texte
%macro DITEMO 6                ; element avec titre = ressource numerique (image)
 align 4
 dd %1, 0
 dw %2,%3,%4,%5,%6
 dw 0xFFFF,0x0082
 dw 0xFFFF,1
 dw 0
%endmacro
%macro DITEM 8
 align 4
 dd %1, 0
 dw %2,%3,%4,%5,%6
 dw 0xFFFF,%7
 dw __utf16__(%8),0
 dw 0
%endmacro

section .data
align 4
dlg_tpl:
 dd 0x80C808C0, 0               ; POPUP|CAPTION|SYSMENU|MODALFRAME|CENTER|SETFONT
 dw 16, 0,0,230,262
 dw 0,0
 dw __utf16__("Particle system — settings"),0
 dw 8
 dw __utf16__("MS Shell Dlg"),0
 DITEMO 0x5000000E, 8,4,213,108,  0xFFFF
 DITEM 0x50000000, 25,116,180,10,   0xFFFF,0x0082,"coder : Ivandoe — music : Ap3d€m@k"
 DITEM 0x50000000, 25,136,55,10,   0xFFFF,0x0082,"Resolution :"
 DITEM 0x50210003, 85,134,120,100, ID_RES,0x0085,""
 DITEM 0x50000000, 25,154,55,10,   0xFFFF,0x0082,"Frames/sec :"
 DITEM 0x50210003, 85,152,120,100, ID_FPS,0x0085,""
 DITEM 0x50000000, 25,172,55,10,   0xFFFF,0x0082,"Quality :"
 DITEM 0x50210003, 85,170,120,100, ID_QUA,0x0085,""
 DITEM 0x50010003, 25,192,80,10,   ID_MUS,0x0080,"Music (on / off)"
 DITEM 0x50010003, 115,192,100,10, ID_FX,0x0080,"Effects (shader)"
 DITEM 0x50030009, 25,206,80,10,   ID_WIN,0x0080,"Windowed"
 DITEM 0x50010009, 115,206,90,10,  ID_FUL,0x0080,"Fullscreen"
 DITEM 0x50010001, 25,228,48,16,   1,0x0080,"Run"
 DITEM 0x50010000, 80,228,48,16,   2,0x0080,"Quit"
 DITEM 0x50010040, 140,223,24,24,  ID_EXP,0x0080,""
 DITEM 0x50000000, 168,230,58,10,  0xFFFF,0x0082,"Export MP4"

res_names:
 db "Native resolution",0
 db "960 x 540",0
 db "1280 x 720",0
 db "1600 x 900",0
 db "1920 x 1080",0
 db "2560 x 1440",0
fps_names:
 db "30 fps",0
 db "60 fps",0
qua_names:
 db "Low",0
 db "Medium",0
 db "High",0
align 4
res_tab:                         ; largeur, hauteur (0 = natif)
 dd 0,0, 960,540, 1280,720, 1600,900, 1920,1080, 2560,1440
crf_tab dd 27,21,15              ; qualite x264 de l'export (selon la qualite choisie)
qsh_tab dd 2,1,0                 ; qualite d'affichage : 1 point sur 4 / 2 / tous
qps_tab dd 2.0,1.4142135,1.0     ; compensation de la taille des points
msg_noff db "ffmpeg.exe not found.",13,10,"Put ffmpeg.exe next to the exe (or in PATH).",13,10,"The music was saved as : klisje_musique.wav",0
msg_done db "Export finished : klisje.mp4",0
msg_ttl  db "Particle system",0

section .bss
alignb 4
cfg_res  resd 1                  ; index dans res_tab
cfg_mus  resd 1                  ; 1 = musique audible
cfg_fs   resd 1
cfg_fx   resd 1                  ; 1 = plein ecran
cfg_w    resd 1
cfg_h    resd 1
cfg_x    resd 1
cfg_y    resd 1
cfg_hwnd resd 1
dlg_h    resd 1
cfg_fps  resd 1                  ; 30 ou 60
cfg_q    resd 1                  ; 0 lav, 1 middels, 2 hoy
cfg_exp  resd 1                  ; 1 = export MP4 demande
q_sh     resd 1                  ; decalage : 1 point sur 2^q_sh est dessine
q_ps     resd 1                  ; facteur de taille des points (float)
rect     resd 4
devmode  resb 156

section .text
; remplit une liste deroulante : esi = chaines, ebx = nombre, edi = id
fillcombo:
.l:
 push esi
 push 0
 push 0x143                      ; CB_ADDSTRING
 push edi
 push dword [dlg_h]
 call _SendDlgItemMessageA@20
.s:
 cmp byte [esi],0
 lea esi,[esi+1]
 jne .s
 dec ebx
 jnz .l
 ret

; selectionne l element ecx de la liste edi
selcombo:
 push 0
 push ecx
 push 0x14E                      ; CB_SETCURSEL
 push edi
 push dword [dlg_h]
 call _SendDlgItemMessageA@20
 ret

; ---------- procedure de la boite de dialogue ----------
setup_proc:
 mov eax,[esp+8]
 cmp eax,0x110                   ; WM_INITDIALOG
 je .init
 cmp eax,0x111                   ; WM_COMMAND
 je .cmd
 cmp eax,0x10                    ; WM_CLOSE
 je .cancel
 xor eax,eax
 ret 16
.init:
 mov eax,[esp+4]
 mov [dlg_h],eax
 push esi
 push ebx
 push edi
 mov esi,res_names
 mov ebx,NRES
 mov edi,ID_RES
 call fillcombo
 xor ecx,ecx
 mov edi,ID_RES
 call selcombo
 mov esi,fps_names
 mov ebx,2
 mov edi,ID_FPS
 call fillcombo
 mov ecx,1
 mov edi,ID_FPS
 call selcombo
 mov esi,qua_names
 mov ebx,3
 mov edi,ID_QUA
 call fillcombo
 mov ecx,2
 mov edi,ID_QUA
 call selcombo
 push 1
 push ID_MUS
 push dword [dlg_h]
 call _CheckDlgButton@12
 push 1
 push ID_FUL
 push dword [dlg_h]
 call _CheckDlgButton@12
 push 1
 push ID_FX
 push dword [dlg_h]
 call _CheckDlgButton@12
 ; icone vectorielle du bouton d'export (ressource ICON groupe 2)
 push 0
 push 32
 push 32
 push 1                          ; IMAGE_ICON
 push 2
 push 0
 call _GetModuleHandleA@4
 push eax
 call _LoadImageA@24
 push eax                        ; lParam : HICON
 push 1                          ; wParam : IMAGE_ICON
 push 0xF7                       ; BM_SETIMAGE
 push ID_EXP
 push dword [dlg_h]
 call _SendDlgItemMessageA@20
 pop edi
 pop ebx
 pop esi
 mov eax,1
 ret 16
.cmd:
 mov eax,[esp+12]
 movzx eax,ax
 cmp eax,1                       ; IDOK = Run
 je .run
 cmp eax,ID_EXP                  ; bouton d'export MP4
 je .exp
 cmp eax,2                       ; IDCANCEL = Quit
 je .cancel
 xor eax,eax
 ret 16
.exp:
 mov dword [cfg_exp],1
.run:
 mov eax,[esp+4]
 mov [dlg_h],eax
 push 0
 push 0
 push 0x147                      ; CB_GETCURSEL
 push ID_RES
 push eax
 call _SendDlgItemMessageA@20
 mov [cfg_res],eax
 push 0
 push 0
 push 0x147
 push ID_FPS
 push dword [dlg_h]
 call _SendDlgItemMessageA@20
 mov ecx,30
 test eax,eax
 jz .f30
 mov ecx,60
.f30:
 mov [cfg_fps],ecx
 push 0
 push 0
 push 0x147
 push ID_QUA
 push dword [dlg_h]
 call _SendDlgItemMessageA@20
 mov [cfg_q],eax
 push ID_MUS
 push dword [dlg_h]
 call _IsDlgButtonChecked@8
 mov [cfg_mus],eax
 push ID_FUL
 push dword [dlg_h]
 call _IsDlgButtonChecked@8
 mov [cfg_fs],eax
 push ID_FX
 push dword [dlg_h]
 call _IsDlgButtonChecked@8
 mov [cfg_fx],eax
 push 1
 push dword [dlg_h]
 call _EndDialog@8
 mov eax,1
 ret 16
.cancel:
 push 0
 push dword [esp+8]
 call _EndDialog@8
 mov eax,1
 ret 16

; ---------- affiche le setup ; quitte si Quit ----------
setup_show:
 push 0
 call _GetModuleHandleA@4
 push 0
 push setup_proc
 push 0
 push dlg_tpl
 push eax
 call _DialogBoxIndirectParamA@20
 cmp eax,1
 je .ok
 push 0
 call _ExitProcess@4
.ok:
 mov ecx,[cfg_q]                 ; qualite d'affichage : pas de points + taille de compensation
 mov eax,[qsh_tab+ecx*4]
 mov [q_sh],eax
 mov eax,[qps_tab+ecx*4]
 mov [q_ps],eax
 cmp dword [cfg_exp],0
 je .fin
 mov dword [cfg_fs],0            ; export : fenetre sans bordure de la taille choisie, pas de changement de mode
 mov dword [cfg_mus],1
 cmp dword [cfg_res],0
 jne .fin
 mov dword [cfg_res],2           ; "natif" -> 1280 x 720
.fin:
 ret

; ---------- taille de la fenetre / mode d'affichage ----------
; sortie : cfg_w, cfg_h ; scr_w/scr_h = ecran natif
setup_mode:
 push 0
 call _GetSystemMetrics@4
 mov [scr_w],eax
 push 1
 call _GetSystemMetrics@4
 mov [scr_h],eax
 mov ecx,[cfg_res]
 mov eax,[res_tab+ecx*8]
 mov edx,[res_tab+ecx*8+4]
 test ecx,ecx
 jnz .set
 mov eax,[scr_w]                 ; natif
 mov edx,[scr_h]
 cmp dword [cfg_fs],0
 jne .set
 lea eax,[eax+eax*2]             ; fenetre : 3/4 de l ecran
 shr eax,2
 lea edx,[edx+edx*2]
 shr edx,2
.set:
 mov [cfg_w],eax
 mov [cfg_h],edx
 cmp dword [cfg_fs],0
 je .fin
 test ecx,ecx
 jz .fin
 mov word [devmode+36],156       ; plein ecran : on change le mode d affichage
 mov dword [devmode+40],0x1C0000      ; largeur | hauteur | profondeur
 mov dword [devmode+104],32
 mov [devmode+108],eax
 mov [devmode+112],edx
 push 4                          ; CDS_FULLSCREEN
 push devmode
 call _ChangeDisplaySettingsA@8
 test eax,eax
 jz .fin
 ; echec (mode non supporte par l ecran) : fenetre sans bordure de la taille choisie, centree sur l ecran
 mov eax,[scr_w]
 sub eax,[cfg_w]
 sar eax,1
 mov [cfg_x],eax
 mov eax,[scr_h]
 sub eax,[cfg_h]
 sar eax,1
 mov [cfg_y],eax
.fin:
 ret

%ifdef AVEC_MUTE_BUF                 ; optionnel : necessite BUFB (taille tampon) et pcm (tampons audio) de la demo
; ---------- coupe la musique : remplit le tampon ebx de silence ----------
mute_buf:
 cmp dword [cfg_mus],0
 jne .r
 mov edi,ebx
 imul edi,BUFB
 add edi,pcm
 mov ecx,BUFB/4
 xor eax,eax
 rep stosd
.r:
 ret
%endif
