	addi	x1	x0	#3	X1 <- X0 + 3		n<-3
	addi	x3	x0	#4	X3 <- X0 + 4		X3<-4
	addi	x4	x0	#8	X4 <- X0 + 8		X3<-8
	addi	x5	x0	#12	X5 <- X0 + 12		X3<-12
loop	beq	x1	x0	done		Have we run n times
	addi	x1	x1	#-1	X1  <- X1 - 1		X1--
	flw	f0	x2	a	FO  <- X2(a)		F0  <- a[n]
	flw	f8	x2	b	F8  <- X2(b)		F8  <- b[n]
	flw	f16	x2	c	F16 < X2(c)		F16 <- c[n]
	flw	f2	x3	a	F2  <- X3(a)		F2  <- a[n+1]
	flw	f10	x3	b	F8  <- X3(b)		F10 <- b[n+1]
	flw	f18	x3	c	F18 <- X3(c)		F18 <- c[n+1]
	flw	f4	x4	a	F4  <- X4(a)		F4  <- a[n+2]
	flw	f12	x4	b	F8  <- X4(b)		F12 <- b[n+2]
	flw	f20	x4	c	F20 <- X4(c)		F20 <- c[n+2]
	flw	f6	x5	a	F6  <- X5(a)		F6  <- a[n+3]
	flw	f14	x5	b	F8  <- X5(b)		F14 <- b[n+3]
	flw	f22	x5	c	F22 <- X5(c)		F22 <- c[n+3]
	fmult.s	f0	f0	f8 	F0  <- F0 * F8		F0  <- a[n]   * b[n]
	fmult.s	f2	f2	f10	F2  <- F2 * F10		F2  <- a[n+1] * b[n+1] 
	fmult.s	f4	f4	f12	F4  <- F4 * F12		F4  <- a[n+1] * b[n+1] 
	fmult.s	f6	f6	f14	F6  <- F6 * F14		F6  <- a[n+1] * b[n+1] 
	fadd.s	f0	f0	f16	F0  <- F0 + F16		F0  <- a[n]   + b[n] 
	fadd.s	f2	f2	f18	F2  <- F2 + F18		F2  <- a[n+1] + b[n+1]
	fadd.s	f4	f4	f20	F4  <- F4 + F20		F4  <- a[n+2] + b[n+2]
	fadd.s	f6	f6	f22	F6  <- F6 + F22		F6  <- a[n+3] + b[n+3]
	fsw	f0	x2	answer	answer+X2 = [F0]
	fsw	f2	x3	answer	answer+X3 = [F2]
	fsw	f4	x4	answer	answer+X4 = [F4] 
	fsw	f6	x5	answer	answer+X5 = [F6] 
	addi	x2	x2	#16 X2  <- X2 + 4
	addi	x3	x3	#16 X3  <- X3 + 4
	addi	x4	x4	#16 X4  <- X4 + 4
	addi	x5	x5	#16	X5	<- X5 + 4
	jal	x0	loop
done	halt
a	.df	0.01
	.df	0.02
	.df	0.03
	.df	0.04
	.df	0.05
	.df	0.06
	.df	0.07
	.df	0.08
	.df	0.09
	.df	0.10
	.df	0.11
	.df	0.12
b	.df	1
	.df	2
	.df	3
	.df	4
	.df	5
	.df	6
	.df	7
	.df	8
	.df	9
	.df	10
	.df	11
	.df	12
c	.df	100
	.df	101
	.df	102
	.df	103
	.df	104
	.df	105
	.df	106
	.df	107
	.df	108
	.df	109
	.df	110
	.df	111
answer	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
	.df	0
