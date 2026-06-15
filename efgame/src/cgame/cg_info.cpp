
#include "cg_local.h"
#include "cg_media.h"
#include "../game/objectives.h"
#include "cg_text.h"

static int missionYcnt;
static int tacticalYcnt;
static int missionYpos;
static int tacticalYpos;
static int	missionInfoScreenY;

int statusTextIndex = -1;

extern ginfoitem_t	bg_infoItemList[];

#define BASE_X 102
#define ADD_X 170

/*
====================
TourTacticalPrint_Line
====================
*/
static void TourTacticalPrint_Line(int strIndex,int figureCount,int figureTotal)
{
	char *str,*strBegin;
	int y,pixelLen,xPos;
	int charLen;
	char holdText[1024], holdText2[2];

	assert(tactical_text[strIndex]);

	str =  tactical_text[strIndex];

	pixelLen = CG_ProportionalStringWidth(str,CG_TINYFONT);

	y = 0;
	xPos = 0;

	if (tacticalYcnt < 6)
	{
		y = tacticalYpos + ((PROP_TINY_HEIGHT + 4) * (tacticalYcnt));
		xPos = BASE_X;
	}
	else if (tacticalYcnt < 12)
	{
		y = (missionInfoScreenY + 198) + ((PROP_TINY_HEIGHT + 4) * (tacticalYcnt - 6));
		xPos = BASE_X + ADD_X;
	}
	else if (tacticalYcnt < 18)
	{
		y = (missionInfoScreenY + 198) + ((PROP_TINY_HEIGHT + 4) * (tacticalYcnt - 12));
		xPos = BASE_X + ADD_X + ADD_X;
	}

	// The little dot marking the beginning of a block of tactical info
	cgi_R_SetColor( colorTable[CT_LTBLUE1]);
	CG_DrawPic( xPos - 8,  y + 3,  4,  4,cgs.media.notpending);	// Corner

	if (pixelLen < 502)	// One shot - small enough to just print on screen
	{
		if (strIndex == TACT_HOWMANY_DOLLS)
		{
			CG_DrawProportionalString(xPos, y, va(tactical_text[strIndex],figureCount,figureTotal), CG_TINYFONT, colorTable[CT_LTBLUE2] );
		}		
		else
		{
			CG_DrawProportionalString(xPos, y, tactical_text[strIndex], CG_TINYFONT, colorTable[CT_LTBLUE2] );
		}
		++tacticalYcnt;
	}
	// Text is too long.
	else
	{
		pixelLen = 0;
		charLen = 0;
		holdText2[1] = NULL;
		strBegin = str;

		while( *str ) 
		{
			holdText2[0] = *str;
			pixelLen += CG_ProportionalStringWidth(holdText2,CG_TINYFONT);
			pixelLen += 1; // For kerning
			++charLen;

			if (pixelLen > 502 ) 
			{	//Reached max length of this line
				//step back until we find a space
				while ((charLen) && (*str != ' ' ))
				{
					--str;
					--charLen;
				}

				if (*str==' ')
				{
					++str;	// To get past space
				}

				Q_strncpyz( holdText, strBegin, charLen);
				holdText[charLen] = NULL;
				strBegin = str;
				pixelLen = 0;
				charLen = 1;

				y = tacticalYpos + (14 * tacticalYcnt);
				CG_DrawProportionalString(xPos, y, holdText, CG_TINYFONT, colorTable[CT_LTBLUE2] );
				++tacticalYcnt;
			} 
			else if (*(str+1) == NULL)
			{
				Q_strncpyz( holdText, strBegin, charLen);
				y = tacticalYpos + (14 * tacticalYcnt);
				CG_DrawProportionalString(xPos, y, holdText, CG_TINYFONT, colorTable[CT_LTBLUE2] );
				++tacticalYcnt;
				break;
			}
			++str; 
		} 
	}
}

