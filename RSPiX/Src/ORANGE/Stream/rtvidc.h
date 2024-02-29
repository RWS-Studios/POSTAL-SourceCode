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
		short			sNumFrames;			// Number of frames. Max 4000.
		short			sWidth;				// Width in pixels (always 320 in FLI)
		short			sHeight;				// Height in pixels (always 200 in FLI)
		short			sDepth;				// Bits per pixel (always 8)
		long			lMilliPerFrame;	// Milliseconds between frames.
		short			sNoDelta;			// TRUE if no deltas, FALSE otherwise.
		short			sTransparent;		// Blt with transparency if TRUE.
		short			sX;					// Intended position in x direction.
		short			sY;					// Intended position in y direction.
		ULONG			ulFCCHandler;		// FCC of Windows' VIDC handler.
	
		// Header info for our use.
		CImage*		pImage;				// Where to blt.
		short			sCurFrame;			// Current frame number (0 origin).
		short			sColorsModified;	// TRUE if colors were modified last
												// decompression.
		long			lMaxLag;				// Maximum lag before skipping frames.
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
		void SetCallbackHeader(RTVIDC_CALL callback, short sChannel);
		// Sets callback(s) called before decompression.
		void SetCallbackBefore(RTVIDC_CALL callback);
		void SetCallbackBefore(RTVIDC_CALL callback, short sChannel);
		// Sets callback(s) called after decompression.
		void SetCallbackAfter(RTVIDC_CALL callback);
		void SetCallbackAfter(RTVIDC_CALL callback, short sChannel);

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
		short DecompressFrame(	PVIDC_RT_HDR pvidchdr, CNFile* pfile, 
										ULONG ulFlags, PBMI pbmiIn, PBMI pbmiOut);

		// Use handler for RtVidc buffers.
		// Returns RET_FREE if done with data on return, RET_DONTFREE otherwise.
		short Use(	UCHAR* puc, long lSize, USHORT usType, UCHAR ucFlags, 
						long lTime);
		// Static entry point for above.
		static short UseStatic(	UCHAR* puc, long lSize, USHORT usType, 
										UCHAR ucFlags, long lTime, long l_pRtVidc);

	protected:	// Internal typedefs.

	public:		// Members.
		

	protected:	// Members.
		VIDC_RT_HDR	m_avidchdrs[MAX_VID_CHANNELS];// Info for each channel.
		USHORT		m_usState;					// The current state of this CRtVidc.
		CDispatch*	m_pdispatch;				// The dispatcher for this CRtVidc.

	};


#endif // RTVIDC_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
