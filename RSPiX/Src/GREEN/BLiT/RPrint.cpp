////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// This is the new version of the higher level printing.  Hopefully,
// this time it will be less of a hack.

// One of the biggest difference about RPrint now is that it does NOT
// contain a text buffer.  The entire reason for the buffer was to 
// enable multi-column printing, but now I have an easier way.

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/BLiT/BLIT.H"
	#include "GREEN/BLiT/Cfnt.h"
	#include "RPrint.h"
#else
	#include "BLIT.H"
	#include "Cfnt.h"
	#include "RPrint.h"
#endif

//========================================================
// Instantiate the static members:
char	RPrint::ms_szInput[4096]; // temporary column buffer
uint8_t	RPrint::ms_szLineText[1024]; // Stores a line at a time
int16_t	RPrint::ms_sCharPosX[1024]; // positions of each character in the line...

// CURRENTLY we are going with the unconventional 
// system of positioning text from the TOP OF THE CELL

// returns non-zero if you are starting a fresh line
// THIS POSITIONS based on m_sCellW/H, which is INCONSISTENT!!!!!
// (Address this later)

/*
void RPrint::OffsetShadow()
	{
	if (m_aAbsEffect[SHADOW_X] < 0) m_sCurX -= m_aAbsEffect[SHADOW_X] ;
	if (m_aAbsEffect[SHADOW_Y] < 0) m_sCurY -= m_aAbsEffect[SHADOW_Y] ;
	}
	*/

// will NOT check the end of line condition!
// return -1 if off the bottom!!!!
//
int16_t	RPrint::FrameIt() // uses m_rClip, m_sCurX, m_sCurY, m_sCellH
	{
	// check for above top:
	if (m_sCurY < m_rClip.sY)
		{
		// go to upper left corner...
		m_sCurX = m_rClip.sX;
		m_sCurY = m_rClip.sY;
		return 1;
		}

	// check for just left, assumes not above:
	if (m_sCurX <= m_rClip.sX)
		{
		m_sCurX = m_rClip.sX; // jump to right
		return 1;
		}

	// check for right and include in bottom....
	// do NOT check if there is space for a character...
	//
	if (m_sCurX + m_sCellW > m_rClip.sX + m_rClip.sW)
		{ // do a carriage return:
		m_sCurX = m_rClip.sX;
		m_sCurY += m_sCellH;

		// top left positioning...
		if (m_sCurY + m_sCellH > m_rClip.sY + m_rClip.sH)
			return -1; // column stop
		else
			return 1;
		}

	// check for column stop
	if (m_sCurY + m_sCellH > m_rClip.sY + m_rClip.sH)
		return -1; // column stop
	
	// keep printing in place without justifying
	return 0;
	}

