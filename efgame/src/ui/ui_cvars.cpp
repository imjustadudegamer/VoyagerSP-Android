#include "ui_local.h"
#include "gameinfo.h"

//===================================================================
//
// Functions to set Cvars from menus
//
//===================================================================


/*
=================
InvertMouseCallback
=================
*/
void InvertMouseCallback( void *s, int notification )
{
	menulist_s *box = (menulist_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	if ( box->curvalue )
		ui.Cvar_SetValue( "m_pitch", -fabs( ui.Cvar_VariableValue( "m_pitch" ) ) );
	else
		ui.Cvar_SetValue( "m_pitch", fabs( ui.Cvar_VariableValue( "m_pitch" ) ) );

}

/*
=================
MouseSpeedCallback
=================
*/
void MouseSpeedCallback( void *s, int notification )
{
	menuslider_s	*slider = (menuslider_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "sensitivity", slider->curvalue );
}

/*
=================
SmoothMouseCallback
=================
*/
void SmoothMouseCallback( void *s, int notification )
{
	menulist_s *box = (menulist_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "m_filter", box->curvalue );
}

/*
=================
JoyXButtonCallback
=================
*/
void JoyXButtonCallback( void *s, int notification )
{
	menulist_s *box = (menulist_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "joy_xbutton", box->curvalue );
}

/*
=================
JoyYButtonCallback
=================
*/
void JoyYButtonCallback( void *s, int notification )
{
	menulist_s *box = (menulist_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "joy_ybutton", box->curvalue );
}

/*
=================
GammaCallback
=================
*/
void GammaCallback( void *s, int notification )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "r_gamma", slider->curvalue / 10.0f );
}


/*
=================
IntensityCallback
=================
*/
void IntensityCallback( void *s, int notification )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "r_intensity", slider->curvalue / 10.0f );
}

/*
=================
ScreensizeCallback
=================
*/
void ScreensizeCallback( void *s, int notification )
{
	menuslider_s *slider = ( menuslider_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "cg_viewsize", slider->curvalue);
}

/*
=================
AlwaysRunCallback
=================
*/
void AlwaysRunCallback( void *s, int notification )
{
	 menulist_s *s_alwaysrun_box = ( menulist_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "cl_run", s_alwaysrun_box->curvalue );
}


/*
=================
AutoswitchCallback
=================
*/
void AutoswitchCallback( void *s, int notification )
{
	menulist_s	*s_autoswitch_box = ( menulist_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "cg_autoswitch", s_autoswitch_box->curvalue );
}


/*
=================
VideoUsingLoResCallback
=================
*/
void VideoUsingLoResCallback( void *s, int notification )
{
	menulist_s *s_cinematic_quality = ( menulist_s * ) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "cl_VideoUsingLoRes", s_cinematic_quality->curvalue );
}

/*
=================
JoystickEnableCallback
=================
*/
void JoystickEnableCallback( void *s, int notification )
{
	menulist_s *s_joyenable_box = (menulist_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "in_joystick", s_joyenable_box->curvalue );
	ui.Cmd_ExecuteText( EXEC_APPEND, "in_restart\n" );
}


/*
=================
JoystickThresholdCallback
=================
*/
void JoystickThresholdCallback( void *s, int notification )
{

	menuslider_s *s_joythreshold_slider = (menuslider_s *) s;

	if (notification != QM_ACTIVATED)
		return;
	ui.Cvar_SetValue( "joy_threshold", s_joythreshold_slider->curvalue );
}

/*
=================
GamepadLookCallback
=================
*/
void GamepadLookCallback( void *s, int notification )
{
	menuslider_s *slider = (menuslider_s *) s;

	if (notification != QM_ACTIVATED)
		return;
	ui.Cvar_SetValue( "gp_lookSens", slider->curvalue );
}


/*
=================
TurnSpeedCallback
=================
*/
void TurnSpeedCallback( void *s, int notification )
{

	menuslider_s *s_keyturnspeed_slider = (menuslider_s *) s;

	if (notification != QM_ACTIVATED)
		return;
	ui.Cvar_SetValue( "cl_anglespeedkey", s_keyturnspeed_slider->curvalue );
}


/*
=================
AnisotropicFilteringCallback
=================
*/
void AnisotropicFilteringCallback( void *s, int notification )
{
	menulist_s *anisotropicfiltering = ( menulist_s * ) s;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	ui.Cvar_SetValue( "r_ext_texture_filter_anisotropic", anisotropicfiltering->curvalue );
}

/*
=================
CompressedTextureCallback
=================
*/
void CompressedTextureCallback( void *s, int notification )
{
	menulist_s *anisotropicfiltering = ( menulist_s * ) s;

	if (notification != QM_ACTIVATED)
	{
		return;
	}

	ui.Cvar_SetValue( "r_ext_texture_filter_anisotropic", anisotropicfiltering->curvalue );
}

/*
=================
ForcefeedBackCallback
=================
*/
void ForcefeedBackCallback( void *s, int notification )
{
	menulist_s *s_forcefeedback_box = (menulist_s *) s;

	if (notification != QM_ACTIVATED)
		return;

	ui.Cvar_SetValue( "use_ff", s_forcefeedback_box->curvalue );
	ui.Cmd_ExecuteText( EXEC_APPEND, "in_restart\n" );
}