/*
====================
TacticalPrint_Line
====================
*/
static void TacticalPrint_Line(int strIndex)
{
	char *str,*strBegin;
	int y,pixelLen,charLen,xPos;
	char holdText[1024], holdText2[2];

	assert(tactical_text[strIndex]);

	str =  tactical_text[strIndex];

	pixelLen = CG_ProportionalStringWidth(str,CG_TINYFONT);

	xPos = 102;

	// The little dot marking the beginning of a block of tactical info
	cgi_R_SetColor( colorTable[CT_LTBLUE1]);
	y = tacticalYpos + ((PROP_TINY_HEIGHT + 4) * (tacticalYcnt));
	CG_DrawPic( xPos - 8,  y + 3,  4,  4,cgs.media.notpending);	// Corner

	if (pixelLen < 502)	// One shot - small enough to just print on screen
	{
		y = tacticalYpos + (14 * (tacticalYcnt));
		CG_DrawProportionalString(xPos, y, tactical_text[strIndex], CG_TINYFONT, colorTable[CT_LTBLUE2] );
		++tacticalYcnt;
	}
	// Text is too long, break into lines.
	else
	{
		pixelLen = 0;
		charLen = 0;
		holdText2[1] = NULL;
		strBegin = str;

		while( *str ) 
		{
			holdText2[0] = *str;
			pixelLen += CG_ProportionalStringWidth(holdText2,CG_TINYFONT);
			pixelLen += 1; // For kerning
			++charLen;

			if (pixelLen > 502 ) 
			{	//Reached max length of this line
				//step back until we find a space
				while ((charLen) && (*str != ' ' ))
				{
					--str;
					--charLen;
				}

				if (*str==' ')
				{
					++str;	// To get past space
				}

				Q_strncpyz( holdText, strBegin, charLen);
				holdText[charLen] = NULL;
				strBegin = str;
				pixelLen = 0;
				charLen = 1;

				y = tacticalYpos + (14 * tacticalYcnt);
				CG_DrawProportionalString(xPos, y, holdText, CG_TINYFONT, colorTable[CT_LTBLUE2] );
				++tacticalYcnt;
			} 
			else if (*(str+1) == NULL)
			{
				Q_strncpyz( holdText, strBegin, charLen);
				y = tacticalYpos + (14 * tacticalYcnt);
				CG_DrawProportionalString(xPos, y, holdText, CG_TINYFONT, colorTable[CT_LTBLUE2] );
				++tacticalYcnt;
				break;
			}
			++str; 
		} 
	}
}

#define MAX_OBJECTIVETEXT 2048

/*
====================
MissionPrint_Line
====================
*/
static void MissionPrint_Line(objectives_t objective,int color,centity_t *cent,char *missionText)
{
	char *str,*strBegin;
	int y,pixelLen,charLen;
	char holdText[1024], holdText2[2];
	char finalText[MAX_OBJECTIVETEXT];
	int len,len_s;
	char *s;

	assert(missionText);

	str =  missionText;

	if (objective.status == OBJECTIVE_STAT_PENDING)
	{
		s = "\0";
	}
	else if (objective.status == OBJECTIVE_STAT_FAILED)
	{
		s = ingame_text[IGT_FAILED];
	}
	else
	{
		s = ingame_text[IGT_SUCCEEDED];
	}

	len = strlen(str);
	len++;
	Q_strncpyz(finalText,str,len);
	len_s = strlen(str);
	Q_strncpyz((finalText+(len-1)), s, len_s);

	pixelLen = CG_ProportionalStringWidth(finalText,CG_SMALLFONT);

	str = finalText;

	if (pixelLen < 500)	// One shot - small enough to print entirely on one line
	{
		y =missionYpos + (18 * (missionYcnt));
		CG_DrawProportionalString(108, y,str, CG_SMALLFONT, colorTable[color] );
		++missionYcnt;
	}
	// Text is too long, break into lines.
	else
	{
		pixelLen = 0;
		charLen = 0;
		holdText2[1] = NULL;
		strBegin = str;

		while( *str ) 
		{
			holdText2[0] = *str;
			pixelLen += CG_ProportionalStringWidth(holdText2,CG_SMALLFONT);
			pixelLen += 2; // For kerning
			++charLen;

			if (pixelLen > 500 ) 
			{	//Reached max length of this line
				//step back until we find a space
				while ((charLen) && (*str != ' ' ))
				{
					--str;
					--charLen;
				}

				if (*str==' ')
				{
					++str;	// To get past space
				}

				Q_strncpyz( holdText, strBegin, charLen);
				holdText[charLen] = NULL;
				strBegin = str;
				pixelLen = 0;
				charLen = 1;

				y = missionYpos + (18 * missionYcnt);
				CG_DrawProportionalString(108, y, holdText, CG_SMALLFONT, colorTable[color] );
				++missionYcnt;
			} 
			else if (*(str+1) == NULL)
			{
				++charLen;

				y = missionYpos + (18 * missionYcnt);
				Q_strncpyz( holdText, strBegin, charLen);
				CG_DrawProportionalString(108, y, holdText, CG_SMALLFONT, colorTable[color] );
				++missionYcnt;
				break;
			}
			++str; 
		} 
	}
}

