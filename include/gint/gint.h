//---
//	gint - An alternative runtime environment for fx9860g and fxcg50
//---

#ifndef GINT_GINT
#define GINT_GINT

#ifdef __cplusplus
extern "C" {
#endif

#include <gint/defs/types.h>
#include <gint/defs/call.h>
#include <gint/config.h>
#include <gint/intc.h>

/* gint_world_switch(): Switch out of gint to execute a function

   This function can be used to leave gint, restore the OS's hardware state,
   and execute code there before returning to gint. By doing this one can
   effectively interleave gint with the standard OS execution. gint drivers
   will be inactive during this time but OS features such as BFile or the
   main menu are available.

   This main uses for this switch are going back to the main menu and using
   BFile function. You can go back to the main menu easily by calling getkey()
   (or getkey_opt() with the GETKEY_MENU flag set) and pressing the MENU key,
   or by calling gint_osmenu() below which uses this switch.

   The code to execute while in OS mode is passed as a gint call; you can use
   GINT_CALL() to create one. This allows you to pass arguments to your
   function, as well as return an int.

   @function  A GINT_CALL() to execute while in OS mode
   -> Returns the return value of (function), if any, 0 if function is NULL. */
int gint_world_switch(gint_call_t function);

/* This function is an older version of gint_world_switch() which only accepts
   functions with no arguments and no return value. It will be removed in
   gint 3. */
__attribute__((deprecated("Use gint_world_switch() instead")))
void gint_switch(void (*function)(void));

/* gint_world_sync(): Synchronize asynchronous drivers

   This function waits for asynchronous tasks to complete by unbinding all
   drivers. This is useful in certain hardware operations while remaining in
   gint. */
void gint_world_sync(void);

/* gint_osmenu(): Call the calculator's main menu

   This function safely invokes the calculator's main menu with gint_switch().
   If the user selects the gint application again in the menu, this function
   reloads gint and returns. Otherwise, the add-in is fully unloaded by the
   system and the application terminates.

   This function is typically called when the [MENU] key is pressed during a
   call to getkey(), but can also be called manually. */
void gint_osmenu(void);

/* gint_osmenu_native(): Like gint_osmenu() without the world switch
   This is a replacement for gint_osmenu() which can be used when the current
   kernel is already the native OS kernel. */
void gint_osmenu_native(void);

/* gint_setrestart(): Set whether to restart the add-in after exiting

   An add-in that returns from its main() function automatically exits to the
   OS' main menu. However, when this happens the OS does not allow the add-in
   to be restarted unless another add-in is launched first. (This is because
   the OS tries to *resume* the current add-in, which then proceeds to exit
   again immediately.)

   This function enables a gint trick where after main() returns the add-in
   will invoke the main menu with gint_osmenu() rather than exiting. If the
   add-in is selected again, gint will jump back to the entry point, creating
   the illusion that the add-in exited and was then restarted.

   @restart  0 to exit, 1 to restart by using gint_osmenu() */
void gint_setrestart(int restart);

/* This function has been moved to the INTC driver */
__attribute__((deprecated("Use intc_handler() instead")))
static GINLINE void *gint_inthandler(int code, void const *h, size_t size) {
	return intc_handler(code, h, size);
}

/* gint_inth_callback(): Callback from interrupt handler to userland

   This function performs an indirect call as with gint_call(), afters saving
   the user context, enabling interrupts and going to user bank. This is useful
   to call user code from interrupt handlers. You can think of it as a kernel-
   space escape to virtualized userland during interrupt handling.

   This function can only be useful in an interrupt handler's assembler code.
   It is loaded at a runtime-determined address and accessed through a function
   pointer, like this:

        mov.l	.callback, r0
        mov.l	@r0, r0 # because function pointer
        mov	<address of gint_call_t object>, r4
        jsr	@r0
        nop
   .callback:
        .long	_gint_inth_callback

   @call  Address of a gint_call_t object
   Returns the return value of the callback. */
extern int (*gint_inth_callback)(gint_call_t const *call);

/* gint_set_quit_handler(): Setup a call to be invoked when leaving the add-in

   This function sets up the provided GINT_CALL() to be invoked when the
   add-in is unloaded, which is either when we exit from main() or when
   starting another application from the main menu. Crucially, this is only
   *after* selecting an application, not before opening the main menu. The
   quit handler is not invoked if the user re-enters the add-in.

   This is based on the SetQuitHandler() syscall, and therefore the callback
   runs in the OS world by default. If [run_in_os_world] is set to false, a
   world switch will be performed to run the callback in the gint world.

   TODO: Currently the quit handler is not called when exiting from main().
   TODO: Detail how this interacts with destructor functions!
   TODO: [run_in_os_world == false] is not honored yet (because unstable)

   @call             Callback to be performed when leaving add-in
   @run_in_os_world  true to stay in OS world, false to use gint world */
void gint_set_quit_handler(gint_call_t gcall, bool run_in_os_world);

#ifdef __cplusplus
}
#endif

#endif /* GINT_GINT */
