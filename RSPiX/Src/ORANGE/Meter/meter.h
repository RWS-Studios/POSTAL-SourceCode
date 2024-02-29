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
#ifndef METER_H
#define METER_H

//////////////////////////////////////////////////////////////////////////////
// Please see the CPP file for an explanation of this API.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////

#include "System.h"
// If PATHS_IN_INCLUDES macro is defined, we can utilized relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GUI/dlg.h"
#else
	#include "Dlg.h"
#endif // PATHS_IN_INCLUDES

#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// Number of values remembered for histogram.
#define METER_HISTOGRAM_HISTORY	20		// Values.
// Maximum length for the unit string.
#define MAX_UNIT_LEN					128	// Characters.

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
class RMeter : public RDlg
	{
	public:	// Construction/Destruction.
		// Default constructor.
		RMeter(void);
		// Destructor.
		~RMeter(void);

//////////////////////////////////////////////////////////////////////////////

	public:	// Enums.

		////////////////////////////////////////////////////////////////////////
		// Enums.
		////////////////////////////////////////////////////////////////////////
		typedef enum
			{
			Needle,				// Analog needle meter.
			Digital,				// Digital meter.
			Bar,					// Bar (like progress bar).
			Histogram,			// Histogram.
			NumDisplayTypes	// Ceiling.
			} DisplayType;		// Type of meter display.

		typedef enum
			{
			Percentage,			// Info displayed as percentage.
			Value,				// Info displayed as value.
			NumInfoTypes		// Ceiling.
			} InfoType;			// Type of meter info.

	public:	// Methods.

		////////////////////////////////////////////////////////////////////////
		// Methods.
		////////////////////////////////////////////////////////////////////////

		//////////////////////// Virtual Overrides /////////////////////////////

		// Composes the meter into the image provided.  For consistency, you
		// should probably always draw the meter.
		short Draw(					// Returns 0 on success.
			RImage* pimDst,		// Destination image.
			short sDstX	= 0,		// X position in destination.
			short sDstY	= 0,		// Y position in destination.
			short sSrcX = 0,		// X position in source.
			short sSrcY = 0,		// Y position in source.
			short sW = 0,			// Amount to draw.
			short sH = 0,			// Amount to draw.
			RRect* prc = NULL);	// Clip to.

		// Activate or deactivate mouse reaction.
		void SetActive(		// Returns nothing.
			short sActive)		// TRUE to make active, FALSE otherwise.
			{ 
			RDlg::SetActive(sActive); 
			m_guiMeter.SetActive(sActive);
			}

		////////////////////// Original to this class ///////////////////////////

		// Set the new value for the meter.  Do this BEFORE drawing for most
		// accurate results.
		long SetNewValue(	// Returns previous value.
			long lNewVal)	// In: New value.
			{
			long	lRes	= m_lCurVal;
			// Store new val.
			m_lCurVal	= lNewVal;
			// Accumulate total.
			m_lCurTotal	+= lNewVal;
			// Count updates.
			m_lNumValues++;
			// If the new value is less than the min . . .
			if (lNewVal < m_lMinValue)
				{
				m_lMinValue	= lNewVal;
				}
			// If the new value is greater than the max . . .
			if (lNewVal > m_lMaxValue)
				{
				m_lMaxValue	= lNewVal;
				}
			// Return old.
			return lRes;
			}

		// This begins a period to be timed with rspGetMilliseconds().
		// Call EndPeriod() to end the period and update the meter.
		void StartPeriod(void)
			{
			m_lStartPeriod	= rspGetMilliseconds();
			}

		// This ends a period and updates the meter with the length of
		// the period in milliseconds.
		long EndPeriod(void)	// Returns period.
			{
			SetNewValue(rspGetMilliseconds() - m_lStartPeriod);
			return m_lCurVal;
			}

		// Set duration between updates.
		void SetUpdateInterval(	// Returns nothing.
			long lDuration)		// Time in milliseconds between updates.
			{
			m_lDuration	= lDuration;
			}

		// Set the type of meter you want.
		void SetType(				// Returns nothing.
			DisplayType dtType,	// In: Type of meter display.  
										// See Enums above.
			InfoType itType)		// In: Type of info display.
										// See Enums above.
			{
			m_dtType	= dtType;
			m_itType	= itType;
			}

		// Set the range for the meter.
		void SetRange(		// Returns nothing.
			long lMin,		// Minimum value.
			long lMax)		// Maximum value.
			{
			m_lMin	= lMin;
			m_lMax	= lMax;
			}

		// Set the unit of measurement for info display.
		void SetUnit(			// Returns nothing.
			char* pszUnit)		// Unit string (ex: "Mbytes" or "ms").
			{
			ASSERT(strlen(pszUnit) < sizeof(m_szUnit));
			strcpy(m_szUnit, pszUnit);
			}

		// Set the colors.
		void SetColors(			// Returns nothing.
			U32 u32Background,	// Background color.
			U32 u32Meter,			// Meter color.
			U32 u32Needle,			// Needle, bar, etc. color.
			U32 u32Text,			// Text foreground color.
			U32 u32Overflow)		// Needle color for over/underflow.
			{
			m_u32BackColor		= u32Background;
			m_u32Meter			= u32Meter;
			m_u32Needle			= u32Needle;
			m_u32TextColor		= u32Text;
			m_u32Overflow		= u32Overflow;
			}

		// Compose the static portions.  Fills the parts of the image
		// that don't change.
		void Compose(						// Returns 0 nothing.
			RImage*	pimDst = NULL);	// In: Destination.  NULL == use internal m_im.

		// Advance to next meter type.  Called by btn callback.
		void NextType(void)
			{
			SetType(
						(DisplayType)((m_dtType + 1) % NumDisplayTypes),
						m_itType);
			Compose();
			}

		////////////////////////////////////////////////////////////////////////
		// Querries.
		////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////

	public:	// Static

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.
		// Callbacks from m_guiMeter's Hot.
		static void BtnCall(RGuiItem* pgui)
			{ ((RMeter*)pgui->m_ulUserInstance)->NextType(); }

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.
		long				m_lCurVal;							// Value for next draw.
		long				m_lStartPeriod;					// Start period.
		char				m_szUnit[MAX_UNIT_LEN + 1];	// Unit of measurement text.
		long				m_lMin;								// Minimum value on meter.
		long				m_lMax;								// Maximum value on meter.
		DisplayType		m_dtType;							// Type of meter display.
		InfoType			m_itType;							// Type of meter info.
		U32				m_u32Meter;							// Meter color.
		U32				m_u32Needle;						// Needle, bar, etc. color.
		U32				m_u32Overflow;						// Needle color for over/underflow.

		long				m_lDuration;						// Time between updates in
																	// milliseconds.
		long				m_lNextUpdate;						// Time of next update in 
																	// milliseconds.
		long				m_lCurTotal;						// Current total.
		long				m_lNumValues;						// Number of values since
																	// total was last cleared.
		long				m_lMaxValue;						// Maximum value since
																	// total was last cleared.
		long				m_lMinValue;						// Minimum value since
																	// total was last cleared.
		RGuiItem			m_guiMeter;							// Actual meter gui.

		// History of values for histogram.
		short				m_asQHistory[METER_HISTOGRAM_HISTORY];
		long				m_lQIndex;


	protected:	// Internal typedefs.

	protected:	// Protected member variables.
		short				m_sInfoY;	

	};

#endif // METER_H
//////////////////////////////////////////////////////////////////////////////
//	EOF
//////////////////////////////////////////////////////////////////////////////