/*
====================
MissionInformation_Draw
====================
*/
static void MissionInformation_Draw( centity_t *cent )
{
	int i,addLength,color;
	vec4_t newColor; 
	int tacticalCnt,lines,pixelLen;

	missionInfoScreenY = 50;

	// Upper Background
	newColor[0] = colorTable[CT_BLACK][0];
	newColor[1] = colorTable[CT_BLACK][1];
	newColor[2] = colorTable[CT_BLACK][2];
	newColor[3] = 0.5;
	cgi_R_SetColor(newColor);
	CG_DrawPic( 77, missionInfoScreenY,   535, 174, cgs.media.whiteShader);	
	CG_DrawPic( 30, missionInfoScreenY + 168,  47, 9, cgs.media.whiteShader);	

	// Count active tactical lines
	tacticalCnt=0;
	for (i=0;i<MAX_TACTICAL_TXT;++i)
	{
		if ((tactical_info[i]) && (tactical_text[i]))
		{
			pixelLen = CG_ProportionalStringWidth(tactical_text[i],CG_TINYFONT);
			lines = pixelLen / 500;
			lines += 1;

			tacticalCnt+=lines;					
		}
	}

	if (tacticalCnt == 0)
	{
		tacticalCnt=1;
	}

	// Lower background
	cgi_R_SetColor(newColor);
	CG_DrawPic( 77, missionInfoScreenY + 174, 535, ((PROP_TINY_HEIGHT + 4) * tacticalCnt) + 16 + 14, cgs.media.whiteShader);


	// Frame
	cgi_R_SetColor( colorTable[CT_DKPURPLE3]);
	CG_DrawPic( 30,  missionInfoScreenY,  64,   32, cgs.media.objcorner1);	// Top Corner

	CG_DrawPic( 50,  missionInfoScreenY,  353,  22, cgs.media.whiteShader);	// Top line

	CG_DrawPic( 30, missionInfoScreenY + 17,   47, 154, cgs.media.whiteShader);	// Middle column

	CG_DrawProportionalString(611, missionInfoScreenY, ingame_text[IGT_MISSIONINFORMATION], CG_RIGHT | CG_BIGFONT, colorTable[CT_LTBLUE1] );

	// Tactical Info
	cgi_R_SetColor( colorTable[CT_DKPURPLE3]);
	CG_DrawPic( 81,  missionInfoScreenY + 174, 521, 16, cgs.media.whiteShader);		// Top
	CG_DrawPic( 602,  missionInfoScreenY + 174,  16,  32, cgs.media.objcorner2);	// Corner


	CG_DrawProportionalString( 96,  missionInfoScreenY + 174, ingame_text[IGT_TACTICALINFO],CG_SMALLFONT, colorTable[CT_BLACK] );
	CG_DrawProportionalString( 96, missionInfoScreenY +  6, ingame_text[IGT_OBJECTIVES], CG_SMALLFONT, colorTable[CT_BLACK] );

	missionYcnt = 0;

	missionYpos = missionInfoScreenY + 29;
	tacticalYpos = missionInfoScreenY + 198;

	// Print active objectives
	for (i=0;i<MAX_OBJECTIVES;++i)
	{
		if (cent->gent->client->sess.mission_objectives[i].display)
		{

			if (cent->gent->client->sess.mission_objectives[i].status == OBJECTIVE_STAT_SUCCEEDED)
			{
				cgi_R_SetColor( colorTable[CT_LTBLUE1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.notpending);	// Full Circle
				cgi_R_SetColor( colorTable[CT_DKBLUE1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.pending);	// Circle in front
				color = CT_LTBLUE1;
			}
			else if (cent->gent->client->sess.mission_objectives[i].status == OBJECTIVE_STAT_FAILED)
			{
				cgi_R_SetColor( colorTable[CT_LTRED1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.notpending);	// Full Circle
				cgi_R_SetColor( colorTable[CT_DKRED1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.pending);	// Circle in front
				color = CT_LTRED1;
			}
			else		//	OBJECTIVE_STAT_PENDING
			{
				cgi_R_SetColor( colorTable[CT_LTGOLD1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.pending);	// Circle in front
				color = CT_LTGOLD1;
			}

			MissionPrint_Line(cent->gent->client->sess.mission_objectives[i],color,cent,mission_objective_text[i]);
		}
	}

	if (!missionYcnt)
	{
		CG_DrawProportionalString(108, missionYpos, ingame_text[IGT_NONE1], CG_SMALLFONT, colorTable[CT_LTBLUE1] );
	}


	tacticalYcnt = 0;

	// Print active tactical info
	for (i=0;i<MAX_TACTICAL_TXT;++i)
	{
		if ((tactical_info[i]) && (tactical_text[i]))
		{
			TacticalPrint_Line(i);
		}
	}

	if (!tacticalYcnt)
	{
		CG_DrawProportionalString(102, tacticalYpos + (14 * (tacticalYcnt)), ingame_text[IGT_NONE1], CG_TINYFONT, colorTable[CT_LTBLUE2] );
		tacticalYcnt = 1;
	}

	addLength = (tacticalYcnt * 14);

	cgi_R_SetColor( colorTable[CT_DKPURPLE3]);
	CG_DrawPic( 30,   missionInfoScreenY + 174,   47, (8 + 16 + 14 + addLength), cgs.media.whiteShader);	// Middle column
	CG_DrawPic( 604,  missionInfoScreenY + 182,   8,  (     16 + 14 + addLength), cgs.media.whiteShader);	// Right Side
	CG_DrawPic( 30, (missionInfoScreenY + 174 + 16 + 14 + addLength),  582, 8, cgs.media.whiteShader);	// Bottom

}

/*
====================
TourModeObjectives
====================
*/
static void TourModeObjectives( centity_t *cent )
{
	int i,addLength,color;
	vec4_t newColor; 
	int tacticalCnt,lines,pixelLen,figureCount,figureTotal;

	missionInfoScreenY = 50;

	// Count number of action figures collected
	figureCount = 0;
	figureTotal = 9;	// Change this if more Dolls are added.
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_MUNRO].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}

	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_FOSTER].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_TELSIA].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_ALEXANDRIA].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_BIESSMAN].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_CHELL].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_CHANG].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_JUROT].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}
	if (cent->gent->client->tourSess.tour_objectives[OBJ_DOLL_BORG_FOSTER].status == OBJECTIVE_STAT_SUCCEEDED)
	{
		figureCount++;
	}

	// Count number of action figures collected
	if (cent->gent->client->tourSess.tour_objectives[OBJ_COLLECTEDACTIONFIGURES].status != OBJECTIVE_STAT_SUCCEEDED)
	{
		// Collected all action figures so don't print count
		if (figureCount==figureTotal)		// Got all the action figures so show the tactical
		{
			if (cent->gent->client->tourSess.tour_objectives[OBJ_TOURSUCCESS].display != OBJECTIVE_SHOW)
			{
				tactical_info[TACT_EASTEREGG4] = qtrue;
			}
			else
			{
				tactical_info[TACT_EASTEREGG4] = qfalse;
			}

			tactical_info[TACT_HOWMANY_DOLLS] = qfalse;
		}
		else if (figureCount>0)				// Got at least one of the action figures
		{
			tactical_info[TACT_EASTEREGG4] = qfalse;
			tactical_info[TACT_HOWMANY_DOLLS] = qtrue;
		}
		else								// Got no action figures
		{
			tactical_info[TACT_EASTEREGG4] = qfalse;
			tactical_info[TACT_HOWMANY_DOLLS] = qfalse;
		}
	}
	else
	{
			tactical_info[TACT_EASTEREGG4] = qfalse;
			tactical_info[TACT_HOWMANY_DOLLS] = qfalse;
	}

	// Upper Background
	newColor[0] = colorTable[CT_BLACK][0];
	newColor[1] = colorTable[CT_BLACK][1];
	newColor[2] = colorTable[CT_BLACK][2];
	newColor[3] = 0.5;
	cgi_R_SetColor(newColor);
	CG_DrawPic( 77, missionInfoScreenY,   535, 174, cgs.media.whiteShader);	
	CG_DrawPic( 30, missionInfoScreenY + 168,  47, 9, cgs.media.whiteShader);	

	// Count active tactical lines
	tacticalCnt=0;
	for (i=0;i<MAX_TACTICAL_TXT;++i)
	{
		if ((tactical_info[i]) && (tactical_text[i]))
		{
			pixelLen = CG_ProportionalStringWidth(tactical_text[i],CG_TINYFONT);
			lines = pixelLen / 500;
			lines += 1;

			tacticalCnt+=lines;					
		}
	}

	if (tacticalCnt == 0)
	{
		tacticalCnt=1;
	}
	else if (tacticalCnt > 6)
	{
		tacticalCnt = 6;
	}

	// Lower background
	cgi_R_SetColor(newColor);
	CG_DrawPic( 77, missionInfoScreenY + 174, 535, ((PROP_TINY_HEIGHT + 4) * tacticalCnt) + 16 + 14, cgs.media.whiteShader);


	// Frame
	cgi_R_SetColor( colorTable[CT_RED]);
	CG_DrawPic( 30,  missionInfoScreenY,  64,   32, cgs.media.objcorner1);	// Top Corner

	CG_DrawPic( 50,  missionInfoScreenY,  353,  22, cgs.media.whiteShader);	// Top line

	CG_DrawPic( 30, missionInfoScreenY + 17,   47, 154, cgs.media.whiteShader);	// Middle column

	CG_DrawProportionalString(611, missionInfoScreenY, ingame_text[IGT_SECURITY_STATUS], CG_RIGHT | CG_BIGFONT, colorTable[CT_LTBLUE1] );

	// Tactical Info
	cgi_R_SetColor( colorTable[CT_RED]);
	CG_DrawPic( 81,  missionInfoScreenY + 174, 521, 16, cgs.media.whiteShader);		// Top
	CG_DrawPic( 602,  missionInfoScreenY + 174,  16,  32, cgs.media.objcorner2);	// Corner


	CG_DrawProportionalString( 96,  missionInfoScreenY + 174, ingame_text[IGT_ITEMS],CG_SMALLFONT, colorTable[CT_BLACK] );

	missionYcnt = 0;

	missionYpos = missionInfoScreenY + 29;
	tacticalYpos = missionInfoScreenY + 198;

	// Print active objectives
	for (i=0;i<MAX_TOUR_OBJECTIVES;++i)
	{
		if (cent->gent->client->tourSess.tour_objectives[i].display)
		{

			if (cent->gent->client->tourSess.tour_objectives[i].status == OBJECTIVE_STAT_SUCCEEDED)
			{
				cgi_R_SetColor( colorTable[CT_LTBLUE1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.notpending);	// Full Circle
				cgi_R_SetColor( colorTable[CT_DKBLUE1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.pending);	// Circle in front
				color = CT_LTBLUE1;
			}
			else if (cent->gent->client->tourSess.tour_objectives[i].status == OBJECTIVE_STAT_FAILED)
			{
				cgi_R_SetColor( colorTable[CT_LTRED1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.notpending);	// Full Circle
				cgi_R_SetColor( colorTable[CT_DKRED1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.pending);	// Circle in front
				color = CT_LTRED1;
			}
			else		//	OBJECTIVE_STAT_PENDING
			{
				cgi_R_SetColor( colorTable[CT_LTGOLD1]);
				CG_DrawPic( 88,   missionYpos + (18 * (missionYcnt)),   16,  16, cgs.media.pending);	// Circle in front
				color = CT_LTGOLD1;
			}

			MissionPrint_Line(cent->gent->client->tourSess.tour_objectives[i],color,cent,tour_objective_text[i]);
		}
	}

	if (!missionYcnt)
	{
		CG_DrawProportionalString(108, missionYpos, ingame_text[IGT_NONE1], CG_SMALLFONT, colorTable[CT_LTBLUE1] );
	}


	tacticalYcnt = 0;

	// Print active tactical info
	for (i=0;i<MAX_TACTICAL_TXT;++i)
	{
		if ((tactical_info[i]) && (tactical_text[i]))
		{
			TourTacticalPrint_Line(i,figureCount,figureTotal);
		}
	}

	if (!tacticalYcnt)
	{
		CG_DrawProportionalString(102, tacticalYpos + (14 * (tacticalYcnt)), ingame_text[IGT_NONE1], CG_TINYFONT, colorTable[CT_LTBLUE2] );
		tacticalYcnt = 1;
	}
	else if (tacticalYcnt > 6)
	{
		tacticalYcnt = 6;
	}

	addLength = (tacticalYcnt * 14);

	cgi_R_SetColor( colorTable[CT_RED]);
	CG_DrawPic( 30,   missionInfoScreenY + 174,   47, (8 + 16 + 14 + addLength), cgs.media.whiteShader);	// Middle column
	CG_DrawPic( 604,  missionInfoScreenY + 182,   8,  (     16 + 14 + addLength), cgs.media.whiteShader);	// Right Side
	CG_DrawPic( 30, (missionInfoScreenY + 174 + 16 + 14 + addLength),  582, 8, cgs.media.whiteShader);	// Bottom

}

/*
====================
CG_DrawMissionInformation
====================
*/
void CG_DrawMissionInformation( void ) 
{
	centity_t *cent;

	// Don't show if dead
	if (cg.predicted_player_state.pm_type == PM_DEAD)
	{
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];

	if (cg_virtualVoyager.value == 1)
	{
		TourModeObjectives(cent);
	}
	else
	{
		MissionInformation_Draw(cent);
	}


	missionInfo_Updated = qfalse;	// Player saw it
	cg.missionInfoFlashTime = 0;

}

/*
================
CG_CenterLine
================
*/
void CG_CenterLine( float y, const char *string ) 
{
	CG_DrawProportionalString( (SCREEN_WIDTH/2), y, string, UI_CENTER | UI_BIGFONT,colorTable[CT_LTBLUE1]);
}


void CG_LoadBar(void)
{
	int			x,y;

	// Round LCARS buttons
	y = 244;
	x = 10;
	if (cg.loadLCARSStage < 1)
	{
		cgi_R_SetColor( colorTable[CT_VDKPURPLE3]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_VLTPURPLE3]);
	}
	CG_DrawPic( x + 18,   y +102, 128,  64,cgs.media.loading1);


	if (cg.loadLCARSStage < 2)
	{
		cgi_R_SetColor( colorTable[CT_VDKBLUE1]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_VLTBLUE1]);
	}
	CG_DrawPic(      x,   y + 37,  64,  64,cgs.media.loading2);


	if (cg.loadLCARSStage < 3)
	{
		cgi_R_SetColor( colorTable[CT_VDKPURPLE1]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_LTPURPLE1]);
	}
	CG_DrawPic( x + 17,        y, 128,  64,cgs.media.loading3);


	if (cg.loadLCARSStage < 4)
	{
		cgi_R_SetColor( colorTable[CT_VDKPURPLE2]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_LTPURPLE2]);
	}
	CG_DrawPic( x + 99,        y, 128, 128,cgs.media.loading4);


	if (cg.loadLCARSStage < 5)
	{
		cgi_R_SetColor( colorTable[CT_VDKBLUE2]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_VLTBLUE2]);
	}
	CG_DrawPic( x +137,   y + 81,  64,  64,cgs.media.loading5);


	if (cg.loadLCARSStage < 6)
	{
		cgi_R_SetColor( colorTable[CT_VDKORANGE]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_LTORANGE]);
	}
	CG_DrawPic( x + 45,   y + 99, 128,  64,cgs.media.loading6);


	if (cg.loadLCARSStage < 7)
	{
		cgi_R_SetColor( colorTable[CT_VDKBLUE2]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_LTBLUE2]);
	}
	CG_DrawPic( x + 38,   y + 24,  64, 128,cgs.media.loading7);

	if (cg.loadLCARSStage < 8)
	{
		cgi_R_SetColor( colorTable[CT_VDKPURPLE1]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_LTPURPLE1]);
	}
	CG_DrawPic( x + 78,   y + 20, 128,  64,cgs.media.loading8);

	if (cg.loadLCARSStage < 9)
	{
		cgi_R_SetColor( colorTable[CT_VDKBROWN1]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_VLTBROWN1]);
	}
	CG_DrawPic( x +112,   y + 66,  64, 128,cgs.media.loading9);


	if (cg.loadLCARSStage < 9)
	{
		cgi_R_SetColor( colorTable[CT_DKBLUE2]);
	}
	else
	{
		cgi_R_SetColor( colorTable[CT_LTBLUE2]);
	}
	CG_DrawPic( x + 62,   y + 44, 128, 128,cgs.media.loadingcircle);	// Center arrows

	cg.loadLCARScnt++;
	if (cg.loadLCARScnt > 3)
	{
		cg.loadLCARScnt = 0;
	}

	cgi_R_SetColor( colorTable[CT_DKPURPLE2]);
	CG_DrawPic( x +  61,   y + 43,  32,  32,cgs.media.loadingquarter);	// Quad UL
	CG_DrawPic( x + 135,   y + 43, -32,  32,cgs.media.loadingquarter);	// Quad UR
	CG_DrawPic( x + 135,   y +117, -32, -32,cgs.media.loadingquarter);	// Quad LR
	CG_DrawPic( x +  61,   y +117,  32, -32,cgs.media.loadingquarter);	// Quad LL

	cgi_R_SetColor( colorTable[CT_LTPURPLE2]);
	switch (cg.loadLCARScnt)
	{
	case 0 :
		CG_DrawPic( x +  61,   y + 43,  32,  32,cgs.media.loadingquarter);	// Quad UL
		break;
	case 1 :
		CG_DrawPic( x + 135,   y + 43, -32,  32,cgs.media.loadingquarter);	// Quad UR
		break;
	case 2 :
		CG_DrawPic( x + 135,   y +117, -32, -32,cgs.media.loadingquarter);	// Quad LR
		break;
	case 3 :
		CG_DrawPic( x +  61,   y +117,  32, -32,cgs.media.loadingquarter);	// Quad LL
		break;
	} 

