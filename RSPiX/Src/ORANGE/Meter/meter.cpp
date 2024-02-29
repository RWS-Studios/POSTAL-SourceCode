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
//////////////////////////////////////////////////////////////////////////////
//
// METER.CPP
// 
// History:
//		09/19/96 JMI	Started.
//
//		09/24/96	JMI	Now consists of a CDlg and a CGuiItem.
//
//		10/31/96	JMI	Changed:
//							Old label:			New label:
//							=========			=========
//							CMeter				RMeter
//							CImage				RImage
//							CGuiItem				RGuiItem
//							NEEDLE				Needle
//							DIGITAL				Digital
//							BAR					Bar
//							HISTOGRAM			Histogram
//							NUM_DISPLAY_TYPES	NumDisplayTypes
//							PERCENTAGE			Percentage
//							VALUE					Value
//							NUM_INFO_TYPES		NumInfoTypes
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							Rect				RRect
//
//							Also, changed all members referenced in RImage to
//							m_ and all position/dimension members referenced in
//							RImage to type short usage.
//
//		12/19/96	JMI	Uses new m_justification (as m_sJustification) and 
//							upgraded to new RFont/RPrint.
//
//////////////////////////////////////////////////////////////////////////////
//
// This object allows one to create a meter of varying type with a particular
// value either set explicitly or by calling Start/EndPeriod() to time an
// operation or group of operations.  The meter can even be used to time
// itself.  If you keep several meters running in debug mode of your program,
// you will be more likely to notice right away when certain changes impact
// performance.  Try 'em!
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "Blue.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/Meter/meter.h"
#else
	#include "meter.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////////////
// Module specific macros.
//////////////////////////////////////////////////////////////////////////////

// Sets val to def if val is -1.
#define DEF(val, def)	((val == -1) ? def : val)

#define DEF_WIDTH		75
#define DEF_HEIGHT	50

#define BORDER_THICKNESS	1

#define INIT_MAX	0
#define INIT_MIN	0x7FFFFFFF

//////////////////////////////////////////////////////////////////////////////
// Module specific typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Module specific (static) variables.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Default constructor.
//
//////////////////////////////////////////////////////////////////////////////
RMeter::RMeter()
	{
	m_lCurVal				= 0L;		// Value for next draw.      
	m_lStartPeriod			= 0L;		// Start period.             
	strcpy(m_szUnit, "");			// Unit of measurement text. 
	m_lMin					= 0L;		// Minimum value.            
	m_lMax					= 100L;	// Maximum value.            
	m_dtType					= (DisplayType)(rand() % NumDisplayTypes);	// Type of meter display.
	m_itType					= (InfoType)(rand() % NumInfoTypes);			// Type of meter info.
	m_u32Meter				= RSP_WHITE_INDEX;	// Meter color.              
	m_u32Needle				= RSP_BLACK_INDEX;	// Needle, bar, etc. color.  
	m_u32Overflow			= RSP_BLACK_INDEX;	// Needle color for over/underflow.	

	m_lDuration				= 100;		// Time between updates in milliseconds.
	m_lNextUpdate			= 0;			// Time of next update in 
												// milliseconds.
	m_lCurTotal				= 0;			// Current total.
	m_lNumValues			= 0;			// Number of values since
												// total was last cleared.
	m_lMaxValue				= INIT_MAX;	// Maximum value since
												// total was last cleared.
	m_lMinValue				= INIT_MIN;	// Minimum value since
												// total was last cleared.

	m_lQIndex				= 0;		// Index for histogram history queue.
	memset(m_asQHistory, 0, sizeof(m_asQHistory));

	// Override RGuiItem's default justification.
	m_justification		= RGuiItem::Centered;

	m_guiMeter.SetParent(this);
	m_guiMeter.m_bcUser				= BtnCall;
	m_guiMeter.m_ulUserInstance	= (ULONG)this;
	}

