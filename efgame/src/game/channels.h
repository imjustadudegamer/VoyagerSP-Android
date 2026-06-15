typedef enum //# soundChannel_e
{
	CHAN_AUTO,	//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" # Auto-picks an empty channel to play sound on
	CHAN_LOCAL,	//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" # menu sounds, etc
	CHAN_WEAPON,//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" 
	CHAN_VOICE, //## %s !!"Q:\quake\baseEF\!!sound\voice\*.wav;*.mp3" # Voice sounds cause mouth animation
	CHAN_VOICE_ATTEN, //## %s !!"Q:\quake\baseEF\!!sound\voice\*.wav;*.mp3" # Causes mouth animation but still use normal sound falloff 
	CHAN_ITEM,  //## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3"
	CHAN_BODY,	//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3"
	CHAN_AMBIENT,//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" # added for ambient sounds
	CHAN_LOCAL_SOUND,	//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" #chat messages, etc
	CHAN_ANNOUNCER,		//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" #announcer voices, etc
	CHAN_MENU1,		//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" #menu stuff, etc
	CHAN_MENU2,		//## %s !!"Q:\quake\baseEF\!!sound\*.wav;*.mp3" #menu stuff, etc
} soundChannel_t;