// This scans through the characters, maps their positions,
// and determines how many fit on this line.
// It returns NULL for end of input, or points to the 
// first character of the following line to be printed.
// It looks at the FIELD proper to se if the line is partially
// in progress.
// Assume the Y-values are kosher
//
char* RPrint::ScanLine(char* pszInput)
	{
	int16_t sFirst = TRUE;
	if (m_eModes & FIELD) sFirst = FALSE;
	char* pLast = pszInput;
	int16_t i=0,sWhiteSpace = 0;
	int16_t sX = m_sCurX;
	int16_t sLeft,sW,sRight;
	int16_t sBreakedLine = FALSE;

	// You'll have to augment this to catch the case of TAB stops!
	// (But put that into the \t interpreter!)
	while (1)
		{
		if ((*pLast == 0) || *pLast == '\r' || *pLast == '\n')
			{
			// This only applies to full justify case!
			//m_eModes = Mode(m_eModes | FIELD); // don't justify!
			break; // got all the characters!
			}
	
		// Not zero:
		GetPropCellX((int16_t)uint8_t(*pLast),&sLeft,&sW,&sRight,sFirst);
		sFirst = FALSE;

		if ((sX + sW) > (m_rClip.sX + m_rClip.sW))
			{
			sBreakedLine = TRUE;
			break; // hit EOL!
			}

		if (IsWhiteSpace(*pLast)) 
											sWhiteSpace++;

		// fit another char on the line:
		ms_sCharPosX[i] = sX + sLeft; // store for prosperity
		sX += sRight;
		ms_sCharPosX[i+1] = sX + sLeft; // store for continuation
		ms_szLineText[i] = uint8_t(*pLast);
		pLast++; i++;
		}

	// Do I need Back up a word and eat whitespace?
	if (sBreakedLine) // my line is flowing along:
		{
		if (m_eModes & WORD_WRAP) // choose white space eatin':
			{
			if (IsWhiteSpace(*pLast)) // just eat the whitespace:
				{
				while (pLast >= pszInput)
					{
					if (IsWhiteSpace(*pLast)) {pLast--;sWhiteSpace--;i--;}
					else break;
					}
				}
			else // back up a word and whitespace
				{
				if ((*pLast != '\n') && (*pLast != '\0')) // special, don't eat!
					{
					while (pLast >= pszInput)
						{
						if (!IsWhiteSpace(*pLast)) {pLast--;sWhiteSpace--;i--;}
						else break;
						}
					while (pLast >= pszInput)
						{
						if (IsWhiteSpace(*pLast)) {pLast--;sWhiteSpace--;i--;}
						else break;
						}
					}
				else
					{ pLast--;i--;}
					
				}
			pLast++;i++;
			}
		}
	else	// I hit a newline or return but STILL wish to eat whitespace
		{
		// Don't change pLast but DO back up m_sNumChars:
		i--;
		while (i && IsWhiteSpace(ms_szLineText[i])) i--;
		i++;
		}

	m_sNumWhite = sWhiteSpace;
	m_sNumChar = i;

	// Store the final width based on the last characters full extent:
	int16_t sExt = 0;
	GetPropCellX(ms_szLineText[i-1],NULL,&m_sExtX);
	m_sExtX += ms_sCharPosX[i-1];

	return pLast;
	}

// This is the stripped down user version
// It returns the basic width in pixels of the string...
// It does NOT handle multiline effects...
// (ScanLine, for example, eats whitespace characters
// and partial words which GetWidth would include!)
//
int16_t RPrint::GetWidth(char* pszInput)
	{
	int16_t sFirst = TRUE;
	char* pLast = pszInput;
	int16_t i=0;
	int16_t sX = 0; // for this version
	int16_t sLeft,sW,sRight;

	// You'll have to augment this to catch the case of TAB stops!
	// (But put that into the \t interpreter!)
	while (1)
		{
		if ((*pLast == 0) || *pLast == '\r' || *pLast == '\n')
			{
			break; // got all the characters!
			}

		// Not zero:
		GetPropCellX((int16_t)uint8_t(*pLast),&sLeft,&sW,&sRight,sFirst);
		sFirst = FALSE;

		// fit another char on the line:
		ms_sCharPosX[i] = sX; // store for prosperity
		sX += sRight;
		ms_sCharPosX[i+1] = sX; // store for continuation
		ms_szLineText[i] = uint8_t(*pLast);
		pLast++; i++;
		}

	m_sNumChar = i;

	return ms_sCharPosX[m_sNumChar];
	}

// returns 0 if not a printable character
//
int16_t RPrint::GetBlitW(uint8_t c)
	{
	int16_t sW;
	if (!m_pCurFontSet->m_ppimCharacters[c]) return 0;
	sW = m_pCurFontSet->m_ppimCharacters[c]->m_sWidth;
	// Add in effects for BLiT needs only!
	// For now, use stretch as a cheating bold!
	sW = int16_t(m_fWidthScale * sW * m_aARelEffect[STRETCH] * 
		(m_aARelEffect[BOLD] + 1.0));

	return sW;
	}