//////////////////////////////////////////////////////////////////////////////
//
// Destructor.
//
//////////////////////////////////////////////////////////////////////////////
RMeter::~RMeter()
	{
	}

////////////////////////////////////////////////////////////////////////
// Methods.
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Helper that keeps us from getting TRACE messages from rspRect when
// we have a height or width of 0.
//
////////////////////////////////////////////////////////////////////////
inline short Rectangle(		// Returns 0 on success.
	U32 u32Color,				// Color.
	RImage* pimDst,			// Destination.
	short sDstX,				// Destination x coordinate.
	short sDstY,				// Destination y coordinate.
	short sDstW,				// Width.
	short sDstH,				// Height.
	RRect* prClip = NULL)	// Optional clipping rectangle.
	{
	short	sRes	= 0;	// Assume success.

	if (sDstW > 0 && sDstH > 0)
		{
		sRes = rspRect(u32Color, pimDst, sDstX, sDstY, sDstW, sDstH, prClip);
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
//
// Compose the static portions.  Fills the parts of the image
// that don't change.
//
////////////////////////////////////////////////////////////////////////
void RMeter::Compose(				// Returns nothing.
	RImage*	pimDst /*= NULL*/)	// In: Destination.  NULL == use 
											// internal m_im.
	{
	short	sRes	= 0;	// Assume success.

	if (pimDst == NULL)
		{
		pimDst	= &m_im;
		}

	// Call base class.
	RDlg::Compose();

	short	sX, sY, sW, sH;
	GetClient(&sX, &sY, &sW, &sH);

	short	sCellH;
	m_pprint->GetPos(NULL, NULL, NULL, &sCellH);

	short	sMeterX;
	short	sMeterY;
	short	sMeterW;
	short	sMeterH;

	// Behave by type.
	switch (m_dtType)
		{
		case Digital:
			sMeterX	= sX + BORDER_THICKNESS;
			sMeterY	= sY + BORDER_THICKNESS;
			sMeterW	= sW - BORDER_THICKNESS * 2;
			sMeterH	= sH - BORDER_THICKNESS * 2;
			m_sInfoY	= sMeterY + (sMeterH / 2 - sCellH / 2);
			break;
		case Needle:
		case Bar:
		case Histogram:
			sMeterX	= sX + BORDER_THICKNESS;
			sMeterY	= sY + BORDER_THICKNESS;
			sMeterW	= sW - BORDER_THICKNESS * 2;
			sMeterH	= sH - BORDER_THICKNESS * 2 - sCellH;
			m_sInfoY	= sMeterY + sMeterH;
			break;
		default:
			TRACE("Compose(): Invalid display type.\n");
			break;
		}

	m_guiMeter.m_u32BackColor					= m_u32Meter;
	m_guiMeter.m_u32TextColor					= m_u32TextColor;
	m_guiMeter.m_u32BorderColor				= m_u32BorderColor;
	m_guiMeter.m_u32BorderShadowColor		= m_u32BorderShadowColor;
	m_guiMeter.m_u32BorderHighlightColor	= m_u32BorderHighlightColor;
	m_guiMeter.m_u32BorderEdgeColor			= m_u32BorderEdgeColor;
	m_guiMeter.m_sBorderThickness				= m_sBorderThickness;
	m_guiMeter.m_sInvertedBorder				= !m_sInvertedBorder;

	// If there is already data . . .
	if (m_guiMeter.m_im.m_pData != NULL)
		{
		m_guiMeter.m_im.DestroyData();
		}

	// Attempt to create display . . .
	if (m_guiMeter.Create(sMeterX, sMeterY, sMeterW, sMeterH, 
		m_im.m_sDepth) == 0)
		{
		// Set hot area to entire button.
		m_guiMeter.GetClient(
			&(m_guiMeter.m_hot.m_sX),
			&(m_guiMeter.m_hot.m_sY),
			&(m_guiMeter.m_hot.m_sW),
			&(m_guiMeter.m_hot.m_sH) );

		m_guiMeter.ChildPosToTop(&(m_guiMeter.m_hot.m_sX), &(m_guiMeter.m_hot.m_sY));
		}
	else
		{
		TRACE("Compose(): Failed to create meter display Gui.\n");
		}
	}

////////////////////////////////////////////////////////////////////////
//
// Composes the meter into the image provided.  For consistency, you
// should probably always draw the meter.
//
////////////////////////////////////////////////////////////////////////
short RMeter::Draw(					// Returns 0 on success.
			RImage* pimDst,			// Destination image.
			short sDstX	/*= 0*/,		// X position in destination.
			short sDstY	/*= 0*/,		// Y position in destination.
			short sSrcX /*= 0*/,		// X position in source.
			short sSrcY /*= 0*/,		// Y position in source.
			short sW /*= 0*/,			// Amount to draw.
			short sH /*= 0*/,			// Amount to draw.
			RRect* prc /*= NULL*/)	// Clip to.
	{
	short	sRes	= 0;	// Assume success.

	ASSERT(pimDst != NULL);	// Duh!

	// If visible . . .
	if (m_sVisible != FALSE)
		{
		long	lTime	= rspGetMilliseconds();
		if (lTime >= m_lNextUpdate)
			{
			// First copy precomposed stuff.
			if (m_im.m_pData != NULL)
				{
				RDlg::Draw(pimDst, sDstX, sDstY, sSrcX, sSrcY, sW, sH, prc);
				}

			short	sMeterX;
			short	sMeterY;
			short	sMeterW;
			short sMeterH;
			m_guiMeter.GetClient(&sMeterX, &sMeterY, &sMeterW, &sMeterH);
			sMeterX	+= m_sX + m_guiMeter.m_sX + sDstX;
			sMeterY	+= m_sY + m_guiMeter.m_sY + sDstY;

			short	sInfoY	= m_sY + sDstY + m_sInfoY;

			long	lAvg	= 0;
			if (m_lNumValues > 0)
				{
				lAvg = m_lCurTotal / m_lNumValues;
				}

			// Draw text label.
			if (m_pprint->GetFont() != NULL)
				{
				// Determine info based on type.
				long	lVal	= lAvg;
				char	szExtra[32]	= "";
				switch (m_itType)
					{
					case Value:
						break;
					case Percentage:
						lVal	= ((lVal - m_lMin) * 100) / (m_lMax - m_lMin);
						strcpy(szExtra, "%");
						break;
					default:
						TRACE("Draw(): Invalid info type.\n");
						break;
					}
				
				// If digital . . .
				if (m_dtType == Digital)
					{
					m_pprint->SetColor((short)m_u32Needle);
					}

				SetJustification();
				m_pprint->SetDestination(pimDst);
				m_pprint->SetColumn(sMeterX, sInfoY, sMeterW, sMeterH);

				// Draw text info.
				m_pprint->print(sMeterX, sInfoY, "%ld%s%s", lVal, szExtra, m_szUnit);
				}

			// Contain within range.
			lAvg			= MIN(m_lMax, lAvg);
			lAvg			= MAX(m_lMin, lAvg);

			m_lMaxValue	= MIN(m_lMax, m_lMaxValue);
			m_lMaxValue	= MAX(m_lMin, m_lMaxValue);

			m_lMinValue	= MIN(m_lMax, m_lMinValue);
			m_lMinValue	= MIN(m_lMin, m_lMinValue);

			// Amount to adapt value to match meter width.
			float	fAdaptor;
			// Adapted values.
			short	sMeterVal;
			short	sMeterMax;
			short	sMeterMin;
			// Draw meter based on type.
			switch (m_dtType)
				{
				case Digital:
					// Done.
					break;
				case Needle:
					// Compute adaptor.
					fAdaptor		= (float)sMeterW / (float)(m_lMax - m_lMin);
					// Compute adapted.
					sMeterVal	= (short)((float)lAvg * fAdaptor);
					sMeterMin	= (short)((float)m_lMinValue * fAdaptor);
					sMeterMax	= (short)((float)m_lMaxValue * fAdaptor);
					// Draw min.
					Rectangle(
						m_u32Overflow, 
						pimDst,
						sMeterX + sMeterMin, sMeterY, 
						1, sMeterH
						);
					// Draw avg.
					Rectangle(
						m_u32Needle, 
						pimDst,
						sMeterX + sMeterVal, sMeterY, 
						1, sMeterH
						);
					// Draw max.
					Rectangle(
						m_u32Overflow, 
						pimDst,
						sMeterX + sMeterMax, sMeterY, 
						1, sMeterH
						);
					break;
				case Bar:
					// Compute adaptor.
					fAdaptor	= (float)sMeterW / (float)(m_lMax - m_lMin);
					// Compute adapted.
					sMeterVal	= (short)((float)lAvg * fAdaptor);
					sMeterMin	= (short)((float)m_lMinValue * fAdaptor);
					sMeterMax	= (short)((float)m_lMaxValue * fAdaptor);
					// Draw min.
					Rectangle(
						m_u32Overflow, 
						pimDst,
						sMeterX, sMeterY, 
						sMeterMin, sMeterH
						);
					// Draw avg.
					Rectangle(
						m_u32Needle, 
						pimDst,
						sMeterX + sMeterMin, sMeterY, 
						sMeterVal - sMeterMin, sMeterH
						);
					// Draw max.
					Rectangle(
						m_u32Overflow, 
						pimDst,
						sMeterX + sMeterVal, sMeterY, 
						sMeterMax - sMeterVal, sMeterH
						);
					break;
				case Histogram:
					{
					// Compute adaptor.
					fAdaptor	= (float)sMeterH / (float)(m_lMax - m_lMin);
					// Compute adapted.
					sMeterVal	= (short)((float)lAvg * fAdaptor);
					sMeterMin	= (short)((float)m_lMinValue * fAdaptor);
					sMeterMax	= (short)((float)m_lMaxValue * fAdaptor);
					// Store new value.
					m_asQHistory[m_lQIndex]	= sMeterVal;
					// Increase index.
					m_lQIndex	= (m_lQIndex + 1) % METER_HISTOGRAM_HISTORY;
					long	l;
					short	sVal;
					short	sBarWidth	= sMeterW / METER_HISTOGRAM_HISTORY;
					short	sBarPos		= sMeterX;
					for (l = 0; l < (METER_HISTOGRAM_HISTORY - 1); l++, sBarPos += sBarWidth)
						{
						// Get value.
						sVal	= m_asQHistory[(m_lQIndex + l) % METER_HISTOGRAM_HISTORY];
						// Draw avg bar.
						Rectangle(
							m_u32Needle,
							pimDst,
							sBarPos, 
							sMeterY + (sMeterH - sVal),
							sBarWidth, sVal
							);
						}

					short	sPosY	= sMeterY + sMeterH - sMeterMax;
					short	sDiff	= sMeterMax - sMeterVal;
					Rectangle(
						m_u32Overflow,
						pimDst,
						sBarPos,
						sPosY,
						sBarWidth, sDiff);

					// Draw avg.
					sPosY	+= sDiff;
					sDiff	= sMeterVal - sMeterMin;
					Rectangle(
						m_u32Needle,
						pimDst,
						sBarPos,
						sPosY,
						sBarWidth, sDiff);

					// Draw max.
					sPosY	+= sDiff;
					// Draw min.
					Rectangle(
						m_u32Overflow,
						pimDst,
						sBarPos,
						sPosY,
						sBarWidth, sMeterMin);
					break;
					}
				}

			// Reset counter, accumulator, max, min.
			m_lMaxValue		= INIT_MAX;
			m_lMinValue		= INIT_MIN;
			m_lCurTotal		= 0;
			m_lNumValues	= 0;

			// Remember when to update next.
			m_lNextUpdate	= lTime + m_lDuration;
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////
// Querries.
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
