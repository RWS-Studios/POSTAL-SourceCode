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
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Green Video Module
//
// History:
//
//		05/17/95 PPL	Started.
//		05/18/95 PPL	Complete rewrite using the MCIWnd Class instead of the thousand parameters
//							infested mciSendCommand function.
//		05/22/95 PPL	Implemented status checking
//		05/23/95 PPL	Commented out all the traces within the status checking function for speed.
//		08/10/95 JMI	Altered IsPaused, IsStopped, IsPlaying, Destroy, Close, Pause, & Resume to be aware of
//							possibility of HWND not being valid.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Blue.h"
#include "bluewin.h"

#include <winuser.h>
#include <vfw.h>

#include "video.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// External global variables.
////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Module specific constants
////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Module specific variables
////////////////////////////////////////////////////////////////////////////////////////////////////////////
char 				ms_szBuf[80];			// Can be used anytime we need a char buffer.


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Module specific functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Module specific functions:
////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions:
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				CVideo
// Description:	This is the default constructor for the class.  The responsibility of this function will
//						be to initialize any member and/or module static variables.
// Input:			none
// Output:			none
// History:			05/17/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVideo::CVideo(void)
	{
	// Initialize member variables.
	Init();
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				CVideo
// Description:	This is an overloaded constructor.  The responsibility of this function will
//						be to initialize any member and/or module static variables.  Additionaly, the video
//						playback device will be opened (created).
// Input:			See below for description
// Output:			none
// History:			05/17/95, PPL,	Started coding.
//						05/18/95, PPL, Rewrote using the MCIWnd Class.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVideo::CVideo(char* szFilename,						// filename of the video
					long x, 									// left edge of playback window
					long y, 									// top of the playback window
					long w, 									// width of the playback window
					long h, 									// height of the playback window
					short sClip) 							// clipping flag
	{
	// Call the default constructor.
	CVideo();

	// Create the playback window and allocate the digital video device.
	if (Create() == 0)
		{
		// Call the create function with the given parameters.
		if (Open(szFilename, x, y, w, h, sClip) == 0)
			{
			// Now that we've successfully created and opened the video file, let's play it.
			if (Play() != 0)
				{
				TRACE("Failed to play the current video.\n");
				}
			}
		else
			{
			TRACE("Failed to open the requested video file within the overloaded construct!\n");
			}
		}
	else
		{
		TRACE("Failed to auto create video device within the overload constructor!\n");
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				~CVideo
// Description:	This is the default destructor for the class.  The responsibility of this function will
//						be to release the current device, if opened.
// Input:			none
// Output:			none
// History:			05/17/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVideo::~CVideo(void)
	{
	// Destroy an device currently opened.
	Destroy();
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Create
// Description:	This function creates a window and opens(allocates) the digital video device.
//	Input:			none.
// Output:			0 if successful, error # otherwise.
// History:			05/17/95, PPL,	Started coding.
//						05/18/95, PPL, Rewrote using the MCIWnd Class.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::Create(void)
	{
	// Let's create the avi video window if one has not already been created.
	if (m_lhwnd == 0)
		{
		m_lhwnd = (long)MCIWndCreate(gsi.hWnd, 
											  gsi.hInstance,
											  (DWORD)(WS_CHILD |
//											  			 WS_EX_TOPMOST |
//														 WS_THICKFRAME |
//											  			 WS_CAPTION |
//											  			 WS_BORDER | 
//											  			 MCIWNDF_NOAUTOSIZEMOVIE |
//											  			 MCIWNDF_NOAUTOSIZEWINDOW |
//											  			 MCIWNDF_SHOWALL |
											  			 MCIWNDF_NOPLAYBAR | 
											  			 MCIWNDF_NOOPEN | 
											  			 MCIWNDF_NOMENU | 
											  			 MCIWNDF_NOERRORDLG),
											  "avivideo");
		if (TraceMCIError("CVideo::Create(MCIWndCreate)") == 0)
			{
			// Let's subclass the window proc for m_lhwnd.
			// First, get and save the window's original proc.
//			m_lhwndProc = GetWindowLong((HWND)m_lhwnd, GWL_WNDPROC);

			// Now, let's set the window's proc to our's.
//			if (SetWindowLong((HWND)m_lhwnd, GWL_WNDPROC, (long)&(this->OurMCIWndProc)) == 0)
//				{
//				TRACE("Failed to subclass the MCI window's proc to our's.");
//				}

			return 0;
			}
		else
			return VIDEO_ERR_CREATE;
		}
	else
		return VIDEO_ERR_ALREADY_CREATED;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Destroy
// Description:	This function destroys(releases) the current digital video device and the child window.
//	Input:			none.
// Output:			none.
// History:			05/17/95, PPL,	Started coding.
//						05/18/95, PPL, Rewrote using the MCIWnd class.
//						08/10/95, JMI, Altered to be aware of possibility of HWND not being valid.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideo::Destroy(void)
	{
	Close();

	// If a window has been created . . .
	if (m_lhwnd != NULL)
		{
		MCIWndDestroy((HWND)m_lhwnd);
		}

	// Initialize all the member variables.
	Init();
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				CVideo
// Description:	This function will open a video file.
// Input:			See below for description
// Output:			0 - Success.
//						1 - Error.  (more detailed error codes may be defined later.)
// History:			05/17/95, PPL,	Started coding.
//						05/18/95, PPL, Rewrote using MCIWnd Class.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::Open(char* szFilename,						// name of the video file
						 long x,									// left edge of the playback window
						 long y,									// top of the playback window
						 long w,									// width of the playback window
						 long h,									// height of the playback window
						 short sClip)								// clipping flag, use following values:
				 														//    0: no clipping, window will be resized to 
																		//			actual video's size.
																		//		1:	video will be clipped to the size supplied.
																		//		2:	video will not be clipped, video will be
																		//			stretched/shrinked to the window's size.
	{
	short		sResult;
	RECT		rect;
	long 		lXdelta;
	long		lYdelta;

	sResult = VIDEO_SUCCESS;		// initially no error.

	// If no mci window has been created yet, then we need to create one.
	if (m_lhwnd == 0)
		{
		sResult = Create();
		}
	
	// Let's issue the actual open command, now that it's safe.
	if (sResult == VIDEO_SUCCESS)
		{
		// Let's open the file first before moving the window to the desired location.
		if (MCIWndOpen((HWND)m_lhwnd, szFilename, 0) == 0)
			{
			// Now that a video file has been properly opened, let's move it and resize if necessary.
			// Let's get the current video's rect.
			if (MCIWndGetSource((HWND)m_lhwnd, &rect) == 0)
				{
				// Let's just adjust the location of the window. (find the delta from current)
				lXdelta = x - rect.left;
				lYdelta = y - rect.top;
				rect.left = rect.left + lXdelta;
				rect.top = rect.top + lYdelta;
				rect.right = rect.right + lXdelta;
				rect.bottom = rect.bottom + lYdelta;

				// Depending on the mode requested, we might have to adjust the width and height.
				switch (sClip)
					{
					case MODE_STRETCH:
						rect.right = rect.left + w;
						rect.bottom = rect.top + h;

					case MODE_NORMAL:
						// Make sure that autowindowsize and automoviesize is turned on.
						MCIWndChangeStyles((HWND)m_lhwnd, 
												 MCIWNDF_NOAUTOSIZEWINDOW | MCIWNDF_NOAUTOSIZEMOVIE,
												 0);

						if (!SetWindowPos((HWND)m_lhwnd,
												(HWND)NULL,
												rect.left,
												rect.top,
												rect.right - rect.left,
												rect.bottom - rect.top,
												SWP_NOACTIVATE|SWP_NOZORDER))
							{
							TRACE("Failed to move and size window using SetWindowPos()!\n");
							sResult = VIDEO_ERR_RESIZE_FAIL;
							}
						break;

					case MODE_CLIP:
						// We need to try and clip.
						// First, make sure that autowindowsize and automoviesize is turned on.
						// We'll set the window to the default movie's size.
						MCIWndChangeStyles((HWND)m_lhwnd, 
												 MCIWNDF_NOAUTOSIZEWINDOW | MCIWNDF_NOAUTOSIZEMOVIE,
												 0);

						if (SetWindowPos((HWND)m_lhwnd,
											  (HWND)NULL,
											  rect.left,
											  rect.top,
											  rect.right - rect.left,
											  rect.bottom - rect.top,
											  SWP_NOACTIVATE))
							{
							// Now, let's turn of the autowindowsize and antomoviesize and then
							// resize to the actual size passed.
							MCIWndChangeStyles((HWND)m_lhwnd,
													 MCIWNDF_NOAUTOSIZEWINDOW | MCIWNDF_NOAUTOSIZEMOVIE,
													 MCIWNDF_NOAUTOSIZEWINDOW | MCIWNDF_NOAUTOSIZEMOVIE);
							if (!SetWindowPos((HWND)m_lhwnd,
													(HWND)NULL,
													rect.left,
													rect.top,
													w,
													h,
													SWP_NOACTIVATE))
								{
								TRACE("Failed to clip the window!\n");
								sResult = VIDEO_ERR_RESIZE_FAIL;
								}
							}
						else
							{
							TRACE("Failed to move and size window using SetWindowPos()!\n");
							sResult = VIDEO_ERR_RESIZE_FAIL;
							}
						break;
					}
				}
			else
				{
				TraceMCIError("CVideo::Open(MCIWndGetDest)");
				sResult = VIDEO_ERR_GETTING_WRECT;
				}
			}
		else
			{
			TraceMCIError("CVideo::Open(MCIWndOpen)");
			sResult = VIDEO_ERR_OPEN;
			}
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Close
// Description:	This function simulates closing the video file.
// Input:			none
// Output:			0 - Success.
//						1 - Error.  (more detailed error codes may be defined later.)
// History:			05/17/95, PPL,	Started coding.
//						05/18/95, PPL, Rewrote using MCIWnd Class.
//						08/10/95, JMI, Altered to be aware of possibility of HWND not being valid.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::Close(void)
	{
	short	sRes = VIDEO_ERR_CLOSE;

	// If there is a window . . .
	if (m_lhwnd != NULL)
		{
		// No matter what happens here, let's hide the window.
		ShowWindow((HWND)m_lhwnd, SW_HIDE);

		if (MCIWndClose((HWND)m_lhwnd) == 0)
			{
			sRes = VIDEO_SUCCESS;
			}
		else
			{
			TraceMCIError("CVideo::Close(MCIWndClose)");
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Play
// Description:	This function plays the currently loaded video -- optionally a segment within.
// Input:			sMode: 	see constants in the header file
//						lStart: 	start of the selection
//						lEnd:		end of the selection
// Output:			0 - Success.
//						1 - Error.  (more detailed error codes may be defined later.)
// History:			05/17/95, PPL,	Started coding.
//						05/18/95, PPL, Rewrote using MCIWnd Class.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::Play(short sMode, long lStart, long lEnd)
	{
	long lResult;

	if (m_lhwnd)
		{
		// Let's just show the window.
		ShowWindow((HWND)m_lhwnd, SW_SHOW);

		switch (sMode)
			{
			case PLAY_ALL:
				lResult = MCIWndPlay((HWND)m_lhwnd);
				if (lResult != 0)
					TraceMCIError("CVideo::Play(MCIWndPlay)");
				break;

			case PLAY_SELECTION_TIME:
				MCIWndUseTime((HWND)m_lhwnd);
				lResult = MCIWndPlayFromTo((HWND)m_lhwnd, lStart, lEnd);
				if (lResult != 0)
					TraceMCIError("CVideo::PlayStartEnd<TIME>");
				break;

			case PLAY_SELECTION_FRAME:
				MCIWndUseFrames((HWND)m_lhwnd);
				lResult = MCIWndPlayFromTo((HWND)m_lhwnd, lStart, lEnd);
				if (lResult != 0)
					TraceMCIError("CVideo::PlayStartEnd<FRAME>");
				break;
			}
		}
	else
		lResult = 1;

	if (lResult != 0)
		return VIDEO_ERR_PLAY;
	else
		return VIDEO_SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Pause
// Description:	This function pauses the currently playing video.
// Input:			none
// Output:			0 - Success.
//						1 - Error.  (more detailed error codes may be defined later.)
// History:			05/17/95, PPL,	Started coding.
//						08/10/95, JMI, Altered to be aware of possibility of HWND not being valid.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::Pause(void)
	{
	short sRes = VIDEO_ERR_PAUSE;

	if (m_lhwnd != NULL)
		{
		if (MCIWndPause((HWND)m_lhwnd) == 0)
			{
			sRes = VIDEO_SUCCESS;
			}
		else
			{
			TraceMCIError("CVideo::Pause(MCIWndPause)");
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Resume
// Description:	This function resumes a previously paused video.
// Input:			none
// Output:			0 - Success.
//						1 - Error.  (more detailed error codes may be defined later.)
// History:			05/17/95, PPL,	Started coding.
//						08/10/95, JMI, Altered to be aware of possibility of HWND not being valid.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::Resume(void)
	{
	short sRes = VIDEO_ERR_RESUME;

	if (m_lhwnd != NULL)
		{
		if (MCIWndResume((HWND)m_lhwnd) == 0)
			{
			sRes = VIDEO_SUCCESS;
			}
		else
			{
			TraceMCIError("CVideo::Resume(MCIWndResume)");
			}
		}

	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Stop
// Description:	This function stops a video which may current be playing.
// Input:			none
// Output:			0 - Success.
//						1 - Error.  (more detailed error codes may be defined later.)
// History:			05/17/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::Stop(void)
	{
	long lResult;

	lResult = MCIWndStop((HWND)m_lhwnd);
	if (lResult != 0)
		{
		TraceMCIError("CVideo::Stop(MCIWndStop)");
		return VIDEO_ERR_STOP;
		}
	else
		return VIDEO_SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				GetFrame
// Description:	This function gets the current frame of a video.
// Input:			none
// Output:			long: current frame number.
// History:			05/17/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CVideo::GetFrame(void)
	{
	short sResult;

	sResult = VIDEO_SUCCESS;

	// First, let's change the format to use frames.
	if (MCIWndUseFrames((HWND)m_lhwnd) == 0)
		{
		return MCIWndGetPosition((HWND)m_lhwnd);
		}
	else
		{
		TraceMCIError("CVideo::GetFrame(MCIWndUseFrames)");
		sResult = VIDEO_ERR_SET_FRAMES;
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				GetPosition
// Description:	This function gets the current position(milliseconds) of a video.
// Input:			none
// Output:			long: current position
// History:			05/17/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CVideo::GetPosition(void)
	{
	short sResult;

	sResult = VIDEO_SUCCESS;

	// First, let's change the format to use frames.
	if (MCIWndUseTime((HWND)m_lhwnd) == 0)
		{
		return MCIWndGetPosition((HWND)m_lhwnd);
		}
	else
		{
		TraceMCIError("CVideo::GetFrame(MCIWndUseFrames)");
		sResult = VIDEO_ERR_SET_TIME;
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				IsOpened
// Description:	This function checks to see if a video is currently opened.
// Input:			none
// Output:			short: TRUE/FALSE
// History:			05/22/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::IsOpened(void)
	{
	long lResult;

	lResult = MCIWndGetMode((HWND)m_lhwnd, ms_szBuf, 80);

//	TRACE("Video status:  %s\n", ms_szBuf);
	TraceMCIError("CVideo::IsOpened(MCIWndGetMode)");

	if (lResult == MCI_MODE_OPEN)
		{
		// A video is currently opened.
		return TRUE;
		}
	else
		{
		// No video has been opened yet.
		return FALSE;
		}
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				IsPlaying
// Description:	This function checks to see if a video is currently playing.
// Input:			none
// Output:			short: TRUE/FALSE
// History:			05/22/95, PPL,	Started coding.
//						08/10/95, JMI, Altered to be aware of possibility of HWND not being valid.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::IsPlaying(void)
	{
	short sRes = FALSE;

	if (m_lhwnd != NULL)
		{
		if (MCIWndGetMode((HWND)m_lhwnd, ms_szBuf, 80) == MCI_MODE_PLAY)
			{
			sRes = TRUE;
			}
		//	TRACE("Video status:  %s\n", ms_szBuf);
		}
	//	TraceMCIError("CVideo::IsPlaying(MCIWndGetMode)");

	return sRes;
	}
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				IsPaused
// Description:	This function checks to see if a video is currently paused.
// Input:			none
// Output:			short: TRUE/FALSE
// History:			05/22/95, PPL,	Started coding.
//						08/10/95, JMI, Altered to be aware of possibility of HWND not being valid.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::IsPaused(void)
	{
	short	sRes	= FALSE;

	if (m_lhwnd != NULL)
		{
		if (MCIWndGetMode((HWND)m_lhwnd, ms_szBuf, 80) == MCI_MODE_PAUSE)
			{
			sRes = TRUE;
			}
		}

	//	TRACE("Video status:  %s\n", ms_szBuf);
	//	TraceMCIError("CVideo::IsPaused(MCIWndGetMode)");
	
	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				IsStopped
// Description:	This function checks to see if a video has been stopped.
// Input:			none
// Output:			short: TRUE/FALSE
// History:			05/22/95, PPL,	Started coding.
//						08/10/95, JMI, Altered to be aware of possibility of HWND not being valid.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
short CVideo::IsStopped(void)
	{
	short	sRes = FALSE;

	if (m_lhwnd != NULL)
		{
		if (MCIWndGetMode((HWND)m_lhwnd, ms_szBuf, 80) == MCI_MODE_STOP)
			{
			sRes = TRUE;
			}
		}

//	TRACE("Video status:  %s\n", ms_szBuf);
//	TraceMCIError("CVideo::IsStopped(MCIWndGetMode)");
	
	return sRes;
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions:
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				Init
// Description:	This is private function initializes all the member variables.
// Input:			none
// Output:			none
// History:			05/17/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CVideo::Init(void)
	{
	// To be consistent with the way all our functions return sucess, 0 is success (or TRUE) while all other
	// numbers denote error of some sort (or FALSE).  All of our 'boolean' shorts are therefore intialized
	// to 0.
	m_ulDeviceID = 0;
	m_lhwnd = 0;
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Name:				TraceMCIError
// Description:	This function simply put up a trace message containing the string description of 
//						the current MCIWndError
// Input:			szErrOrigin: a description for where the error might have originated.
// Output:			long: if 0, then error was not shown
//								else, the error number is returned.
// History:			05/17/95, PPL,	Started coding.
////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CVideo::TraceMCIError(char* szFunctionName)
	{
	long lResult;

	lResult = MCIWndGetError((HWND)m_lhwnd, ms_szBuf, 80);

	if (lResult != 0)
		{
		TRACE("The following MCIError occurred in %s:\n", szFunctionName);
		TRACE("%s\n", ms_szBuf);
		}

	return lResult;
	}