// This should be used AFTER the line has been scanned:
// it operates ONLY on global parameters within Print.
// It will do final advanced line formatting.
//
void  RPrint::FormatText()
	{
	int16_t sTotW,sLineW;
	int16_t sJustOffset = 0;
	int16_t i;

	// Simple Justify: Left, Right, Center, but not full
	if ((m_eModes & JUSTIFY_ON) && !(m_eModes & FIELD))
		{
		// NOT 100% perfect...
		// It's position must include the clipping offset in an absolute sense...
		// For whatever reason, I am assuming the justifying will occur between
		// ms_sCharPosX[0] (abs) and (m_rClip.sX + m_rClip.sW) (abs)

		// m_sExtX is the rightmost x position (abs)

		sTotW = m_sExtX - ms_sCharPosX[0]; // actual text size
		sLineW = MIN (m_rClip.sW - ms_sCharPosX[0],int(m_rClip.sW)); // space to format in...

		if (!(m_eModes & JUSTIFY_FULL))
			{
			if (m_eModes & JUSTIFY_RIGHT) // right justify
				{
				sJustOffset = (sLineW - sTotW);
				}
			else									// center justify
				{
				// check for NP case!
				sJustOffset = (sLineW - sTotW)>>1;
				}

			// Maske the simple justification:
			for (i=0;i <= m_sNumChar;i++)
				{
				ms_sCharPosX[i] += sJustOffset;
				}
			}
		else
			{			// else progressive justify
			if (m_sNumWhite > 0)
				{
				double dAddWhite = double(sLineW - sTotW)/m_sNumWhite;
				double dTotOffset = 0.0;

				for (i = 0; i <= m_sNumChar; i++)
					{
					if (IsWhiteSpace(ms_szLineText[i]))
						dTotOffset += dAddWhite;

					ms_sCharPosX[i] += int16_t(dTotOffset);
					}
				}
			}
		}
	}


// This should be used AFTER the line has been scanned
// and formatted.
// it operates ONLY on global parameters within Print.
// It merely draws the line of text, including text effects.
// It repositions the cursor at the end of the line of text.
//
void	RPrint::DrawText()
	{
	// assumes everything's set up already!
	// Must determine if full justification is necessary
	
	// For this instant, DO NOT JUSTIFY
	// test som first!
	int16_t i;
	int16_t sX,sY;

	int16_t sShadowX = m_aAbsEffect[SHADOW_X];
	int16_t sShadowY = m_aAbsEffect[SHADOW_Y];
	int16_t sDoShadow = sShadowX | sShadowY;
	int16_t sMainX = 0,sMainY = 0;

	if (sShadowX < 0) {sMainX = -sShadowX; sShadowX = 0; }
	if (sShadowY < 0) {sMainY = -sShadowY; sShadowY = 0; }

	sY = m_sCurY;

	if (sDoShadow) // do the shadow!
		{
		for (i=0;i<m_sNumChar;i++)
			{
			sX = ms_sCharPosX[i];
			// But DO SUPPORT SCALING:
			int16_t sW;
			RImage* pim = m_pCurFontSet->m_ppimCharacters[ms_szLineText[i]];

			sW = GetBlitW(ms_szLineText[i]);

			if (sW)
				switch (pim->m_type)
					{
					case RImage::FSPR1:
							rspBlit(m_ShadowColor,pim,m_pimDst,sX+sShadowX,sY+sShadowY,sW,m_sCellH);
					}
			}
		}

	for (i=0;i<m_sNumChar;i++)
		{
		sX = ms_sCharPosX[i];
		int16_t sW;
		RImage* pim = m_pCurFontSet->m_ppimCharacters[ms_szLineText[i]];

		sW = GetBlitW(ms_szLineText[i]);

		if (sW)
			switch (pim->m_type)
				{
				case RImage::FSPR1:
					if (m_pimDst->m_type == RImage::BMP1)
						rspBlitToMono(pim,m_pimDst,sX,sY,sW,m_sCellH);
					else
						rspBlit(m_ForegroundColor,pim,m_pimDst,sX + sMainX,sY + sMainY,sW,m_sCellH);
				break;
				case RImage::BMP8:
					rspBlitT(pim,m_pimDst,sX,sY,sW,m_sCellH);
				break;
				case RImage::FSPR8:
					// Currently, no scaling possible for FSPR8
					rspBlit(pim,m_pimDst,sX,sY);
				break;
				default:
					TRACE("RPrint::DrawText: Image type not supported yet.\n");
				}
		}

	// MOVE ACTUAL CURSOR POSITION TO END OF LINE:
	// If line formatting takes place, than the cursor 
	// will be reset anyway.  This only screw up stops.

	m_sCurX = ms_sCharPosX[m_sNumChar]; // one past...
	}

// Attempting to pass on
char* RPrint::print(char* pszFormat,...)
	{
	//char szInput[4096];

	va_list vararg;
	
	va_start(vararg,pszFormat);
	vsprintf(ms_szInput,pszFormat,vararg);

	return printInt(ms_szInput);
	}

