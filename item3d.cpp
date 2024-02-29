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
// item3d.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CItem3d class, which is a simple one animationed
// 3D object.
//
// History:
//		03/06/97	JMI	Started this 3D item class.
//
//		03/06/97	JMI	EditModify() now allows you to pick or specify an anim.
//
//		03/06/97	JMI	Fixed Custom selection in EditModify() dialog.
//
//		03/06/97	JMI	Now loads parents instance ID and overrides Render().
//
//		03/06/97	JMI	Added Trumpet, Horn, and Sax and descriptions for known
//							types.
//
//		03/07/97	JMI	Added handy everything-to-get-started Setup().
//
//		03/12/97	JMI	Update() now calculates the duration in seconds, updates
//							m_lPrevTime, and passes the duration to DeluxeUpdatePosVel.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/18/97	JMI	Now based on CThing3d instead of CCharacter.
//
//		03/18/97	JMI	Now processes State_BlownUp.
//
//		03/18/97	JMI	EditModify() was not selecting the current item in the
//							listbox.
//							Also, removed Render() override.
//
//		03/18/97	JMI	Forgot to set the parent instance ID to the one passed in
//							the Setup() function.
//
//		03/19/97	JMI	m_lAnimTime was not being reset to 0 ever.  Now it is set
//							in Init().
//
//		03/19/97	JMI	Now starts object spinning around Y and Z when an explosive
//							force affects it and stops the spinning when WhileBlownUp() 
//							returns false.
//
//		03/25/97	JMI	Changed EDIT_GUI_FILE to 8.3 compliant name.
//
//		04/23/97	JMI	Now sets its m_smash's bits to Misc instead of Item.
//
//		05/13/97	JMI	Now FreeResources() checks to make sure we have some 
//							some animation resources before Release()ing them.
//							This allows GetResources() to call FreeResources()
//							indiscriminantly before getting new resources.
//
//		06/09/97	JMI	Added rotation velocities to EditModify().
//							Now saves these values as of FileVersion 18.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/25/97	JMI	Now calls PrepareShadow() in Init() which loads and sets up
//							a shadow sprite.
//
//		06/25/97	JMI	Uses smaller shadow now....If we need multiple sizes later,
//							we can make an additional array.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		08/07/97	JMI	Added ability to optionally load a rigid body and child
//							anim for this item3d.
//
//		08/07/97	JMI	Added additional parameter to CAnim3D::Get() call.
//
//		08/13/97	JMI	EditModify() sets m_type to Custom if it is None.
//
//		08/28/97	JMI	Now if a GetResources() fails, it removes the sprite from
//							the scene just in case it was already there.  Also, only
//							call base class Render(), if there's loaded anim data.
//							The problem is that the EditModify() can fail b/c the
//							user's anim choice failed to load.  In this case, we're
//							forced to persist without anim data.  This will probably
//							cause the realm not to load, though, if saved in such a
//							state.
//
////////////////////////////////////////////////////////////////////////////////
#define ITEM3D_CPP

#include "RSPiX.h"
#include <math.h>

#include "item3d.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define NUM_RES_NAMES	7

#define EDIT_GUI_FILE	"res/editor/Item3d.gui"

#define SHADOW_RES_NAME		"SmallShadow.img"

#define LIST_BOX_ID				200
#define RESNAME_EDIT_ID			100
#define RIGID_RESNAME_EDIT_ID	101
#define CHILD_RESNAME_EDIT_ID	102
#define YROTVEL_EDIT_ID			300
#define ZROTVEL_EDIT_ID			301

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Let this auto-init to 0
short CItem3d::ms_sFileCount;

// Array of known animation base names.
char*	CItem3d::ms_apszKnownAnimBaseNames[CItem3d::NumTypes]	=
	{
	"None",				// None.
	"Custom",			// Custom.
	"3d/Trumpet",		// Trumpet.
	"3d/Horn",			// Horn.
	"3d/Sax",			// Sax.
	};

// Array of known animation descriptions.
char*	CItem3d::ms_apszKnownAnimDescriptions[CItem3d::NumTypes]	=
	{
	"None",
	"Custom",
	"Trumpet",
	"Horn",
	"Sax",
	};

