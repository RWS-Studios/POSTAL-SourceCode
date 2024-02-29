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
// explode.cpp
// Project: Postal
//
// This module implements the CExplode weapon class which is an unguided
//	rocket missile.
// 
//
// History:
//		01/17/97 BRH	Started this weapon object.
//
//		01/23/97 BRH	Updated the time to GetGameTime rather than using
//							real time.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/06/97 BRH	Added RAnimSprite animation of the explosion for now.  
//							We are going to do an Alpha effect on the explosion, so
//							there are two animations, one of the image and one of
//							the Alpha information stored as a BMP8 animation.  When
//							the Alpha effect is ready, we will pass a frame from
//							each animation to a function to draw it.
//
//		02/06/97 BRH	Fixed problem with timer.  Since all Explosion objects
//							are using the same resource managed animation, they cannot
//							use the animation timer, they have to do the timing 
//							themselves.
//
//		02/07/97 BRH	Changed the sprite from CSprite2 to CSpriteAlpha2 for
//							the Alpha Blit effect.
//
//		02/10/97	JMI	rspReleaseResource() now takes a ptr to a ptr.
//
//		02/18/97 BRH	Changed the explosion to use the Channel Animations
//							rather than the RAnimSprite animations.
//
//		02/19/97 BRH	Empties the message queue in update so it doesn't fill up.
//							Also checks for collisions with other Characters and sends
//							them an explosion message.
//
//		02/23/97 BRH	Explosion now checks for all things that it blew up, not
//							just the first thing in the list.
//
//		02/23/97 BRH	Added Preload() function so that explosions are cached
//							by the resource manager before play begins.
//
//		02/24/97 BRH	Added Map3Dto2D so that the explosions were mapping the Y
//							coordinate also, before they were assuming they were on the
//							ground so explosions in the air weren't working correctly.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		03/05/97 BRH	Added center of and velocity of explosion to the explosion
//							message.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/17/97	JMI	Now includes CSmash::Item in the things that can be
//							exploded.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/20/97 BRH	Added an additional parameter to Setup to allow the
//							explosion to use different animations. In this case we 
//							want the standard explosion for rockets and barrels, and
//							a special one for the grenades.  This will allow you to
//							pass a number to incicate which animation to use.
//
//		04/21/97 BRH	Added second animation file and changed filename of second
//							asset to match.
//
//		04/23/97	JMI	CExplode no longer puts it's m_smash in the smashatorium.
//							Now sends messages to Characters, Miscs, Barrels, and 
//							Mines.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/07/97 BRH	Added smoke to the end of all explosions.
//
//		06/11/97 BRH	Pass the shooter ID on through the explosion message.
//
//		06/18/97 BRH	Changed over to using GetRandom()
//
//		06/26/97 BRH	Added CSmash::AlmostDead bits to the explosion check
//							so that writhing guys can be blown up.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/27/97	JMI	Changed to use Z position (i.e., X/Z plane) instead of
//							Y2 position (i.e., viewing plane) position for draw 
//							priority.
//
//		07/30/97	JMI	Added m_u16ExceptID (an ID to except when sending 
//							explosion messages).
//
//		08/15/97 BRH	Fixed problem with stationary smoke which had been
//							started under ground.
//
//		08/28/97 BRH	Now caches the grenade explosion animation as well in
//							its Preload function.
//
//		09/02/97	JMI	Now targets CSmash::Sentry too.
//
//		09/03/97	JMI	Now marks Civilian as a dont care bit.
//
////////////////////////////////////////////////////////////////////////////////
#define EXPLODE_CPP

#include "RSPiX.h"
#include <math.h>

#include "explode.h"
#include "dude.h"
#include "game.h"
#include "reality.h"
#include "fire.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

//#define IMAGE_FILE			"res\\explode.bmp"
//#define ANIM_FILE				"2d/explode.anm"
//#define ALPHA_FILE			"2d/explode_a.anm"

// Minimum elapsed time (in milliseconds)
//#define MIN_ELAPSED_TIME	10

#define AA_FILE				"explo.aan"
#define GE_FILE				"GExplo.aan"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!

