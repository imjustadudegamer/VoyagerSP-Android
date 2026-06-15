
#include "cg_local.h"
#include "cg_media.h"
#include "cg_text.h"
#include "../game/objectives.h"
#include "../game/b_local.h"

#define	SCOREBOARD_WIDTH	(26*BIGCHAR_WIDTH)

typedef enum 
{
	TMG_CURRENT_TITLE,
	TMG_CURRENT_HIGHLITETITLE,
	TMG_TITLE1,
	TMG_TITLE2,
	TMG_TITLE3,
	TMG_TITLE4,
	TMG_TITLE5,
	TMG_TITLE6,
	TMG_TITLE7,
	TMG_TITLE8,
	TMG_TITLE9,
	TMG_TITLE10,
	TMG_TITLE11,
	TMG_TITLE12,
	TMG_TITLE13,
	TMG_TITLE14,

	TMG_NUM1,
	TMG_NUM2,
	TMG_NUM3,
	TMG_NUM4,
	TMG_NUM5,
	TMG_NUM6,
	TMG_NUM7,
	TMG_NUM8,
	TMG_NUM9,
	TMG_NUM10,
	TMG_NUM11,

	TMG_RIGHTSIDE1,
	TMG_RIGHTSIDE2,
	TMG_RIGHTSIDE3,
	TMG_RIGHTSIDE4,
	TMG_RIGHTSIDE5,
	TMG_RIGHTSIDE6,

	TMG_LEFTSIDE1,
	TMG_LEFTSIDE2,
	TMG_LEFTSIDE3,
	TMG_LEFTSIDE4,
	TMG_LEFTSIDE5,
	TMG_LEFTSIDE6,

	TMG_LEFT_NUM1,
	TMG_LEFT_NUM2,
	TMG_LEFT_NUM3,
	TMG_LEFT_NUM4,
	TMG_LEFT_NUM5,
	TMG_LEFT_NUM6,
	TMG_LEFT_NUM7,
	TMG_LEFT_NUM8,
	TMG_LEFT_NUM9,
	TMG_LEFT_NUM10,
	TMG_LEFT_NUM11,
	TMG_LEFT_NUM12,
	TMG_LEFT_NUM13,
	TMG_LEFT_NUM14,
	TMG_LEFT_NUM15,
	TMG_LEFT_NUM16,

	TMG_GOODOUTCOME,
	TMG_BADOUTCOME,

	TMG_BADOUTCOME1,
	TMG_BADOUTCOME2,
	TMG_BADOUTCOME3,
	TMG_BADOUTCOME4,
	TMG_BADOUTCOME5,
	TMG_BADOUTCOME6,
	TMG_BADOUTCOME7,
	TMG_BADOUTCOME8,
	TMG_BADOUTCOME9,
	TMG_BADOUTCOME10,
	TMG_BADOUTCOME11,
	TMG_BADOUTCOME12,
	TMG_BADOUTCOME13,
	TMG_BADOUTCOME14,
	TMG_BADOUTCOME15,
	TMG_BADOUTCOME16,

	TMG_LOADTEXT,
	TMG_MAX
} tourstatsmenu_graphics_t;


