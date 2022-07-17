; A boot sector that boots a C kernel in 32 - bit protected mode
[org 0x7c00]
KERNEL_OFFSET equ 0x1000    ; memory offset of which to load the kernel

    mov [BOOT_DRIVE], dl        ; BIOS stores the boot drive in dl

    mov bp, 0x9000              ; setup the stack
    mov sp, bp

    mov bx, MSG_REAL_MODE       ; announce that we are starting booting
    call print_string           ; in 16 bit real mode

    call load_kernel            ; load kernel

    call switch_to_pm           ; switch to 32 bit protected mode


    jmp $


; include processes
%include "boot/print_string.asm"
%include "boot/print_hex.asm"
%include "boot/disk_load.asm"
%include "boot/gdt.asm"
%include "boot/print_string_pm.asm"
%include "boot/switch_to_pm.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL     ; print a message to say that we are loading the kernel
    call print_string

    mov bx, KERNEL_OFFSET       ; set up parameters for the disk_load routine , so
    mov dh, 15                  ; that we load the first 15 sectors ( excluding
    mov dl, [BOOT_DRIVE]        ; the boot sector ) from the boot disk ( i.e. the
    call disk_load              ; kernel code ) to address KERNEL_OFFSET

    ret

[bits 32]
; this is where we arrive after switching to and initializing protected mode
BEGIN_PM:
    mov ebx, MSG_PROT_MODE      ; print a message to announce
    call print_string_pm        ; we are in 32 bit protected mode

    call KERNEL_OFFSET          ; jump to the address of the loaded kernel code


    jmp $                       ; hang

; global variables
BOOT_DRIVE      db 0
MSG_REAL_MODE   db "Started in 16 bit real mode...", 0
MSG_LOAD_KERNEL db "Loading kernel...", 0
MSG_PROT_MODE   db "Entered 32 bit protected mode...", 0

; bootsector padding
times 510 - ($-$$ ) db 0
dw 0xaa55