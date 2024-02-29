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
// weapon.cpp
// Project: Nostril (aka Postal)
//
// This module implements the CWeapon class which is base class for the weapons
//
// History:
//		02/27/97 BRH	Started this file from CDoofus and modified it
//							to be a base class for the weapons.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/19/97 BRH	Added virtual functions for processing messages and
//							virtual OnMessage handler functions so that it follows
//							the model of the CThing3d base class object.  
//
//		06/25/97 BRH	Added rendering of 2D shadow sprite to the Render
//							function.  Also added PrepareShadow function to
//							load the default shadow resource if no resource is 
//							loaded for the shadow and then make the shadow visible.
//
//		06/30/97	JMI	Now maps the Z to 3D when loading fileversions previous to
//							24.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/21/97	JMI	Now checks upper bound on m_sAlphaLevel of shadow sprite.
//	
//		07/30/97	JMI	Now hides shadow if mainsprite is hidden.
//
////////////////////////////////////////////////////////////////////////////////
#define WEAPON_CPP

#include "RSPiX.h"
#include "weapon.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SHADOW_FILE	"shadow.img"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Let this auto-init to 0
short CWeapon::ms_sFileCount;


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CWeapon::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	short sResult = 0;

	// Call the CThing base class load to get the instance ID
	sResult	= CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Load static data.
			switch (ulFileVersion)
				{
				default:
				case 1:
					break;
				}
			}

		switch (ulFileVersion)
			{
			default:
			case 1:
				// Load object data
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				pFile->Read(&m_dRot);
				pFile->Read(&m_dVertVel);
				pFile->Read(&m_dHorizVel);
				pFile->Read(&m_eState);
				break;
			}

		
		// If the file version is earlier than the change to real 3D coords . . .
		if (ulFileVersion < 24)
			{
			// Convert to 3D.
			m_pRealm->MapY2DtoZ3D(
				m_dZ,
				&m_dZ);
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// Get resources
	//		sResult = GetResources();
			}
		else
			{
			sResult = -1;
			TRACE("CWeapon::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CWeapon::Load():  CThing::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CWeapon::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	// Call the base class save to save the u16InstanceID
	CThing::Save(pFile, sFileCount);

	// Save common data just once per file (not with each object)
	if (ms_sFileCount != sFileCount)
		{
		ms_sFileCount = sFileCount;

		// Save static data
		}

	// Save object data
	pFile->Write(&m_dX);
	pFile->Write(&m_dY);
	pFile->Write(&m_dZ);
	pFile->Write(&m_dRot);
	pFile->Write(&m_dVertVel);
	pFile->Write(&m_dHorizVel);
	pFile->Write(&m_eState);

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CWeapon::Startup(void)								// Returns 0 if successfull, non-zero otherwise
	{

	// Init other stuff
	m_lPrevTime = m_pRealm->m_time.GetGameTime();

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Setup object
////////////////////////////////////////////////////////////////////////////////

short CWeapon::Setup(short sX, short sY, short sZ)
	{
	m_dX = sX;
	m_dY = sY;
	m_dZ = sZ;
	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CWeapon::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Suspend(void)
	{
	m_sSuspend++;
	}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Resume(void)
	{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
	if (m_sSuspend == 0)
		m_lPrevTime = m_pRealm->m_time.GetGameTime();
	}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Update(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::Render(void)
{
	CSprite* pSprite = GetSprite();

	// If the shadow is enabled and the main sprite is visible . . .
	if (m_spriteShadow.m_pImage && (pSprite->m_sInFlags & CSprite::InHidden) == 0)
	{
		// Get the height of the terrain from the attribute map
		short sY = m_pRealm->GetHeight((short) m_dX, (short) m_dZ);
		// Map from 3d to 2d coords
		Map3Dto2D(m_dX, (double) sY, m_dZ, &(m_spriteShadow.m_sX2), &(m_spriteShadow.m_sY2) );
		// Offset hotspot to center of image.
		m_spriteShadow.m_sX2 -= m_spriteShadow.m_pImage->m_sWidth / 2;
		m_spriteShadow.m_sY2 -= m_spriteShadow.m_pImage->m_sHeight / 2;

		// Priority is based on bottom edge of sprite on X/Z plane!
		m_spriteShadow.m_sPriority = MAX(pSprite->m_sPriority - 1, 0);//m_dZ;

		// Layer should be based on info we get from attribute map.
		m_spriteShadow.m_sLayer = pSprite->m_sLayer;

		// Set the alpha level based on the height difference
		m_spriteShadow.m_sAlphaLevel = 200 - ((short) m_dY - sY);
		// Check bounds . . .
		if (m_spriteShadow.m_sAlphaLevel < 0)
			{
			m_spriteShadow.m_sAlphaLevel	= 0;
			}
		else if (m_spriteShadow.m_sAlphaLevel > 255)
			{
			m_spriteShadow.m_sAlphaLevel	= 255;
			}

		// If the main sprite is on the ground, then hide the shadow.
		if ((short) m_dY - sY == 0)
			m_spriteShadow.m_sInFlags |= CSprite::InHidden;
		else
			m_spriteShadow.m_sInFlags &= ~CSprite::InHidden;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_spriteShadow);
	}
	else
	{
		m_spriteShadow.m_sInFlags |= CSprite::InHidden;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CWeapon::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CWeapon::EditModify(void)
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CWeapon::EditMove(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to update object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::EditUpdate(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CWeapon::EditRender(void)
	{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
	}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CWeapon::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CWeapon::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
// BounceAngle
////////////////////////////////////////////////////////////////////////////////

double CWeapon::BounceAngle(double dRot)
{
	short sRot = (short) dRot;
	short sBounceAngle = (((((sRot / 90) + 1) * 180) - sRot) % 360);
	return (double) sBounceAngle;
}

////////////////////////////////////////////////////////////////////////////////
// Process all messages currently in the message queue through 
// ProcessMessage().
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::ProcessMessages(void)
	{
	// Check queue of messages.
	GameMessage	msg;
	while (m_MessageQueue.DeQ(&msg) == true)
		{
		ProcessMessage(&msg);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Process the specified message.  For most messages, this function
// will call the equivalent On* function.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::ProcessMessage(		// Returns nothing.
	GameMessage* pmsg)					// Message to process.
	{
	switch (pmsg->msg_Generic.eType)
		{
		case typeShot:
			OnShotMsg(&(pmsg->msg_Shot) );
			break;
		
		case typeExplosion:
			OnExplosionMsg(&(pmsg->msg_Explosion) );
			break;
		
		case typeBurn:
			OnBurnMsg(&(pmsg->msg_Burn) );
			break;
		
		case typeObjectDelete:
			OnDeleteMsg(&(pmsg->msg_ObjectDelete) );
			break;

		case typeTrigger:
			OnTriggerMsg(&(pmsg->msg_Trigger) );
			break;
		
		default:
			// Should this complain when it doesn't know a message type?
			break;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a msg_Shot.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnShotMsg(	// Returns nothing.
	Shot_Message* pshotmsg)		// In:  Message to handle.
{
}

////////////////////////////////////////////////////////////////////////////////
// Handles an Explosion_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnExplosionMsg(			// Returns nothing.
	Explosion_Message* pexplosionmsg)	// In:  Message to handle.
{
}

////////////////////////////////////////////////////////////////////////////////
// Handles a Burn_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnBurnMsg(	// Returns nothing.
	Burn_Message* pburnmsg)		// In:  Message to handle.
{
}

////////////////////////////////////////////////////////////////////////////////
// Handles an ObjectDelete_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnDeleteMsg(				// Returns nothing.
	ObjectDelete_Message* pdeletemsg)	// In:  Message to handle.
{
}

////////////////////////////////////////////////////////////////////////////////
// Handles a Trigger_Message
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CWeapon::OnTriggerMsg(			// Returns nothing
	Trigger_Message* ptriggermsg)		// In: Message to handle
{
}


////////////////////////////////////////////////////////////////////////////////
// PrepareShadow
////////////////////////////////////////////////////////////////////////////////

short CWeapon::PrepareShadow(void)
{
	short sResult = SUCCESS;

	// If the shadow doesn't have resource loaded yet, load the default
	if (m_spriteShadow.m_pImage == NULL)
	{
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(SHADOW_FILE), &(m_spriteShadow.m_pImage), RFile::LittleEndian);
	}

	// If a resource is available, set the shadow to visible.
	if (sResult == SUCCESS)
		m_spriteShadow.m_sInFlags &= ~CSprite::InHidden;

	return sResult;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
