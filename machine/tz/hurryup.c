/*
 * Copyright 2006-2010 by Ewan Meadows <sonny_jim@hotmail.com>
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

/* CALLSET_SECTION (hurryup, __machine3__) */

#include <freewpc.h>

U8 hurryup_mode_timer;
score_t hurryup_score;

void hurryup_mode_init (void);
void hurryup_mode_exit (void);

struct timed_mode_ops hurryup_mode = {
	DEFAULT_MODE,
	.init = hurryup_mode_init,
	.exit = hurryup_mode_exit,
	.gid = GID_HURRYUP_MODE_RUNNING,
	.music = MUS_FASTLOCK_COUNTDOWN,
	.deff_running = DEFF_HURRYUP_MODE,
	.prio = PRI_GAME_MODE6,
	.init_timer = 15,
	.timer = &hurryup_mode_timer,
	.grace_timer = 3,
	.pause = system_timer_pause,
};

void hurryup_mode_deff (void)
{
//	while (hurryup_running ())
	for (;;)
	{
		dmd_alloc_low_clean ();
		font_render_string_center (&font_fixed10, 64, 8, "HURRY UP");
		sprintf_score (hurryup_score);
		font_render_string_center (&font_fixed6, 64, 17, sprintf_buffer);
		font_render_string_center (&font_var5, 64, 27, "SHOOT POWER PAYOFF");
		sprintf ("%d", hurryup_mode_timer);
		font_render_string (&font_var5, 2, 2, sprintf_buffer);
		font_render_string_right (&font_var5, 126, 2, sprintf_buffer);
		dmd_show_low ();
		task_sleep (TIME_500MS);
	}
}

void hurryup_awarded_deff (void)
{
	dmd_alloc_low_clean ();
	font_render_string_center (&font_var5, 64, 5, "HURRY UP AWARDED");
	sprintf_score (hurryup_score);
	font_render_string_center (&font_fixed6, 64, 16, sprintf_buffer);
	sound_send (SND_CLOCK_CHAOS_END_BOOM);
	dmd_show_low ();
	task_sleep_sec (2);
	deff_exit ();
}

/* Task to countdown the Hurry up score */
void hurryup_countdown_score_task (void)
{
	task_sleep_sec (2);
	while (hurryup_mode_timer > 0)
	{
		/* Pause whilst waiting for a kickout */
		while (kickout_locks > 0)
			task_sleep (TIME_500MS);
		score_sub (hurryup_score, score_table[SC_250K]);
		task_sleep (TIME_500MS);
	}
	task_exit ();
	
}

void hurryup_mode_init (void)
{
	score_zero (hurryup_score);
	score_copy (hurryup_score, score_table[SC_20M]);
//	task_create_gid (GID_HURRYUP_SCORE_COUNTDOWN, hurryup_countdown_score_task);
}

void hurryup_mode_exit (void)
{
	lamp_tristate_off (LM_POWER_PAYOFF);
	task_kill_gid (GID_HURRYUP_SCORE_COUNTDOWN);
}

void hurryup_mode_expire (void)
{
	lamp_tristate_off (LM_POWER_PAYOFF);
	task_kill_gid (GID_HURRYUP_SCORE_COUNTDOWN);
}


static inline void award_hurryup (void)
{
	task_kill_gid (GID_HURRYUP_SCORE_COUNTDOWN);
	score_long (hurryup_score);
	deff_start (DEFF_HURRYUP_AWARDED);
	timed_mode_end (&hurryup_mode);
}

CALLSET_ENTRY (hurryup, sw_power_payoff)
{
	if (timed_mode_running_p (&hurryup_mode))
	{
		award_hurryup ();
	}
}

CALLSET_ENTRY (hurryup, lamp_update)
{
	if (timed_mode_running_p (&hurryup_mode))
		lamp_tristate_flash (LM_POWER_PAYOFF);
}

CALLSET_ENTRY (hurryup, end_ball)
{
	timed_mode_end (&hurryup_mode);
}

CALLSET_ENTRY (hurryup, display_update)
{
	timed_mode_display_update (&hurryup_mode);
}

CALLSET_ENTRY (hurryup, music_refresh)
{
	timed_mode_music_refresh (&hurryup_mode);
}

CALLSET_ENTRY (hurryup, start_hurryup)
{
	timed_mode_begin (&hurryup_mode);
}

