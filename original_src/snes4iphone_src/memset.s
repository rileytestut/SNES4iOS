		.text
		.global gp2x_memset;
		.type gp2x_memset,%function
		.align 4;

gp2x_memset:
	mov	a4, a1
	cmp	a3, $8		@ at least 8 bytes to do?
	blt	2f
	orr	a2, a2, a2, lsl $8
	orr	a2, a2, a2, lsl $16
1:
	tst	a4, $3		@ aligned yet?
	strneb	a2, [a4], $1
	subne	a3, a3, $1
	bne	1b
	mov	ip, a2
1:
	cmp	a3, $8		@ 8 bytes still to do?
	blt	2f
	stmia	a4!, {a2, ip}
	sub	a3, a3, $8
	cmp	a3, $8		@ 8 bytes still to do?
	blt	2f
	stmia	a4!, {a2, ip}
	sub	a3, a3, $8
	cmp	a3, $8		@ 8 bytes still to do?
	blt	2f
	stmia	a4!, {a2, ip}
	sub	a3, a3, $8
	cmp	a3, $8		@ 8 bytes still to do?
	stmgeia	a4!, {a2, ip}
	subge	a3, a3, $8
	bge	1b
2:
	movs	a3, a3		@ anything left?
	moveq	pc, lr		@ nope
	rsb	a3, a3, $7
	add	pc, pc, a3, lsl $2
	mov	r0, r0
	strb	a2, [a4], $1
	strb	a2, [a4], $1
	strb	a2, [a4], $1
	strb	a2, [a4], $1
	strb	a2, [a4], $1
	strb	a2, [a4], $1
	strb	a2, [a4], $1
	mov	pc, lr

.size gp2x_memset,.-gp2x_memset;

