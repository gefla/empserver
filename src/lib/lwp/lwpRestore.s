.set r0,0; .set SP,1; .set RTOC,2; .set r3,3; .set r4,4
.set r5,5; .set r6,6; .set r7,7; .set r8,8; .set r9,9
.set LR,8

	.rename	lwpRestore{PR},""
	.rename	lwpRestore{TC},"lwpRestore"

	.lglobl	lwpRestore{PR}      
	.globl	.lwpRestore             
	.globl	lwpRestore{DS}          


# .text section


	.csect	lwpRestore{PR}      
.lwpRestore:                            # 0x00000000 (lwpRestore)
	l	r5,0x8(r3)
	l	SP,0xc(r3)
	l	RTOC,0x10(r3)
	mtlr	r5
	lil	r3,0x1
	br
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
	.byte	0x01			# FIXEDPARMS=1
	.byte	0x01			# FLOATPARMS=0,PARMSONSTK=1
	.long	0x00000000		# 
	.long	0x00000010		# TB_OFFSET
	.short	10			# NAME_LEN
	.byte	"lwpRestore"
# End of traceback table
# End	csect	lwpRestore{PR}

# .data section


	.toc	                        # 0x00000030 
LWPRestore:
	.tc	lwpRestore{TC},lwpRestore{DS}


	.csect	lwpRestore{DS}          
	.long	.lwpRestore             # "\0\0\0\0"
	.long	TOC{TC0}                # "\0\0\0000"
	.long	0x00000000              # "\0\0\0\0"
# End	csect	lwpRestore{DS}



# .bss section

