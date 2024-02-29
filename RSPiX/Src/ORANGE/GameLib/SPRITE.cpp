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
//*****************************************************************************
//
// SPRITE.CPP
//
// History:
//		01/22/95	MR		Sample stuff. (IsZoneCollision)
//		01/23/95 BH		Constructor, Destructor, SetZ
//		12/08/95	BH		Changed sprite for use with Jeff's Blit routines.
//							Now that the CImage has been settled and because
//							there are many different Blit routines, the drawing
//							functions have been removed from sprite and instead
//							will be handled by another module in conjunction with
//							the blit routines.  A few variables were addes to sprites
//							to make them compatible with Jeff's sprite format.
//
//		10/08/96	JMI	Added CreateImage() and DestroyImage() to allocate and
//							deallocate the m_pImage.  Load() now calls CreateImage()
//							and ~CSprite() now calls DestroyImage().
//
//		10/09/96	JMI	Added Init() to initialize CSprites consistently no matter
//							which constructor is used.
//
//		10/09/96	JMI	Now passes FALSE to CSList constructor for SpriteList to
//							tell it not to initalize or automatically deallocate.
//
//		10/10/96	JMI	Added m_sHotSpotX/Y offsets, serialization for such, and
//							virtual Update() hook.
//
//		10/15/96 BRH	Added ability to save sprites without an Image.  If the
//							m_pImage pointer is NULL, then file is written without
//							an image.  This change required a new version of the
//							sprite file because a flag was added to specify whether
//							the sprite file contained a CImage or not.  Also changed
//							the reading and writing of doubles to work with the
//							updated CNFile which now supports floats and doubles.
//
//		10/31/96 BRH	Changed CSprite to RSprite to match RSPiX naming
//							convention.  Also changed references to other RSPiX
//							classes from the C prefix to the R prefix.
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							ENDIAN_LITTLE	RFile::LittleEndian
//
//*****************************************************************************
//
// This module implements the RSprite class.
//
//*****************************************************************************

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/GameLib/SPRITE.H"
#else
	#include "SPRITE.H"
#endif // PATHS_IN_INCLUDES

////////////////// Instantiate Static members ////////////////////////////////

RSList<RSprite, short>	RSprite::SpriteList(FALSE);	// Master list of active sprites


//*****************************************************************************
//
// Constructor
//
// Description:
//		Initializes the elements of this sprite and adds itself to the list
//		static list of sprites
//
// Parameters:
//		none
//
// Returns:
//		none
//
//*****************************************************************************

RSprite::RSprite()
{
	Init();
}

//*****************************************************************************
//
// Constructor
//
// Description:
//		This constructor initializes the sprite with the parameters you supply
//		and adds itself to the static list of sprites
//
// Parameters:
//		pImage = pointer to RImage that goes with this image (NULL if none yet)
//		ulFlags = sprite flags you want set
//
// Returns:
//		none
//
//*****************************************************************************

RSprite::RSprite(RImage* pImage, ULONG ulFlags)
{
	Init();

	m_pImage = pImage;
	m_ulFlags = ulFlags;
	m_sOwnImage	= FALSE;
}

//*****************************************************************************
//
// Constructor
//
// Description:
//		This constructor initializes all values of the sprite with the
//		parameters you supply and adds itself to the static list of sprites
//
// Parameters:
//		sX = starting x position
//		sY = starting y position
//		sZ = starting z position
//		sAngle = starting angle of rotation
//		lWidth = desired width of sprite image
//		lHeight = desired height of sprite image
//		ulFlags = initial sprite flags
//		pImage = RImage for the sprite
//
// Returns:
//		none
//
//*****************************************************************************