static char* ms_apszResExtensions[NUM_RES_NAMES]	=
	{
	"sop",
	"mesh",
	"tex",
	"hot",
	"bounds",
	"floor",
	};

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CItem3d::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	short sResult = 0;

	// In most cases, the base class Load() should be called.
	sResult	= CThing3d::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// Load common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Load static data
			switch (ulFileVersion)
				{
				default:
				case 1:
					break;
				}
			}

		// Load object data
		switch (ulFileVersion)
			{
			default:
			case 42:
				pFile->Read(m_szAnimRigidName);
				pFile->Read(m_szChildAnimBaseName);

			case 41:
			case 40:
			case 39:
			case 38:
			case 37:
			case 36:
			case 35:
			case 34:
			case 33:
			case 32:
			case 31:
			case 30:
			case 29:
			case 28:
			case 27:
			case 26:
			case 25:
			case 24:
			case 23:
			case 22:
			case 21:
			case 20:
			case 19:
			case 18:
				pFile->Read(&m_dExtRotVelY);
				pFile->Read(&m_dExtRotVelZ);

			case 17:
			case 16:
			case 15:
			case 14:
			case 13:
			case 12:
			case 11:
			case 10:
			case 9:
			case 8:
			case 7:
			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
				pFile->Read(m_szAnimBaseName);

				U32	u32Temp;
				pFile->Read(&u32Temp);
				m_type	= (ItemType)u32Temp;

				pFile->Read(&m_u16IdParent);
				break;
			}
		
		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// Init item3d
			sResult = Init();
			}
		else
			{
			sResult = -1;
			TRACE("CItem3d::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CItem3d::Load(): CThing3d::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CItem3d::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	short sResult = 0;

	// In most cases, the base class Save() should be called.
	sResult	= CThing3d::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Save static data
			}

		// Save object data
		pFile->Write(m_szAnimRigidName);
		pFile->Write(m_szChildAnimBaseName);

		pFile->Write(m_dExtRotVelY);
		pFile->Write(m_dExtRotVelZ);

		pFile->Write(m_szAnimBaseName);

		pFile->Write((U32)m_type);

		pFile->Write(m_u16IdParent);
		
		sResult	= pFile->Error();
		}
	else
		{
		TRACE("CItem3d::Save(): CThing3d::Save() failed.\n");
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CItem3d::Update(void)
	{
	if (!m_sSuspend)
		{
		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime();

		// Advance the animation timer.
		long	lDifTime		= lThisTime - m_lAnimPrevUpdateTime;
		m_lAnimTime			+= lDifTime;

		// Update prev time.
		m_lAnimPrevUpdateTime	= lThisTime;

		// Service message queue.
		ProcessMessages();

		// Switch on state.
		switch (m_state)
			{
			case State_BlownUp:
				// Handle state.  If done . . .
				if (WhileBlownUp() == false)
					{
					m_state	= State_Idle;
					// Stop the spinning.
					m_dExtRotVelY	= 0;
					m_dExtRotVelZ	= 0;
					}
				break;
			default:
				if (m_u16IdParent == CIdBank::IdNil)
					{
					// Get time from last call in seconds.
					double	dSeconds	= double(lThisTime - m_lPrevTime) / 1000.0;
			
					DeluxeUpdatePosVel(dSeconds);
					}

				break;
			}

		// Update sphere.
		m_smash.m_sphere.sphere.X			= m_dX;
		m_smash.m_sphere.sphere.Y			= m_dY;
		m_smash.m_sphere.sphere.Z			= m_dZ;
		m_smash.m_sphere.sphere.lRadius	= m_sprite.m_sRadius;

		// Update the smash.
		m_pRealm->m_smashatorium.Update(&m_smash);

		// Save time for next time
		m_lPrevTime = lThisTime;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CItem3d::Render(void)								// Returns nothing.
	{
	// Sometimes this thing can exist after a failed EditModify().
	if (m_panimCur->m_psops)
		{
		// Call base class.
		CThing3d::Render();
		}

	// If we have a child and a rigid transform . . .
	if (m_animChild.m_pmeshes && m_anim.m_ptransRigid)
		{
		// Update the child sprite.
		m_spriteChild.m_pmesh	= m_animChild.m_pmeshes->GetAtTime(m_lAnimTime);    
		m_spriteChild.m_psop		= m_animChild.m_psops->GetAtTime(m_lAnimTime);        
		m_spriteChild.m_ptex		= m_animChild.m_ptextures->GetAtTime(m_lAnimTime);
		m_spriteChild.m_psphere	= m_animChild.m_pbounds->GetAtTime(m_lAnimTime);   
		// Use parent's rigid body transform for transform.
		m_spriteChild.m_ptrans	= m_panimCur->m_ptransRigid->GetAtTime(m_lAnimTime);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CItem3d::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short sResult = CThing3d::EditNew(sX, sY, sZ);
	if (sResult == 0)
		{
		sResult	= EditModify();
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CItem3d::EditModify(void)					// Returns 0 if successfull, non-zero otherwise
	{
	short	sResult	= CThing3d::EditModify();

	RGuiItem*	pguiRoot	= RGuiItem::LoadInstantiate(FullPathVD(EDIT_GUI_FILE));
	if (pguiRoot != NULL)
		{
		// Get listbox.
		RListBox* plb				= (RListBox*)pguiRoot->GetItemFromId(LIST_BOX_ID);
		REdit*	peditName		= (REdit*)pguiRoot->GetItemFromId(RESNAME_EDIT_ID);
		REdit*	peditRigidName	= (REdit*)pguiRoot->GetItemFromId(RIGID_RESNAME_EDIT_ID);
		REdit*	peditChildName	= (REdit*)pguiRoot->GetItemFromId(CHILD_RESNAME_EDIT_ID);
		REdit*	peditRotY		= (REdit*)pguiRoot->GetItemFromId(YROTVEL_EDIT_ID);
		REdit*	peditRotZ		= (REdit*)pguiRoot->GetItemFromId(ZROTVEL_EDIT_ID);
		if (peditName && plb && peditRotY && peditRotZ && peditChildName && peditRigidName)
			{
			ASSERT(peditName->m_type == RGuiItem::Edit);
			ASSERT(peditRotY->m_type == RGuiItem::Edit);
			ASSERT(peditRotZ->m_type == RGuiItem::Edit);
			ASSERT(plb->m_type == RGuiItem::ListBox);

			peditName->SetText("%s", m_szAnimBaseName);
			peditName->Compose();

			peditRigidName->SetText("%s", m_szAnimRigidName);
			peditRigidName->Compose();

			peditChildName->SetText("%s", m_szChildAnimBaseName);
			peditChildName->Compose();

			peditRotY->SetText("%g", m_dExtRotVelY);
			peditRotY->Compose();

			peditRotZ->SetText("%g", m_dExtRotVelZ);
			peditRotZ->Compose();

			// If no type . . .
			if (m_type == None)
				{
				// Default to custom for ease of user interface.
				m_type	= Custom;
				}

			short	i;
			RGuiItem*	pguiItem;
			for (i = Custom; i < NumTypes; i++)
				{
				pguiItem	= plb->AddString(ms_apszKnownAnimDescriptions[i]);
				if (pguiItem != NULL)
					{
					// Set item number.
					pguiItem->m_ulUserData	= i;
					// If this item is the current type . . .
					if (m_type == i)
						{
						// Select it.
						plb->SetSel(pguiItem);
						}
					}
				}

			plb->AdjustContents();

			if (DoGui(pguiRoot) == 1)
				{
				RGuiItem*	pguiSel	= plb->GetSel();
				if (pguiSel != NULL)
					{
					m_type	= (ItemType)pguiSel->m_ulUserData;
					if (m_type != Custom)
						{
						strcpy(m_szAnimBaseName, ms_apszKnownAnimBaseNames[m_type]);
						}
					else
						{
						strcpy(m_szAnimBaseName, peditName->m_szText);
						}
					}
				else
					{
					sResult	= 1;
					}

				// Get new rigid body name.
				strcpy(m_szAnimRigidName, peditRigidName->m_szText);

				// Get new child anim name.
				strcpy(m_szChildAnimBaseName, peditChildName->m_szText);

				// Get new rotation velocities.
				m_dExtRotVelY	= strtod(peditRotY->m_szText, NULL);
				m_dExtRotVelZ	= strtod(peditRotZ->m_szText, NULL);
				}
			else
				{
				sResult	= 1;
				}
			}
		else
			{
			TRACE("EditModify(): Missing GUI items in  %s.\n", EDIT_GUI_FILE);
			sResult	= -2;
			}
		}
	else
		{
		TRACE("EditModify(): Failed to load %s.\n", EDIT_GUI_FILE);
		sResult	= -1;
		}

	// If successful so far . . .
	if (sResult == 0)
		{
		// Init item3d
		sResult = Init();
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Init item3d
////////////////////////////////////////////////////////////////////////////////
short CItem3d::Init(void)									// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	m_lPrevTime			= m_pRealm->m_time.GetGameTime();

	// Get resources
	sResult = GetResources();

	// Prepare shadow (get resources and setup sprite).
	sResult	|= PrepareShadow();

	m_smash.m_bits		= CSmash::Misc;
	m_smash.m_pThing	= this;
	m_panimCur			= &m_anim;
	m_lAnimTime			= 0;

	// If the child sprite is functional . . .
	if (m_animChild.m_pmeshes && m_anim.m_ptransRigid)
		{
		// Attach to main sprite.
		m_sprite.AddChild(&m_spriteChild);
		}

	// No special flags.
	m_sprite.m_sInFlags = 0;

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Kill item3d
////////////////////////////////////////////////////////////////////////////////
void CItem3d::Kill(void)
	{
	// Free resources
	FreeResources();
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CItem3d::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// Free existing resources, if any.
	FreeResources();

	ASSERT(m_type > None && m_type < NumTypes);

	// If a known type . . .
	if (m_type != Custom)
		{
		strcpy(m_szAnimBaseName, ms_apszKnownAnimBaseNames[m_type]);
		}

	// Load main anim.
	sResult	= m_anim.Get(m_szAnimBaseName, m_szAnimRigidName, NULL, NULL, RChannel_LoopAtStart | RChannel_LoopAtEnd);

	// If there is a child name . . .
	if (m_szChildAnimBaseName[0])
		{
		// Load child naim.
		sResult	|= m_animChild.Get(m_szChildAnimBaseName, NULL, NULL, NULL, RChannel_LoopAtStart | RChannel_LoopAtEnd);
		}

	// Get shadow.
	sResult	|= rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(SHADOW_RES_NAME), &m_spriteShadow.m_pImage);

	// If errors . . .
	if (sResult)
		{
		// Remove just in case we're already in scene.  Can happen in editor after
		// a failed EditModify().
		m_pRealm->m_scene.RemoveSprite(&m_sprite);
		}
		
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
void CItem3d::FreeResources(void)
	{
	// If we have any anim resources . . .
	if (m_anim.m_pmeshes)
		{
		// Release resources for animation.
		m_anim.Release();
		}

	if (m_animChild.m_pmeshes)
		{
		// Release resources for animation.
		m_animChild.Release();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a msg_Shot.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CItem3d::OnShotMsg(			// Returns nothing.
	Shot_Message* pshotmsg)		// In:  Message to handle.
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an Explosion_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CItem3d::OnExplosionMsg(					// Returns nothing.
	Explosion_Message* pexplosionmsg)	// In:  Message to handle.
	{
	CThing3d::OnExplosionMsg(pexplosionmsg);

	m_state = State_BlownUp;

	// Send it spinning.
	m_dExtRotVelY	= GetRand() % 720;
	m_dExtRotVelZ	= GetRand() % 720;
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Burn_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CItem3d::OnBurnMsg(			// Returns nothing.
	Burn_Message* pburnmsg)		// In:  Message to handle.
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an ObjectDelete_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CItem3d::OnDeleteMsg(					// Returns nothing.
	ObjectDelete_Message* pdeletemsg)	// In:  Message to handle.
	{
	CThing3d::OnDeleteMsg(pdeletemsg);
	}

////////////////////////////////////////////////////////////////////////////////
// Setup object after creating it
// (virtual).
////////////////////////////////////////////////////////////////////////////////
short CItem3d::Setup(			// Returns 0 on success.
	short sX,						// In: Starting X position
	short sY,						// In: Starting Y position
	short sZ,						// In: Starting Z position
	ItemType type,					// In:  Known item type or Custom.
	char*	pszCustomBaseName /*= NULL*/,	// In:  Required if type == Custom.
													// Base name for custom type resources.
	U16	u16IdParentInstance /*= CIdBank::IdNil*/)	// In:  Parent instance ID.
	{
	short	sResult	= 0;

	m_dX	= sX;
	m_dY	= sY;
	m_dZ	= sZ;

	m_type	= type;

	if (pszCustomBaseName != NULL)
		{
		strcpy(m_szAnimBaseName, pszCustomBaseName);
		}

	m_u16IdParent	= u16IdParentInstance;

	sResult	= Init();

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
