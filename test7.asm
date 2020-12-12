;Demonstrates errors and warnings.

ldc 100
label#1:	;invalid label name
adc 2 
addd 		;invalid mnemonic
sub 4		;unexpected operand
loop: br loop	;infinite loop
		
