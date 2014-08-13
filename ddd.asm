

CLR R0  # R0 <-  0
SR0 1  # R0 <-  1
MOV R3,R0 # R3 <-  1 (define delay to be 1/100 of a second)
SR0 0  # R0 <-  0
SRH0 3  # R0 <-  30(hex)=48(dec)
MOV R1,R0 # R1 <-  30(hex)=48(dec)
CLR R0  # R0 <-  0 (set-up loop iterator)
SR0 10  # R0 <-  10(decimal) - (loop repeats 10 times)
MOVR R1  # Move the motor 48 steps (1 full turn) clockwise
SUBI R0,1 # R0 <-  R0-1
BRZ 2  # Branch 2 instructions forward if R0 has reached 0
BR -3  # Branch 3 locations backwards
PAUSE  # pause 1/100 of a second
...  # 8 times for a total of 8/100 of a second
SR0 0  # R0 <-  0
SRH0 13  # R0 <-  D0(hex)=-48(dec) 
MOV R1,R0 # R1 <-  D0(hex)=-48(dec)
CLR R0  # R0 <-  0
SR0 10  # R0 <-  10(decimal)
MOVR R1  # Move the motor 48 steps (1 full turn) counter-clockwise
SUBI R0,1 # R0 <-  R0-1
BRZ 2  # Branch 2 instructions forward if R0 has reached 0
BR -3  # Branch 3 locations backwards
PAUSE  # pause 1/100 of a second
PAUSE  # pause 1/100 of a second
PAUSE  # pause 1/100 of a second
PAUSE  # pause 1/100 of a second
PAUSE  # pause 1/100 of a second
PAUSE  # pause 1/100 of a second
PAUSE  # pause 1/100 of a second
PAUSE  # pause 1/100 of a second
MOVRHS R1 # Move the motor 48 half-steps (1/2 a turn) counter-clockwise
BR 0  # infinite loop (branches to PC+0, i.e. itself)
