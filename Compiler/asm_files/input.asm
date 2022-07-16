begin 4096
st: ldx  numb
	lda  fact
	comp two
	jlt	 e
loop: mul numb
	sub fact
	comp one
	jeq loop
rsub
one:  dw 1
two:  dw 2
numb: dw 1
fact: dw 5
e: rsub
end st