RSprite::RSprite(short sX, short sY, short sZ, short sAngle, 
                 long lWidth, long lHeight, ULONG ulFlags, RImage* pImage)
{
	// This basic init, among other things, adds this RSprite to the
	// list with the key pointing at m_sZ.
	Init();

	m_sX = sX;
	m_sY = sY;
	m_sZ = sZ;
	m_sAngle = sAngle;
	m_lWidth = lWidth;
	m_lHeight = lHeight;
	m_ulFlags = ulFlags;
	m_pImage = pImage;

	// Now that we have changed m_sZ, we should reposition our node.
	RSprite::SpriteList.Reposition(this);
}

//*****************************************************************************
//
// Init
//
// Description:
//		Initializes the members to the basic defaults.
//		Should not be called after CreateImage() unless DestroyImage() is called
//		or special care is taken with the *m_pImage data.
//
// Parameters:
//		none
//
//	Affects:
//		m_pImage, m_sOwnImage, m_sX, m_sY, m_sZ, m_sAngle, SpriteList
//
// Returns:
//		none
//
//*****************************************************************************

void RSprite::Init(void)
{
	m_pImage = NULL;
	m_sOwnImage	= FALSE;
	m_ulFlags = 0;
	m_sX = m_sY = m_sZ = m_sAngle = 0;
	m_lWidth = m_lHeight = 0;

	m_sHotSpotX	= 0;
	m_sHotSpotY	= 0;
	m_sHotSpotZ	= 0;

	RSprite::SpriteList.Insert(this, &m_sZ);
//	TRACE("RSprite::RSprite - adding sprite to list\n");
	if (RSprite::SpriteList.IsEmpty())
		TRACE("but the list is empty or something\n");
}

//*****************************************************************************
//
// Destructor
//
// Description:
//		Removes this sprite from the static list of sprites before 
//		destroying itself.  Also frees the list of regions if any
//
// Parameters:
//		none
//
// Returns:
//		none
//
//*****************************************************************************

RSprite::~RSprite()
{
	// Free the list of regions
	while (!m_clRegions.IsEmpty())
		m_clRegions.Remove();

	// Destroy *m_pImage, if we allocated it.
	DestroyImage();

	// Remove yourself from the list of sprites
//	if (RSprite::SpriteList.IsEmpty())
	RSprite::SpriteList.Remove(this);
}

//*****************************************************************************
//
// CreateImage
//
// Description:
//		Allocates an image pointed to by m_pImage.  Use DestroyImage() to
//		destroy.
//
// Parameters:
//		none
//
//	Affects:
//		m_pImage, m_sOwnImage, dynamic memory allocation
//
// Returns:
//		none
//
//*****************************************************************************

short RSprite::CreateImage(void)	// Returns 0 on success.
{
	short	sRes	= 0;	// Assume success.

	m_pImage	= new RImage;
	if (m_pImage != NULL)
	{
		// Succesfully allocated an image.
		// Remember we allocated it.
		m_sOwnImage	= TRUE;
	}
	else
	{
		TRACE("CreateImage(): Failed to allocate new RImage.\n");
		sRes	= -1;
	}

	return sRes;
}

//*****************************************************************************
//
// DestroyImage
//
// Description:
//		Destroys an image previously allocated by CreateImage().  If m_pImage
//		is NULL or m_sOwnImage is FALSE, this function does nothing.
//
// Parameters:
//		none
//
//	Affects:
//		m_pImage, m_sOwnImage, dynamic memory deallocation
//
// Returns:
//		none
//
//*****************************************************************************

void RSprite::DestroyImage(void)	// Returns nothing.
{
	if (m_pImage != NULL && m_sOwnImage != FALSE)
	{
		delete m_pImage;
		m_pImage		= NULL;
		m_sOwnImage	= FALSE;
	}
}

//*****************************************************************************
//
// Save
//
// Description:
//		This version of the function takes a filename and creates a new
//		file in which to save the sprite data.  It then calls the other
//		save function with the open RFile pointers.  See next Save 
//		description for more details.
//
// Parameters:
//		pszFilename = filename of image to be saved
//
// Returns:
//		SUCCESS if the file was saved successfully
//		FAILURE if there was an error - TRACE message will help pinpoint
//				  the failure
//
//*****************************************************************************

short RSprite::Save(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "wb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RSprite::Save - could not open file %s for output\n", pszFilename);
		return FAILURE;
	}

	sReturn = Save(&cf);

	cf.Close();

	return sReturn;
}

//*****************************************************************************
//
// Save
//
// Description:
//		This version of the function takes an open RFile pointer and writes
//		the sprite data to the current position in the file.  It saves the 
//		header for the sprite file and all of the current sprite information 
//		and then calls the RImage save funciton to save the image.  If the
//		current format of the image does not have a save function, then it
//		calls ConvertFrom to convert the image to one of the standard types and
//		then calls save and sets the desitnation type to its original format so
//		that the image will be automatically converted upon load.
//
// Parameters:
//		pcf = pointer to open RFile where the data will be saved.
//
// Returns:
//		SUCCESS if the file was saved
//		FAILURE if there wan an error - TRACE messages will help pinpoint
//				  the failure
//
//*****************************************************************************

short RSprite::Save(RFile* pcf)
{
	short sReturn = SUCCESS;
	ULONG ulFileType = SPRITE_COOKIE;
	ULONG ulCurrentVersion = SPRITE_CURRENT_VERSION;
	ULONG ulImageFlag = 0;

	if (pcf && pcf->IsOpen())
	{
		if (pcf->Write(&ulFileType) == 1)
		{
			if (pcf->Write(&ulCurrentVersion) == 1)
			{
				if (pcf->Write(&m_sX) == 1)
				{
					if (pcf->Write(&m_sY) == 1)
					{
						if (pcf->Write(&m_sZ) == 1)
						{
							if (pcf->Write(&m_sAngle) == 1)
							{
								if (pcf->Write(&m_lWidth) == 1)
								{
									if (pcf->Write(&m_lHeight) == 1)
									{
										if (pcf->Write(&m_dX) == 1)
										{
											if (pcf->Write(&m_dY) == 1)
											{
												if (pcf->Write(&m_dZ) == 1)
												{
													if (pcf->Write(&m_dXacc) == 1)
													{
														if (pcf->Write(&m_dYacc) == 1)
														{
															if (pcf->Write(&m_dZacc) == 1)
															{
																if (pcf->Write(&m_dXvel) == 1)
																{
																	if (pcf->Write(&m_dYvel) == 1)
																	{
																		if (pcf->Write(&m_dZvel) == 1)
																		{
																			if (pcf->Write(&m_ulFlags) == 1)
																			{
																				if (pcf->Write(&m_sHotSpotX) == 1)
																				{
																					if (pcf->Write(&m_sHotSpotY) == 1)
																					{
																						if (pcf->Write(&m_sHotSpotZ) == 1)
																						{
																							if (m_pImage)
																							{
																								ulImageFlag = 1;
																								if (pcf->Write(&ulImageFlag) == 1)
																								{
																									sReturn = m_pImage->Save(pcf);
																								}
																								else
																								{
																									TRACE("RSprite::Save - Error writing image flag\n");
																									sReturn = FAILURE;
																								}
																							}
																							else
																							{
																								ulImageFlag = 0;
																								if (pcf->Write(&ulImageFlag) == 1)
																								{
																									sReturn = SUCCESS;
																								}
																								else
																								{
																									TRACE("RSprite::Save - Error writing image flag\n");
																									sReturn = FAILURE;
																								}	
																							}
																						}
																						else
																						{
																							TRACE("RSprite::Save - Error writing m_sHotSpotZ.\n");
																							sReturn = FAILURE;
																						}
																					}
																					else
																					{
																						TRACE("RSprite::Save - Error writing m_sHotSpotY.\n");
																						sReturn = FAILURE;
																					}
																				}
																				else
																				{
																					TRACE("RSprite::Save - Error writing m_sHotSpotX.\n");
																					sReturn = FAILURE;
																				}
																			}
																			else
																			{
																				TRACE("RSprite::Save - Error writing m_ulFlags\n");
																				sReturn = FAILURE;
																			}
																		}
																		else
																		{
																			TRACE("RSprite::Save - Error writing m_dZvel\n");
																			sReturn = FAILURE;
																		}	
																	}
																	else
																	{
																		TRACE("RSprite::Save - Error writing m_dYvel\n");
																		sReturn = FAILURE;
																	}
																}
																else
																{
																	TRACE("RSprite::Save - Error writing m_dXvel\n");
																	sReturn = FAILURE;
																}
															}
															else
															{
																TRACE("RSprite::Save - Errow writing m_dZacc\n");
																sReturn = FAILURE;
															}
														}
														else
														{
															TRACE("RSprite::Save - Error writing m_dYacc\n");
															sReturn = FAILURE;
														}
													}
													else
													{
														TRACE("RSprite::Save - Error writing m_dXacc\n");
														sReturn = FAILURE;
													}
												}
												else
												{
													TRACE("RSprite::Save - Error writing m_dZ\n");
													sReturn = FAILURE;
												}
											}
											else
											{
												TRACE("RSprite::Save - Error writing m_dY\n");
												sReturn = FAILURE;
											}
										}
										else
										{
											TRACE("RSprite::Save - Error writing m_dX\n");
											sReturn = FAILURE;
										}	
									}
									else
									{
										TRACE("RSprite::Save - Error writing m_lHeight\n");
										sReturn = FAILURE;
									}
								}
								else
								{
									TRACE("RSprite::Save - Error writing m_lWidth\n");
									sReturn = FAILURE;
								}
							}
							else
							{
								TRACE("RSprite::Save - Error writing m_sAngle\n");
								sReturn = FAILURE;
							}						
						}
						else
						{
							TRACE("RSprite::Save - Error writing m_sZ position\n");
							sReturn = FAILURE;
						}					
					}
					else
					{
						TRACE("RSprite::Save - Error writing m_sY position\n");
						sReturn = FAILURE;
					}					
				}
				else
				{
					TRACE("RSprite::Save - Error writing m_sX position\n");
					sReturn = FAILURE;
				}				
			}
			else
			{
				TRACE("RSprite::Save - Error writing version number\n");
				sReturn = FAILURE;
			}		
		}
		else
		{
			TRACE("RSprite::Save - Error writing file type\n");
			sReturn = FAILURE;
		}	
	}
	else
	{
		TRACE("RSprite::Save - Error RFile pointer does not refer to an open file\n");
		sReturn = FAILURE;
	}

	return sReturn;
}

//*****************************************************************************
//
// Load
//
// Description:
//		Loads the sprite data and the image.  This version of the function
//		takes a filename and opens that file to read the sprite information.
//		Once the file is open, it calls the other Load function to read the
//		data from the open RFile.  See the next description for more info.
//
// Parameters:
//		pszFilename = name of file to open
//
// Returns:
//		SUCCESS if the file was loaded correctly
//		FAILURE if there was an error - TRACE messages will help
//				  pinpoint the failure.
//
//*****************************************************************************

short RSprite::Load(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "rb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RSprite::Load - could not open file %s for input\n", pszFilename);
		return FAILURE;
	}

	sReturn = Load(&cf);

	cf.Close();

	return sReturn;
}

//*****************************************************************************
//
// Load
//
// Description:
//		Loads the sprite data and the image.  This version of the function
//		takes an open RFile pointer and reads the data from the current
//		position.  It makes sure that the data is sprite data and if so, 
//		reads all of the fields of the sprite and then calls RImage::Load
//		to read the image data for this sprite.
//
// Parameters:
//		pcf = open RFile pointer where the data is to be read
//
// Returns:
//		SUCCESS if the file was read correctly
//		FAILURE if there was an error - TRACE messages will help
//				  pinpoing the failure.
//
//*****************************************************************************

short RSprite::Load(RFile* pcf)
{
	short sReturn = SUCCESS;
	ULONG ulFileType = 0;
	ULONG ulVersion = 0;
	ULONG ulImageFlag = 0;

	if (pcf && pcf->IsOpen())
	{
		if (pcf->Read(&ulFileType) == 1)
		{
			if (ulFileType == SPRITE_COOKIE)
			{
				if (pcf->Read(&ulVersion) == 1)
				{
					if (ulVersion == SPRITE_CURRENT_VERSION)
					{
						if (pcf->Read(&m_sX) == 1)
						{
							if (pcf->Read(&m_sY) == 1)
							{
								if (pcf->Read(&m_sZ) == 1)
								{
									if (pcf->Read(&m_sAngle) == 1)
									{
										if (pcf->Read(&m_lWidth) == 1)
										{
											if (pcf->Read(&m_lHeight) == 1)
											{
												if (pcf->Read(&m_dX) == 1)
												{
													if (pcf->Read(&m_dY) == 1)
													{
														if (pcf->Read(&m_dZ) == 1)
														{
															if (pcf->Read(&m_dXacc) == 1)
															{
																if (pcf->Read(&m_dYacc) == 1)
																{
																	if (pcf->Read(&m_dZacc) == 1)
																	{
																		if (pcf->Read(&m_dXvel) == 1)
																		{
																			if (pcf->Read(&m_dYvel) == 1)
																			{
																				if (pcf->Read(&m_dZvel) == 1)
																				{
																					if (pcf->Read(&m_ulFlags) == 1)
																					{
																						if (pcf->Read(&m_sHotSpotX) == 1)
																						{
																							if (pcf->Read(&m_sHotSpotY) == 1)
																							{
																								if (pcf->Read(&m_sHotSpotZ) == 1)
																								{
																									if (pcf->Read(&ulImageFlag) == 1)
																									{
																										if (ulImageFlag == 1)
																										{
																											if (CreateImage() == 0)
																											{
																												sReturn = m_pImage->Load(pcf);
																											}
																											else
																											{
																												TRACE("RSprite::Load - Error allocating m_pImage\n");
																												sReturn = FAILURE;
																											}
																										}
																										else
																										{
																											sReturn = SUCCESS; // No image to read, just continue
																										}
																									}
																									else
																									{
																										TRACE("RSprite::Load - Error reading image flag\n");
																										sReturn = FAILURE;
																									}
																								}
																								else
																								{
																									TRACE("RSprite::Load - Error reading m_sHotSpotZ\n");
																									sReturn = FAILURE;
																								}
																							}
																							else
																							{
																								TRACE("RSprite::Load - Error reading m_sHotSpotY\n");
																								sReturn = FAILURE;
																							}
																						}
																						else
																						{
																							TRACE("RSprite::Load - Error reading m_sHotSpotX\n");
																							sReturn = FAILURE;
																						}
																					}
																					else
																					{
																						TRACE("RSprite::Load - Error reading m_ulFlags\n");
																						sReturn = FAILURE;
																					}
																				}
																				else
																				{
																					TRACE("RSprite::Load - Error reading m_dZvel\n");
																					sReturn = FAILURE;
																				}
																			}
																			else
																			{
																				TRACE("RSprite::Load - Error reading m_dYvel\n");
																				sReturn = FAILURE;
																			}
																		}
																		else
																		{
																			TRACE("RSprite:;Load - Error reading m_dXvel\n");
																			sReturn = FAILURE;
																		}
																	}
																	else
																	{
																		TRACE("RSprite::Load - Error reading m_dZacc\n");
																		sReturn = FAILURE;
																	}
																}
																else
																{
																	TRACE("RSprite::Load - Error reding m_dYacc\n");
																	sReturn = FAILURE;
																}
															}
															else
															{
																TRACE("RSprite::Load - Error reading m_dXacc\n");
																sReturn = FAILURE;
															}
														}
														else
														{
															TRACE("RSprite::Load - Error reading m_dZ\n");
															sReturn = FAILURE;
														}	
													}	
													else
													{
														TRACE("RSprite::Load - Error reading m_dY\n");
														sReturn = FAILURE;
													}
												}
												else
												{
													TRACE("RSprite::Load - Error reding m_dX\n");
													sReturn = FAILURE;
												}
											}
											else
											{
												TRACE("RSprite::Load - Error reading m_lHeight\n");
												sReturn = FAILURE;
											}
										}
										else
										{
											TRACE("RSprite::Load - Error reading m_lWidth\n");
											sReturn = FAILURE;
										}
									}
									else
									{
										TRACE("RSprite::Load - Error reading m_sAngle\n");
										sReturn = FAILURE;
									}
								}
								else
								{
									TRACE("RSprite::Load - Error reading m_sZ position\n");
									sReturn = FAILURE;
								}
							}
							else
							{
								TRACE("RSprite::Load - Error reading m_sY position\n");
								sReturn = FAILURE;
							}
						}
						else
						{
							TRACE("RSprite::Load - Error reading m_sX position\n");
							sReturn = FAILURE;
						}
					}
					else
					{
						TRACE("RSprite::Load - Error Wrong version.\n");
						TRACE("RSprite::Load - Current RSprite version %d\n", SPRITE_CURRENT_VERSION);
						TRACE("RSprite::Load - This file's version %d\n", ulVersion);
						sReturn = FAILURE;

					}
				}
				else
				{
					TRACE("RSprite::Load - Error reading file version number\n");
					sReturn = FAILURE;
				}
			}
			else
			{
				TRACE("RSprite::Load - Error: Wrong file type.  Sprite files should start with 'SPRT'\n");
				sReturn = FAILURE;
			}
		}
		else
		{
			TRACE("RSprite::Load - Error reading file type\n");
			sReturn = FAILURE;
		}
	}
	else
	{
		TRACE("RSprite::Load - Error RFile pointer does not refer to an open file\n");
		sReturn = FAILURE;
	}
	
	return sReturn;
}



