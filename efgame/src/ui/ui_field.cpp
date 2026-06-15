#include "ui_local.h"

#ifndef UI_HARD_LINKED		// Cash: rename these functions to not clash

/*
===================
Field_Draw

Handles horizontal scrolling and cursor blinking
x, y, amd width are in pixels
===================
*/
void Field_Draw( field_t *edit, int x, int y, int width, int size,int color,int color2, qboolean showCursor ) {
	int		len;
	int		drawLen;
	int		prestep;
	int		cursorChar;
	char	str[MAX_STRING_CHARS];
	int		style;
	int		finalColor;

	drawLen = edit->widthInChars;
	len = strlen( edit->buffer ) + 1;

	// guarantee that cursor will be visible
	if ( len <= drawLen ) {
		prestep = 0;
	} else {
		if ( edit->scroll + drawLen > len ) {
			edit->scroll = len - drawLen;
			if ( edit->scroll < 0 ) {
				edit->scroll = 0;
			}
		}
		prestep = edit->scroll;
	}

	if ( prestep + drawLen > len ) {
		drawLen = len - prestep;
	}


	// extract <drawLen> characters from the field at <prestep>
	if ( drawLen >= MAX_STRING_CHARS ) {
		ui.Error( ERR_DROP, "drawLen >= MAX_STRING_CHARS" );
	}
	memcpy( str, edit->buffer + prestep, drawLen );
	str[ drawLen ] = 0;

	if (showCursor)	{
//		style = UI_LEFT|UI_PULSE;
		style = UI_LEFT;
		finalColor = color2;
	}
	else  {
		style = UI_LEFT;
		finalColor = color;
	}

	// draw it
	if ( size == SMALLCHAR_WIDTH ) 
	{
		UI_DrawString( x, y, str, UI_LEFT|UI_SMALLFONT, colorTable[finalColor] );
	} 
	else 
	{
		// draw big string with drop shadow
		UI_DrawString( x, y, str, style,  colorTable[finalColor] );
	}

	// draw the cursor
	if ( !showCursor ) {
		return;
	}

	if ( ( uis.realtime >> 8 ) & 1 ) {
		return;		// off blink
	}

	if ( ui.Key_GetOverstrikeMode() ) {
		cursorChar = 11;
	} else {
		cursorChar = 10;
	}

	if ( size == SMALLCHAR_WIDTH ) 
	{
		ui.R_SetColor( colorTable[CT_YELLOW]);
		UI_DrawHandlePic( x + ( edit->cursor - prestep ) * size, y + (SMALLCHAR_HEIGHT - 2),  10, 3, uis.whiteShader);	

//		UI_DrawChar( x + ( edit->cursor - prestep ) * size, y, cursorChar, UI_LEFT|UI_SMALLFONT, menu_text_color );
	} else 
	{
		ui.R_SetColor( colorTable[CT_YELLOW]);
		UI_DrawHandlePic( x + ( edit->cursor - prestep ) * size, y,  10, 3, uis.whiteShader);	
//		UI_DrawChar( x + ( edit->cursor - prestep ) * size, y, cursorChar, style, color );
	}
}

/*
================
Field_Paste
================
*/
void Field_Paste( field_t *edit ) {
	char	pasteBuffer[64];
	int		pasteLen, i;

	ui.GetClipboardData( pasteBuffer, 64 );

	// send as if typed, so insert / overstrike works properly
	pasteLen = strlen( pasteBuffer );
	for ( i = 0 ; i < pasteLen ; i++ ) {
		Field_CharEvent( edit, pasteBuffer[i] );
	}
}

