.set r0,0; .set SP,1; .set RTOC,2; .set r3,3; .set r4,4
.set r5,5; .set r6,6; .set r7,7; .set r8,8; .set r9,9
.set LR,8

	.rename	LwpInitContext{PR},""
	.rename	LwpInitContext{TC},"lwpInitContext"
	.rename	LwpEntryPoint{TC},"lwpEntryPoint"

	.lglobl	LwpInitContext{PR}      
	.globl	.lwpInitContext         
	.globl	lwpInitContext{DS}      
	.extern	lwpEntryPoint{DS}       


# .text section


	.csect	LwpInitContext{PR}      
	.function	LwpInitContext{PR},.lwpInitContext,2,0
.lwpInitContext:                        # 0x00000000 (LwpInitContext)
	stu	SP,-0x40(SP)
	st	r3,0x58(SP)		# r3 = newp pointer
	st	r4,0x5c(SP)		# r4 = sp pointer
	.bf	234
	st	SP,0x0(r4)
	st	SP,0xf8(r3)		# store prev SP
	st	r4,0xc(r3)		# store entrypoint at newp[3]
	l	r3,LWPEntryPoint(RTOC)
	l	r4,0x0(r3)
	l	r3,0x58(SP)		# load newp into r3
	st	r4,0x8(r3)		# store sp at newp[2]
	st	RTOC,0x10(r3)		# store RTOC at newp[4]
	ai	SP,SP,0x40
	br	
	.ef	236
# traceback table
	.long	0x00000000
	.byte	0x00			# VERSION=0
	.byte	0x00			# LANG=TB_C
	.byte	0x20			# IS_GL=0,IS_EPROL=0,HAS_TBOFF=1
					# INT_PROC=0,HAS_CTL=0,TOCLESS=0
					# FP_PRESENT=0,LOG_ABORT=0
	.byte	0x40			# INT_HNDL=0,NAME_PRESENT=1
					# USES_ALLOCA=0,CL_DIS_INV=WALK_ONCOND
					# SAVES_CR=0,SAVES_LR=0
	.byte	0x80			# STORES_BC=1,FPR_SAVED=0
	.byte	0x00			# GPR_SAVED=0
	.byte	0x02			# FIXEDPARMS=2
	.byte	0x01			# FLOATPARMS=0,PARMSONSTK=1
	.long	0x00000000		# 
	.long	0x0000002c		# TB_OFFSET
	.short	14			# NAME_LEN
	.byte	"lwpInitContext"
# End of traceback table
# End	csect	LwpInitContext{PR}

# .data section


	.toc	                        # 0x00000050 
LWPInitContext:
	.tc	LwpInitContext{TC},lwpInitContext{DS}
LWPEntryPoint:
	.tc	LwpEntryPoint{TC},lwpEntryPoint{DS}


	.csect	lwpInitContext{DS}      
	.long	.lwpInitContext         # "\0\0\0\0"
	.long	TOC{TC0}                # "\0\0\0P"
	.long	0x00000000              # "\0\0\0\0"
# End	csect	lwpInitContext{DS}



# .bss section