screengraphics_s tourstatsmenu_graphics[TMG_MAX] = 
{
//	type		timer	x		y		width	height	file/text								graphic,	min		max		target		inc		style				color			pointer
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,	0,								NULL,		0,		0,		0,			0,		0,					CT_NONE,		NULL,	// SMG_CURRENT_TITLE
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,	0,								NULL,		0,		0,		0,			0,		0,					CT_NONE,		NULL,	// SMG_CURRENT_HIGHLITETITLE

//	type		timer	x		y		width	height	file/text								graphic,	min				max					target		inc		style				color			pointer
	SG_STRING,	0.0,	155,	120,	0,		0,		NULL,IGT_ENEMIES,				NULL,		TMG_LEFTSIDE1,	TMG_RIGHTSIDE1,		TMG_NUM1,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE1
	SG_STRING,	0.0,	155,	150,	0,		0,		NULL,IGT_TOURSHOTS,				NULL,		TMG_LEFTSIDE2,	TMG_RIGHTSIDE2,		TMG_NUM2,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE2
	SG_STRING,	0.0,	255,	150,	0,		0,		NULL,IGT_TOURFIRED,				NULL,		TMG_LEFTSIDE2,	TMG_RIGHTSIDE2,		TMG_NUM2,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE3
	SG_STRING,	0.0,	400,	150,	0,		0,		NULL,IGT_TOUREFFECTIVE,			NULL,		TMG_LEFTSIDE2,	TMG_RIGHTSIDE2,		TMG_NUM3,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE4
	SG_STRING,	0.0,	155,	170,	0,		0,		NULL,IGT_ACCURACY,				NULL,		TMG_LEFTSIDE2,	TMG_RIGHTSIDE2,		TMG_NUM4,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE5

	SG_STRING,	0.0,	155,	200,	0,		0,		NULL,IGT_TOURSTEALTH,			NULL,		TMG_LEFTSIDE2,	TMG_RIGHTSIDE2,		TMG_NUM5,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE6
	SG_STRING,	0.0,	155,	220,	0,		0,		NULL,IGT_TOUREXPLORATION,		NULL,		TMG_LEFTSIDE2,	TMG_RIGHTSIDE2,		TMG_NUM6,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE7

	SG_STRING,	0.0,	155,	250,	0,		0,		NULL,IGT_TOURDAMAGETAKEN,		NULL,		TMG_LEFTSIDE3,	TMG_RIGHTSIDE3,		TMG_NUM7,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE8

	SG_STRING,	0.0,	155,	280,	0,		0,		NULL,IGT_TOUROBJECTIVES,		NULL,		TMG_LEFTSIDE4,	TMG_RIGHTSIDE4,		TMG_NUM8,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE9
	SG_STRING,	0.0,	255,	280,	0,		0,		NULL,IGT_TOUROBJSUCCEEDED,		NULL,		TMG_LEFTSIDE4,	TMG_RIGHTSIDE4,		TMG_NUM8,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE10
	SG_STRING,	0.0,	400,	280,	0,		0,		NULL,IGT_TOUROBJFAILED,			NULL,		TMG_LEFTSIDE4,	TMG_RIGHTSIDE4,		TMG_NUM9,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE11

	SG_STRING,	0.0,	155,	310,	0,		0,		NULL,IGT_DURATION,				NULL,		TMG_LEFTSIDE5,	TMG_RIGHTSIDE5,		TMG_NUM9,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE12

	SG_STRING,	0.0,	155,	340,	0,		0,		NULL,IGT_TOURPERFORMANCERATING,	NULL,		TMG_LEFTSIDE6,	TMG_RIGHTSIDE6,		TMG_NUM10,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE13
	SG_STRING,	0.0,	155,	370,	0,		0,		NULL,IGT_ANALYSIS,				NULL,		TMG_LEFTSIDE6,	TMG_RIGHTSIDE6,		0,			0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// TMG_TITLE14


//	type		timer	x		y		width	height	file/text								graphic,	min		max		target		inc		style				color			pointer
	SG_NUMBER,	0.0,	320,	122,	16,		16,		NULL,	0,								NULL,		0,		3,		0,			0,		NUM_FONT_SMALL,		CT_WHITE,		NULL,	// TMG_NUM1
	SG_NUMBER,	0.0,	310,	152,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_WHITE,		NULL,	// TMG_NUM2
	SG_NUMBER,	0.0,	485,	152,	16,		16,		NULL,	0,								NULL,		0,		5,		0,			0,		NUM_FONT_SMALL,		CT_WHITE,		NULL,	// TMG_NUM3
	SG_NUMBER,	0.0,	242,	172,	16,		16,		NULL,	0,								NULL,		0,		5,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM4
	SG_NUMBER,	0.0,	260,	202,	16,		16,		NULL,	0,								NULL,		0,		3,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM5
	SG_NUMBER,	0.0,	280,	222,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM6
	SG_NUMBER,	0.0,	280,	252,	16,		16,		NULL,	0,								NULL,		0,		5,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM7
	SG_NUMBER,	0.0,	320,	282,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM8
	SG_NUMBER,	0.0,	500,	282,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM9
	SG_NUMBER,	0.0,	320,	340,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM10
	SG_NUMBER,	0.0,	320,	300,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// TMG_NUM11

	SG_GRAPHIC,	0.0,	120,	115,	18,		46,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// TMG_RIGHTSIDE1
	SG_GRAPHIC,	0.0,	120,	164,	18,		67,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// TMG_RIGHTSIDE2
	SG_GRAPHIC,	0.0,	120,	234,	18,		27,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// TMG_RIGHTSIDE3
	SG_GRAPHIC,	0.0,	120,	264,	18,		27,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// TMG_RIGHTSIDE4
	SG_GRAPHIC,	0.0,	120,	294,	18,		56,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// TMG_RIGHTSIDE5
	SG_GRAPHIC,	0.0,	120,	352,	18,		62,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// TMG_RIGHTSIDE6

//	type		timer	x		y		width	height	file/text								graphic,	min				max		target	inc		style				color			pointer
	SG_GRAPHIC,	0.0,	98,		115,	8,		46,		"menu/common/square.tga",	0,			NULL,		TMG_LEFT_NUM1,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE1
	SG_GRAPHIC,	0.0,	98,		164,	8,		67,		"menu/common/square.tga",	0,			NULL,		TMG_LEFT_NUM4,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE2
	SG_GRAPHIC,	0.0,	98,		234,	8,		27,		"menu/common/square.tga",	0,			NULL,		TMG_LEFT_NUM9,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE3
	SG_GRAPHIC,	0.0,	98,		264,	8,		27,		"menu/common/square.tga",	0,			NULL,		TMG_LEFT_NUM11,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE4
	SG_GRAPHIC,	0.0,	98,		294,	8,		56,		"menu/common/square.tga",	0,			NULL,		TMG_LEFT_NUM13,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE5
	SG_GRAPHIC,	0.0,	98,		352,	8,		62,		"menu/common/square.tga",	0,			NULL,		0,				0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// TMG_RIGHTSIDE6

	SG_NUMBER,	0.0,	40,		121,	16,		16,		NULL,						0,			NULL,		TMG_LEFT_NUM2,	4,		9999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM1
	SG_NUMBER,	0.0,	40,		138,	16,		16,		NULL,						0,			NULL,		0,	8,		99999999,0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM2
	SG_VAR,	0.0,	40,		147,	16,		16,		NULL,						0,			NULL,		0,				6,		999999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM3

	SG_NUMBER,	0.0,	40,		166,	16,		16,		NULL,						0,			NULL,		TMG_LEFT_NUM5,	2,		99,		0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM4
	SG_NUMBER,	0.0,	40,		182,	16,		16,		NULL,						0,			NULL,		TMG_LEFT_NUM6,	9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM5
	SG_NUMBER,	0.0,	40,		198,	16,		16,		NULL,						0,			NULL,		TMG_LEFT_NUM7,	8,		99999999,0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM6
	SG_NUMBER,	0.0,	40,		214,	16,		16,		NULL,						0,			NULL,		0,				8,		99999999,0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM7
	SG_VAR,		0.0,	40,		230,	16,		16,		NULL,						0,			NULL,		0,				4,		9999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM8

	SG_NUMBER,	0.0,	40,		241,	16,		16,		NULL,						0,			NULL,		0,				6,		999999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM9
	SG_VAR,		0.0,	40,		246,	16,		16,		NULL,						0,			NULL,		0,				9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM10

	SG_NUMBER,	0.0,	40,		271,	16,		16,		NULL,						0,			NULL,		0,				3,		999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM11
	SG_VAR,		0.0,	40,		276,	16,		16,		NULL,						0,			NULL,		0,				3,		999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM12

	SG_NUMBER,	0.0,	40,		298,	16,		16,		NULL,						0,			NULL,		TMG_LEFT_NUM14,	9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM13
	SG_NUMBER,	0.0,	40,		316,	16,		16,		NULL,						0,			NULL,		TMG_LEFT_NUM15,	9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM14
	SG_NUMBER,	0.0,	40,		334,	16,		16,		NULL,						0,			NULL,		0,				9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM15
	SG_VAR,		0.0,	40,		330,	16,		16,		NULL,						0,			NULL,		0,				9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM16

	SG_STRING,	0.0,	287,	371,	0,		0,		NULL,	IGT_MISSIONSUCCESSFUL,					NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_GOODOUTCOME
	SG_STRING,	0.0,	287,	371,	0,		0,		NULL,	IGT_MISSIONFAILED,						NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME

	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_INADEQUATE,				NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME1
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_RESPONSETIME,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME2
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_SHOOTINRANGE,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME3
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_TRYAGAIN,				NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME4
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_TRAINONHOLODECK,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME5
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_WHATCOLORSHIRT,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME6
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_NOTIMPRESS7OF9,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME7
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_NEELIXFAREDBETTER,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME8
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_THATMUSTHURT,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME9
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_TUVOKDISAPPOINTED,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME10
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_STARFLEETNOTIFYFAMILY,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME11
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_TEAMMATESWILLMISSYOU,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME12
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_LESSTHANEXEMPLARY,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME13
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_SACRIFICEDFORTHEWHOLE,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME14
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_NOTLIVELONGANDPROSPER,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME15
	SG_STRING,	0.0,	156,	390,	0,		0,		NULL,	IGT_BETTERUSEOFSIMULATIONS,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME16

	SG_VAR,		0.0,	170,	420,	0,		0,		NULL,	IGT_RELOADMISSION,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_BLACK,	NULL,		// SMG_LOADTEXT

};

