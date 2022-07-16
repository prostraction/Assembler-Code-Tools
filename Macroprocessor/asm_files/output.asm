begin 4096
st: ldx numb ; загрузить в регистр x
lda fact 
comp two  
jlt e
loop: mul numb
sub fact
comp one
jeq loop  
rsub 
one: dw 1
two: dw 2
numb: dw 1
fact: dw 5
e: rsub ; метка конца программы
end st
======= 
COPY: START 0
FISRT: STL RETADR
; CLOOP: RDBUFF F1, BUFFER, LENGTH
CLOOP: CLEAR X'F1'
CLEAR A
CLEAR S
LDT 4096
TD = X'F1'
JEQ *-3
RD = X'F1'
COMPR A, S
JEQ *-11
STCH BUFFER
TIXR T
JLT *-19
STX LENGTH
LDA LENGTH
COMP 0
JEQ ENDIL
; : WRBUFF 5, BUFFER, LENGTH
: CLEAR X
LDT LENGTH
LDCH BUFFER
TD = X'5'
JEQ *-3
WD = X'5'
TJXR T
JLT *-14
J CLOOP
; ENDIL: WRBUFF 0, EOF, THREE
ENDIL: CLEAR X
LDT THREE
LDCH EOF
TD = X'0'
JEQ *-3
WD = X'0'
TJXR T
JLT *-14
J CLOOP
EOF: BYTE C'EOF'
THREE: WORD 3
RETADR: RESW 1
LENGTH: RESW 1
BUFFER: RESB 4096
END FIRST
