/*
**	gint:core:syscalls - calls to CASIOWIN
**
**	This file can be seen as a list of everywhere gint relies on the
**	underlying OS. Although I wish to make gint free-standing, there are
**	still a few hard questions, namely:
**	* MMU management, because doing it wrong might break the calculator.
**	* Dynamic allocation, because we can't trash the system heap.
**	* File system, because it's a mess and we might ruin the ROM.
*/

.text

/* Dynamic allocation */
.global ___malloc
.global ___free
.global ___calloc
.global ___realloc

/* Bfile driver */
.global	_BFile_Remove
.global	_BFile_Rename
.global	_BFile_Create
.global	_BFile_Open
.global	_BFile_Close
.global	_BFile_Size
.global	_BFile_Seek
.global	_BFile_GetPos
.global	_BFile_Write
.global	_BFile_Read
.global	_BFile_FindFirst
.global	_BFile_FindNext
.global	_BFile_FindClose

/* Return to menu */
.global ___Timer_Install
.global ___Timer_Start
.global ___Timer_Stop
.global ___Timer_Deinstall
.global ___PutKeyCode
.global ___GetKeyWait
.global ___ClearKeyBuffer
.global ___GetVRAMAddress
.global ___ConfigureStatusArea
.global ___SetQuitHandler

/* Reset */
.global ___Reset

#define syscall_(id, syscall_table)	\
	mov.l	syscall_table, r2	;\
	mov.l	1f, r0			;\
	jmp	@r2			;\
	nop				;\
.align 4				;\
1:	.long	id

#define syscall(id) syscall_(id, syscall_table)

#ifdef FX9860G

/* Dynamic allocation */

___malloc:
	syscall(0x0acd)
___free:
	syscall(0x0acc)
___realloc:
	syscall(0x0e6d)

/* BFile driver */

_BFile_Remove:
	mov #0, r5
	syscall(0x0439)
_BFile_Create:
	syscall(0x0434)
_BFile_Open:
	syscall(0x042c)
_BFile_Close:
	syscall(0x042d)
_BFile_Size:
	syscall(0x042f)
_BFile_Seek:
	syscall(0x0431)
_BFile_GetPos:
	rts
	mov	#-1, r0
_BFile_Write:
	syscall(0x0435)
_BFile_Read:
	syscall(0x0432)
_BFile_FindFirst:
    syscall(0x043b)
_BFile_FindNext:
    syscall(0x043c)
_BFile_FindClose:
    syscall(0x043d)

/* Return to menu */

___Timer_Install:
	syscall(0x118)
___Timer_Start:
	syscall(0x11a)
___Timer_Stop:
	syscall(0x11b)
___Timer_Deinstall:
	syscall(0x119)
___PutKeyCode:
	syscall(0x248)
___GetKeyWait:
	syscall(0x247)
___ClearKeyBuffer:
	syscall(0x241)
___GetVRAMAddress:
	syscall(0x135)
___SetQuitHandler:
	syscall(0x494)

/* Reset */

___Reset:
	syscall(0x236)

syscall_table:
	.long	0x80010070

#endif /* FX9860G */

#ifdef FXCG50

/* Dynamic allocation */

___malloc:
	syscall(0x1f44)
___free:
	syscall(0x1f42)
___realloc:
	syscall(0x1f46)

/* BFile driver */

_BFile_Remove:
	syscall(0x1db4)
_BFile_Rename:
	syscall(0x1db3)
_BFile_Create:
	syscall(0x1dae)
_BFile_Open:
	mov #0, r6
	syscall(0x1da3)
_BFile_Close:
	syscall(0x1da4)
_BFile_Size:
	syscall(0x1da6)
_BFile_Seek:
	syscall(0x1da9)
_BFile_GetPos:
	syscall(0x1dab)
_BFile_Write:
	syscall(0x1daf)
_BFile_Read:
	syscall(0x1dac)
_BFile_FindFirst:
	syscall(0x1db6)
_BFile_FindNext:
	syscall(0x1db8)
_BFile_FindClose:
	syscall(0x1dba)

/* Return to menu */

___Timer_Install:
	syscall(0x8d9)
___Timer_Start:
	syscall(0x8db)
___Timer_Stop:
	syscall(0x8dc)
___Timer_Deinstall:
	syscall(0x8da)
___PutKeyCode:
	syscall(0x12c6)
___GetKeyWait:
	syscall(0x12bf)
___ClearKeyBuffer:
	syscall(0x12c7)
___GetVRAMAddress:
	syscall(0x1e6)
___ConfigureStatusArea:
	syscall(0x2b7)
___SetQuitHandler:
	syscall(0x1e6e)

.global ___SpecialMatrixCodeProcessing
___SpecialMatrixCodeProcessing:
	syscall(0x1e60)

/* Reset */

___Reset:
	syscall(0x1187)

syscall_table:
	.long	0x80020070

#endif /* FXCG50 */