typedef enum 
{
	SMG_CURRENT_TITLE,
	SMG_CURRENT_HIGHLITETITLE,
	SMG_TITLE1,
	SMG_TITLE2,
	SMG_TITLE3,
	SMG_TITLE4,
	SMG_TITLE5,
	SMG_TITLE6,
	SMG_TITLE7,
	SMG_TITLE8,

	SMG_NUM1,
	SMG_NUM2,
	SMG_NUM3,
	SMG_NUM4,
	SMG_NUM5,
	SMG_NUM6,
	SMG_NUM7,
	SMG_NUM8,

	SMG_RIGHTSIDE1,
	SMG_RIGHTSIDE2,
	SMG_RIGHTSIDE3,
	SMG_RIGHTSIDE4,
	SMG_RIGHTSIDE5,

	SMG_LEFTSIDE1,
	SMG_LEFTSIDE2,
	SMG_LEFTSIDE3,
	SMG_LEFTSIDE4,
	SMG_LEFTSIDE5,

	SMG_LEFT_NUM1,
	SMG_LEFT_NUM2,
	SMG_LEFT_NUM3,
	SMG_LEFT_NUM4,
	SMG_LEFT_NUM5,
	SMG_LEFT_NUM6,
	SMG_LEFT_NUM7,
	SMG_LEFT_NUM8,
	SMG_LEFT_NUM9,
	SMG_LEFT_NUM10,
	SMG_LEFT_NUM11,
	SMG_LEFT_NUM12,
	SMG_LEFT_NUM13,
	SMG_LEFT_NUM14,
	SMG_LEFT_NUM15,
	SMG_LEFT_NUM16,

	SMG_GOODOUTCOME,
	SMG_BADOUTCOME,

	SMG_BADOUTCOME1,
	SMG_BADOUTCOME2,
	SMG_BADOUTCOME3,
	SMG_BADOUTCOME4,
	SMG_BADOUTCOME5,
	SMG_BADOUTCOME6,
	SMG_BADOUTCOME7,
	SMG_BADOUTCOME8,
	SMG_BADOUTCOME9,
	SMG_BADOUTCOME10,
	SMG_BADOUTCOME11,
	SMG_BADOUTCOME12,
	SMG_BADOUTCOME13,
	SMG_BADOUTCOME14,
	SMG_BADOUTCOME15,
	SMG_BADOUTCOME16,

	SMG_LOADTEXT,
	SMG_MAX
} statsmenu_graphics_t;

