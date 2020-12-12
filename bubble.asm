; Oindrila Bhadra
; 1801CS34

ldc array
stl 509		;[509]=address of array
ldc end
ldc array
sub		;A=end of array-start of array
stl 500		;[500]=no.of elements=n

call sort
HALT

sort: 
stl 501		;[501]=return address
ldc 0		
stl 502		;[502]=i=0
ldl 500
adc -1		
stl 505		;[505]=n-1


loop1:
ldc 0
stl 503		;[503]=j
ldc 1
stl 504		;[504]=j+1
ldl 505
ldl 502
sub		;A=n-1-i
stl 506 	;[506]=n-i-1

loop2:
ldl 503
ldl 509
add
stl 510		;[510]=array address+j
ldl 504
ldl 509
add		;A=array address+j+1
ldnl 0
ldl 510
ldnl 0
sub		;arr[j+1]-arr[j]
brlz swap	;if arr[j+1]<arr[j]

position:
ldl 504
adc 1		
stl 504		;[504]=(j+1)+1=j+2
ldl 503
adc 1
stl 503		;[503]=(j)+1=j+1
ldl 503
ldl 506
sub		;j-(n-i-1)
brlz loop2	;if j<n-i-1

ldl 502
adc 1
stl 502		;[502]=i+1
ldl 505
sub		;i-(n-1)
brlz loop1	;if i<n-1


ldl 501		;return address
return

swap:
ldl 503
ldl 509
add	
ldnl 0		;A=arr[j]
ldc 507	
stnl 0		;[507]=arr[j]
ldl 504
ldl 509
add
ldnl 0		;A=arr[j+1]
ldc 508		
stnl 0		;[508]=arr[j+1]

ldl 503
ldl 509
add
stl 511		;[511]=array address+j
ldl 508
ldl 511		;A=array address+j
stnl 0		;arr[j]=arr[j+1]

ldl 504
ldl 509
add
stl 511		;[511]=array address+j+1
ldl 507
ldl 511		;A=array address+j+1
stnl 0		;arr[j+1]=arr[j]

br position


array:  data    7
	data    9
      	data    6
	data 	1
	data 	10
	data 	2
	

end:
