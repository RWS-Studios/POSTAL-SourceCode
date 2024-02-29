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
///////////////////////////////////////////////////////////////////////////////
//
// ANIMSPRT.CPP
//
// History:
//		01/23/95 BH		Constructor and Desctuctor
//		01/24/95 BH		Skeleton code for the member functions
//		01/06/96 BH		Changed the animation format to work with
//							the new versions of CSprite and CImage.
//		04/16/96 BH		Changed the NextFrame and Previous frame to check
//							the m_sLoopToFrame value rather than checking the
//							animation flags.
//		10/31/96 BH		Changed the names from CAnimSprite to RAnimSprite
//							for the new RSPiX naming convention.  Also filled
//							in missing comments.
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							ENDIAN_LITTLE	RFile::LittleEndian
//
///////////////////////////////////////////////////////////////////////////////
//
// This module implements the RAnimSprite class which is a derived class of
// RSprite.
//
///////////////////////////////////////////////////////////////////////////////

#include "ANIMSPRT.H"

///////////////////////////////////////////////////////////////////////////////
// 
// Default Constructor
//
///////////////////////////////////////////////////////////////////////////////

RAnimSprite::RAnimSprite()	
	: RSprite()
{
	m_sVersion = ANIMSPRITE_CURRENT_VERSION;
	m_sNumFrames = 0;
	m_sNumPictures = 0;
	m_sLoopToFrame = -1;
	m_lTimer = 0;
	m_sCurrFrame = -1;
	m_ulAnimFlags = 0;
	m_aFrames = NULL;
	m_apPictures = NULL;
	m_sAllocatedPics = 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////////////////////////

RAnimSprite::~RAnimSprite()
{
	FreeFrames();
	FreePictures();
}


///////////////////////////////////////////////////////////////////////////////
//
// Load
//
// Description:
//		Load an animation from a .ani file with the given name
//
// Parameters:
//		pszFilename = name of animation file to load from
//
// Returns:
//		SUCCESS if file was loaded correctly
//		ERROR otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::Load(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "rb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RAnimSprite::Load - could not open file %s for input\n", pszFilename);
		return FAILURE;
	}

	sReturn = Load(&cf);

	cf.Close();

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// Load
//
// Description:
//		Load an animation from the current position in an open RFile
//
// Parameters:
//		pcf = pointer to an open RFile
//
// Returns:
//		SUCCESS if the file was loaded correctly
//		ERROR otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::Load(RFile* pcf)
{
	short sReturn = SUCCESS;
	ULONG ulFileType;

	if (pcf && pcf->IsOpen())
	{
		if (pcf->Read(&ulFileType) == 1)
		{
			if (ulFileType == ANIMSPRITE_COOKIE)
			{
				if (pcf->Read(&m_sVersion) == 1)
				{
					if (m_sVersion == ANIMSPRITE_CURRENT_VERSION)
					{
						if (pcf->Read(&m_sNumFrames) == 1)
						{
							if (pcf->Read(&m_sNumPictures) == 1)
							{
								if (pcf->Read(&m_sLoopToFrame) == 1)
								{
									if (pcf->Read(&m_ulAnimFlags) == 1)
									{
										if (ReadPictures(pcf) == SUCCESS)
										{
											if (ReadFrames(pcf) != SUCCESS)
											{
												TRACE("RAnimSprite::Load - Error reading frames\n");
												sReturn = FAILURE;
											}
										}
										else
										{
											TRACE("RAnimSprite::Load - Error reading pictures\n");
											sReturn = FAILURE;
										}
									}
									else
									{
										TRACE("RAnimSprite::Load - Error reading AnimFlags\n");
										sReturn = FAILURE;
									}
								}
								else
								{
									TRACE("RAnimSprite::Load - Error reading LoopToFrame\n");
									sReturn = FAILURE;
								}
							}
							else
							{
								TRACE("RAnimSprite::Load - Error reading number of pictures\n");
								sReturn = FAILURE;
							}
						}
						else
						{
							TRACE("RAnimSprite::Load - Error reading number of frames\n");
							sReturn = FAILURE;
						}
					}	
					else
					{
						TRACE("RAnimSprite::Load - The file's version does not match the current version\n");
						sReturn = FAILURE;
					}
				}
				else
				{
					TRACE("RAnimSprite::Load - Error reading file version number\n");
					sReturn = FAILURE;
				}
			}
			else
			{
				TRACE("RAnimSprite::Load - Wrong filetype, animations should start with 'ANIM'\n");
				sReturn = FAILURE;
			}
		}
		else
		{
			TRACE("RAnimSprite::Load - Error reading file type marker\n");
			sReturn = FAILURE;
		}
	}
	else
	{
		TRACE("RAnimSprite::Load - The RFile* pcf does not refer to an open file\n");
		sReturn = FAILURE;
	}
	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// ReadPictures:
//
// Description:
//		Function called by Load to read the pictures portion of the animation
//		file.
//
// Parameters:
//		pointer to open RFile at the position for the picture data
//
// Returns:
//		SUCCESS if pictures were read correctly
//		FAILURE otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::ReadPictures(RFile* pcf)
{
	short sReturn = SUCCESS;
	short i = 0;

	AllocatePictures(m_sNumPictures);

	while(sReturn == SUCCESS && i < m_sNumPictures)
		sReturn = m_apPictures[i++]->Load(pcf);

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// ReadFrames
//
// Description:
//		Function called by Load to read the frame portion of the animation
//		file.
//
// Parameters:
//		pointer to open RFile at the position for the frame data
//
// Returns:
//		SUCCESS if the frame data was read correctly
//		FAILURE otherwise - TRACE messages will describe the failure
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::ReadFrames(RFile* pcf)
{
	short sReturn = SUCCESS;
	short i = 0;

	AllocateFrames(m_sNumFrames);

	while (sReturn == SUCCESS && i < m_sNumFrames)
	{
		if (pcf->Read(&(m_aFrames[i].sOffsetX)) == 1)
		{
			if (pcf->Read(&(m_aFrames[i].sOffsetY)) == 1)
			{
				if (pcf->Read(&(m_aFrames[i].sOffsetZ)) == 1)
				{
					if (pcf->Read(&(m_aFrames[i].sRotDeg)) == 1)
					{
						if (pcf->Read(&(m_aFrames[i].lScaleWidth)) == 1)
						{
							if (pcf->Read(&(m_aFrames[i].lScaleHeight)) == 1)
							{
								if (pcf->Read(&(m_aFrames[i].sHold)) == 1)
								{
									if (pcf->Read(&(m_aFrames[i].sPicIndex)) == 1)
									{
										m_aFrames[i].pImage = m_apPictures[m_aFrames[i].sPicIndex];
										i++;
									}
									else
									{
										TRACE("RAnimSprite::ReadFrames - Error reading frame %d picture index\n", i);
										sReturn = FAILURE;
									}
								}
								else
								{
									TRACE("RAnimSprite::ReadFrames - Error reading frame %d sHold\n", i);
									sReturn = FAILURE;
								}
							}
							else
							{
								TRACE("RAnimSprite::ReadFrames - Error reading frame %d scale height\n", i);
								sReturn = FAILURE;
							}
						}
						else
						{
							TRACE("RAnimSprite::ReadFrames - Error reading frame %d scale width\n", i);
							sReturn = FAILURE;
						}
					}
					else
					{
						TRACE("RAnimSprite::ReadFrames - Error reading frame %d rotational degrees\n", i);
						sReturn = FAILURE;
					}
				}
				else
				{
					TRACE("RAnimSprite::ReadFrames - Error reading frame %d Z offset\n", i);
					sReturn = FAILURE;
				}
			}
			else
			{
				TRACE("RAnimSprite::ReadFrames - Error reading frame %d Y offset\n", i);
				sReturn = FAILURE;
			}
		}
		else
		{
			TRACE("RAnimSprite::ReadFrames - Error reading frame %d X offset\n", i);
			sReturn = FAILURE;
		}
	}

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// Save
//
// Description:
//		Save an animation to a .ani file with the given name
//
// Parameters:
//		pszFilename = the filename you wish to save
//
// Returns:
//		SUCCESS if the file was saved correctly
//		ERROR otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::Save(char* pszFilename)
{	
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "wb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RAnimSprite::Save - could not open file for output %s\n", pszFilename);
		return FAILURE;
	}

	sReturn = Save(&cf);
	
	cf.Close();

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// Save
//
// Description:
//		Save an animation to the current location in an open RFile.
//		This version may be useful if you choose to save a set of animations
//		in one file.
//
// Parameters:
//		pcf = pointer to open RFile
//
// Returns:
//		SUCCESS if the file was saved correctly
//		ERROR otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::Save(RFile* pcf)
{
	short sReturn = SUCCESS;
	ULONG ulFileType = ANIMSPRITE_COOKIE;

	if (pcf && pcf->IsOpen())
	{
		if (pcf->Write(&ulFileType) == 1)
		{
			if (pcf->Write(&m_sVersion) == 1)
			{
				if (pcf->Write(&m_sNumFrames) == 1)
				{
					if (pcf->Write(&m_sNumPictures) == 1)
					{
						if (pcf->Write(&m_sLoopToFrame) == 1)
						{
							if (pcf->Write(&m_ulAnimFlags) == 1)
							{
								if (WritePictures(pcf) == SUCCESS)
									sReturn = WriteFrames(pcf);
								else
								{
									TRACE("RAnimSprite::Save - Error saving pictures\n");
									sReturn = FAILURE;
								}
							}
							else
							{
								TRACE("RAnimSprite::Save - Error writing animation flags\n");
								sReturn = FAILURE;
							}
						}
						else
						{
							TRACE("RAnimSprite::Save - Error writing loop flag\n");
							sReturn = FAILURE;
						}
					}
					else
					{
						TRACE("RAnimSprite::Save - Error writing number of pictures\n");
						sReturn = FAILURE;
					}
				}
				else
				{
					TRACE("RAnimSprite::Save - Error writing number of frames\n");
					sReturn = FAILURE;
				}
			}
			else
			{
				TRACE("RAnimSprite::Save - Error writing animation version number\n");
				sReturn = FAILURE;
			}	
		}
		else
		{
			TRACE("RAnimSprite::Save - Error writing animation filetype marker\n");
			sReturn = FAILURE;		
		}		
	}
	else
	{
		TRACE("RAnimSprite::Save - RFile* pcf does not refer to an open file\n");
		sReturn = FAILURE;
	}

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// WritePictures
//
// Description:
//		Function called by Save to write the pictures portion of the animation
//		file.
//
// Parameters:
//		pointer to open RFile at the point where the pictures should be written
//
// Returns:
//		SUCCESS if the pictures were written
//		FAILURE otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::WritePictures(RFile* pcf)
{
	short sReturn = SUCCESS;
	short i = 0;

	while (sReturn == SUCCESS && i < m_sNumPictures)
		sReturn = m_apPictures[i++]->Save(pcf);

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// WriteFrames
//
// Description:
//		Function called by Save to write the frames portion of the animation
//		file.
//
// Parameters:
//		pointer to open RFile at the point where the frames should be written
//
// Returns:
//		SUCCESS if the frames were written
//		FAILURE otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::WriteFrames(RFile* pcf)
{
	short sReturn = SUCCESS;
	short i = 0;

	while (sReturn == SUCCESS && i < m_sNumFrames)
	{
		if (pcf->Write(&(m_aFrames[i].sOffsetX)) == 1)
		{
			if (pcf->Write(&(m_aFrames[i].sOffsetY)) == 1)
			{
				if (pcf->Write(&(m_aFrames[i].sOffsetZ)) == 1)
				{
					if (pcf->Write(&(m_aFrames[i].sRotDeg)) == 1)
					{
						if (pcf->Write(&(m_aFrames[i].lScaleWidth)) == 1)
						{
							if (pcf->Write(&(m_aFrames[i].lScaleHeight)) == 1)
							{
								if (pcf->Write(&(m_aFrames[i].sHold)) == 1)
								{
									if (pcf->Write(&(m_aFrames[i].sPicIndex)) == 1)
										i++;
									else
									{
										TRACE("RAnimSprite::WriteFrame - Error writing frame %d sPicIndex\n", i);
										sReturn = FAILURE;
									}
								}
								else
								{
									TRACE("RAnimSprite::WriteFrame - Error writing frame %d sHold\n", i);
									sReturn = FAILURE;
								}
							}
							else
							{
								TRACE("RAnimSprite::WriteFrames - Error writing frame %d scaled height\n", i);
								sReturn = FAILURE;
							}
						}
						else
						{
							TRACE("RAnimSprite::WriteFrames - Error writing frame %d scaled width\n", i);
							sReturn = FAILURE;
						}
					}
					else
					{
						TRACE("RAnimSprite::WriteFrames - Error writing frame %d rotational degrees\n", i);
						sReturn = FAILURE;
					}
				}	
				else
				{
					TRACE("RAnimSprite::WriteFrame - Error writing frame %d Z offset\n", i);
					sReturn = FAILURE;
				}
			}
			else
			{
				TRACE("RAnimSprite::WriteFrames - Error writing frame %d Y offset\n", i);
				sReturn = FAILURE;
			}
		}
		else
		{
			TRACE("RAnimSprite::WriteFrames - Error writing frame %d X offset\n", i);
			sReturn = FAILURE;
		}
	}

	return sReturn;
}

///////////////////////////////////////////////////////////////////////////////
//
// SetFrame
//
// Description:
//		Set to specified frame in current animation.
//
//	Parameters
//		sFrameNum = Frame number (0 to n-1)
//
// Return:
//		none
//
// Notes:
//		If frame is too high then ending frame is used
//		A sequential search for specified frame is required
//		If ANIMSPRITE_FLAGS_NOSKIP flag is set, nearest key-frame is used
//		(nearest before, nearest after, or just nearest?)
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::SetFrame(short sFrameNum)
{
 	if (sFrameNum < 0 || sFrameNum >= m_sNumFrames)
	{
		TRACE("RAnimSprite::SetFrame - frame number supplied is not in range 0-%d so using last frame\n", m_sNumFrames);
		sFrameNum = m_sNumFrames-1;
	}

	// Change current frame
	m_sCurrFrame = sFrameNum;

	// Set the expiration time for this frame
	m_lTimer = rspGetMilliseconds() + m_aFrames[m_sCurrFrame].sHold;

	// Set the sprite's image pointer to the picture for this frame
	m_pImage = m_aFrames[m_sCurrFrame].pImage;

	// Check the animation flags to see if the sprite should be modified
	// with various frame data.
	if (m_ulAnimFlags & ANIMSPRITE_FLAG_OFFSET)
	{
		m_sX += m_aFrames[m_sCurrFrame].sOffsetX;
		m_sY += m_aFrames[m_sCurrFrame].sOffsetY;
		m_sZ += m_aFrames[m_sCurrFrame].sOffsetZ;
	}

	if (m_ulAnimFlags & ANIMSPRITE_FLAG_ROTATION)
		m_sAngle = m_aFrames[m_sCurrFrame].sRotDeg;

	if (m_ulAnimFlags & ANIMSPRITE_FLAG_SCALE)
	{
		m_lWidth = m_aFrames[m_sCurrFrame].lScaleWidth;
		m_lHeight = m_aFrames[m_sCurrFrame].lScaleHeight;
	}	

	return SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//
// NextFrame
//
// Description:
//		Go to the next frame of the animation immediately (not using the 
//		timer).  If it is on the last frame and the ANIMSPRITE_FLAG_LOOP
//		is set, then it will loop back to the m_sLoopToFrame number.  If
//		the flag is not set then it will stay on the last frame.
//
//	Parameters:
//		none
//
//	Return:
//		frame number it is on
//		ANIMSPRITE_LAST_FRAME if it is stuck on the last frame (not looping)
//	
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::NextFrame()
{
	// See if it is on the last frame
	if (m_sCurrFrame == m_sNumFrames-1)
	{
//		if (m_ulAnimFlags & ANIMSPRITE_FLAG_LOOP)
		if (m_sLoopToFrame > -1 && m_sLoopToFrame < m_sNumFrames)
			if (m_sLoopToFrame >= 0 && m_sLoopToFrame < m_sNumFrames)
				m_sCurrFrame = m_sLoopToFrame;
			else
			{
				TRACE("RAnimSprite::NextFrame - Anim is supposed to loop but m_sLoopToFrame is out of range 0-%d\n", m_sLoopToFrame);
				return ANIMSPRITE_LAST_FRAME;
			}
		else
			return ANIMSPRITE_LAST_FRAME;
	}
	// If not on the last frame then increment to the next frame
	else
		m_sCurrFrame++;

	// Set the expiration time for this frame
	m_lTimer = rspGetMilliseconds() + m_aFrames[m_sCurrFrame].sHold;

	// Set the sprite's image pointer to the picture for this frame
	m_pImage = m_aFrames[m_sCurrFrame].pImage;

	// Check the animation flags to see if the sprite should be modified
	// with various frame data.
	if (m_ulAnimFlags & ANIMSPRITE_FLAG_OFFSET)
	{
		m_sX += m_aFrames[m_sCurrFrame].sOffsetX;
		m_sY += m_aFrames[m_sCurrFrame].sOffsetY;
		m_sZ += m_aFrames[m_sCurrFrame].sOffsetZ;
	}

	if (m_ulAnimFlags & ANIMSPRITE_FLAG_ROTATION)
		m_sAngle = m_aFrames[m_sCurrFrame].sRotDeg;

	if (m_ulAnimFlags & ANIMSPRITE_FLAG_SCALE)
	{
		m_lWidth = m_aFrames[m_sCurrFrame].lScaleWidth;
		m_lHeight = m_aFrames[m_sCurrFrame].lScaleHeight;
	}	

	return m_sCurrFrame;
}

///////////////////////////////////////////////////////////////////////////////
//
// PrevFrame
//
// Description:
//		Go to the previous frame, and if it is already on the first frame
//		it will loop to the sLoopToFrame if ANIMSPRITE_FLAG_LOOP
//
//	Parameters:
//		none
//
// Returns:
//		current frame number or ANIMSPRITE_FIRST_FRAME if looping is not
//		on and it is stuck on the first frame.
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::PrevFrame()
{
	// See if it is on the first frame
	if (m_sCurrFrame == 0)
	{
//		if (m_ulAnimFlags & ANIMSPRITE_FLAG_LOOP)
		if (m_sLoopToFrame > -1 && m_sLoopToFrame < m_sNumFrames)
			if (m_sLoopToFrame >= 0 && m_sLoopToFrame < m_sNumFrames)
				m_sCurrFrame = m_sLoopToFrame;
			else
			{
				TRACE("RAnimSprite::PrevFrame - Anim is supposed to loop but m_sLoopToFrame is out of range 0-%d\n", m_sLoopToFrame);
				return ANIMSPRITE_FIRST_FRAME;
			}
		else
			return ANIMSPRITE_FIRST_FRAME;
	}
	// If not on the last frame then increment to the next frame
	else
		m_sCurrFrame--;

	// Set the expiration time for this frame
	m_lTimer = rspGetMilliseconds() + m_aFrames[m_sCurrFrame].sHold;

	// Set the sprite's image pointer to the picture for this frame
	m_pImage = m_aFrames[m_sCurrFrame].pImage;

	// Check the animation flags to see if the sprite should be modified
	// with various frame data.
	if (m_ulAnimFlags & ANIMSPRITE_FLAG_OFFSET)
	{
		m_sX += m_aFrames[m_sCurrFrame].sOffsetX;
		m_sY += m_aFrames[m_sCurrFrame].sOffsetY;
		m_sZ += m_aFrames[m_sCurrFrame].sOffsetZ;
	}

	if (m_ulAnimFlags & ANIMSPRITE_FLAG_ROTATION)
		m_sAngle = m_aFrames[m_sCurrFrame].sRotDeg;

	if (m_ulAnimFlags & ANIMSPRITE_FLAG_SCALE)
	{
		m_lWidth = m_aFrames[m_sCurrFrame].lScaleWidth;
		m_lHeight = m_aFrames[m_sCurrFrame].lScaleHeight;
	}	

	return m_sCurrFrame;
}

///////////////////////////////////////////////////////////////////////////////
//
// NextKeyFrame
//
// Description:
//		Go to next key frame of the animation.
//
//	Parameters:
//		none
//
// Return:
//		current frame number
//
///////////////////////////////////////////////////////////////////////////////


short RAnimSprite::NextKeyFrame()		// goes to next key frame of animation
{
	return SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//
// Animate
//
// Description:
//		Go to the next frame of the animation if the time for the current
//		frame has expired.  It will loop to the m_sLoopToFrame if the 
//		ANIMSPRITE_FLAG_LOOP is set.
//
// Parameters:
//		none
//
// Returns:
//		ANIMSPRITE_WAITING if it is not time yet
//		current frame number or ANIMSPRITE_LAST_FRAME (see next frame)
//
///////////////////////////////////////////////////////////////////////////////
	
short RAnimSprite::Animate()
{
	if (rspGetMilliseconds() > m_lTimer)
		return NextFrame();
	else
		return ANIMSPRITE_WAITING;
}

///////////////////////////////////////////////////////////////////////////////
//
// AllocateFrames
//
// Description:
//		Allocates the given number of frames for an animation
//
// Parameters:
//		sNumFrames = number of frames to be allocated
//
// Returns:
//		SUCCESS if the memory was allocated for the frames
//		FAILURE otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::AllocateFrames(short sNumFrames)
{
	if (m_aFrames)
	{
		TRACE("RAnimSprite::AllocateFrames - Frames are already allocated, free these before allocating more\n");
		return FAILURE;
	}

	m_aFrames = new FRAME[sNumFrames];
	if (m_aFrames)
		return SUCCESS;
	else
		return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////
//
// AllocatePictures
//
// Description:
//		Allocates an array of RImage pointers of the given size
//
// Parameters:
//		sNumPicutures = number of picture pointers to allocate
//
// Returns:
//		SUCCESS if the picture pointers were allocated
//		FAILURE otherwise
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::AllocatePictures(short sNumPictures)
{
	if (m_apPictures)
	{
		TRACE("RAnimSprite::AllocatePictures - Pictures are already allocated, free these before allocating more\n");
		return FAILURE;
	}

	m_apPictures = new RImage*[sNumPictures];
	if (m_apPictures == NULL)
		return FAILURE;
	
	short i;

	for (i = 0; i < sNumPictures; i++)
		m_apPictures[i] = new RImage;

	// Save this so that this many will be freed later
	m_sAllocatedPics = sNumPictures;

	return SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//
// FreeFrames
//
// Description:
//		Frees the frames that were allocated for this animation
//
// Parameters:
//		none
//
// Returns:
//		SUCCESS if the pointer was valid
//		FAILURE if the frame pointer was NULL
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::FreeFrames()
{
	if (m_aFrames)
	{
		delete []m_aFrames;
		m_aFrames = NULL;
		return SUCCESS;
	}
	else	
		return FAILURE;
}

///////////////////////////////////////////////////////////////////////////////
//
// FreePictures
//
// Description:
//		Frees the Images of each picture and then frees the array of picture
//		pointers.
//
// Parameters:
//		none
//
// Returns:
//		SUCCESS if there were allocated pictures to free
//		FAILURE if there were no pictures
//
///////////////////////////////////////////////////////////////////////////////

short RAnimSprite::FreePictures()
{
	if (m_apPictures)
	{
		short i;
		// free each picture
		for (i = 0; i < m_sAllocatedPics; i++)
			delete m_apPictures[i];
		// free array of image pointers
		delete []m_apPictures;
		m_sAllocatedPics = 0;
		m_apPictures = NULL;
		return SUCCESS;
	}
	else
		return FAILURE;
}


//*****************************************************************************
// EOF
//*****************************************************************************