screengraphics_s statsmenu_graphics[SMG_MAX] = 
{
//	type		timer	x		y		width	height	file/text								graphic,	min		max		target		inc		style				color			pointer
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,	0,								NULL,		0,		0,		0,			0,		0,					CT_NONE,		NULL,	// SMG_CURRENT_TITLE
	SG_VAR,		0.0,	0,		0,		0,		0,		NULL,	0,								NULL,		0,		0,		0,			0,		0,					CT_NONE,		NULL,	// SMG_CURRENT_HIGHLITETITLE

//	type		timer	x		y		width	height	file/text								graphic,	min				max					target		inc		style				color			pointer
	SG_STRING,	0.0,	155,	120,	0,		0,		NULL,IGT_ENEMIES,				NULL,		SMG_LEFTSIDE1,	SMG_RIGHTSIDE1,		SMG_NUM1,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE1
	SG_STRING,	0.0,	155,	140,	0,		0,		NULL,IGT_CASULATIES,			NULL,		SMG_LEFTSIDE1,	SMG_RIGHTSIDE1,		SMG_NUM2,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE2
	SG_STRING,	0.0,	155,	170,	0,		0,		NULL,IGT_SHOTSFIRED,			NULL,		SMG_LEFTSIDE2,	SMG_RIGHTSIDE2,		SMG_NUM3,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE3
	SG_STRING,	0.0,	155,	190,	0,		0,		NULL,IGT_EFFECTIVE,				NULL,		SMG_LEFTSIDE2,	SMG_RIGHTSIDE2,		SMG_NUM4,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE4
	SG_STRING,	0.0,	155,	210,	0,		0,		NULL,IGT_ACCURACY,				NULL,		SMG_LEFTSIDE2,	SMG_RIGHTSIDE2,		SMG_NUM5,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE5
	SG_STRING,	0.0,	155,	240,	0,		0,		NULL,IGT_PUZZLESSOLVED,			NULL,		SMG_LEFTSIDE3,	SMG_RIGHTSIDE3,		SMG_NUM6,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE6
	SG_STRING,	0.0,	155,	270,	0,		0,		NULL,IGT_DURATION,				NULL,		SMG_LEFTSIDE4,	SMG_RIGHTSIDE4,		SMG_NUM7,	0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE7
	SG_STRING,	0.0,	155,	300,	0,		0,		NULL,IGT_ANALYSIS,				NULL,		SMG_LEFTSIDE5,	SMG_RIGHTSIDE5,		0,			0,		UI_SMALLFONT,		CT_LTPURPLE1,	NULL,	// SMG_TITLE8

//	type		timer	x		y		width	height	file/text								graphic,	min		max		target		inc		style				color			pointer
	SG_NUMBER,	0.0,	320,	120,	16,		16,		NULL,	0,								NULL,		0,		3,		0,			0,		NUM_FONT_SMALL,		CT_WHITE,		NULL,	// SMG_NUM1
	SG_NUMBER,	0.0,	320,	140,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_WHITE,		NULL,	// SMG_NUM2
	SG_NUMBER,	0.0,	320,	170,	16,		16,		NULL,	0,								NULL,		0,		5,		0,			0,		NUM_FONT_SMALL,		CT_WHITE,		NULL,	// SMG_NUM3
	SG_NUMBER,	0.0,	320,	190,	16,		16,		NULL,	0,								NULL,		0,		5,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// SMG_NUM4
	SG_NUMBER,	0.0,	320,	210,	16,		16,		NULL,	0,								NULL,		0,		3,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// SMG_NUM5
	SG_NUMBER,	0.0,	320,	240,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// SMG_NUM6
	SG_NUMBER,	0.0,	320,	270,	16,		16,		NULL,	0,								NULL,		0,		5,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// SMG_NUM7
	SG_NUMBER,	0.0,	320,	300,	16,		16,		NULL,	0,								NULL,		0,		2,		0,			0,		NUM_FONT_SMALL,		CT_LTGOLD1,		NULL,	// SMG_NUM8

	SG_GRAPHIC,	0.0,	120,	115,	18,		46,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// SMG_RIGHTSIDE1
	SG_GRAPHIC,	0.0,	120,	164,	18,		67,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// SMG_RIGHTSIDE2
	SG_GRAPHIC,	0.0,	120,	234,	18,		27,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// SMG_RIGHTSIDE3
	SG_GRAPHIC,	0.0,	120,	264,	18,		27,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// SMG_RIGHTSIDE4
	SG_GRAPHIC,	0.0,	120,	294,	18,		56,		"menu/common/square.tga",	0,			NULL,		0,		0,		0,			0,		0,					CT_DKBROWN1,	NULL,	// SMG_RIGHTSIDE5

//	type		timer	x		y		width	height	file/text								graphic,	min				max		target	inc		style				color			pointer
	SG_GRAPHIC,	0.0,	98,		115,	8,		46,		"menu/common/square.tga",	0,			NULL,		SMG_LEFT_NUM1,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE1
	SG_GRAPHIC,	0.0,	98,		164,	8,		67,		"menu/common/square.tga",	0,			NULL,		SMG_LEFT_NUM4,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE2
	SG_GRAPHIC,	0.0,	98,		234,	8,		27,		"menu/common/square.tga",	0,			NULL,		SMG_LEFT_NUM9,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE3
	SG_GRAPHIC,	0.0,	98,		264,	8,		27,		"menu/common/square.tga",	0,			NULL,		SMG_LEFT_NUM11,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE4
	SG_GRAPHIC,	0.0,	98,		294,	8,		56,		"menu/common/square.tga",	0,			NULL,		SMG_LEFT_NUM13,	0,		0,		0,		0,					CT_DKBROWN1,	NULL,	// SMG_LEFTSIDE5

	SG_NUMBER,	0.0,	40,		121,	16,		16,		NULL,						0,			NULL,		SMG_LEFT_NUM2,	4,		9999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM1
	SG_NUMBER,	0.0,	40,		138,	16,		16,		NULL,						0,			NULL,		0,	8,		99999999,0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM2
	SG_VAR,	0.0,	40,		147,	16,		16,		NULL,						0,			NULL,		0,				6,		999999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM3

	SG_NUMBER,	0.0,	40,		166,	16,		16,		NULL,						0,			NULL,		SMG_LEFT_NUM5,	2,		99,		0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM4
	SG_NUMBER,	0.0,	40,		182,	16,		16,		NULL,						0,			NULL,		SMG_LEFT_NUM6,	9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM5
	SG_NUMBER,	0.0,	40,		198,	16,		16,		NULL,						0,			NULL,		SMG_LEFT_NUM7,	8,		99999999,0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM6
	SG_NUMBER,	0.0,	40,		214,	16,		16,		NULL,						0,			NULL,		0,				8,		99999999,0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM7
	SG_VAR,		0.0,	40,		230,	16,		16,		NULL,						0,			NULL,		0,				4,		9999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM8

	SG_NUMBER,	0.0,	40,		241,	16,		16,		NULL,						0,			NULL,		0,				6,		999999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM9
	SG_VAR,		0.0,	40,		246,	16,		16,		NULL,						0,			NULL,		0,				9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM10

	SG_NUMBER,	0.0,	40,		271,	16,		16,		NULL,						0,			NULL,		0,				3,		999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM11
	SG_VAR,		0.0,	40,		276,	16,		16,		NULL,						0,			NULL,		0,				3,		999,	0,		NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM12

	SG_NUMBER,	0.0,	40,		298,	16,		16,		NULL,						0,			NULL,		SMG_LEFT_NUM14,	9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM13
	SG_NUMBER,	0.0,	40,		316,	16,		16,		NULL,						0,			NULL,		SMG_LEFT_NUM15,	9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM14
	SG_NUMBER,	0.0,	40,		334,	16,		16,		NULL,						0,			NULL,		0,				9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM15
	SG_VAR,		0.0,	40,		330,	16,		16,		NULL,						0,			NULL,		0,				9,		999999999,0,	NUM_FONT_SMALL,		CT_DKGOLD1,		NULL,	// SMG_LEFT_NUM16

	SG_STRING,	0.0,	287,	301,	0,		0,		NULL,	IGT_MISSIONSUCCESSFUL,					NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_GOODOUTCOME
	SG_STRING,	0.0,	287,	301,	0,		0,		NULL,	IGT_MISSIONFAILED,						NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME

	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_INADEQUATE,				NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME1
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_RESPONSETIME,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME2
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_SHOOTINRANGE,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME3
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_TRYAGAIN,				NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME4
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_TRAINONHOLODECK,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME5
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_WHATCOLORSHIRT,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME6
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_NOTIMPRESS7OF9,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME7
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_NEELIXFAREDBETTER,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME8
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_THATMUSTHURT,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME9
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_TUVOKDISAPPOINTED,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME10
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_STARFLEETNOTIFYFAMILY,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME11
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_TEAMMATESWILLMISSYOU,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME12
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_LESSTHANEXEMPLARY,		NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME13
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_SACRIFICEDFORTHEWHOLE,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME14
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_NOTLIVELONGANDPROSPER,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME15
	SG_STRING,	0.0,	156,	320,	0,		0,		NULL,	IGT_BETTERUSEOFSIMULATIONS,	NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_LTORANGE,	NULL,	// SMG_BADOUTCOME16

	SG_VAR,		0.0,	170,	358,	0,		0,		NULL,	IGT_RELOADMISSION,			NULL,		0,				0,		0,		0,		UI_SMALLFONT,		CT_BLACK,	NULL,		// SMG_LOADTEXT

};

static int success,failed;
static char objectiveString[256];
static char objectiveString2[256];

