//---
//	gint:keydev - Generic input handling on keyboard devices
//---

#include <gint/keyboard.h>
#include <gint/cpu.h>
#include <gint/drivers/keydev.h>
#include <gint/defs/types.h>
#include <gint/defs/util.h>

#include <string.h>
#include <stdarg.h>

void keydev_init(keydev_t *d)
{
	memset(d, 0, sizeof *d);
}

static int standard_repeater(GUNUSED int key, GUNUSED int duration, int count)
{
	/* FIXME: Do not use the delays from keydev_std() on every device */
	keydev_t *d = keydev_std();
	return count ? d->rep_standard_next : d->rep_standard_first;
}

//---
// Driver event generation
//---

/* keydev_queue_push(): Add an event in a device's buffer
   Returns false if the event cannot be pushed. */
bool keydev_queue_push(keydev_t *d, key_event_t ev)
{
	if(d->async_filter && !d->async_filter(ev))
		return true;

	int next = (d->queue_end + 1) % KEYBOARD_QUEUE_SIZE;
	if(next == d->queue_next)
	{
		d->events_lost++;
		return false;
	}

	d->queue[d->queue_end] = ev;
	d->queue_end = next;
	return true;
}

/* queue_poll(): Generate key events from the buffer
   Sets (*e) and returns true on success, otherwise false. */
static bool queue_poll(keydev_t *d, key_event_t *ev)
{
	if(d->queue_next == d->queue_end) return false;

	*ev = d->queue[d->queue_next];
	d->queue_next = (d->queue_next + 1) % KEYBOARD_QUEUE_SIZE;
	return true;
}

/* keydev_process_state(): Process the new keyboard states for events */
void keydev_process_state(keydev_t *d, uint8_t scan[12])
{
	key_event_t ev = { 0 };
	ev.time = d->time;

	/* Compare new data with the internal state. Push releases before
	   presses so that a key change occurring within a single analysis
	   frame can be performed. This happens all the time when going back to
	   the main MENU via gint_osmenu() on a keybind. */

	ev.type = KEYEV_UP;
	for(int mode = 0; mode < 2; mode++)
	{
		for(int row = 0; row < 12; row++)
		{
			int diff = mode
				? ~d->state_now[row] & scan[row]
				: d->state_now[row] & ~scan[row];
			if(!diff) continue;

			ev.key = (row << 4);

			for(int mask = 0x80; mask != 0; mask >>= 1)
			{
				/* Update state only if the push succeeds */
				if((diff & mask) && keydev_queue_push(d, ev))
					d->state_now[row] = mode
					? d->state_now[row] | mask
					: d->state_now[row] & ~mask;
				ev.key++;
			}
		}

		ev.type = KEYEV_DOWN;
	}
}

static bool can_repeat(keydev_t *d, int key)
{
	int tr = d->tr.enabled;
	int shift = tr & (KEYDEV_TR_DELAYED_SHIFT | KEYDEV_TR_INSTANT_SHIFT);
	int alpha = tr & (KEYDEV_TR_DELAYED_ALPHA | KEYDEV_TR_INSTANT_ALPHA);

	return !(key == KEY_SHIFT && shift) && !(key == KEY_ALPHA && alpha);
}

/* keydev_repeat_event(): Generate a repeat event if applicable */
key_event_t keydev_repeat_event(keydev_t *d)
{
	key_event_t ev = { 0 };
	ev.time = d->time;
	/* <Repeats> is disabled */
	if(!(d->tr.enabled & KEYDEV_TR_REPEATS)) return ev;
	/* No key is being repeated, or it's too early */
	if(!d->rep_key || d->rep_delay != 0) return ev;
	/* Key is blocked by transform options modified during the streak */
	if(!can_repeat(d, d->rep_key)) return ev;

	/* Plan the next repeat the currently-pressed key */
	int elapsed = (int16_t)(d->time - d->rep_time);
	d->rep_delay = -1;
	d->rep_count++;

	/* Returning < 0 will block further repeats */
	if(d->tr.repeater)
		d->rep_delay = d->tr.repeater(d->rep_key,elapsed,d->rep_count);

	/* Don't return an event on the first call (it's a KEYEV_DOWN) */
	if(!d->rep_count) return ev;

	ev.type = KEYEV_HOLD;
	ev.key = d->rep_key;
	return ev;
}

void keydev_tick(keydev_t *d, uint us)
{
	/* Generate the next repeat */
	key_event_t repeat = keydev_repeat_event(d);
	if(repeat.type != KEYEV_NONE)
		keydev_queue_push(d, repeat);

	d->time++;

	if(d->rep_key != 0)
	{
		if(d->rep_delay >= 0)
			d->rep_delay = max(d->rep_delay - (int)us, 0);
		d->rep_time += us;
	}
}

keydev_async_filter_t keydev_async_filter(keydev_t const *d)
{
	return d->async_filter;
}

void keydev_set_async_filter(keydev_t *d, keydev_async_filter_t filter)
{
	d->async_filter = filter;
}

//---
// Keyboard event generation
//---