// Attempting to pass on
char* RPrint::print(int16_t sX,int16_t sY,char* pszFormat,...)
	{
	//char szInput[4096];

	va_list vararg;
	
	va_start(vararg,pszFormat);
	vsprintf(ms_szInput,pszFormat,vararg);

	m_sCurX = sX;
	m_sCurY = sY;

	return printInt(ms_szInput);
	}

// Attempting to pass on
char* RPrint::print(RImage* pimDst,int16_t sX,int16_t sY,char* pszFormat,...)
	{
	//char szInput[4096];

	va_list vararg;
	
	va_start(vararg,pszFormat);
	vsprintf(ms_szInput,pszFormat,vararg);

	m_sCurX = sX;
	m_sCurY = sY;

	SetDestination(pimDst);

	return printInt(ms_szInput);
	}

// returns null if sucessful.
// Points to the character to start printing the
// next column at if it goes off the bottom of the
// screen.
//
char* RPrint::printInt(char* pszInput)
	{

#ifdef _DEBUG
	if ( (m_pCurFontSet == NULL) || (m_sCellH == 0) )
		{
		TRACE("RPrint::print: No font installed yet.\n");
		return NULL;
		}

	if (m_pimDst == NULL)
		{
		TRACE("RPrint::print: No destination image specified.\n");
		return NULL;
		}
#endif

	// Move back onto the screen, if possible.
	int16_t sRet = 0;
	if ((sRet = FrameIt()) == -1)
		{
		return NULL; // of the bottom of the screen
		}

	//OffsetShadow();

	// Am I starting mid line?
	if (sRet == 1) m_eModes = (Mode)(m_eModes & (~FIELD));
	else m_eModes = (Mode)(m_eModes | (FIELD));

	// BEGIN MULTILINE LOGIC!

	char* pTest = pszInput;
	int16_t sStopped;
	do	{
		sStopped = FALSE;

		pTest = ScanLine(pTest); // Moves cursor to EOL!
		
		FormatText();
		DrawText();

		// A crude hack for now:
		// Eat initial white due to backing up:
		if (m_eModes & WORD_WRAP)
			{
			while (IsWhiteSpace(*pTest)) pTest++;
			}

		// Determine state of printing (\0 terminated...)
		switch (*pTest)
			{
			case '\n':
			case '\r':
				pTest++; // eat carriage return...
			break;
			case '\0': // not relly an end of line:
				// LEAVE the null as a signal to control!!!
				sStopped = TRUE; // MID LINE!
				m_eModes = (Mode)(m_eModes | FIELD);
			break;
			}

		// Check for new line scenario:
		if (sStopped == FALSE)
			{
			// perform the carriage return:
			// Begin a fresh line!
			m_eModes = (Mode)(m_eModes & (~FIELD));
			m_sCurX = m_rClip.sX;
			m_sCurY += m_sCellH;
			//OffsetShadow();

			// Eat initial white space:
			if (m_eModes & WORD_WRAP)
				{
				while (IsWhiteSpace(*pTest)) pTest++;
				}
			}

		} while ((sStopped == FALSE) && (m_sCurY + m_sCellH <= m_rClip.sY + m_rClip.sH));


	if (*pTest == '\0') return NULL;
	else	return pTest;
	}