static void TourScoreboard_MenuBlinkies( void )
{
	int i,numIndex,graphicIndex;
	int index,final,index2;

	// First time here?
	if (tourstatsmenu_graphics[TMG_CURRENT_TITLE].target == 0)
	{

		// Precache all menu graphics in array
		for (i=0;i<TMG_MAX;++i)
		{
			if (tourstatsmenu_graphics[i].type == SG_GRAPHIC)
			{
				tourstatsmenu_graphics[i].graphic = cgi_R_RegisterShaderNoMip(tourstatsmenu_graphics[i].file);
			}
		}

		// Create numbers
		for (i=TMG_LEFT_NUM1;i<=TMG_LEFT_NUM16;++i)
		{
			tourstatsmenu_graphics[i].target = random() * tourstatsmenu_graphics[i].target;
		}

		tourstatsmenu_graphics[TMG_GOODOUTCOME].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME].type = SG_OFF;

		tourstatsmenu_graphics[TMG_BADOUTCOME1].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME2].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME3].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME4].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME5].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME6].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME7].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME8].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME9].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME10].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME11].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME12].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME13].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME14].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME15].type = SG_OFF;
		tourstatsmenu_graphics[TMG_BADOUTCOME16].type = SG_OFF;

		// --------------------------------------------------
		// Stuff in the player feedback values
		// TMG_NUM1 = Enemies Eliminated
		// TMG_NUM2 = Shots Fired
		// TMG_NUM3 = Shots Effective
		// TMG_NUM4 = Shot Accuracy
		// TMG_NUM5 = Stealth
		// TMG_NUM6 = Exploration
		// TMG_NUM7 = Damage Taken
		// TMG_NUM8 = Objectives Succeeded
		// TMG_NUM9 = Objectives Failed
		// TMG_NUM10 = Performance Rating
		// --------------------------------------------------
		gentity_t *ent = &g_entities[0];
		tourstatsmenu_graphics[TMG_NUM1].target = ent->client->ps.persistant[PERS_ENEMIES_KILLED];
		tourstatsmenu_graphics[TMG_NUM2].target = ent->client->ps.persistant[PERS_TEAMMATES_KILLED];
		tourstatsmenu_graphics[TMG_NUM3].target = ent->client->ps.persistant[PERS_ACCURACY_SHOTS];
		tourstatsmenu_graphics[TMG_NUM4].target = ent->client->ps.persistant[PERS_ACCURACY_HITS];




		// Avoid divide by zero
		if ( ent->client->ps.persistant[PERS_ACCURACY_SHOTS] )
		{
			tourstatsmenu_graphics[TMG_NUM5].target = ( ent->client->ps.persistant[PERS_ACCURACY_HITS] * 100.0f ) / ent->client->ps.persistant[PERS_ACCURACY_SHOTS];
		}

		// How many successful objectives - how many failed
		success=0;
		failed=0;
		for (i=0;i<MAX_OBJECTIVES;++i)
		{
			if (ent->client->sess.mission_objectives[i].status == OBJECTIVE_STAT_SUCCEEDED)
			{
				success++;
			}
			else if (ent->client->sess.mission_objectives[i].status == OBJECTIVE_STAT_FAILED)
			{
				failed++;
			}
		}

		float totaltime;
		int		hours,minutes,seconds;

		totaltime = level.time/1000;

		hours = totaltime / (60 * 60);
		totaltime -= hours * (60 * 60);

		minutes = totaltime / 60;
		totaltime -= minutes * (60);

		seconds = totaltime;

		sprintf(objectiveString2,va(ingame_text[IGT_DURATION_FINAL],hours,minutes,seconds));

		// A value of two means show scoreboard without flashing numbers one at a time
		if (cg.missionStatusShow!=2)	
		{
			tourstatsmenu_graphics[TMG_CURRENT_TITLE].target = TMG_TITLE1;

			for (i=TMG_TITLE1;i<=TMG_TITLE13;++i)
			{
				tourstatsmenu_graphics[i].type = SG_OFF;
			}

			for (i=TMG_NUM1;i<=TMG_NUM11;++i)
			{
				tourstatsmenu_graphics[i].type = SG_OFF;
			}

			tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].type = SG_OFF;

			tourstatsmenu_graphics[TMG_CURRENT_TITLE].timer = cg.time + 50;
			tourstatsmenu_graphics[TMG_TITLE1].type = SG_STRING;
			tourstatsmenu_graphics[TMG_CURRENT_TITLE].type = SG_VAR;
		}
		else	// Just show data without all the flashy stuff.
		{
			for (i=TMG_TITLE1;i<=TMG_TITLE13;++i)
			{
				tourstatsmenu_graphics[i].type = SG_STRING;

				tourstatsmenu_graphics[i].color = CT_LTPURPLE1;	// Title color
				numIndex = tourstatsmenu_graphics[i].target;	//	Big number color
				tourstatsmenu_graphics[numIndex].color = CT_LTPURPLE1;

				// Change right side graphic
				graphicIndex = tourstatsmenu_graphics[i].max;
				tourstatsmenu_graphics[graphicIndex].color = CT_DKBROWN1;
				// Change left side graphic
				graphicIndex = tourstatsmenu_graphics[i].min;
				tourstatsmenu_graphics[graphicIndex].color = CT_DKBROWN1;
			}

			tourstatsmenu_graphics[TMG_NUM9].type = SG_OFF;
			tourstatsmenu_graphics[TMG_NUM10].type = SG_OFF;

			tourstatsmenu_graphics[TMG_TITLE10].file = objectiveString2;

			tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].type = SG_OFF;
			tourstatsmenu_graphics[TMG_CURRENT_TITLE].target = TMG_TITLE8;
			tourstatsmenu_graphics[TMG_CURRENT_TITLE].type = SG_OFF;

			tourstatsmenu_graphics[TMG_TITLE8].type = SG_OFF;
			tourstatsmenu_graphics[TMG_NUM8].type = SG_OFF;

		}
	}


	// Print out titles one after the other
	if ((tourstatsmenu_graphics[TMG_CURRENT_TITLE].timer < cg.time) && 
		(tourstatsmenu_graphics[TMG_CURRENT_TITLE].type != SG_OFF))
	{
		cgi_S_StartSound( NULL, 0, CHAN_MENU1, cgs.media.interfaceSnd1);

		tourstatsmenu_graphics[TMG_CURRENT_TITLE].timer = cg.time + 50;
		++tourstatsmenu_graphics[TMG_CURRENT_TITLE].target;

		index = tourstatsmenu_graphics[TMG_CURRENT_TITLE].target;

		if (index <=  TMG_TITLE14)
		{
			tourstatsmenu_graphics[index].type = SG_STRING;
		}
		else 
		{
			tourstatsmenu_graphics[TMG_CURRENT_TITLE].type = SG_OFF;
			tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].type = SG_VAR;

			// Turn on title 1
			tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].target = TMG_TITLE1;
			tourstatsmenu_graphics[TMG_TITLE1].color = CT_WHITE;
			tourstatsmenu_graphics[TMG_NUM1].type = SG_NUMBER;
			tourstatsmenu_graphics[TMG_NUM1].color = CT_WHITE;

			tourstatsmenu_graphics[TMG_RIGHTSIDE1].color = CT_LTBROWN1;
			tourstatsmenu_graphics[TMG_LEFTSIDE1].color = CT_LTBROWN1;

			tourstatsmenu_graphics[TMG_LEFT_NUM1].color = CT_LTGOLD1;
			tourstatsmenu_graphics[TMG_LEFT_NUM2].color = CT_LTGOLD1;