//*****************************************************************************
//
//	DoRegionsCollide
//
// Description:
//		Checks to see if the specified region of this sprite collide with
//		the specified region type of another sprite
//
// Parameters:
//		sThisRegionType = the region type of this sprite to check
//		sOtherRegionType = the region type of the other sprite to check
//		pSprite = the other sprite to check
//
// Returns:
//		1 if regions collide
//		0 otherwise
//
//*****************************************************************************
short RSprite::DoRegionsCollide(short /*sThisRegionType*/, short /*sOtherRegionType*/,
											RSprite* /*pSprite*/)	
{
	short bCollision = 0;

	// Call the region's collision detection routines here

	return bCollision;
}

//*****************************************************************************
//
// AddRegion
//
// Description:
//		Add a region to this sprite's region list
//
// Parameters:
//		RRegion* = pointer to region to add
//
// Returns:
//		SUCCESS if the region was added
//		FAILURE if there was a problem adding it to the list
//
//*****************************************************************************

short RSprite::AddRegion(RRegion* pRegion)
{
	return m_clRegions.Add(pRegion);
}

//*****************************************************************************
//
// RemoveRegion
//
// Description:
//		Remove a region from the sprite's region list
//
// Parameters:
//		RRegion* = pointer to region to be removed
//
// Returns:
//		SUCCESS if the region was found and removed from the list
//		FAILURE if the region could not be found or removed
//
//*****************************************************************************

short RSprite::RemoveRegion(RRegion* pRegion)
{
	return m_clRegions.Remove(pRegion);
}


//*****************************************************************************
// EOF
//*****************************************************************************
