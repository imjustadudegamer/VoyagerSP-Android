//NPC_sounds.cpp
#include "b_local.h"
#include "Q3_Interface.h"

void NPC_AngerSound (void)
{
	if(NPCInfo->investigateSoundDebounceTime)
		return;

	NPCInfo->investigateSoundDebounceTime = 1;

	switch((int)NPC->client->race)
	{
	case RACE_KLINGON:
		//G_Sound(NPC, G_SoundIndex(va("sound/mgtest/klingon/talk%d.wav",	Q_irand(1, 4))));
		break;
	}
}

void G_AddVoiceEvent( gentity_t *self, int event, int speakDebounceTime )
{
	if ( !self->NPC )
	{
		return;
	}

	if ( !self->client || self->client->ps.pm_type >= PM_DEAD )
	{
		return;
	}

	if ( self->NPC->blockedSpeechDebounceTime > level.time )
	{
		return;
	}

	if ( Q3_TaskIDPending( self, TID_CHAN_VOICE ) )
	{
		return;
	}

	if ( (self->NPC->scriptFlags&SCF_NO_COMBAT_TALK) && (event < EV_FF_1A || event > EV_FF_3C) && (event < EV_RESPOND1 || event > EV_MISSION3) )
	{
		return;
	}
	//FIXME: Also needs to check for teammates. Don't want
	//		everyone babbling at once

	G_AddEvent( self, event, 0 );
	//won't speak again for 5 seconds (unless otherwise specified)
	self->NPC->blockedSpeechDebounceTime = level.time + ((speakDebounceTime==0) ? 5000 : speakDebounceTime);
}
