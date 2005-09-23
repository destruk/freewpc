
#include <freewpc.h>
#include <mach/switch.h>
#include <mach/coil.h>


void sw_rocket_handler (void)
{
	device_sw_handler (3);
}

DECLARE_SWITCH_DRIVER (sw_rocket)
{
	.fn = sw_rocket_handler,
};

void rocket_kick_sound (void)
{
	sound_send (SND_ROCKET_KICK_DONE);
	task_exit ();
}

void rocket_kick_attempt (device_t *dev)
{
	db_puts ("Sending rocket kick sound\n");
	sound_send (SND_ROCKET_KICK_REVVING);
	task_sleep (TIME_100MS * 7);
	task_create_gid (0, rocket_kick_sound);
}



device_ops_t rocket_ops = {
	.kick_attempt = rocket_kick_attempt,
};

device_properties_t rocket_props = {
	.ops = &rocket_ops,
	.name = "ROCKET KICKER",
	.sol = SOL_ROCKET,
	.sw_count = 1,
	.init_max_count = 0,
	.sw = { SW_ROCKET },
};


void rocket_init (void)
{
	device_register (3, &rocket_props);
}

