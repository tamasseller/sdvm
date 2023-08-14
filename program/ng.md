FMT0 (1): OP(x, #)

    - lit s <- l

FMT1 (1): OP(x, y)

    - movr r <- r
    - uny s <- s : mov, neg, i2f, f2i, x1i, x1u, x2i, x2u

FMT2 (20): OP(x, y, #)

    - get r <- r.f
    - put r -> r.f
    - get s <- r.f
    - put s -> r.f
    - b{cond} s, s, +offs:  EqI, NeI, LtI, GtI, LeI, GeI, LtU, GtU, LeU, GeU, EqF, NeF, LtF, GtF, LeF, GeF,      
    
FMT3 (15): OP(x, y, z):

    - bop s <- s, s : AddI, MulI, SubI, DivI, Mod, ShlI, ShrI, ShrU, AndI, OrI, XorI, AddF, MulF, SubF, DivF,

FMT4 (1): OP(#):

    - jump +offs

FMT5 (2): OP(#N):

    - raise N
    - drop N

FMT6 (2): OP(#N, #M):

    - call N, M
    - ret N, M