/*
=================
Field_KeyDownEvent

Performs the basic line editing functions for the console,
in-game talk, and menu fields

Key events are used for non-printable characters, others are gotten from char events.
=================
*/
void Field_KeyDownEvent( field_t *edit, int key ) {
	int		len;

	// shift-insert is paste
	if ( ( ( key == K_INS ) || ( key == K_KP_INS ) ) && ui.Key_IsDown( K_SHIFT ) ) {
		Field_Paste( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( key == K_DEL ) {
		if ( edit->cursor < len ) {
			memmove( edit->buffer + edit->cursor, 
				edit->buffer + edit->cursor + 1, len - edit->cursor );
		}
		return;
	}

	if ( key == K_RIGHTARROW ) 
	{
		if ( edit->cursor < len ) {
			edit->cursor++;
		}
		if ( edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= len )
		{
			edit->scroll++;
		}
		return;
	}

	if ( key == K_LEFTARROW ) 
	{
		if ( edit->cursor > 0 ) {
			edit->cursor--;
		}
		if ( edit->cursor < edit->scroll )
		{
			edit->scroll--;
		}
		return;
	}

	if ( key == K_HOME || ( tolower(key) == 'a' && ui.Key_IsDown( K_CTRL ) ) ) {
		edit->cursor = 0;
		return;
	}

	if ( key == K_END || ( tolower(key) == 'e' && ui.Key_IsDown( K_CTRL ) ) ) {
		edit->cursor = len;
		return;
	}

	if ( key == K_INS ) {
		ui.Key_SetOverstrikeMode( !ui.Key_GetOverstrikeMode() );
		return;
	}
}

/*
==================
Field_CharEvent
==================
*/
void Field_CharEvent( field_t *edit, int ch ) {
	int		len;

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		Field_Paste( edit );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		Field_Clear( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			memmove( edit->buffer + edit->cursor - 1, 
				edit->buffer + edit->cursor, len + 1 - edit->cursor );
			edit->cursor--;
			if ( edit->cursor < edit->scroll )
			{
				edit->scroll--;
			}
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		edit->cursor = len;
		edit->scroll = edit->cursor - edit->widthInChars;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < 32 ) {
		return;
	}

	if ( !ui.Key_GetOverstrikeMode() ) {	
		if ((edit->cursor == MAX_EDIT_LINE - 1) || (edit->maxchars && edit->cursor >= edit->maxchars))
			return;
	} else {
		// insert mode
		if (( len == MAX_EDIT_LINE - 1 )	|| (edit->maxchars && len >= edit->maxchars))
			return;
		memmove( edit->buffer + edit->cursor + 1, edit->buffer + edit->cursor, len + 1 - edit->cursor );
	}

	edit->buffer[edit->cursor] = ch;
	if (!edit->maxchars || edit->cursor < edit->maxchars-1)
		edit->cursor++;

	if ( edit->cursor >= edit->widthInChars )
	{
		edit->scroll++;
	}

	if ( edit->cursor == len + 1) {
		edit->buffer[edit->cursor] = 0;
	}
}

/*
==================
Field_Clear
==================
*/
void Field_Clear( field_t *edit ) {
	edit->buffer[0] = 0;
	edit->cursor = 0;
	edit->scroll = 0;
}


#endif

/*
==================
MenuField_Init
==================
*/
void MenuField_Init( menufield_s* m ) {
	int	l;

	Field_Clear( &m->field );

	if (m->generic.name) {
		l = (strlen( m->generic.name )+1) * SMALLCHAR_WIDTH;		
	}
	else {
		l = 0;
	}

	m->generic.left   = m->generic.x - l;
	m->generic.top    = m->generic.y - SMALLCHAR_HEIGHT/4;
	m->generic.right  = m->generic.x + m->field.widthInChars*SMALLCHAR_WIDTH;
	m->generic.bottom = m->generic.y + SMALLCHAR_HEIGHT + SMALLCHAR_HEIGHT/4;
}

/*
==================
MenuField_Draw
==================
*/
void MenuField_Draw( menufield_s *f )
{
	float	x;
	qboolean focus;

	if (Menu_ItemAtCursor( f->generic.parent ) == f) 
	{
		focus = qtrue;
	}
	else 
	{
		focus = qfalse;
	}

	if ( f->field.textEnum ) 
	{
//		UI_DrawString( f->generic.x - BIGCHAR_WIDTH, f->generic.y, f->generic.name, UI_RIGHT, focus ? menu_highlight_color : menu_dark_color );
		UI_DrawProportionalString(  f->generic.x - 10, f->generic.y, menu_normal_text[f->field.textEnum],UI_RIGHT | UI_SMALLFONT, colorTable[CT_BLACK]);	
	}

	x = f->generic.x;
	Field_Draw( &f->field, x + SMALLCHAR_WIDTH, f->generic.y, f->field.widthInChars*SMALLCHAR_WIDTH, SMALLCHAR_WIDTH,f->field.textcolor,f->field.textcolor2, focus );

	// Android: a focused text field (e.g. the save-name entry) wants the soft
	// keyboard. Publish focus + the field's 640-space rect through ui_textfield/
	// ui_textfieldRect; the engine (sdl_input.c IN_Frame) pops/anchors the IME
	// off these. Cleared each frame in UI_DrawMenu (ui_atoms.cpp) so it only
	// stays set while a focused field is actually being drawn.
	if ( focus )
	{
		char rect[64];
		ui.Cvar_Set( "ui_textfield", "1" );
		Com_sprintf( rect, sizeof(rect), "%f %f %f %f",
			x + SMALLCHAR_WIDTH, (float)f->generic.y,
			(float)( f->field.widthInChars * SMALLCHAR_WIDTH ), (float)SMALLCHAR_WIDTH );
		ui.Cvar_Set( "ui_textfieldRect", rect );
	}
}