//			tourstatsmenu_graphics[TMG_LEFT_NUM3].color = CT_LTGOLD1;

			tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].timer = cg.time + 500;
		}
	}

	// Highlight each section and give values
	if ((tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].timer < cg.time) && 
		(tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].type != SG_OFF))
	{
		cgi_S_StartSound( NULL, 0, CHAN_MENU1, cgs.media.interfaceSnd3);

		// Change number and title to normal color
		index = tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].target;
		tourstatsmenu_graphics[index].color = CT_LTPURPLE1;

		// Change big number
		numIndex = tourstatsmenu_graphics[index].target;
		tourstatsmenu_graphics[numIndex].color = CT_LTPURPLE1;

		// Change right side graphic
		graphicIndex = tourstatsmenu_graphics[index].max;
		tourstatsmenu_graphics[graphicIndex].color = CT_DKBROWN1;

		// Change left side graphic
		graphicIndex = tourstatsmenu_graphics[index].min;
		tourstatsmenu_graphics[graphicIndex].color = CT_DKBROWN1;

		// Return numbers to normal 
		numIndex = 	graphicIndex = tourstatsmenu_graphics[graphicIndex].min;
		while (numIndex)
		{
			tourstatsmenu_graphics[numIndex].color = CT_DKGOLD1;
			numIndex = tourstatsmenu_graphics[numIndex].min;
		}

		tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].timer = cg.time + 500;
		++tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].target;

		index = tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].target;

		if (index <= TMG_TITLE14)
		{
			tourstatsmenu_graphics[index].color = CT_WHITE;

			// Change right side graphic
			graphicIndex = tourstatsmenu_graphics[index].max;
			tourstatsmenu_graphics[graphicIndex].color = CT_LTBROWN1;

			// Change left side graphic
			graphicIndex = tourstatsmenu_graphics[index].min;
			tourstatsmenu_graphics[graphicIndex].color = CT_LTBROWN1;

			if (index == TMG_TITLE12)
			{
				tourstatsmenu_graphics[TMG_TITLE12].file = objectiveString2;
			}
			else
			{
				// Turn on number
				numIndex = tourstatsmenu_graphics[index].target;
				tourstatsmenu_graphics[numIndex].type = SG_NUMBER;
				tourstatsmenu_graphics[numIndex].color = CT_WHITE;

				// Highlight numbers
				numIndex = 	graphicIndex = tourstatsmenu_graphics[graphicIndex].min;
				while (numIndex)
				{
					tourstatsmenu_graphics[numIndex].color = CT_LTGOLD1;
					numIndex = 	tourstatsmenu_graphics[numIndex].min;
				}
			}

			// Turn on analysis
			if (index == TMG_TITLE14)
			{
				if (statusTextIndex<0)
				{
					tourstatsmenu_graphics[TMG_BADOUTCOME].type = SG_STRING;
					
					final = (int) (random() * 15);
					
					index2 = TMG_BADOUTCOME1 + final;
					if ((index2 < TMG_BADOUTCOME1) || (index2 > TMG_BADOUTCOME16))
					{
						index2 = TMG_BADOUTCOME1;
					}
					tourstatsmenu_graphics[index2].type = SG_STRING;

				}
				else
				{
					tourstatsmenu_graphics[TMG_BADOUTCOME].type = SG_STRING;

					tourstatsmenu_graphics[TMG_BADOUTCOME1].file = ingame_text[statusTextIndex];
					tourstatsmenu_graphics[TMG_BADOUTCOME1].type = SG_STRING;
				}
				tourstatsmenu_graphics[TMG_CURRENT_TITLE].type = SG_OFF;
				tourstatsmenu_graphics[TMG_CURRENT_HIGHLITETITLE].type = SG_OFF;

				tourstatsmenu_graphics[TMG_LOADTEXT].type = SG_STRING;
			}

		}
	}

}

static void TourScoreboard_Draw( void )
{
	vec4_t	newColor;

	cg.LCARSTextTime = 0;	//	Turn off LCARS screen

	// Background
	newColor[0] = colorTable[CT_BLACK][0];
	newColor[1] = colorTable[CT_BLACK][1];
	newColor[2] = colorTable[CT_BLACK][2];
	newColor[3] = 0.5;
	cgi_R_SetColor(newColor);
	CG_DrawPic( 137, 73, 475, 348, cgs.media.whiteShader);	// Background
	CG_DrawPic( 120, 99,  18, 324, cgs.media.whiteShader);	// Background
	CG_DrawPic(  40, 94,  66, 334, cgs.media.whiteShader);	// Background

	TourScoreboard_MenuBlinkies();

	// Right side box
	cgi_R_SetColor( colorTable[CT_DKBROWN1]);
	CG_DrawPic( 120,  73,  32, 32, cgs.media.status_corner_18_22);
	CG_DrawPic( 120, 417,  32, 32, cgs.media.status_corner_16_18);
	CG_DrawPic(  94, 419,  16, 32, cgs.media.status_corner_8_16_b);
	CG_DrawPic(  94, 73,  16, 32, cgs.media.status_corner_8_22);

	CG_DrawPic(135,73,  302, 22, cgs.media.whiteShader);	// Top

	CG_DrawPic(120, 100,  18, 12, cgs.media.whiteShader);	// Middle Top
	CG_DrawPic(120, 416,  18, 4,  cgs.media.whiteShader);	// Middle Bottom
  
	CG_DrawPic(130,420,  482, 18, cgs.media.whiteShader);	// Bottom

	// Left side box
	cgi_R_SetColor( colorTable[CT_DKBROWN1]);
	CG_DrawPic(40,73,  56, 22, cgs.media.whiteShader);	// Top
	CG_DrawPic(98,95,  8, 17, cgs.media.whiteShader);		// Middle Top

	CG_DrawPic(98,416, 8, 7, cgs.media.whiteShader);		// Middle Bottom
	CG_DrawPic(40,420, 58, 18, cgs.media.whiteShader);	// Bottom

//	CG_DrawProportionalString( 356, 208, "%", UI_RIGHT | UI_SMALLFONT, colorTable[CT_LTPURPLE1] );
	CG_DrawProportionalString( 610, 72, ingame_text[IGT_MISSIONANALYSIS],UI_RIGHT| CG_BIGFONT, colorTable[CT_LTORANGE] );

	CG_PrintScreenGraphics(tourstatsmenu_graphics,TMG_MAX);

}

