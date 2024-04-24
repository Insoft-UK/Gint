.global _gint_tlbh
.section .gint.tlbh, "ax"
.align 4

_gint_tlbh:
	sts.l	pr, @-r15
	stc.l	gbr, @-r15
	sts.l	mach, @-r15
	sts.l	macl, @-r15

	/* Get HWMPU in gint's hardware info. If last bit is set, we're SH3 */
	mov.l	.gint, r0
	mov.l	@r0, r0
	tst	#1, r0
	mov.l	.tea_sh4, r0
	bt	test_tea
	mov.l	.tea_sh3, r0

test_tea:
	/* Check TEA to see if we want to map a page or raise a SysERROR */
	mov.l	@r0, r0
	mov.l	.max_mapped_rom, r1
	cmp/ge	r1, r0
	bt	panic
	mov.l	.min_mapped_rom, r1
	cmp/ge	r1, r0
	bf	panic

map:
	/* If TEA is mappable, map a page and return */
	#ifdef FX9860G
	mov	#3, r0
	#endif
	#ifdef FXCG50
	mov	#12, r0
	#endif

	mov.l	.syscall, r2
	jsr	@r2
	nop

	lds.l	@r15+, macl
	lds.l	@r15+, mach
	ldc.l	@r15+, gbr
	lds.l	@r15+, pr
	rte
	nop

panic:
	/* Otherwise, panic by defaulting to the exception handler (the TLB
	   miss may still be resolved by a panic handler) */
	lds.l	@r15+, macl
	lds.l	@r15+, mach
	ldc.l	@r15+, gbr
	lds.l	@r15+, pr

	stc	vbr, r0
	mov	#1, r1
	shll8	r1
	add	r1, r0
	jmp	@r0
	nop

.align 4

.gint:
	.long	_gint
.tea_sh4:
	.long	0xff00000c
.tea_sh3:
	.long	0xfffffffc
.min_mapped_rom:
	.long	0x00300000
.max_mapped_rom:
	.long	0x00300000 + _srom

#ifdef FX9860G
.syscall:
	.long	0x80010070
#endif
#ifdef FXCG50
.syscall:
	.long	0x80020070
#endif
