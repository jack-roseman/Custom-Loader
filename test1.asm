.CODE
.ADDR 0x0020
FACTORIAL
CONST R0, #7   ; A = 5
AND R1, R0 #-1   ;; B = A, essentially copying R0 to R1
LOOP
 CMPI R0, #1     ; set NZP bits
 BRnz END        ; go to end if A <= 0
 ADD R0, R0, #-1 ; A = A - 1
 MUL R1, R1, R0   ; B = B * A
 BRnzp LOOP      ; go to LOOP
END

.DATA
.ADDR 0x2000
.STRINGZ "JACK"