static void Scoreboard_MenuBlinkies( void )
{
	int i,numIndex,graphicIndex;
	int index,final,index2;

	// First time here?
	if (statsmenu_graphics[SMG_CURRENT_TITLE].target == 0)
	{

		// Precache all menu graphics in array
		for (i=0;i<SMG_MAX;++i)
		{
			if (statsmenu_graphics[i].type == SG_GRAPHIC)
			{
				statsmenu_graphics[i].graphic = cgi_R_RegisterShaderNoMip(statsmenu_graphics[i].file);
			}
		}

		// Create numbers
		for (i=SMG_LEFT_NUM1;i<=SMG_LEFT_NUM16;++i)
		{
			statsmenu_graphics[i].target = random() * statsmenu_graphics[i].target;
		}

		statsmenu_graphics[SMG_GOODOUTCOME].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME].type = SG_OFF;

		statsmenu_graphics[SMG_BADOUTCOME1].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME2].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME3].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME4].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME5].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME6].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME7].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME8].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME9].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME10].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME11].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME12].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME13].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME14].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME15].type = SG_OFF;
		statsmenu_graphics[SMG_BADOUTCOME16].type = SG_OFF;


		// Stuff in the player feedback values
		gentity_t *ent = &g_entities[0];
		statsmenu_graphics[SMG_NUM1].target = ent->client->ps.persistant[PERS_ENEMIES_KILLED];
		statsmenu_graphics[SMG_NUM2].target = ent->client->ps.persistant[PERS_TEAMMATES_KILLED];
		statsmenu_graphics[SMG_NUM3].target = ent->client->ps.persistant[PERS_ACCURACY_SHOTS];
		statsmenu_graphics[SMG_NUM4].target = ent->client->ps.persistant[PERS_ACCURACY_HITS];

		// Avoid divide by zero
		if ( ent->client->ps.persistant[PERS_ACCURACY_SHOTS] )
		{
			statsmenu_graphics[SMG_NUM5].target = ( ent->client->ps.persistant[PERS_ACCURACY_HITS] * 100.0f ) / ent->client->ps.persistant[PERS_ACCURACY_SHOTS];
		}

		// How many successful objectives - how many failed
		success=0;
		failed=0;
		for (i=0;i<MAX_OBJECTIVES;++i)
		{
			if (ent->client->sess.mission_objectives[i].status == OBJECTIVE_STAT_SUCCEEDED)
			{
				success++;
			}
			else if (ent->client->sess.mission_objectives[i].status == OBJECTIVE_STAT_FAILED)
			{
				failed++;
			}
		}

		float totaltime;
		int		hours,minutes,seconds;

		totaltime = level.time/1000;

		hours = totaltime / (60 * 60);
		totaltime -= hours * (60 * 60);

		minutes = totaltime / 60;
		totaltime -= minutes * (60);

		seconds = totaltime;

		sprintf(objectiveString2,va(ingame_text[IGT_DURATION_FINAL],hours,minutes,seconds));

		// A value of two means show scoreboard without flashing numbers one at a time
		if (cg.missionStatusShow!=2)	
		{
			statsmenu_graphics[SMG_CURRENT_TITLE].target = SMG_TITLE1;

			for (i=SMG_TITLE1;i<=SMG_TITLE8;++i)
			{
				statsmenu_graphics[i].type = SG_OFF;
			}

			for (i=SMG_NUM1;i<=SMG_NUM8;++i)
			{
				statsmenu_graphics[i].type = SG_OFF;
			}

			statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].type = SG_OFF;

			statsmenu_graphics[SMG_CURRENT_TITLE].timer = cg.time + 50;
			statsmenu_graphics[SMG_TITLE1].type = SG_STRING;
			statsmenu_graphics[SMG_CURRENT_TITLE].type = SG_VAR;
		}
		else	// Just show data without all the flashy stuff.
		{
			for (i=SMG_TITLE1;i<=SMG_TITLE8;++i)
			{
				statsmenu_graphics[i].type = SG_STRING;

				statsmenu_graphics[i].color = CT_LTPURPLE1;	// Title color
				numIndex = statsmenu_graphics[i].target;	//	Big number color
				statsmenu_graphics[numIndex].color = CT_LTPURPLE1;

				// Change right side graphic
				graphicIndex = statsmenu_graphics[i].max;
				statsmenu_graphics[graphicIndex].color = CT_DKBROWN1;
				// Change left side graphic
				graphicIndex = statsmenu_graphics[i].min;
				statsmenu_graphics[graphicIndex].color = CT_DKBROWN1;
			}

			statsmenu_graphics[SMG_NUM6].type = SG_OFF;
			statsmenu_graphics[SMG_NUM7].type = SG_OFF;

			sprintf(objectiveString,va(ingame_text[IGT_PUZZLESSOLVED_FINAL],success,failed));
			statsmenu_graphics[SMG_TITLE6].file = objectiveString;
			statsmenu_graphics[SMG_TITLE7].file = objectiveString2;

			statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].type = SG_OFF;
			statsmenu_graphics[SMG_CURRENT_TITLE].target = SMG_TITLE8;
			statsmenu_graphics[SMG_CURRENT_TITLE].type = SG_OFF;

			statsmenu_graphics[SMG_TITLE8].type = SG_OFF;
			statsmenu_graphics[SMG_NUM8].type = SG_OFF;

		}
	}


	// Print out titles one after the other
	if ((statsmenu_graphics[SMG_CURRENT_TITLE].timer < cg.time) && 
		(statsmenu_graphics[SMG_CURRENT_TITLE].type != SG_OFF))
	{
		cgi_S_StartSound( NULL, 0, CHAN_MENU1, cgs.media.interfaceSnd1);

		statsmenu_graphics[SMG_CURRENT_TITLE].timer = cg.time + 50;
		++statsmenu_graphics[SMG_CURRENT_TITLE].target;

		index = statsmenu_graphics[SMG_CURRENT_TITLE].target;

		if (index <=  SMG_TITLE8)
		{
			statsmenu_graphics[index].type = SG_STRING;
		}
		else 
		{
			statsmenu_graphics[SMG_CURRENT_TITLE].type = SG_OFF;
			statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].type = SG_VAR;

			// Turn on title 1
			statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].target = SMG_TITLE1;
			statsmenu_graphics[SMG_TITLE1].color = CT_WHITE;
			statsmenu_graphics[SMG_NUM1].type = SG_NUMBER;
			statsmenu_graphics[SMG_NUM1].color = CT_WHITE;

			statsmenu_graphics[SMG_RIGHTSIDE1].color = CT_LTBROWN1;
			statsmenu_graphics[SMG_LEFTSIDE1].color = CT_LTBROWN1;

			statsmenu_graphics[SMG_LEFT_NUM1].color = CT_LTGOLD1;
			statsmenu_graphics[SMG_LEFT_NUM2].color = CT_LTGOLD1;
