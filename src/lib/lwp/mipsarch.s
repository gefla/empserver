	.verstamp	3 0
	.extern	tzname 0
	.extern	LwpCurrent 4
	.lcomm	$$9 4
	.text	
	.align	2
	.file	2 "arch.new.c"
	.globl	lwpSave
	.ent	lwpSave 2
lwpSave:
	.option	O1
        subu    $sp, 32 
	sw      $31, 28($sp)
	sw	$4, 32($sp) 
	.mask	0x80000000, -4
	.frame	$sp, 32, $31
	sw	$0, $$9
	sw	$30, 0($4)
	sw	$sp, 4($4)
	sw      $31, 8($4)
	sd	$16, 12($4)
	sd	$18, 20($4)
	sd	$20, 28($4)
	sd	$22, 36($4)
	lw	$2, $$9	
	.livereg	0x2000FF0E,0x00000FFF
	lw	$31, 28($sp)
	addu	$sp, 32
	j	$31
	.end	lwpSave
	.text	
	.align	2
	.file	2 "arch.new.c"
	.globl	lwpRestore
	.ent	lwpRestore 2
lwpRestore:
	.option	O1
        subu    $sp, 32
        sw      $31, 28($sp)
        sw      $4, 32($sp)
        .mask   0x80000000, -4
	.frame	$sp, 32, $31
	li	$14, 1
	sw	$14, $$9
	lw	$30, 0($4)
	lw	$sp, 4($4)
	lw	$31, 8($4)
	ld	$16, 12($4)
	ld	$18, 20($4)
	ld	$20, 28($4)
	ld	$22, 36($4)
	lw	$2, $$9	
	.livereg	0x0000FF0E,0x00000FFF
 	addu	$sp, 32
	j	$31
	.end	lwpRestore
	.text	
	.align	2
	.file	2 "arch.new.c"
	.globl	lwpInitContext
	.ent	lwpInitContext 2
lwpInitContext:
	.option	O1
	.frame	$sp, 0, $31
	sw	$5, 4($4)
	la	$14, lwpEntryPoint
	sw	$14, 8($4)
	.livereg	0x0000FF0E,0x00000FFF
	j	$31
	.end	lwpInitContext

