.syntax unified
.thumb

.global jit, startVm

.set vmTabOffsEnterNonLeaf, 0 << 2
.set vmTabOffsLeaveNonLeaf, 1 << 2
.set vmTabOffsResume,       2 << 2
.set vmTabOffsJit,          3 << 2

.set resumeOffset, 8
.set startOffset,  14

# Before:
#  - r0, r1: forwarded args
#  - r2: vmTab offset
# After.
#  - r0, r1: forwarded args (unchanged)
#  - r2: targetAddr
#  - r3: vmTab
#  - pc: targetAddr
vmDispatch:
    ldr r3, =vmTab
    ldr r3, [r3, r2]
    bx r3
vmTab:
    .long enterNonLeaf
    .long leaveNonLeaf
    .long resume
    .long jit

# Before
#   - r0:  orig lr (actual return address)
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
    rsbs r3, r3, 0
    add r3, r0

    # store caller offset and &fntab[callerIdx]
    push {r2, r3}

    # put &fntab[callee] in r11
    lsls r1, 2
    add r1, r10
    mov r11, r1

    #adjust lr to skip resume
    mov r0, lr
    adds r0, startOffset - resumeOffset
    bx r0


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
    mov lr, r1

    # jump into resume header
    ldr r0, [r0]
    adds r0, resumeOffset
    bx r0

# Before
#   - r0: resume offset
#   - lr: start of code
# After
#   - jump to correct offset
#
.thumb_func
resume:
    subs r0, startOffset
    add lr, r0
    bx lr

.macro jitdFuncHeader fnIdx
.thumb_func
Lentry\fnIdx:
    mov r0, lr
    movs r1, \fnIdx
    movs r2, vmTabOffsEnterNonLeaf
    blx r9

Lresume\fnIdx:
    mov r0, lr
    movs r2, vmTabOffsResume
    blx r9

Lstart\fnIdx:

.if Lresume\fnIdx - Lentry\fnIdx != resumeOffset
# || Lstart\fnIdx - Lentry\fnIdx != LstartOffset
.err
.endif
.endm

.macro jitdFuncFooter
.thumb_func
    movs r2, vmTabOffsLeaveNonLeaf
    blx r9

.endm

.macro jitdCall callerFnIdx calleeFnIdx
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
    adds r3, startOffset
    mov lr, r3

# Intenitional no break

jitEntry:
    mov r4, r0
    movs r5, 0
    b jitInvoke
    nop
jitResume:
    mov r4, r11
    movs r5, resumeOffset

jitInvoke:
    mov r0, r4

    push {lr}
    movs r2, vmTabOffsJit
    blx r9

    pop {r0}
    mov lr, r0

    ldr r0, [r4]
    add r0, r5
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
    jitdCall 0, 1
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