// Returns -1 if requested font or character doen't exist:
// Parameters are relative to current location:
// All style parameters are taken into account!
// This is NOT for drawing, but for CELL estimation!!!
// It should NOT be called if your are in UP MODE!
//
int16_t RPrint::GetPropCellX(int16_t sChar,int16_t *psX,int16_t *psE,int16_t *psNext,
									int16_t sFirst)
	{
	RImage* pimLetter = NULL;
	int16_t a=0,b=0,c=0; // LKERN, true Width, RKERN
	int16_t sSpecial = FALSE;

	// In case of error:
	if (psX) *psX = 0;
	if (psE) *psE = 0;
	if (psNext) *psNext = 0;

	// HOOK SPECIAL CHARACTERS:
	switch (sChar)
		{
	case '\t':
		// Here hook all the advanced Tab stuff:
		// For now, just use a normal Tab:
		a = c = 0;
		b = m_aAbsEffect[TAB];
		sSpecial = TRUE;
	break;

	case ' ': // BEFORE effects are added!
		a = c = 0;
		b = int16_t(m_sCellH / 3);
		sSpecial = TRUE;
	break;
		}

	if (!sSpecial) // a normal ascii character
		{
		if (m_eModes & UNPROPORTIONAL)
			{
			if (psX) *psX = 0;
			if (psE) *psE = m_sUP_W;
			if (psNext) *psNext = m_sUP_W;
			}

		if (!m_pCurFontSet) return -1;
		if (!(m_pCurFontSet->m_ppimCharacters)) return -1;
		if (!(pimLetter=m_pCurFontSet->m_ppimCharacters[sChar])) return -1;

		if (pimLetter->m_type != RImage::FSPR1) // assume no kerning infoL
			{
			a = c = 0;
			b = pimLetter->m_sWidth;
			}
		else
			{
			RSpecialFSPR1 *pInfo = (RSpecialFSPR1*) pimLetter->m_pSpecial;
			a = pInfo->m_s16KernL;
			b = (int16_t)pInfo->m_u16Width;
			c = pInfo->m_s16KernR;
			}
		}

	// Convert to offsets from your start:
	if (psX) *psX = a;
	if (psE) *psE = a + b;
	if (psNext) *psNext = a + b + c;

	// Now account for effects, INCLUDING scale size, if applicable:
	if (!sSpecial) GetPropEffX(a,a + b,a + b + c,psX,psE,psNext);

	if (sFirst && (*psX < 0) ) { *psE -= *psX; *psNext -= *psX; *psX = 0;}
	return 0;
	}

// This is NOT for drawing, but for CELL estimation!!!
// This CAN be called if in UP mode -> it just returns basic values
// You must pass the base values..
// Includes effect of AddW but NOT UP_W
//
void RPrint::GetPropEffX(int16_t sX,int16_t sE,int16_t sNext,
								  int16_t *psEffX,int16_t *psEffE,int16_t *psEffNext)
	{
	// 1) SCALE THE BASE FONT INFO ACCORDINGLY:
	int16_t sEffX = sX;
	int16_t sEffE = sE;
	int16_t sEffNext = sNext;

	sEffX = int16_t(m_fWidthScale * sX);
	sEffE = int16_t(m_fWidthScale * sE);
	sEffNext = int16_t(m_fWidthScale * sNext);

	// 2) Add in factors which effect horizontal parameters:
	
	// Put off effects of stretch...

	if (m_aAbsEffect[ITALIC] < 0) sEffX += m_aAbsEffect[ITALIC];
	else sEffE += m_aAbsEffect[ITALIC];

	sEffE += m_aAbsEffect[BOLD];
	sEffNext += m_aAbsEffect[BOLD];

	// ALWAYS POSITIVE EXTENT!
	if (m_aAbsEffect[SHADOW_X] < 0) sEffX -= m_aAbsEffect[SHADOW_X];
	else sEffE += m_aAbsEffect[SHADOW_X];

	// after other effects, do stretch, since it needs to be a
	// relateive effect:
	sEffX = int16_t(m_aARelEffect[STRETCH] * sEffX);
	sEffE = int16_t(m_aARelEffect[STRETCH] * sEffE);
	sEffNext = int16_t(m_aARelEffect[STRETCH] * sEffNext);

	sEffNext += m_aAbsEffect[ADD_W];

	if (psEffX) *psEffX = sEffX;
	if (psEffE) *psEffE = sEffE;
	if (psEffNext) *psEffNext = sEffNext;
	}

//===================== SETTING UP VALUES ====================
void	RPrint::ResetEffects()
	{
	for (int16_t i=0;i<NUM_OF_EFFECTS;i++) 
		{
		m_aAbsFlag[i] = 0;
		m_aARelEffect[i] = 0.0;
		m_aAbsEffect[i] = 0;
		}

	// Effect specifics:
	m_aARelEffect[STRETCH] = 1.0; // shouldn't matter what abs says
	m_aAbsFlag[TAB] = 1; m_aAbsEffect[TAB] = m_sCellH;
	m_aAbsFlag[UP_W] = 1; m_aAbsEffect[UP_W] = m_sCellW;
	m_aAbsFlag[UP_H] = 1; m_aAbsEffect[UP_H] = m_sCellH;
	}

void RPrint::ResetMode()
	{
	m_eModes = (Mode)0;
	}

void RPrint::SetMode(Mode eMode,int16_t sVal)
	{
	if (sVal) m_eModes  = (Mode)(m_eModes | eMode);
	else m_eModes = (Mode)(m_eModes & (~eMode));
	}