//			statsmenu_graphics[SMG_LEFT_NUM3].color = CT_LTGOLD1;

			statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].timer = cg.time + 500;
		}
	}

	// Highlight each section and give values
	if ((statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].timer < cg.time) && 
		(statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].type != SG_OFF))
	{
		cgi_S_StartSound( NULL, 0, CHAN_MENU1, cgs.media.interfaceSnd3);

		// Change number and title to normal color
		index = statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].target;
		statsmenu_graphics[index].color = CT_LTPURPLE1;

		// Change big number
		numIndex = statsmenu_graphics[index].target;
		statsmenu_graphics[numIndex].color = CT_LTPURPLE1;

		// Change right side graphic
		graphicIndex = statsmenu_graphics[index].max;
		statsmenu_graphics[graphicIndex].color = CT_DKBROWN1;

		// Change left side graphic
		graphicIndex = statsmenu_graphics[index].min;
		statsmenu_graphics[graphicIndex].color = CT_DKBROWN1;

		// Return numbers to normal 
		numIndex = 	graphicIndex = statsmenu_graphics[graphicIndex].min;
		while (numIndex)
		{
			statsmenu_graphics[numIndex].color = CT_DKGOLD1;
			numIndex = statsmenu_graphics[numIndex].min;
		}

		statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].timer = cg.time + 500;
		++statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].target;

		index = statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].target;

		if (index <= SMG_TITLE8)
		{
			statsmenu_graphics[index].color = CT_WHITE;

			// Change right side graphic
			graphicIndex = statsmenu_graphics[index].max;
			statsmenu_graphics[graphicIndex].color = CT_LTBROWN1;

			// Change left side graphic
			graphicIndex = statsmenu_graphics[index].min;
			statsmenu_graphics[graphicIndex].color = CT_LTBROWN1;

			if (index == SMG_TITLE6)
			{
				sprintf(objectiveString,va(ingame_text[IGT_PUZZLESSOLVED_FINAL],success,failed));
				statsmenu_graphics[SMG_TITLE6].file = objectiveString;
			}
			else if (index == SMG_TITLE7)
			{
				statsmenu_graphics[SMG_TITLE7].file = objectiveString2;
			}
			else
			{
				// Turn on number
				numIndex = statsmenu_graphics[index].target;
				statsmenu_graphics[numIndex].type = SG_NUMBER;
				statsmenu_graphics[numIndex].color = CT_WHITE;

				// Highlight numbers
				numIndex = 	graphicIndex = statsmenu_graphics[graphicIndex].min;
				while (numIndex)
				{
					statsmenu_graphics[numIndex].color = CT_LTGOLD1;
					numIndex = 	statsmenu_graphics[numIndex].min;
				}
			}

			// Turn on analysis
			if (index == SMG_TITLE8)
			{
				if (statusTextIndex<0)
				{
					statsmenu_graphics[SMG_BADOUTCOME].type = SG_STRING;
					
					final = (int) (random() * 15);
					
					index2 = SMG_BADOUTCOME1 + final;
					if ((index2 < SMG_BADOUTCOME1) || (index2 > SMG_BADOUTCOME16))
					{
						index2 = SMG_BADOUTCOME1;
					}
					statsmenu_graphics[index2].type = SG_STRING;

				}
				else
				{
					statsmenu_graphics[SMG_BADOUTCOME].type = SG_STRING;

					statsmenu_graphics[SMG_BADOUTCOME1].file = ingame_text[statusTextIndex];
					statsmenu_graphics[SMG_BADOUTCOME1].type = SG_STRING;
				}
				statsmenu_graphics[SMG_CURRENT_TITLE].type = SG_OFF;
				statsmenu_graphics[SMG_CURRENT_HIGHLITETITLE].type = SG_OFF;

				statsmenu_graphics[SMG_LOADTEXT].type = SG_STRING;
			}

		}
	}

}

static void Scoreboard_Draw( void )
{
	vec4_t	newColor;
/*
		player = g_entities[0];
		if( player->client->ps.persistant[PERS_ACCURACY_SHOTS] ) {
			accuracy = player->client->ps.persistant[PERS_ACCURACY_HITS] * 100 / player->client->ps.persistant[PERS_ACCURACY_SHOTS];
		}
*/
	cg.LCARSTextTime = 0;	//	Turn off LCARS screen

	// Background
	newColor[0] = colorTable[CT_BLACK][0];
	newColor[1] = colorTable[CT_BLACK][1];
	newColor[2] = colorTable[CT_BLACK][2];
	newColor[3] = 0.5;
	cgi_R_SetColor(newColor);
	CG_DrawPic( 137, 73, 475, 300, cgs.media.whiteShader);	// Background
	CG_DrawPic( 120, 99,  18, 256, cgs.media.whiteShader);	// Background
	CG_DrawPic(  40, 94,  66, 266, cgs.media.whiteShader);	// Background

	Scoreboard_MenuBlinkies();

	// Right side box
	cgi_R_SetColor( colorTable[CT_DKBROWN1]);
	CG_DrawPic( 120,  73,  32, 32, cgs.media.status_corner_18_22);
	CG_DrawPic( 120, 354,  32, 32, cgs.media.status_corner_16_18);
	CG_DrawPic(  94, 356,  16, 32, cgs.media.status_corner_8_16_b);
	CG_DrawPic(  94, 73,  16, 32, cgs.media.status_corner_8_22);

	CG_DrawPic(135,73,  302, 22, cgs.media.whiteShader);	// Top

	CG_DrawPic(120, 100,  18, 12, cgs.media.whiteShader);	// Middle Top
	CG_DrawPic(120, 353,  18, 4,  cgs.media.whiteShader);	// Middle Bottom
  
	CG_DrawPic(130,357,  482, 18, cgs.media.whiteShader);	// Bottom

	// Left side box
	cgi_R_SetColor( colorTable[CT_DKBROWN1]);
	CG_DrawPic(40,73,  56, 22, cgs.media.whiteShader);	// Top

	CG_DrawPic(98,95,  8, 17, cgs.media.whiteShader);		// Middle Top
	CG_DrawPic(98,353, 8, 7, cgs.media.whiteShader);		// Middle Bottom

	CG_DrawPic(40,357, 58, 18, cgs.media.whiteShader);	// Bottom

	CG_DrawProportionalString( 356, 208, "%", UI_RIGHT | UI_SMALLFONT, colorTable[CT_LTPURPLE1] );
	CG_DrawProportionalString( 610, 72, ingame_text[IGT_MISSIONANALYSIS],UI_RIGHT| CG_BIGFONT, colorTable[CT_LTORANGE] );

	CG_PrintScreenGraphics(statsmenu_graphics,SMG_MAX);

}

/*
=================
CG_DrawScoreboard

Draw the normal in-game scoreboard
=================
*/
qboolean CG_DrawScoreboard( void ) 
{
	// don't draw anything if the menu is up
	if ( cg_paused.integer ) 
	{
		return qfalse;
	}

	// Character is either dead, or a script has brought up the screen
	if (((cg.predicted_player_state.pm_type == PM_DEAD) && (cg.missionStatusDeadTime < level.time)) || 
		(cg.missionStatusShow))
	{
		if (cg_virtualVoyager.value == 1)
		{
			TourScoreboard_Draw();
		}
		else 
		{
			Scoreboard_Draw();
		}
	}

	return qtrue;
}

void ScoreBoardReset(void)
{
	statsmenu_graphics[SMG_CURRENT_TITLE].target = 0;
	tourstatsmenu_graphics[TMG_CURRENT_TITLE].target = 0;
}

//================================================================================

/*
================
CG_CenterGiantLine
================
*/
void CG_CenterGiantLine( float y, const char *string ) {
	float		x;
	vec4_t		color;

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	x = 0.5 * ( 640 - GIANT_WIDTH * CG_DrawStrlen( string ) );

	CG_DrawStringExt( x, y, string, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT );
}

void CG_AnalysisDown_f( void ) 
{
	cg.missionStatusShow = 2;
}

void CG_AnalysisUp_f( void ) 
{
	statsmenu_graphics[SMG_CURRENT_TITLE].target = 0;
	cg.missionStatusShow = 0;
}

