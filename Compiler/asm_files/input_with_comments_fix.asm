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