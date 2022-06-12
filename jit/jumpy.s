.syntax unified
.thumb

.set resumeOffset, 16

.macro jitdFuncHeader fnIdx
.thumb_func
entry\fnIdx:
    mov r0, r11
    ldr r1, [r0]
    rsbs r1, r1, 0
    add lr, r1
# push can be omitted of there are no non-tail calls (with enough nops to keep offset).
.if 1
    push {r0, lr}
    movs r0, \fnIdx
    mov r11, r0
    b start\fnIdx
.else
    b start\fnIdx
    nop; nop; nop
.endif

resume\fnIdx:
    mov r0, lr
    subs r0, . - entry\fnIdx
    add pc, r0

start\fnIdx:

.if resume\fnIdx - entry\fnIdx != resumeOffset
.err
.endif
.endm

.macro jitdFuncFooter
.thumb_func
# pop can be omitted of there are no non-tail calls (with enough nops to keep offset).
.if 1
    pop {r0, r1}
    mov lr, r1
.else
    mov r0, r11
.endif
    ldr r0, [r0]
    adds r0, resumeOffset
    bx r0
.endm

.macro jitdCall callerFnIdx calleeFnIdx
.thumb_func
    movs r0, \calleeFnIdx
    add r0, r12
    ldr r1, [r0]
    bx r1
.endm

.global jit

.global jitEntry
.thumb_func
jitEntry:
    mov r4, r0
    movs r5, 0
    b jitInvoke
    nop;nop;nop;nop;nop
jitResume:
    mov r4, r11
    movs r5, resumeOffset

jitInvoke:
    mov r0, r4

    push {lr}
    blx jit
    pop {r0}
    mov lr, r0

    ldr r0, [r4]
    add r0, r5
    bx r0
.if jitResume - jitEntry != resumeOffset
.err
.endif

.thumb_func
f:
jitdFuncHeader 0
movs r0, 123
push {r0}
jitdCall 13, 69
jitdFuncFooter

.thumb_func
g:
jitdFuncHeader 1
pop {r0}
muls r0, r0, r0
push {r0}
jitdFuncFooter

