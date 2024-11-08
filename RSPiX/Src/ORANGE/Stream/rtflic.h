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
#ifndef RTFLIC_H
#define RTFLIC_H

//////////////////////////////////////////////////////////////////////////////
// Headers.
//////////////////////////////////////////////////////////////////////////////
#include "dispatch.h"
#include "Image.h"
					  
//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////
// This value can be from 0 to 64K - 1.
// The overhead per channel is sizeof(FLX_RT_HDR) bytes.
#define MAX_VID_CHANNELS	25

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
// Forward declare classes for RTFLIC_CALL definition.
class CRtFlic;

class FLX_RT_HDR;

typedef FLX_RT_HDR* PFLX_RT_HDR;

typedef void (*RTFLIC_CALL)(CRtFlic* prtflic, PFLX_RT_HDR pflxhdr);

class FLX_RT_HDR
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
	
		// Header info for our use.
		CImage*		pImage;				// Where to blt.
		int16_t			sCurFrame;			// Current frame number (0 origin).
		int16_t			sPixelsModified;	// TRUE if pixels were modified last
												// decompression.
		int16_t			sColorsModified;	// TRUE if colors were modified last
												// decompression.
		int32_t			lMaxLag;				// Maximum lag before skipping frames.
		RTFLIC_CALL	callbackHeader;	// Callback on header receipt.
		RTFLIC_CALL	callbackBefore;	// Callback before decompression.
		RTFLIC_CALL	callbackAfter;		// Callback after decompression.
	};

class CRtFlic
	{
	public:		// Construction/Destruction.
		// Default constructor.
		CRtFlic();
		// Destructor.
		~CRtFlic();

	public:		// Methods.
		// Sets the dispatcher.
		void SetDispatcher(CDispatch* pdispatch);

		// Sets callback(s) called on channel header receipt.
		void SetCallbackHeader(RTFLIC_CALL callback);
		void SetCallbackHeader(RTFLIC_CALL callback, int16_t sChannel);
		// Sets callback(s) called before decompression.
		void SetCallbackBefore(RTFLIC_CALL callback);
		void SetCallbackBefore(RTFLIC_CALL callback, int16_t sChannel);
		// Sets callback(s) called after decompression.
		void SetCallbackAfter(RTFLIC_CALL callback);
		void SetCallbackAfter(RTFLIC_CALL callback, int16_t sChannel);

	protected:	// Internal methods.
		// Sets variables w/o regard to current values.
		void Set();
		// Resets variables.  Performs deallocation if necessary.
		void Reset();

		// Use handler for RtFlic buffers.
		// Returns RET_FREE if done with data on return, RET_DONTFREE otherwise.
		int16_t Use(	uint8_t* puc, int32_t lSize, uint16_t usType, uint8_t ucFlags, 
						int32_t lTime);
		// Static entry point for above.
		static int16_t UseStatic(	uint8_t* puc, int32_t lSize, uint16_t usType, 
										uint8_t ucFlags, int32_t lTime, int32_t l_pRtFlic);

	protected:	// Internal typedefs.

	public:		// Members.
		

	protected:	// Members.
		FLX_RT_HDR	m_aflxhdrs[MAX_VID_CHANNELS];// Info for each channel.
		uint16_t		m_usState;					// The current state of this CRtFlic.
		CDispatch*	m_pdispatch;				// The dispatcher for this CRtFlic.

	};


#endif // RTFLIC_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
