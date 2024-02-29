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
// trigger.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CTrigger class -> holds trigger attributes
//
// History:
//		12/25/96 MJR	Started.
//
//		05/12/97	JRD	Turned this into a CTrigger to load the trigger attributes
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "trigger.h"
#include "game.h"
#include "realm.h" 


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
// Constructor
// (protected).
////////////////////////////////////////////////////////////////////////////////
CTrigger::CTrigger(CRealm* pRealm)
	: CThing(pRealm, CTriggerID)
	{
	// Insert a default instance into the realm:
	m_pmgi = NULL;
	m_pRealm->m_pTriggerMapHolder = this;
	m_pRealm->m_pTriggerMap = m_pmgi;

	// Assume we don't know the UID's fdor the pylons yet, so clear them all out:
	for (short i=0;i < 256;i++)
		{
		m_ausPylonUIDs[i] = pRealm->m_asPylonUIDs[i] = 0;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Destructor
// (public).
////////////////////////////////////////////////////////////////////////////////
CTrigger::~CTrigger()
	{
	if (m_pmgi) delete m_pmgi;
	m_pmgi = NULL;

	// Clear it from the realm:
	m_pRealm->m_pTriggerMap = NULL;
	m_pRealm->m_pTriggerMapHolder = NULL;
	}

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CTrigger::Load(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,									// In:  File to load from
	bool bEditMode,								// In:  True for edit mode, false otherwise
	short sFileCount,								// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)							// In:  Version of file format to load.
	{
	short sResult = 0;

	// In most cases, the base class Load() should be called.
	sResult	= CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// Load object data
		m_pRealm->m_pTriggerMap = NULL; // clear the shadow
		// ASSUME THERE WILL ALREADY BE AN EMPTY TRIGGER MAP HOLDER!
		if (m_pRealm->m_pTriggerMapHolder == NULL) m_pRealm->m_pTriggerMapHolder = new CTrigger(m_pRealm);

		if (m_pmgi) delete m_pmgi;
		m_pmgi = NULL;

		short sData = 0;
		pFile->Read(&sData);

		// Was there something here?
		if (sData)
			{
			m_pmgi = new RMultiGridIndirect;
			if (m_pmgi) 
				{
				if (m_pmgi->Load(pFile) != SUCCESS)
					{
					TRACE("CTrigger::Load(): Warning - couldn't load trigger attributes!\n");
					sResult = -1;
					}
				else
					{
					// Load the ID list:
					if (pFile->Read(m_ausPylonUIDs,256) != 256) // Grab the ID's
						{
						TRACE("CTrigger::Load(): Warning - I lost my pylon IDs!\n");
						sResult = -1;
						}
					else
						{
						// Install into the Realm
						m_pRealm->m_pTriggerMapHolder = this;
						m_pRealm->m_pTriggerMap = m_pmgi;
						
						// Copy the Pylons to the realm!
						for (short i=0;i < 256;i++) m_pRealm->m_asPylonUIDs[i] = m_ausPylonUIDs[i];
						}
					}
				}
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			}
		else
			{
			sResult = -1;
			TRACE("CTrigger::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CTrigger::Load(): CThing::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CTrigger::Save(								// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,									// In:  File to save to
	short sFileCount)								// In:  File count (unique per file, never 0)
	{
	short	sResult	= 0;

	// In most cases, the base class Save() should be called.
	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save object data
		short sData = 0; // NO DATA

		if (m_pmgi == NULL) 
			{
			sData = 0;
			pFile->Write(sData);
			}
		else
			{
			// There IS an attribute:
			sData = 1;
			pFile->Write(sData);
			if (m_pmgi->Save(pFile) != SUCCESS)
				{
				sResult = -1;
				TRACE("CTrigger::Save(): Error - coudln't save trigger attributes.\n");
				}
			else
				{
				// Save the Pylon Data:
				if (pFile->Write(m_ausPylonUIDs,256) != 256)
					{
					sResult = -1;
					TRACE("CTrigger::Save(): Error - coudln't save Pylon IDs.\n");
					}
				}
			}

		sResult	= pFile->Error();
		if (sResult == 0)
			{
			}
		else
			{
			TRACE("CTrigger::Save(): Error writing to file.\n");
			}
		}
	else
		{
		TRACE("CTrigger::Save(): CThing::Save() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CTrigger::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CTrigger::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CTrigger::Suspend(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CTrigger::Resume(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CTrigger::Update(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CTrigger::Render(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CTrigger::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CTrigger::EditModify(void)
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CTrigger::EditMove(									// Returns 0 if successfull, non-zero otherwise
	short /*sX*/,											// In:  New x coord
	short /*sY*/,											// In:  New y coord
	short /*sZ*/)											// In:  New z coord
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CTrigger::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CTrigger::EditRender(void)
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Add myself into the realm that created me.
////////////////////////////////////////////////////////////////////////////////
void	CTrigger::AddData(RMultiGridIndirect* pmgi)
	{
	m_pRealm->m_pTriggerMap = m_pmgi = pmgi;

	// Copy my Pylon Data into the Realm's:
	for (short i=0;i < 256;i++) m_pRealm->m_asPylonUIDs[i] = m_ausPylonUIDs[i];
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