// Let this auto-init to 0
short CExplode::ms_sFileCount;
short CExplode::ms_sBlastRadius = 30;
short CExplode::ms_sProjectVelocity = 180;


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CExplode::Load(									// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);

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

		// Load instance data.
		switch (ulFileVersion)
		{
			default:
			case 1:
				break;
		}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
		{
			// Get resources
			sResult = GetResources();
		}
		else
		{
			sResult = -1;
			TRACE("CExplode::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CExplode::Load(): CThing::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CExplode::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
{
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Save static data
			}
		}
	else
		{
		TRACE("CExplode::Save(): CThing::Save() failed.\n");
		}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CExplode::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CExplode::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CExplode::Suspend(void)
{
	m_sSuspend++;
}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CExplode::Resume(void)
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
void CExplode::Update(void)
{
	if (!m_sSuspend)
	{
		// Since we don't process any messages, empty the queue
		m_MessageQueue.Empty();

		// Get new time
		long lThisTime = m_pRealm->m_time.GetGameTime(); 
		
		if (m_lTimer < m_pAnimChannel->TotalTime())
		{
			m_lTimer += lThisTime - m_lPrevTime;
			m_lPrevTime = lThisTime;
		}
		else
		{
			short a;
			CFire* pSmoke;
			for (a = 0; a < 8; a++)
			{
				if (CThing::Construct(CThing::CFireID, m_pRealm, (CThing**) &pSmoke) == 0)
					pSmoke->Setup(m_dX - 4 + GetRandom() % 9, MAX(m_dY-20, 0.0), m_dZ - 4 + GetRandom() % 9, 4000, true, CFire::Smoke);
			}

			delete this;
			return;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CExplode::Render(void)
{
	CAlphaAnim* pAnim = (CAlphaAnim*) m_pAnimChannel->GetAtTime(m_lTimer);

	if (pAnim)
	{
		// No special flags
		m_sprite.m_sInFlags = 0; //CSprite::InXrayable;

		// Map from 3d to 2d coords
//		m_sprite.m_sX2 = m_dX + pAnim->m_sX;
//		m_sprite.m_sY2 = m_dZ + pAnim->m_sY;
		Map3Dto2D((short) (m_dX + pAnim->m_sX), (short) m_dY, (short) (m_dZ + pAnim->m_sY), &m_sprite.m_sX2, &m_sprite.m_sY2);

		// Priority is based on our Z position.
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from attribute map.
		m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

		// Copy the color info and the alpha channel to the Alpha Sprite
		m_sprite.m_pImage = &(pAnim->m_imColor);
		m_sprite.m_pimAlpha = &(pAnim->m_pimAlphaArray[0]);

		// temp
		short sTemp = pAnim->m_sNumAlphas;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);

	}
}

////////////////////////////////////////////////////////////////////////////////
// Setup - Called by the object that is creating this explosion to set its
//			  position and initial settings
////////////////////////////////////////////////////////////////////////////////

short CExplode::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ,												// In:  New z coord
	U16	u16ShooterID,									// In:  Who is responsible for this explosion
	short sAnim)											// In:  Which animation to use
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
   m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_lTimer = 0;
	m_u16ShooterID = u16ShooterID;

	// Load resources
	sResult = GetResources(sAnim);

		// Update sphere.
	m_smash.m_sphere.sphere.X			= m_dX;
	m_smash.m_sphere.sphere.Y			= m_dY;
	m_smash.m_sphere.sphere.Z			= m_dZ;
	m_smash.m_sphere.sphere.lRadius	= ms_sBlastRadius;

	// Update the smash.
	ASSERT (m_pRealm != NULL);
//	m_pRealm->m_smashatorium.Update(&m_smash);

	m_smash.m_bits		= 0;
	m_smash.m_pThing	= this;

	// See who we blew up and send them a message
	CSmash* pSmashed = NULL;
	GameMessage msg;
	msg.msg_Explosion.eType = typeExplosion;
	msg.msg_Explosion.sPriority = 0;
	msg.msg_Explosion.sDamage = 100;
	msg.msg_Explosion.sX = (short) m_dX;
	msg.msg_Explosion.sY = (short) m_dY;
	msg.msg_Explosion.sZ = (short) m_dZ;
	msg.msg_Explosion.sVelocity = ms_sProjectVelocity;
	msg.msg_Explosion.u16ShooterID = m_u16ShooterID;
	m_pRealm->m_smashatorium.QuickCheckReset(
		&m_smash, 
		CSmash::Character | CSmash::Misc | CSmash::Barrel | CSmash::Mine | CSmash::AlmostDead | CSmash::Sentry,
		CSmash::Good | CSmash::Bad | CSmash::Civilian,
		0);
	while (m_pRealm->m_smashatorium.QuickCheckNext(&pSmashed))
		{
		ASSERT(pSmashed->m_pThing);
		// If not the excepted thing . . .
		if (pSmashed->m_pThing->GetInstanceID() != m_u16ExceptID)
			{
			SendThingMessage(&msg, pSmashed->m_pThing);
			}
		}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CExplode::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_lTimer = m_pRealm->m_time.GetGameTime() + 1000;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();

	// Load resources
	sResult = GetResources();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CExplode::EditModify(void)
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CExplode::EditMove(									// Returns 0 if successfull, non-zero otherwise
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
void CExplode::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CExplode::EditRender(void)
{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CExplode::GetResources(short sAnim)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	if (sAnim == 0)
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(AA_FILE), &m_pAnimChannel, RFile::LittleEndian);
	else
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(GE_FILE), &m_pAnimChannel, RFile::LittleEndian);

	if (sResult != 0)
		TRACE("CExplosion::GetResources - Error getting explosion animation\n");

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CExplode::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	rspReleaseResource(&g_resmgrGame, &m_pAnimChannel);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching a CExplode 
//				 animation before play begins so that when an explosion occurs for
//				 the first time, there won't be a delay.
////////////////////////////////////////////////////////////////////////////////

short CExplode::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
	ChannelAA* pRes;
	short sResult = rspGetResource(&g_resmgrGame, prealm->Make2dResPath(AA_FILE), &pRes, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pRes);
	sResult = rspGetResource(&g_resmgrGame, prealm->Make2dResPath(GE_FILE), &pRes, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pRes);
	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