//	cgi_R_SetColor( colorTable[CT_DKBROWN1]);
//	CG_DrawPic( x + 130, y - 7 ,  64, 16,cgs.media.loadingtrim);	
//	CG_DrawPic( x +  130, y + 150,  64, -16,cgs.media.loadingtrim);	

//	CG_DrawPic( x +  150, y - 7,   432,  8, cgs.media.whiteShader);		// Top line
//	CG_DrawPic( x +  150, y + 142, 432,  8, cgs.media.whiteShader);		// Bottom line
//	CG_DrawPic( x +  583, y - 3,      16, 148, cgs.media.whiteShader);	// Side line

//	CG_DrawPic( x +  580, y + 4,      32, -16,cgs.media.loadingcorner);	
//	CG_DrawPic( x +  580, y + 139,    32, 16,cgs.media.loadingcorner);	

	CG_DrawProportionalString( x +  21, y + 150, "0987",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x +   3, y +  90,   "18",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x +  24, y +  20,    "7",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x +  93, y +   5,   "51",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x + 103, y +   5,   "35",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x + 165, y +  83,   "21",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x + 101, y + 149,   "67",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x + 123, y +  36,   "8",UI_TINYFONT, colorTable[CT_BLACK]);

	CG_DrawProportionalString( x +  90, y +  65, "1",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x + 105, y +  65, "2",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x + 105, y +  87, "3",UI_TINYFONT, colorTable[CT_BLACK]);
	CG_DrawProportionalString( x +  91, y +  87, "4",UI_RIGHT|UI_TINYFONT, colorTable[CT_BLACK]);

}

