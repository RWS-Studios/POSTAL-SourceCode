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
#ifndef RTVIDC_H
#define RTVIDC_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "dispatch.h"
#include "Image.h"
#include "win.h"
#include <vfw.h>
					  
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// This value can be from 0 to 64K - 1.
// The overhead per channel is sizeof(VIDC_RT_HDR) bytes.
#define MAX_VID_CHANNELS	25

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
// Forward declare classes for RTVIDC_CALL definition.
class CRtVidc;

class VIDC_RT_HDR;

typedef VIDC_RT_HDR* PVIDC_RT_HDR;

typedef void (*RTVIDC_CALL)(CRtVidc* prtvidc, PVIDC_RT_HDR pvidchdr);

class VIDC_RT_HDR
	{
	public:
		// Header info from stream.
		int16_t			sNumFrames;			// Number of frames. Max 4000.
		int16_t			sWidth;				// Width in pixels (always 320 in FLI)
		int16_t			sHeight;				// Height in pixels (always 200 in FLI)
		int16_t			sDepth;				// Bits per pixel (always 8)
		int32_t			lMilliPerFrame;	// Milliseconds between frames.
		int16_t			sNoDelta;			// TRUE if no deltas, FALSE otherwise.
		int16_t			sTransparent;		// Blt with transparency if TRUE.
		int16_t			sX;					// Intended position in x direction.
		int16_t			sY;					// Intended position in y direction.
		uint32_t			ulFCCHandler;		// FCC of Windows' VIDC handler.
	
		// Header info for our use.
		CImage*		pImage;				// Where to blt.
		int16_t			sCurFrame;			// Current frame number (0 origin).
		int16_t			sColorsModified;	// TRUE if colors were modified last
												// decompression.
		int32_t			lMaxLag;				// Maximum lag before skipping frames.
		RTVIDC_CALL	callbackHeader;	// Callback on header receipt.
		RTVIDC_CALL	callbackBefore;	// Callback before decompression.
		RTVIDC_CALL	callbackAfter;		// Callback after decompression.
		HIC			hic;					// Handle to the image compressor/de-
												// compressor.
	};

class CRtVidc
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CRtVidc();
		// Destructor.
		~CRtVidc();

	public:		// Methods.
		// Sets the dispatcher.
		void SetDispatcher(CDispatch* pdispatch);

		// Sets callback(s) called on channel header receipt.
		void SetCallbackHeader(RTVIDC_CALL callback);
		void SetCallbackHeader(RTVIDC_CALL callback, int16_t sChannel);
		// Sets callback(s) called before decompression.
		void SetCallbackBefore(RTVIDC_CALL callback);
		void SetCallbackBefore(RTVIDC_CALL callback, int16_t sChannel);
		// Sets callback(s) called after decompression.
		void SetCallbackAfter(RTVIDC_CALL callback);
		void SetCallbackAfter(RTVIDC_CALL callback, int16_t sChannel);

	protected:	// Internal typedefs.
		typedef struct
			{
			BITMAPINFOHEADER	bmiHeader;
			RGBQUAD				bmiColors[256];
			}	BMI, *PBMI;

	protected:	// Internal methods.
		// Sets variables w/o regard to current values.
		void Set();
		// Resets variables.  Performs deallocation if necessary.
		void Reset();

		// Decompresses a VIDC frame using the opened decompressor.
		// Returns 0 on success.
		int16_t DecompressFrame(	PVIDC_RT_HDR pvidchdr, CNFile* pfile, 
										uint32_t ulFlags, PBMI pbmiIn, PBMI pbmiOut);

		// Use handler for RtVidc buffers.
		// Returns RET_FREE if done with data on return, RET_DONTFREE otherwise.
		int16_t Use(	uint8_t* puc, int32_t lSize, uint16_t usType, uint8_t ucFlags, 
						int32_t lTime);
		// Static entry point for above.
		static int16_t UseStatic(	uint8_t* puc, int32_t lSize, uint16_t usType, 
										uint8_t ucFlags, int32_t lTime, int32_t l_pRtVidc);

	protected:	// Internal typedefs.

	public:		// Members.
		

	protected:	// Members.
		VIDC_RT_HDR	m_avidchdrs[MAX_VID_CHANNELS];// Info for each channel.
		uint16_t		m_usState;					// The current state of this CRtVidc.
		CDispatch*	m_pdispatch;				// The dispatcher for this CRtVidc.

	};


#endif // RTVIDC_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
