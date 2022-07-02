  r0    r1    r2    r3    r4    r5    r6    r7      r8     r9    lr
callee  a0    a1    a2    -     -     -   caller  fnTab  vmTab   ret



s3     a3
s4     a4
s5     aN -> spIn
s6     l0       ^
s7     l1       |
sMax   lN       |
----   (r7)     |  frame size
----   (lr)     |  = smax+gap
s0     (a0)     |   
s1     (a1)     v
s2     (a2) <- sp
       (c3)
       (c4)
       (cN)

.balign 4
    nop
    add pc, lr
entry:     
    ldr r0, [r7]
    negs r0
    add lr, r0
    
/// if has non-tail (can be merged with frame setup)
    push {r7, lr}

xxxxx

/// if has non-tail
    pop {r7} (can be merged with frame setup)
    pop {r3}
    mov lr, r3
   
    ldr r3, [r7]
    subs r3, 2
    bx r3
    

