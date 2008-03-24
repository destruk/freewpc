/*
 * Copyright 2008 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <freewpc.h>
#include <simulation.h>


#define RING_COUNT 32

#define ring_later(ticks) ((ring_now + (ticks)) % RING_COUNT)


/** The current time, modulo the ring count.  This is measured
 * in 1ms increments (more precisely, the number of IRQs). */
unsigned int ring_now = 0;

/** The timer ring.  Each entry contains a list of handlers to
 * be called when the current time reaches the value indicated
 * by the position in the array. */
struct time_handler *time_handler_ring[RING_COUNT] = { NULL, };


/** Allocate a new timer ring entry */
static struct time_handler *ring_malloc (void)
{
	return malloc (sizeof (struct time_handler));
}


/** Free a timer ring entry */
static void ring_free (struct time_handler *elem)
{
	free (elem);
}


/** Register a function to be called after N_TICKS have elapsed.
 * PERIOIDIC_P is nonzero if the timer function should be called indefinitely,
 * every time that much time has elapsed.
 * FN is the function to be called and DATA can be anything at all, passed to
 * the handler. */
void sim_time_register (int n_ticks, int periodic_p, time_handler_t fn, void *data)
{
	unsigned int ring = ring_later (n_ticks);

	struct time_handler *elem = ring_malloc ();
	if (!elem)
		simlog (SLC_DEBUG, "can't alloc ring");

	if (n_ticks > RING_COUNT)
		simlog (SLC_DEBUG, "can't schedule timer that far out");

	elem->next = time_handler_ring[ring];
	elem->periodicity = periodic_p ? n_ticks : 0;
	elem->fn = fn;
	elem->data = data;
	time_handler_ring[ring] = elem;
}


/** Advance the simulation time by 1 tick. */
void sim_time_step (void)
{
	struct time_handler *elem, *elem_next;

	/* Atomically get and clear the list of timers to
	 * be executed on this tick */
	elem = time_handler_ring[ring_now];
	time_handler_ring[ring_now] = NULL;

	/* Call each timer function */
	while (elem != NULL)
	{
		(*elem->fn) (elem->data);

		if (elem->periodicity)
		{
			/* If periodic, just requeue it rather than free/alloc */
			elem_next = elem->next;
			elem->next = time_handler_ring[ring_later (elem->periodicity)];
			time_handler_ring[ring_later (elem->periodicity)] = elem;
			elem = elem_next;
		}
		else
		{
			elem_next = elem->next;
			ring_free (elem);
			elem = elem_next;
		}
	}
	ring_now = ring_later (1);
}
