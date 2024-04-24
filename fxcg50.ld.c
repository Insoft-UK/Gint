/*
	Linker script for fxcg50 add-ins. Most symbols are used in the startup
	routine in core/start.c; some others in core/setup.c.
*/

/* All fxcg50 have SH4 processors (finally rid of compatibility issues) */
OUTPUT_ARCH(sh4)
/* ELF offers a lot of symbol/section/relocation insights */
OUTPUT_FORMAT(elf32-sh)
/* Located in core/start.c */
ENTRY(_start)

#ifdef FXCG50_FASTLOAD
# define _ROM_REGION 0x8c200000
#else
# define _ROM_REGION 0x00300000
#endif

MEMORY
{
	/* Userspace mapping of the add-in (without G3A header) */
	rom   (rx):   o = _ROM_REGION, l = 2M
	/* Static RAM; stack grows down from the end of this region.
	   The first 5k (0x1400 bytes) are reserved by gint for the VBR space,
	   which is loaded dynamically and accessed through P1 */
	ram   (rw):   o = 0x08101400, l = 491k
	/* On-chip IL memory */
	ilram (rwx):  o = 0xe5200000, l = 4k
	/* On-chip X and Y memory */
	xyram (rwx):  o = 0xe500e000, l = 16k
}

SECTIONS
{
	/*
	**  ROM sections
	*/

	/* First address to be mapped to ROM */
	_brom = ORIGIN(rom);
	/* Size of ROM mappings */
	_srom = SIZEOF(.text) + SIZEOF(.rodata)
	      + SIZEOF(.gint.drivers) + SIZEOF(.gint.blocks);

	/* Machine code going to ROM:
	   - Entry function (.text.entry)
	   - Compiler-provided constructors (.ctors) and destructors (.dtors)
	   - All text from .text and .text.* (including user code) */
	.text : {
		*(.text.entry)

		_bctors = . ;
		*(.ctors .ctors.*)
		_ectors = . ;

		_bdtors = . ;
		*(.dtors .dtors.*)
		_edtors = . ;

		_gint_exch_start = . ;
		*(.gint.exch)
		_gint_exch_size = ABSOLUTE(. - _gint_exch_start);

		_gint_tlbh_start = . ;
		*(.gint.tlbh)
		_gint_tlbh_size = ABSOLUTE(. - _gint_tlbh_start);

		*(.text .text.*)
	} > rom

	/* gint's interrupt handler blocks (.gint.blocks)
	   Although gint's blocks end up in VBR space, they are relocated at
	   startup by the library/drivers, so we store them here for now */
	.gint.blocks : {
		KEEP(*(.gint.blocks));
	} > rom

	/* Exposed driver interfaces (.gint.drivers)
	   The driver information is required to start and configure the
	   driver, even if the symbols are not referenced */
	.gint.drivers : {
		_gint_drivers = . ;
		KEEP(*(SORT_BY_NAME(.gint.drivers.*)));
		_gint_drivers_end = . ;
	} > rom

	/* Read-only data going to ROM:
	   - Resources or assets from fxconv or similar converters
	   - Data marked read-only by the compiler (.rodata and .rodata.*) */
	.rodata : SUBALIGN(4) {
		/* Put these first, they need to be 4-aligned */
		*(.rodata.4)

		*(.rodata .rodata.*)
	} > rom



	/*
	**  RAM sections
	*/

	. = ORIGIN(ram);

	/* BSS data going to RAM. The BSS section is to be stripped from the
	   ELF file later, and wiped at startup */
	.bss (NOLOAD) : {
		_rbss = . ;

		*(.bss .bss.* COMMON)

		. = ALIGN(16);
	} > ram :NONE

	_sbss = SIZEOF(.bss);

	/* Read-write data sections going to RAM (.data and .data.*) */
	.data ALIGN(4) : ALIGN(4) {
		_ldata = LOADADDR(.data);
		_rdata = . ;

		*(.data .data.*)
		/* References to mapped code - no relocation needed */
		*(.gint.mappedrel)

		. = ALIGN(16);
	} > ram AT> rom

	/* Read-write data sub-aligned to 4 bytes (mainly from fxconv) */
	.data.4 : SUBALIGN(4) {
		*(.data.4)
		. = ALIGN(16);
	} > ram AT> rom

	_sdata = SIZEOF(.data) + SIZEOF(.data.4);

	/* On-chip memory sections: IL, X and Y memory */

	. = ORIGIN(ilram);
	.ilram ALIGN(4) : ALIGN(4) {
		_lilram = LOADADDR(.ilram);
		_rilram = . ;

		*(.ilram)
		/* Code that must remain mapped is placed here */
		*(.gint.mapped)

		. = ALIGN(16);
	} > ilram AT> rom

	. = ORIGIN(xyram);
	.xyram ALIGN(4) : ALIGN(4) {
		_lxyram = LOADADDR(.xyram);
		_rxyram = . ;

		*(.xram .yram .xyram)

		. = ALIGN(16);
	} > xyram AT> rom

	_silram = SIZEOF(.ilram);
	_sxyram = SIZEOF(.xyram);

	/* gint's uninitialized BSS section, going to static RAM. All the large
	   data arrays will be located here */
	.gint.bss (NOLOAD) : {
		*(.gint.bss)
		. = ALIGN(16);

		/* End of user RAM */
		_euram = . ;
	} > ram :NONE

	_sgbss = SIZEOF(.gint.bss);



	/*
	**  Unused sections
	*/

	/DISCARD/ : {
		/* SH3-only data sections */
		*(.gint.rodata.sh3 .gint.data.sh3 .gint.bss.sh3)
		/* Java class registration (why are they even here?!) */
		*(.jcr)
		/* Asynchronous unwind tables: no C++ exception handling */
		*(.eh_frame_hdr)
		*(.eh_frame)
		/* Comments or anything the compiler might generate */
		*(.comment)
	}
}