int16_t RPrint::SetCellW()
	{
	if (m_pCurFontSet == NULL) return -1;
	// before effects:
	m_sCellW = int16_t(int32_t(m_sCellH) * m_pCurFontSet->m_sMaxWidth /
							m_pCurFontSet->m_sCellHeight);
	// add effects:
	// MAKE SURE THIS WORKS INSPITE OF NO LEADER!
	GetPropEffX(0,m_sCellW,m_sCellW,NULL,NULL,&m_sCellW);
	return 0;
	}

int16_t RPrint::SetEffectAbs(Effect eEffect,int16_t sVal) // absolute:
	{
	if (m_sCellH == 0)
		{
		TRACE("RPrint::SetEffect:ERROR - no font selected!\n");
		return -1;
		}

	m_aAbsFlag[eEffect] = 1;
	m_aAbsEffect[eEffect] = sVal;
	m_aARelEffect[eEffect] = float(sVal)/float(m_sCellH);
	// hook specific effects:
	switch (eEffect)
		{
		case ITALIC: // reset the italic slant
		break;

		}

	SetCellW(); // updates the cellW based on the latest effects
	return 0;
	}

int16_t RPrint::SetEffect(Effect eEffect,double dVal) // relative:
	{
	if (m_sCellH == 0)
		{
		TRACE("RPrint::SetEffect:ERROR - no font selected!\n");
		return -1;
		}

	m_aAbsFlag[eEffect] = 0;
	m_aAbsEffect[eEffect] = int16_t(dVal * m_sCellH);
	m_aARelEffect[eEffect] = float(dVal);
	// hook specific effects:
	switch (eEffect)
		{
		case ITALIC: // reset the italic slant
		break;

		}

	SetCellW(); // updates the cellW based on the latest effects
	return 0;
	}

int16_t RPrint::SetColumn(int16_t sX,int16_t sY,int16_t sW,int16_t sH)
	{
	// Clip the column to the current pimDst:
#ifdef _DEBUG
	if (m_pimDst == NULL)
		{
		TRACE("RPrint::SetColumn: NULL image!\n");
		return -1;
		}

	if ( (sW < 1) || (sH < 1))
		{
		TRACE("RPrint::SetColumn: Bad dimensions!\n");
		return -1;
		}
#endif

	if (sX < 0) sX = 0;
	if (sY < 0) sY = 0;

	if (sW > m_pimDst->m_sWidth) sW = m_pimDst->m_sWidth;
	if (sH > m_pimDst->m_sHeight) sH = m_pimDst->m_sHeight;

	if (sX + sW > m_pimDst->m_sWidth) sW = m_pimDst->m_sWidth - sX;
	if (sY + sH > m_pimDst->m_sWidth) sH = m_pimDst->m_sWidth - sY;

	m_rClip.sX = sX;
	m_rClip.sY = sY;
	m_rClip.sW = sW;
	m_rClip.sH = sH;

	return 0;
	}


int16_t RPrint::SetDestination(RImage* pimDst,RRect* prColumn)
	{
	if (pimDst == NULL)
		{
		TRACE("RPrint::SetDestination: NULL image!\n");
		return -1;
		}

	if (pimDst->m_type != RImage::BMP1) // special case
		if ( (!ImageIsUncompressed(pimDst->m_type)) || (pimDst->m_sDepth > 8))
		{
		TRACE("RPrint::SetDestination: This image type currently not supported!\n");
		return -1;
		}

	m_pimDst = pimDst;

	if (prColumn == NULL)
		{
		m_rClip.sX = m_rClip.sY = 0;
		m_rClip.sW = pimDst->m_sWidth;
		m_rClip.sH = pimDst->m_sHeight;
		}
	else
		{
		m_rClip.sX = prColumn->sX;
		m_rClip.sY = prColumn->sY;
		m_rClip.sW = prColumn->sW;
		m_rClip.sH = prColumn->sH;
		}

	return 0;
	}

RPrint::RPrint()
	{
	ResetEffects();
	ResetMode();
	m_fWidthScale = m_fHeightScale = 1.0;
	m_sCellH = m_sCellW = 0;
	m_sNumTabStops = m_sNumDecStops = 0;
	m_sCurX = m_sCurY = 0;
	m_pfnCurFont = NULL;
	m_pCurFontSet = NULL;
	m_BackgroundColor = 0;
	m_ShadowColor = 2;
	m_ForegroundColor = 1;
	m_pimDst = NULL;
	m_rClip.sX = m_rClip.sY = m_rClip.sW = m_rClip.sH = 0;
	m_sUP_W = m_sUP_H = 0;
	}

