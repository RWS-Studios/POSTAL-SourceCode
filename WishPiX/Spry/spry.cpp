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
// spry.cpp
//
// This module impliments the RSpry class, which is a simple array of RSprites.
//
// History:
//		12/25/96 MJR	Started.
//
//		05/13/97	JMI	Casted instances of warning C4018 (signed/unsigned mismatch)
//							to make MSVC 4.1(Alpha) happy (these seem to fall under
//							Warning Level 3 in 4.1(Alpha) but not in 4.2(Intel)).
//
//		06/29/97 MJR	Replaced STL vector with an RSP list.  STL is an evil
//							entity that should be banished from the face of the earth.
//							Whoever suggested we use it should be shot.  (Good thing
//							I'm the president -- it's against the rules to shoot me.)
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "spry.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Default (and only) constructor
////////////////////////////////////////////////////////////////////////////////
RSpry::RSpry()
	{
	// On construction, the member vector is automatically initialized by its
	// own constructor.
	}


////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
RSpry::~RSpry()
	{
	Clear();
	}


////////////////////////////////////////////////////////////////////////////////
// Clear 
////////////////////////////////////////////////////////////////////////////////
short RSpry::Clear(void)
	{
	short sResult = 0;

	// Destroy any existing sprites and get rid of all list nodes
	while (m_listSprites.GetHead())
		{
		delete m_listSprites.GetHeadData();
		m_listSprites.RemoveHead();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Load from specified file
////////////////////////////////////////////////////////////////////////////////
short RSpry::Load(
	char* pszFile)
	{
	short sResult = 0;

	// Clear any existing sprites
	Clear();

	// Open file
	RFile file;
	sResult = file.Open(pszFile, "rb", RFile::LittleEndian);
	if (sResult == 0)
		{

		// Load everything from file
		sResult = Load(&file);

		file.Close();
		}
	else
		{
		sResult = -1;
		TRACE("RSpry::Load(): Couldn't open file: %s !\n", pszFile);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Load from already-open file
////////////////////////////////////////////////////////////////////////////////
short RSpry::Load(
	RFile* pFile)
	{
	short sResult = 0;

	// Clear any existing sprites
	Clear();

	// Read & validate file ID
	ULONG ulFileID;
	if (pFile->Read(&ulFileID) == 1)
		{
		if (ulFileID == RSpry::FileID)
			{

			// Read & validate file version
			ULONG ulFileVersion;
			if (pFile->Read(&ulFileVersion) == 1)
				{
				if (ulFileVersion == RSpry::FileVersion)
					{

					// Read count of sprites
					short sCount;
					pFile->Read(&sCount);

					// Construct and load indicated number of RSprite's and add them to list
					for (short i = 0; (i < sCount) && !sResult; i++)
						{
						RSprite* pSprite = new RSprite;
						if (pSprite != 0)
							{
							sResult = pSprite->Load(pFile);
							if (sResult == 0)
								m_listSprites.InsertTail(pSprite);
							}
						else
							{
							sResult = -1;
							TRACE("RSpry::Load(): Error creating new RSprite!\n");
							}
						}

					}
				else
					{
					sResult = -1;
					TRACE("RSpry::Load(): Incorrect file version (should be 0x%lx, was 0x%lx)!\n", RSpry::FileVersion, ulFileVersion);
					}
				}
			else
				{
				sResult = -1;
				TRACE("RSpry::Load(): Error reading file version!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("RSpry::Load(): Incorrect file ID (should be 0x%lx, was 0x%lx)!\n", RSpry::FileID, ulFileID);
			}
		}
	else
		{
		sResult = -1;
		TRACE("RSpry::Load(): Error reading file ID!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save to specified file
////////////////////////////////////////////////////////////////////////////////
short RSpry::Save(
	char* pszFile)
	{
	short sResult = 0;

	// Open file
	RFile file;
	sResult = file.Open(pszFile, "wb", RFile::LittleEndian);
	if (sResult == 0)
		{

		// Save everything to file
		sResult = Save(&file);

		file.Close();		
		}
	else
		{
		sResult = -1;
		TRACE("RSpry::Save(): Couldn't open file: %s !\n", pszFile);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save to already-open file
////////////////////////////////////////////////////////////////////////////////
short RSpry::Save(
	RFile* pFile)
	{
	short sResult = 0;

	// Write out file ID and version
	pFile->Write((unsigned long)RSpry::FileID);
	pFile->Write((unsigned long)RSpry::FileVersion);

	// Write out number of sprites
	pFile->Write((short)m_listSprites.GetCount());

	// Write out each sprite
	ListOfSprites::Pointer p = m_listSprites.GetHead();
	while (p && !sResult)
		{
		sResult = m_listSprites.GetData(p)->Save(pFile);
		p = m_listSprites.GetNext(p);
		}

	// If no errors were reported, double-check for I/O errors
	if (!sResult && pFile->Error())
		{
		sResult = -1;
		TRACE("RSpry::Save(): Error writing file!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Convert to specified RImage type
////////////////////////////////////////////////////////////////////////////////
short RSpry::Convert(
	RImage::Type type)
	{
	short sResult = 0;

	// Convert all sprites to specified type
	ListOfSprites::Pointer p = m_listSprites.GetHead();
	while (p && !sResult)
		{
		if (m_listSprites.GetData(p)->m_pImage->m_type != type)
			{
			if (m_listSprites.GetData(p)->m_pImage->Convert(type) != type)
				{
				sResult = -1;
				TRACE("RSpry::Convert(): Couldn't convert sprite!\n");
				}
			}
		p = m_listSprites.GetNext(p);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
