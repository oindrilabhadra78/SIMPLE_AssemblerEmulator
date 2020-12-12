;Calculate the product of 2 numbers. One number is stored at memory location 200, other one at memory location 201. The product is calculated and stored at memory location 203.

;pseudocode
;
;x=12,y=8,i=0,sum=0
;do{
;sum=sum+y
;i++}
;while(i-x<0)
;


ldc 12		;x=12
stl 200		;[200]=12
ldc 8		;y=8
stl 201		;[201]=8
ldc 0		;i=0
stl 202		;[202]=i=0
ldc 0		;sum=0
stl 203		;[203]=sum=0

loop:
ldl 201
ldl 203		;B=[201]=y=8, A=[203]=sum
add		;A=B+A, A=sum+y
stl 203		;store the new sum at [203]

ldl 202		;A=[202]=i
adc 1		;A=A+1, i=i+1
stl 202		;[202]=i

ldl 202		
ldl 200		;B=[202]=i, A=[200]=x=12
sub		;A=B-A, A=i-x
brlz loop	;if i-x<0 then loop
HALT		;else break from loop