/* keydev_unqueue_event(): Retrieve the next keyboard event in queue */
key_event_t keydev_unqueue_event(keydev_t *d)
{
	key_event_t ev = { 0 };
	ev.time = d->time;
	if(!queue_poll(d, &ev))
		return ev;

	/* Update the event state accordingly */
	int row = (ev.key >> 4);
	int col = 0x80 >> (ev.key & 0x7);

	if(ev.type == KEYEV_DOWN)
	{
		d->state_queue[row] |= col;
		/* Mark this key as the currently repeating one */
		if(d->rep_key == 0 && can_repeat(d, ev.key))
		{
			d->rep_key = ev.key;
			d->rep_count = -1;
			d->rep_time = 0;
			d->rep_delay = 0;
		}
	}
	else if(ev.type == KEYEV_UP)
	{
		d->state_queue[row] &= ~col;
		/* End the current repeating streak */
		if(d->rep_key == ev.key)
		{
			d->rep_key = 0;
			d->rep_count = -1;
			d->rep_time = -1;
			d->rep_delay = -1;
			d->delayed_shift = 0;
			d->delayed_alpha = 0;
		}
	}

	return ev;
}
__attribute__((alias("keydev_unqueue_event")))
key_event_t _WEAK_keydev_unqueue_event(keydev_t *d);

/* keydev_keydown(): Check if a key is down according to generated events */
bool keydev_keydown(keydev_t *d, int key)
{
	int row = (key >> 4);
	int col = 0x80 >> (key & 0x7);

	return (d->state_queue[row] & col) != 0;
}
__attribute__((alias("keydev_keydown")))
bool _WEAK_keydev_keydown(keydev_t *d, int key);

//---
// Event transforms
//---

/* keydev_transform(): Obtain current transform parameters */
keydev_transform_t keydev_transform(keydev_t *d)
{
	return d->tr;
}

/* keydev_set_transform(): Set transform parameters */
void keydev_set_transform(keydev_t *d, keydev_transform_t tr)
{
	int change = d->tr.enabled ^ tr.enabled;

	if(change & KEYDEV_TR_DELAYED_SHIFT)
	{
		d->pressed_shift = 0;
		d->delayed_shift = 0;
	}
	if(change & KEYDEV_TR_DELAYED_ALPHA)
	{
		d->pressed_alpha = 0;
		d->delayed_alpha = 0;
	}

	d->tr = tr;
}

/* keydev_set_standard_repeats(): Enable a simple repeater */
void keydev_set_standard_repeats(keydev_t *d, int first, int next)
{
	d->rep_standard_first = first;
	d->rep_standard_next = next;
	d->tr.repeater = standard_repeater;
}

/* keydev_read(): Retrieve the next transformed event */
key_event_t keydev_read(keydev_t *d, bool wait, volatile int *timeout)
{
	#define opt(NAME) (d->tr.enabled & KEYDEV_TR_ ## NAME)
	key_event_t e;

	while(1)
	{
		e = keydev_unqueue_event(d);
		if(e.type == KEYEV_NONE)
		{
			if(!wait || (timeout && *timeout))
				return e;
			sleep();
			continue;
		}

		int k = e.key;
		e.mod = (opt(ALL_MODS) != 0);

		// <Instant SHIFT> and <Instant ALPHA>

		if(e.type == KEYEV_DOWN || e.type == KEYEV_HOLD)
		{
			if(opt(INSTANT_SHIFT) && k != KEY_SHIFT)
				e.shift |= keydev_keydown(d, KEY_SHIFT);
			if(opt(INSTANT_ALPHA) && k != KEY_ALPHA)
				e.alpha |= keydev_keydown(d, KEY_ALPHA);
		}

		// <Delayed SHIFT> and <Delayed ALPHA>

		if(opt(DELAYED_SHIFT))
		{
			if(e.type == KEYEV_DOWN && k == KEY_SHIFT)
			{
				if(!d->delayed_shift)
					d->pressed_shift = 1;
				d->delayed_shift = 0;
			}
			else if(e.type != KEYEV_UP && k == d->rep_key)
			{
				e.shift |= d->delayed_shift;
				d->pressed_shift = 0;
			}
			else if(e.type == KEYEV_UP && d->pressed_shift)
			{
				d->pressed_shift = 0;
				d->delayed_shift = 1;
			}
		}

		if(opt(DELAYED_ALPHA))
		{
			if(e.type == KEYEV_DOWN && k == KEY_ALPHA)
			{
				if(!d->delayed_alpha)
					d->pressed_alpha = 1;
				d->delayed_alpha = 0;
			}
			else if(e.type != KEYEV_UP && k == d->rep_key)
			{
				e.alpha |= d->delayed_alpha;
				d->pressed_alpha = 0;
			}
			else if(e.type == KEYEV_UP && d->pressed_alpha)
			{
				d->pressed_alpha = 0;
				d->delayed_alpha = 1;
			}
		}

		// <Delete Modifiers>
		if(opt(DELETE_MODIFIERS) && !can_repeat(d, k)) continue;

		// <Delete Releases>
		if(opt(DELETE_RELEASES) && e.type == KEYEV_UP) continue;

		return e;
	}

	#undef opt
}
