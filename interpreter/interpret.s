.cpu cortex-m0
.thumb

.balign 4
.thumb_func
doBinary:
    lsl r2, #2
    add pc, r2

    add r0, r1 // Add
    bx lr

    sub r0, r1 // Sub
    bx lr

    lsl r0, r1 // Lsh
    bx lr

    lsr r0, r1 // Rsh
    bx lr

    asr r0, r1 // Ash
    bx lr

    and r0, r1 // And
    bx lr

    orr r0, r1 // Ior
    bx lr

    eor r0, r1 // Xor
    bx lr

    mul r0, r1 // Mul
    bx lr

    bkpt 1     // Div
    bx lr

    bkpt 1     // Mod
    bx lr

.balign 4
.thumb_func
evaluateCondition:
    lsl r2, #2
    cmp r0, r1
    nop
    add pc, r2

    beq retFalse  // Equal
    b retTrue

    bne retFalse  // NotEqual
    b retTrue

    bhi retFalse  // UnsignedGreater
    b retTrue

    bls retFalse  // UnsignedNotGreater
    b retTrue

    blo retFalse  // UnsignedLess
    b retTrue

    bhs retFalse  // UnsignedNotLess
    b retTrue

    bgt retFalse  // SignedGreater
    b retTrue

    ble retFalse  // SignedNotGreater
    b retTrue

    blt retFalse  // SignedLess
    b retTrue

    bge retFalse  // SignedNotLess
    b retTrue

retFalse:
    movs r0, #0
    bx lr
retTrue:
    movs r0, #1
    bx lr

.thumb_func
interpret:
    bx lr
