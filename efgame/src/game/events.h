#ifndef __EVENTS__
#define __EVENTS__

typedef enum //# eventType_e
{
	EV_BAD = 0,
	EV_BEAM_DOWN,
	EV_BEAM_UP,
	EV_SCAV_TELEPORT,
	EV_STASIS_BEAM_IN,
	EV_STASIS_BEAM_OUT,
	EV_DISINTEGRATE,
	EV_BORG_TELEPORT_FX,
	EV_8472_BEAM_OUT

} eventType_t;	

#endif	//__EVENTS__