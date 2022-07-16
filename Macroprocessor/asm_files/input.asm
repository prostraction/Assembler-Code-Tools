begin 4096
st: ldx  numb ; загрузить в регистр x
	lda  fact 	  ; загрузить в регистр a
	comp two  	  ; сравнить fact с 2
	jlt	 e	  ; если fact < 2, завершаем программу
loop: mul numb	; умножим numb на fact
	sub fact	; уменьшим количество итераций цикла
	comp one	; 
	jeq loop  	; если fact > 1, повторим итерацию
rsub
one:  dw 1	    ; константа единицы
two:  dw 2	    ; константа 2
numb: dw 1	    ; текущее значение факториала
fact: dw 5	    ; необходимый факториал (кол-во итераций цикла)
e: rsub		    ; метка конца программы
end st
=======
COPY: START 0
RDBUFF: MACRO &INDEV, &BUFADR, &RECLTH
	CLEAR 	X'&INDEV'
	CLEAR 	A
	CLEAR 	S
	LDT		4096
	TD 		= X'&INDEV'
	JEQ 	*-3
	RD		= X'&INDEV'
	COMPR	A, S
	JEQ		*-11
	STCH	&BUFADR, X
	TIXR	T
	JLT		*-19
	STX		&RECLTH
MEND

WRBUFF: MACRO &OUTDEV, &BUFADR, &RECLTH
	CLEAR 	X
	LDT		&RECLTH
	LDCH 	&BUFADR, X
	TD		= X'&OUTDEV'
	JEQ		*-3
	WD		= X'&OUTDEV'
	TJXR	T
	JLT		*-14
MEND

FISRT: 	STL RETADR
CLOOP: 	RDBUFF F1, BUFFER, LENGTH
	LDA 	LENGTH
	COMP	0
	JEQ		ENDIL
	WRBUFF 5, BUFFER, LENGTH
	J CLOOP
ENDIL:	WRBUFF	0, EOF, THREE
	J CLOOP
EOF:	BYTE	C'EOF'
THREE:	WORD 3
RETADR:	RESW 1
LENGTH:	RESW 1
BUFFER:	RESB 4096
END FIRST
