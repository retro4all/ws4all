@ === void memcpy16(void *dst, const void *src, uint hwcount); =============
@ Reglist:
@  r0, r1: dst, src
@  r2, r4: hwcount
@  r3: tmp and data buffer
    .text
    .align  2
    .code   16
    .thumb_func
    .global memcpy16
memcpy16:
    push    {r4, lr}
    @ (1) under 5 hwords -> std cpy
    cmp     r2, #5
    bls     .Ltail_cpy16
    @ (2) Unreconcilable alignment -> std cpy
    @ if (dst^src)&2 -> alignment impossible
    mov     r3, r0
    eor     r3, r1
    lsl     r3, #31         @ (dst^src), bit 1 into carry
    bcs     .Ltail_cpy16    @ (dst^src)&2 : must copy by halfword
    @ (3) src and dst have same alignment -> word align
    lsl     r3, r0, #31
    bcc     .Lmain_cpy16    @ ~src&2 : already word aligned
    @ Aligning is necessary: copy 1 hword and align
        ldrh    r3, [r1]
        strh    r3, [r0]
        add     r0, #2
        add     r1, #2
        sub     r2, #1
    @ (4) Right, and for the REAL work, we're gonna use memcpy32
.Lmain_cpy16:
    lsl     r4, r2, #31
    lsr     r2, r2, #1
    ldr     r3,=memcpy32
    bl      _call_via_r3
    @ (5) NOTE: r0,r1 are altered by memcpy32, but in exactly the right 
    @ way, so we can use them as is.
    lsr     r2, r4, #31
    beq     .Lend_cpy16
    @ (6) Copy residuals by halfword
.Ltail_cpy16:
    sub     r2, #1
    bcc     .Lend_cpy16     @ r2 was 0, bug out
    lsl     r2, r2, #1      @ r2 is offset (Yes, we're copying backward)
.Lres_cpy16:
        ldrh    r3, [r1, r2]
        strh    r3, [r0, r2]
        sub     r2, r2, #2
        bcs     .Lres_cpy16
.Lend_cpy16:
    pop     {r4}
    pop     {r3}
    bx  r3
