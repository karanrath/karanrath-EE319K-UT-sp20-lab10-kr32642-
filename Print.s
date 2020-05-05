; Print.s
; Student names: Karan Rathore
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


;BINDING
i		equ	0 ; allocate 10 8-bit numbers
x		equ 0

FP		RN	11

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; Lab 7 requirement is for at least one local variable on the stack with symbolic binding
LCD_OutDec

		PUSH	{R4, LR}
		MOV		R3, #10 
		MOV		R4, #0 ; index
	
		;ALLOCATION
		SUB		SP, #8 
		MOV		FP, SP
		
		;ACCESS
loop	CMP		R0, #10; check if unit digit
		BLO		MSD
		UDIV	R1, R0, R3 ; n/10 
		MUL		R2, R1, R3 
		SUB		R2, R0, R2 ; push lowest digit
		PUSH	{R2, R3}
		LDR		R2, [FP, #i]
		ADD		R2, #1 ; increment index
		STR		R2, [FP, #i]
		MOV		R0, R1
		B		loop
MSD		ADD		R0, #0x30
		BL		ST7735_OutChar ; display MSD
loop2	LDR		R2, [FP, #i]
		SUB		R2, #1
		STR		R2, [FP, #i]
		CMP		R2, #0
		BLT		done
		POP		{R0, R3}
		ADD		R0, #0x30
		BL		ST7735_OutChar ; display next value
		B		loop2
		
		
		;DEALLOCATION
done	ADD		SP, #8
		
		POP		{R4, LR}
		
		BX  	LR
		
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
LCD_OutFix

		PUSH	{R0, LR}
		
		MOV		R1, #10 
		MOV		R2, #999
		
		;ALLOCATION
		SUB		SP, #8
		STR		R0, [SP, #x] ; 123
		
		;ACCESS
		CMP		R0, R2 ; wrong
		BHI		wrong
		
		MOV		R2, #100
		
		UDIV	R0, R2 				
		ADD		R0, #0x30
		BL		ST7735_OutChar ; unit number			;1
		
		MOV		R0, #0x2E
		BL		ST7735_OutChar ; decimal
		
		MOV		R1, #10
		MOV		R2, #100
		LDR		R3, [SP, #x]
		UDIV	R0, R3, R2 ; get first digit by dividing by 100
		MUL		R0, R2
		SUB		R0, R3, R0								;23
		STR		R0, [SP, #x] ; saves remaining 2 digits
		
		LDR		R0, [SP, #x]							;23 
		UDIV	R0, R1 ; gets second digit by dividing by 10
		ADD		R0, #0x30
		BL		ST7735_OutChar ; hundreds digit	;2
		
		MOV		R1, #10
		MOV		R2, #100
		LDR		R3, [SP, #x]
		UDIV	R0, R3, R1
		MUL		R0, R1
		SUB		R0, R3, R0 ; thousands digit 
		ADD		R0, #0x30
		BL		ST7735_OutChar 			;3
		B		dn
		
wrong	MOV		R0, #0x2A
		BL		ST7735_OutChar 
		MOV		R0, #0x2E
		BL		ST7735_OutChar 
		MOV		R0, #0x2A
		BL		ST7735_OutChar 
		MOV		R0, #0x2A
		BL		ST7735_OutChar 

		;DEALLOCATE
dn	ADD		 SP, #8
		
		POP		{R0, LR}
		
		BX   LR
 
		ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

		ALIGN          ; make sure the end of this section is aligned
		END            ; end of file
			