RPrint::~RPrint()
	{
	// don't know yet.
	}

// Must find a large enough size...
int16_t RPrint::SetFont(int16_t sCellH,RFont* pFont)
	{
	RFont::RFontSet* pFontSet = NULL;
	double dScale = 0.0;

	if (pFont == NULL) pFont = m_pfnCurFont;

	if (pFont == NULL)
		{
		TRACE("RPrint::SetFont: NULL FONT PASSED!\n");
		return -1;
		}

	if ((pFontSet = pFont->FindSize(sCellH,&dScale)) == NULL)
		{
		TRACE("RPrint::SetFont:  Font does not have this size.\n");
		return-1;
		}

#ifndef	PRINT_NO_WARNING

	if (pFontSet->m_sCellHeight != sCellH)
		{
		if (sCellH <= 24)
			{
			TRACE("SetFont WARNING: Used small non-cached size %d\n",sCellH);
			}
		}

#endif

	m_pfnCurFont = pFont;
	m_pCurFontSet = pFontSet;
	m_fWidthScale = m_fHeightScale = float(dScale);

	//***************** RESIZE ALL THE TEXT EFFECTS:
	// (just re-adjust the relative values...)

	for (int16_t i=0;i<NUM_OF_EFFECTS;i++)
		{
		if (	m_aAbsFlag[i] == 0	// a relative value
			&&	m_sCellH != 0)			// Don't allow divide by zero (JMI	05/09/97
											// And then again on 06/12/97 when changes to
											// RPrint.cpp were overwritten with a previous
											// version...ya bastid :) ).
			{
			m_aAbsEffect[i] = int16_t(double(sCellH) / m_sCellH * m_aAbsEffect[i]);
			}
		}

	// 1) make sure all relative 

	//**********************************************

	m_sCellH = sCellH;
	m_sCellW = int16_t(m_fWidthScale * m_pCurFontSet->m_sMaxWidth + .99);
	return SUCCESS;
	}

int16_t RPrint::SetColor(uint32_t ulForeColor,uint32_t ulBackColor,uint32_t ulShadowColor)
	{
	m_ForegroundColor = ulForeColor;
	if (ulBackColor) m_BackgroundColor = ulBackColor;
	if (ulShadowColor) m_ShadowColor = ulShadowColor;

	return 0;
	}

void	RPrint::SetWordWrap(int16_t sOn)
	{
	if (sOn)	m_eModes = (Mode)(m_eModes | WORD_WRAP);
	else m_eModes = (Mode)(m_eModes & (~WORD_WRAP));
	}

void RPrint::SetJustifyRight()
	{
	m_eModes = (Mode)(m_eModes | JUSTIFY_ON);	
	m_eModes = (Mode)(m_eModes & (~JUSTIFY_FULL));
	m_eModes = (Mode)(m_eModes | JUSTIFY_RIGHT);	
	}

void RPrint::SetJustifyCenter()
	{
	m_eModes = (Mode)(m_eModes | JUSTIFY_ON);	
	m_eModes = (Mode)(m_eModes & (~JUSTIFY_FULL));
	m_eModes = (Mode)(m_eModes & (~JUSTIFY_RIGHT));	
	}

void RPrint::SetJustifyLeft()
	{
	m_eModes = (Mode)(m_eModes & (~JUSTIFY_ON));
	m_eModes = (Mode)(m_eModes & (~JUSTIFY_RIGHT));
	m_eModes = (Mode)(m_eModes & (~JUSTIFY_FULL));
	}

void RPrint::SetJustifyFull()
	{
	m_eModes = (Mode)(m_eModes | JUSTIFY_ON);
	m_eModes = (Mode)(m_eModes & (~JUSTIFY_RIGHT));
	m_eModes = (Mode)(m_eModes | JUSTIFY_FULL);
	}

void	RPrint::GetPos(int16_t *psX,int16_t *psY,int16_t *psW,int16_t *psH)
	{
	if (psX) *psX = m_sCurX;
	if (psY) *psY = m_sCurY;
	if (psW) *psW = m_sCellW;
	if (psH) *psH = m_sCellH;
	}
