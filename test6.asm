;Find the maximum of 3 numbers stored at memory locations 200, 201 and 202. Store the answer at memory location 203.

ldc 40		;x=40
stl 200		;[200]=x=40
ldc 50		;y=50
stl 201		;[201]=y=50
ldc 20		;z=20
stl 202		;[202]=z=20

ldl 200
ldl 201		;B=[200]=x=40, A=[201]=y=50 
sub		;A=B-A=x-y
brlz next2	;if x-y<0 or y>x

ldl 200
ldl 202		;B=[200]=x=40, A=[202]=z=20
sub		;A=B-A=x-z
brlz next1	;if x-z<0 or z>x

ldl 200
stl 203		;[203]=x
HALT

next2:
ldl 201
ldl 202		;B=[202]=y=50, A=[201]=z=20
sub		;A=B-A=y-z
brlz next1	;if y-z<0 or z>y

ldl 201
stl 203		;[203]=y
HALT

next1:
ldl 202
stl 203		;[203]=z