/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
overylays UI_DrawConnectText from ui_connect.cpp
====================
*/
extern void CG_CenterGiantLine( float y, const char *string );	//cg_scoreboard

void CG_DrawInformation( void ) {
	const char	*s;
	const char	*info;
	qhandle_t	levelshot;
	qhandle_t	detail;
	int			y,index;

	info = CG_ConfigString( CS_SERVERINFO );

	// draw the dialog background
	s = Info_ValueForKey( info, "mapname" );
	levelshot = cgi_R_RegisterShaderNoMip( va( "levelshots/%s.tga", s ) );
	
	extern SavedGameJustLoaded_e g_eSavedGameJustLoaded;	// hack! (hey, it's the last week of coding, ok?
	if ( !levelshot || g_eSavedGameJustLoaded == eFULL ) 
	{
		// keep whatever's in the screen buffer so far (either the last ingame rendered-image (eg for maptransition)
		//	or the screenshot built-in to a loaded save game...
		//
		cgi_R_DrawScreenShot( 0, cgs.glconfig.vidHeight, cgs.glconfig.vidWidth, -cgs.glconfig.vidHeight);
	} else {
		// put up the pre-defined levelshot for this map...
		//
		cgi_R_SetColor( NULL );
		CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, levelshot );
	}
	// blend a detail texture over it
	detail = cgi_R_RegisterShader( "levelShotDetail" );
	cgi_R_DrawStretchPic( 0, 0, cgs.glconfig.vidWidth, cgs.glconfig.vidHeight, 0, 0, 1, 1, detail );

	CG_LoadBar();

	// the first 150 rows are reserved for the client connection
	// screen to write into
	if ( cg.processedSnapshotNum == 0 ) {
		// still loading
		// print the current item being loaded

#ifndef NDEBUG
		if (cg.infoScreenText[0] != '@')	// Not a key for load bar
		{
			CG_DrawProportionalString( 28, 407, va("LOADING ... %s",cg.infoScreenText),CG_SMALLFONT,colorTable[CT_LTBLUE1] );
		}
#endif
	}

	// draw info string information

	y = 20;
	// map-specific message (long map name)
	s = CG_ConfigString( CS_MESSAGE );
	index = atoi(s);

	if ( s[0] ) 
	{
		if (index>0)
		{	
			CG_DrawProportionalString( 15, y, va("\"%s\"",bg_infoItemList[index].infoString),CG_BIGFONT,colorTable[CT_WHITE] );
		}
		else 
		{
			CG_DrawProportionalString( 15, y, va("\"%s\"",s),CG_BIGFONT,colorTable[CT_WHITE] );
		}
//		CG_CenterGiantLine( y, s );
		y += 20;
	}
}

