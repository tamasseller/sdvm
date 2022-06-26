#include "VmTab.h"

.syntax unified
.thumb

.global jit, startVm

.set VMTAB_SHIFT, 15

.set resumeOffset, 6
.set startOffset,  8

# Before:
#  - r0, r1: forwarded args
#  - r2: vmTab target offset
# After.
#  - r0, r1: forwarded args (unchanged)
#  - r2, r3: clobbered
#  - pc: target address
.balign 2
vmDispatch:
    mov r1, lr
    subs r1, 1
    ldrh r1, [r1]
    lsrs r2, r1, VMTAB_SHIFT
    lsls r2, 1
    add pc, r2
    nop
    b.n enterNonLeaf
    b.n leaveNonLeaf

# Before
#   - r1:  calleeIdx
#   - r11: &fntab[callerIdx]
# After
#   - stack[-2] = caller offset
#   - stack[-1] = &fntab[callerIdx]
#   - r11 &fntab[calleeIdx]
#   - jump to start of function (skip resume part of header)
#
.thumb_func
enterNonLeaf:
    # compute caller offset
    mov r2, r11
    ldr r3, [r2]
    adds r3, 10
    subs r3, r0, r3

    # store caller offset and &fntab[callerIdx]
    push {r2, r3}

    # put &fntab[callee] in r11
    lsls r1, (32 - VMTAB_SHIFT + 2)
    lsrs r1, (32 - VMTAB_SHIFT)
    add r1, r10
    mov r11, r1

    # adjust orig lr
    movs r1, 4
    add r1, lr
    bx r1

# Before
#   - stack[-2] = caller offset
#   - stack[-1] = &fntab[callerIdx]
#   - r11 &fntab[calleeIdx]
# After
#   - r11: &fntab[callerIdx]
#   - lr:  callerOffset
#   - jump to resume part of the caller's header
.thumb_func
leaveNonLeaf:
    # load and restore caller offset and &fntab[callerIdx] in r11 and lr
    pop {r0, r1}
    mov r11, r0

    ldr r0, [r0]
    adds r0, resumeOffset

    mov lr, r1
    bx r0

.macro jitdFuncHeader fnIdx
.balign 4
.thumb_func
entry\fnIdx:
    mov r0, lr
    blx r9
    .short (vmTabIdxEnterNonLeaf << VMTAB_SHIFT) | \fnIdx

Lresume\fnIdx:
    add pc, lr

Lstart\fnIdx:

.if Lresume\fnIdx - entry\fnIdx != resumeOffset
.err
.endif
.endm

.macro jitdFuncFooter
.thumb_func
    blx r9
    .short vmTabIdxLeaveNonLeaf << VMTAB_SHIFT

.endm

.macro jitdCall calleeFnIdx
.thumb_func
    movs r0, \calleeFnIdx << 2
    add r0, r10
    ldr r1, [r0]
    blx r1
.endm

# Before
#   - r0:  fnTab
#   - r1:  nFnTabEntries (must be at least number of functions + 1)
.thumb_func
startVm:
    # setup fnTab base register r10
    mov r10, r0

    # setup vmDispatch register
    ldr r2, =(vmDispatch + 1)
    mov r9, r2

    # fill fntab with jitEntry (except the last one)
    mov r2, r0
    ldr r3, =(jitEntry +1)

L0:
    subs r1, 1
    beq L1

    stmia r2!, {r3}
    b L0

L1:
    # fill last fntab entry with exitVm
    ldr r3, =(exitVm - resumeOffset + 1)
    str r3, [r2]

    # store last fntab entry as calle
    mov r11, r2

    # initialize lr value (for clarity)
    adds r3, 0
    mov lr, r3

# Intentional no break

jitEntry:
    movs r4, 0
    b jitInvoke
    nop

jitResume:
    mov r0, r11
    movs r4, resumeOffset

jitInvoke:
    mov r5, lr
    bl jit

    ldr r0, [r0]
    add r0, r4

    mov lr, r5
    bx r0
.if jitResume - jitEntry != resumeOffset
.err
.endif

exitVm:
    bkpt 1
    b exitVm

.pool

.global f, fEnd
.thumb_func
f:
jitdFuncHeader 0
    movs r1, 1
Lloop:
    push {r1}
    push {r1}
    jitdCall 1
    pop {r0, r1}
    adds r0, '0'
    ldr r2, =0x4000C000
    str r0, [r2]
    movs r0, '\n'
    str r0, [r2]
    adds r1, 1
    cmp r1, 4
    blo Lloop
jitdFuncFooter
.pool
fEnd:

.global g, gEnd
.thumb_func
g:
jitdFuncHeader 1
    ldr r0, [sp, 8]
    muls r0, r0, r0
    str r0, [sp, 8]

jitdFuncFooter
.pool
gEnd:
