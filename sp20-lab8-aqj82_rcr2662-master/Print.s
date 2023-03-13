; Print.s
; Student names: Alison Jin, Roberto Reyes
; Last modification date: change this to the last modification date or look very silly
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 


    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; Lab 7 requirement is for at least one local variable on the stack with symbolic binding
quotient EQU 0
LCD_OutDec
		SUB SP, #8
		MOV R1, #10
		MOV R12, SP ; R12 is frame pointer
		
		CMP R0, #0
		BEQ zero
		
loop	CMP R0, #0
		BEQ print
		UDIV R2, R0, R1  ; R2 = new num
		MUL R3, R1, R2
		RSB R3, R0		;R3 = num%10
		STR R2, [R12, #quotient]
		ADD R3, #0x30
		PUSH {R3, R12}
		MOV R0, R2
		B loop
print	CMP R12, SP
		BEQ done
		POP{R0, R1}
		PUSH {LR, R0}
		BL ST7735_OutChar
		POP {LR, R0}
		B print
		
zero	MOV R0, #0x30
		PUSH {LR, R0}
		BL ST7735_OutChar
		POP {LR, R0}
done	ADD SP, #8

      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
; Lab 7 requirement is for at least one local variable on the stack with symbolic binding
i EQU 0
LCD_OutFix
	 SUB  SP, #8
	 MOV R1, #1000
	 CMP R0, R1
	 BHS  back
	 STR  R0, [SP, #i]
	 MOV  R1,#100
	 UDIV R0, R1
	 ADD  R0, #0x30
	 PUSH{R0, R1, R12, LR}
	 BL	  ST7735_OutChar
	 POP {R0, R1, R12, LR}
	 LDR  R12, [SP, #i]
     UDIV R2,R12,R1
     MUL  R3,R2,R1
     SUB  R0,R12,R3
	 STR  R0, [SP, #i]
	 MOV  R0, #0x2E
	 PUSH{R0, R1, R12, LR}
	 BL	  ST7735_OutChar
	 POP {R0, R1, R12, LR}
	 LDR  R0, [SP, #i]
	 MOV  R1,#10
	 UDIV R0, R1
	 ADD  R0, #0x30
	 PUSH{R0, R1, R12, LR}
	 BL	  ST7735_OutChar
	 POP {R0, R1, R12, LR}
     UDIV R2,R12,R1
     MUL  R3,R2,R1
     SUB  R0,R12,R3
	 ADD  R0, #0x30
	 PUSH{R0, R1, R12, LR}
	 BL  ST7735_OutChar 
	 POP {R0, R1, R12, LR}
	 B	  out
back MOV  R0, #0x2A
	 PUSH{R0, LR}
	 BL	  ST7735_OutChar
	 POP{R0, LR}
	 MOV  R0, #0x2E
	 PUSH{R0, LR}
	 BL	  ST7735_OutChar
	 POP{R0, LR}
	 MOV  R0, #0x2A
	 PUSH{R0, LR}
	 BL	  ST7735_OutChar
	 POP{R0, LR}
	 MOV  R0, #0x2A
	 PUSH{R0, LR}
	 BL	  ST7735_OutChar
	 POP{R0, LR}
out	 ADD  SP, #8
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
