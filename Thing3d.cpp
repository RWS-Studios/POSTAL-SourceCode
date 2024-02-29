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
// thing3d.cpp
// Project: Postal
//
// This module implements the CThing3d class which is the class of generic
// thing3d functionality for game thing3ds.
//
// History:
//
//		03/03/97	BRH,JMI	Started this generic thing3d object to reduce the
//							amount of redundant code.
//
//		03/17/97	JMI	Turned most of the CCharacter class into this CThing3d 
//							class.
//
//		03/18/97 BRH	Tuned the OnExplosionMsg to add more external velocity
//							on explosion.  The way it was, the object were barely
//							reacting to an explosion.
//
//		03/18/97	JMI	Now OnExplosionMsg, if there's an internal velocity,
//							sets the internal drag to the air drag.
//							Also, the external drag is now set to the air drag instead
//							of the surface drag in OnExplosionMsg.
//
//		03/18/97	JMI	Now Render() does only the necessities, if there is a
//							parent.  Also, added DetachChild().
//
//		03/19/97	JMI	Added m_dExtRotVelY, m_dExtRotVelZ, and m_dRotZ.
//							GetNewPosition() now updates m_dRot and m_dRotZ using
//							velocities m_dExtRotVelY and m_dExtRotVelZ.
//							Also, DetachChild() now returns a pointer to the detached
//							child.
//							Render() now uses m_dRotZ through m_trans.Rz().
//							WhileBlownUp() now sets the m_dExtHorzDrag to 0 when it
//							detects a horizontal collision with terrain.
//
//		04/02/97	JMI	Added #include for fire.h.
//
//		04/10/97 BRH	Changed GetAttributes to two functions for the new
//							multi layred attribute maps.  Now there is a 
//							GetFloorAttributes and GetEffectAttributes that do
//							lookups on the two different attribute maps.
//
//		04/21/97	JMI	Made MakeValidPosition() virtual.
//
//		04/22/97 BRH	Changed default return for GetFloorAttribute to return
//							zero height rather than max height when off of the map.
//
//		05/14/97	JMI	Added generic form of DetachChild(...).
//							Also, added generic PositionChild(...).
//
//		05/19/97 BRH	Added StateNames array so that the names of the states
//							can be shown for thought balloons and refernced for
//							the logic tables.
//
//		05/20/97 BRH	Added strings for the new victim states.
//
//		05/21/97 BRH	Added ShootRun state for an alternative to the shoot
//							animation.  They will use this state when shooting while
//							running.
//
//		05/29/97	JMI	GetNewPosition() was doing Z backwards for external
//							forces.  Fixed.
//							OnShotMsg() now provides momentum from the bullets.
//
//		05/29/97	JMI	Changed m_pRealm->m_pHeightMap->GetVal() calls to
//							m_pRealm->GetFloorMapValue() and 
//							m_pRealm->m_pAttrMap->GetVal() calls to 
//							m_pRealm->GetEffectAttribute().
//							Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//							Also, added a GetLayer().
//
//		05/30/97	JMI	Added a generic function for adding a force vector to
//							the external force.
//
//		06/05/97	JMI	Removed m_sHitPoints and added a CStockPile, m_stockpile,
//							instead.
//
//		06/06/97 BRH	Added additional state descriptions for the new states
//							that had been added a while ago.
//
//		06/12/97 BRH	Set the small fire that burns on the guy when he is
//							on fire to use his ID for the shooter.
//
//		06/13/97	JMI	Added State_ObjectReleased.
//
//		06/15/97	JMI	Now AddForceVector() makes sure there's a force before
//							setting the drag.
//
//		06/17/97	JMI	Forgot to zero m_dDrag in WhileBlownUp().  Fixed.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/25/97 BRH	Added shadow sprite rendering to the Render() function.  
//							Also added PrepareShadow function to turn on the shadow
//							sprite and load the default shadow resource if it doesn't
//							already have a resource loaded.
//
//		06/25/97 BRH	Set the shadow sprite to hidden when the main item is
//							on the ground since the characters looked funny when
//							they were walking around.  Also set the layer of the
//							shadow to the same layer as the main item rather than
//							having it check its own points for layer information.
//
//		06/25/97	JMI	Now WhileBlownUp() sets m_bAboveTerrain appropriately.
//
//		06/27/97	JMI	Now uses TransformPtsToRealm() in EditRect().
//
//		06/29/97	JMI	Now EditHotSpot() merely gets the difference between the
//							EditRect() and the 2D mapping of the 3D Realm position.
//
//		06/30/97	JMI	Now maps the Z to 3D when loading fileversions previous to
//							24.
//
//		07/01/97	JMI	Now GetFloorAttributes() uses MapY2DtoY3D() to map the
//							height into the realm coord system.
//
//		07/01/97	JMI	Changed DetachChild() and PositionChild() to receive the
//							rigid body transform as a parameter rather than assume it
//							is m_panimCur->m_transRigid.
//							Also, added GetLinkPoint().
//
//		07/01/79	JMI	Now GetFloorAttributes() uses MapAttribHeight() instead
//							of MapY2DtoY3D().
//
//		08/09/97	JMI	Converted from macro MAX_STEPUP_THRESHOLD to enum 
//							MaxStepUpThreshold macro.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/12/97 BRH	Added additional text strings for the new states.
//
//		07/17/97 BRH	Added DelayShoot state description.
//
//		07/18/97	JMI	Added PlaySample() that will hook existing PlaySample()
//							calls in CThing3d derived classes and map them to 
//							PlaySample() and, if they don't specify a volume, it will
//							use this object's distance to the ear.
//
//		07/21/97 BRH	Fixed bug in WhileShot() that caused the state to never
//							exit.  When the ostrish started using this function, 
//							it never got out of the shot state.  Most other objects
//							never use this function.
//
//		07/21/97	JMI	Now checks upper bound on m_sAlphaLevel of shadow sprite.
//
//		07/27/97	JMI	Changed to use Z position (i.e., X/Z plane) instead of
//							Y2 position (i.e., viewing plane) position for draw 
//							priority.
//
//		08/01/97 BRH	Added description for new state AvoidFire and DangerNear
//
//		08/02/97 BRH	Added virtual OnHelpMsg function and added the case to
//							ProcessMessage.
//
//		08/06/97 JRD	Added local scaling to render process
//
//		08/08/97 BRH	Added march state description.
//
//		08/11/97 BRH	Added Walk next state description.
//
//		08/18/97	JMI	Moved StartAnim() from CDude to CThing3d so more things
//							could use it.
//
//		08/18/97	JMI	Added m_sLayerOverride which directs Render() to use the
//							specified layer rather than the one based on the 
//							attributes.
//
//		08/24/97	JMI	Changed ms_apt3dAttribCheck to ms_apt2dAttribCheckMedium[]
//							and added ms_apt2dAttribCheckSmall[], 
//							ms_apt2dAttribCheckLarge[], and ms_apt2dAttribCheckHuge[].
//							Also, added pointer so that each object can choose one,
//							m_pap2dAttribCheckPoints.
//							Also, changed type of these arrays to local type Point2D.
//							Also, added two additional points to each of the arrays.
//
//		08/28/97	JMI	Now EditRect() checks to make sure the current animation
//							actually has its components.
//
//		08/28/97 BRH	Added virtual put me down message handler.
//
//		09/02/97	JMI	Now sets the fire's starter ID so it can tell us correctly
//							who to credit for our burn damage in the case that it is
//							our internal flame.
//
////////////////////////////////////////////////////////////////////////////////
#define THING3D_CPP

#include "RSPiX.h"
#include <math.h>

#include "Thing3d.h"
#include "reality.h"
#include "fire.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SHADOW_FILE		"shadow.img"

#define MaxForeVel				80.0
#define MaxBackVel				-60.0

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((GetRand() % sway) - sway / 2)

// Amount of time fire will last.
#define BURN_DURATION			5000	// In ms.

// Light level for burnt thing3d.
#define BURNT_BRIGHTNESS		-40	// -128 to 127.

// Sets a value pointed to if ptr is not NULL.
#define SET(pval, val)					((pval != NULL) ? *pval = val : val)

// Multiply damage by this to get velocity (for absorbing momentum of
// damage cause (e.g., bullets, etc.) ).
#define DAMAGE2VEL_RATIO				1.0

// Terminates arrays of attrib check points.
#define ATTRIB_CHECK_TERMINATOR		-32768

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////


double	CThing3d::ms_dDefaultSurfaceDrag	= 220.0;	// Default drag along surfaces.     
double	CThing3d::ms_dDefaultAirDrag		= 30.0;	// Default drag due to air friction.
short		CThing3d::ms_sBurntBrightness		= -40;	// Brightness level after being burnt

/// These are the points that are checked on the attrib map /////////////////////
// These points are relative to the thing's origin.
// These are arrays of pts to be checked on the attribute map for various
// size of CThing3d derived things.
const CThing3d::Point2D		CThing3d::ms_apt2dAttribCheckSmall[]	=
	{
	//	+	+	+
	//	+	x	+
	//	+	+	+
		{ -2,	-2,	},
		{ 0,	-2,	},
		{ 2,	-2,	},
		{ -2,	0,		},
		{	2,	0,		},
		{ -2,	2,		},
		{ 0,	2,		},
		{ 2,	2,		},
		// Add more points above this one.
		{ ATTRIB_CHECK_TERMINATOR, }
	};

const CThing3d::Point2D		CThing3d::ms_apt2dAttribCheckMedium[]	=
	{
	//	+	+	+
	//	+	x	+
	//	+	+	+
		{ -6,	-6,	},
		{ 0,	-6,	},
		{ 6,	-6,	},
		{ -6,	0,		},
		{	6,	0,		},
		{ -6, 6,		},
		{ 0,	6,		},
		{ 6,	6,		},
		// Add more points above this one.
		{ ATTRIB_CHECK_TERMINATOR, }
	};

const CThing3d::Point2D		CThing3d::ms_apt2dAttribCheckLarge[]	=
	{
	//	+	+	+
	//	+	x	+
	//	+	+	+
		{ -12,	-12,	},
		{ 0,		-12,	},
		{ 12,		-12,	},
		{ -12,	0,		},
		{	12,	0,		},
		{ -12,	12,	},
		{ 0,		12,	},
		{ 12,		12,	},
		// Add more points above this one.
		{ ATTRIB_CHECK_TERMINATOR, }
	};

const CThing3d::Point2D		CThing3d::ms_apt2dAttribCheckHuge[]		=
	{
	//	+	+	+
	//	+	x	+
	//	+	+	+
		{ -48,	-48,	},
		{ 0,		-48,	},
		{ 48,		-48,	},
		{ -48,	0,		},
		{	48,	0,		},
		{ -48,	48,	},
		{ 0,		48,	},
		{ 48,		48,	},
		// Add more points above this one.
		{ ATTRIB_CHECK_TERMINATOR, }
	};


char* CThing3d::ms_apszStateNames[] = 
{
	"State_Idle",
	"State_Shot",
	"State_Blownup",
	"State_Burning",
	"State_Die",
	"State_Dead",
	"State_RunOver",
	"State_Vomit",
	"State_Suicide",
	"State_Persistent",
	"State_Stand",
	"State_Throw",
	"State_ThrowRelease",
	"State_ThrowFinish",
	"State_ThrowDone",
	"State_Run",
	"State_Shooting",
	"State_RunAndShoot",
	"State_Strafe",
	"State_StrafeAndShoot",
	"State_Launch",
	"State_LaunchRelease",
	"State_LaunchFinish",
	"State_LaunchDone",
	"State_GetUp",
	"State_Duck",
	"State_Rise",
	"State_Jump",
	"State_JumpForward",
	"State_Land",
	"State_LandForward",
	"State_Fall",
	"State_March",
	"State_Mingle",
	"State_Panic",
	"State_Load",
	"State_Patrol",
	"State_Shoot",
	"State_Wait",
	"State_Stop",
	"State_Hunt",
	"State_HuntNext",
	"State_Engage",
	"State_Guard",
	"State_Reposition",
	"State_Retreat",
	"State_PopBegin",
	"State_PopWait",
	"State_Popout",
	"State_PopShoot",
	"State_RunShootBegin",
	"State_RunShootRun",
	"State_RunShootWait",
	"State_RunShoot",
	"State_Delete",
	"State_Writhing",
	"State_Execute",
	"State_PutDown",
	"State_PickUp",
	"State_PutOutFire",
	"State_Walk",
	"State_Hide",
	"State_MoveNext",
	"State_PositionSet",
	"State_PositionMove",
	"State_HideBegin",
	"State_ShootRun",
	"State_HuntHold",
	"State_Climb",
	"State_ObjectReleased",
	"PanicBegin",
	"PanicContinue",
	"WalkBegin",
	"WalkContinue",
	"DelayShoot",
	"AvoidFire",
	"Helping",
	"MarchNext",
	"WalkNext",
	"Please Add State Description",
	"Please Add State Description",
	"Please Add State Description",
	"Please Add State Description",
	"Please Add State Description",
	"Please Add State Description",
	"Please Add State Description",
	"Please Add State Description",
	"Please Add State Description",
};

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CThing3d::Load(									// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
	{
	// Call the CThing base class load to get the instance ID
	short sResult = CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
	if (sResult == 0)
		{
		// Load object data
		switch (ulFileVersion)
			{
			default:
			case 16:
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				pFile->Read(&m_dRot);
				sResult	= m_stockpile.Load(pFile, ulFileVersion);
				break;

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
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				pFile->Read(&m_stockpile.m_sHitPoints);
				pFile->Read(&m_dRot);
				break;

			case 2:
			case 1:
				pFile->Read(&m_dX);
				pFile->Read(&m_dY);
				pFile->Read(&m_dZ);
				// PATCH:  To make the CCharacter continue to work.  We need to
				// read the space that is occupied by the weapon type for CCharacter.
				// In file format versions 3 and above, this is written later in
				// the file.
				CThing::ClassIDType	idWeaponDummy;
				pFile->Read(&idWeaponDummy);
				// END PATCH.
				pFile->Read(&m_stockpile.m_sHitPoints);
				pFile->Read(&m_dRot);
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
			// Success.
			}
		else
			{
			sResult = -1;
			TRACE("CThing3d::Load(): Error reading from file!\n");
			}
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CThing3d::Save(									// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to save to
	short sFileCount)										// In:  File count (unique per file, never 0)
	{
	// Call the base class save to save the u16InstanceID
	short	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save object data
		pFile->Write(&m_dX);
		pFile->Write(&m_dY);
		pFile->Write(&m_dZ);
		pFile->Write(&m_dRot);
		sResult	= m_stockpile.Save(pFile);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CThing3d::Startup(void)						// Returns 0 if successfull, non-zero otherwise
	{
	// Init other stuff
	m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_lAnimPrevUpdateTime	= m_lPrevTime;

	// No special flags
	m_sprite.m_sInFlags = 0;

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CThing3d::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CThing3d::Suspend(void)
	{
	if (m_sSuspend == 0)
		{
		// Store current delta so we can restore it.
		long	lCurTime				= m_pRealm->m_time.GetGameTime();
		m_lPrevTime					= lCurTime - m_lPrevTime;
		m_lAnimPrevUpdateTime	= lCurTime - m_lAnimPrevUpdateTime;
		}

	m_sSuspend++;
	}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CThing3d::Resume(void)
	{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
	if (m_sSuspend == 0)
		{
		long	lCurTime				= m_pRealm->m_time.GetGameTime();
		m_lPrevTime					= lCurTime - m_lPrevTime;
		m_lAnimPrevUpdateTime	= lCurTime - m_lAnimPrevUpdateTime;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CThing3d::Render(void)
	{
	U16	u16CombinedAttributes;
	short	sLightTally;
	GetEffectAttributes(m_dX, m_dZ, &u16CombinedAttributes, &sLightTally);

	// Brightness.
	m_sprite.m_sBrightness	= m_sBrightness + sLightTally * gsGlobalBrightnessPerLightAttribute;

	// If no parent . . .
	if (m_u16IdParent == CIdBank::IdNil)
		{
		// Reset transform back to start to set absolute rather than cummulative rotation
		m_trans.Make1();

		m_trans.Scale(m_dScaleX,m_dScaleY,m_dScaleZ);
		m_trans.Ry(rspMod360(m_dRot) );
		m_trans.Rz(rspMod360(m_dRotZ) );

		// Map from 3d to 2d coords
		Map3Dto2D((short) m_dX, (short) m_dY, (short) m_dZ, &m_sprite.m_sX2, &m_sprite.m_sY2);

		// If no layer override . . .
		if (m_sLayerOverride < 0)
			{
			// Layer should be based on info from attribute map.
			GetLayer(m_dX, m_dZ, &(m_sprite.m_sLayer) );
			}
		else
			{
			// Use the layer override.
			m_sprite.m_sLayer	= m_sLayerOverride;
			}

		// Priority is based on our Z position.
		m_sprite.m_sPriority = m_dZ;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);
		
		// Set transform.
		m_sprite.m_ptrans = &m_trans;

		// If the item is above the ground, show the shadow sprite, else hide it.
		if (m_bAboveTerrain)
			m_spriteShadow.m_sInFlags &= ~CSprite::InHidden;
		else
			m_spriteShadow.m_sInFlags |= CSprite::InHidden;

		// If the shadow is enabled
		if (!(m_spriteShadow.m_sInFlags & CSprite::InHidden) && m_spriteShadow.m_pImage != NULL)
			{
			// Get the height of the terrain from the attribute map
			short sY = m_pRealm->GetHeight((short) m_dX, (short) m_dZ);
			// Map from 3d to 2d coords
			Map3Dto2D(m_dX, (double) sY, m_dZ, &(m_spriteShadow.m_sX2), &(m_spriteShadow.m_sY2) );
			// Offset hotspot to center of image.
			m_spriteShadow.m_sX2 -= m_spriteShadow.m_pImage->m_sWidth / 2;
			m_spriteShadow.m_sY2 -= m_spriteShadow.m_pImage->m_sHeight / 2;

			// Priority is based on bottom edge of sprite on X/Z plane!
			m_spriteShadow.m_sPriority = MAX(m_sprite.m_sPriority - 1, 0);//m_dZ;

			// Layer should be based on info we get from attribute map.
			m_spriteShadow.m_sLayer = m_sprite.m_sLayer; //CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

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

			// Update sprite in scene
			m_pRealm->m_scene.UpdateSprite(&m_spriteShadow);
			}
		}

	ASSERT(m_panimCur != NULL);

	m_sprite.m_pmesh = (RMesh*) m_panimCur->m_pmeshes->GetAtTime(m_lAnimTime);
	m_sprite.m_psop = (RSop*) m_panimCur->m_psops->GetAtTime(m_lAnimTime);
	m_sprite.m_ptex = (RTexture*) m_panimCur->m_ptextures->GetAtTime(m_lAnimTime);
	m_sprite.m_psphere = (RP3d*) m_panimCur->m_pbounds->GetAtTime(m_lAnimTime);
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CThing3d::EditRender(void)
	{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CThing3d::EditNew(								// Returns 0 if successfull, non-zero otherwise
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
short CThing3d::EditModify(void)
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CThing3d::EditMove(							// Returns 0 if successfull, non-zero otherwise
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
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CThing3d::EditRect(RRect* pRect)
	{
	if (m_panimCur != NULL)
		{
		if (m_panimCur->m_psops)
			{
			// Get current sphere.
			RP3d		apt3dSrc[2];	// Center and point on outside of
											// bounding sphere in "Randy" coords.
			RP3d		apt3dDst[2];	// Dst for above in Postal coords.

			RTransform	trans;
			apt3dSrc[0]		= *((RP3d*) m_panimCur->m_pbounds->GetAtTime(0));
			
			apt3dSrc[1].x	= apt3dSrc[0].x + apt3dSrc[0].w;
			apt3dSrc[1].y	= apt3dSrc[0].y + apt3dSrc[0].w;
			apt3dSrc[1].z	= apt3dSrc[0].z + apt3dSrc[0].w;

			apt3dSrc[0].w	= 1;
			apt3dSrc[1].w	= 1;

			m_pRealm->m_scene.TransformPtsToRealm(&trans, apt3dSrc, apt3dDst, 2);
			m_sprite.m_sRadius	= sqrt(ABS2(
				apt3dDst[1].x - apt3dDst[0].x, 
				apt3dDst[1].y - apt3dDst[0].y, 
				apt3dDst[1].z - apt3dDst[0].z 
				) );

			Map3Dto2D(
				m_dX + apt3dDst[0].x, 
				m_dY + apt3dDst[0].y, 
				m_dZ + apt3dDst[0].z, 
				&(pRect->sX), &(pRect->sY) );
			}
		}
	else
		{
		Map3Dto2D(m_dX, m_dY, m_dZ, &(pRect->sX), &(pRect->sY) );

		m_sprite.m_sRadius	= 10;	// **FUDGE.
		}

	pRect->sX -= m_sprite.m_sRadius;
	pRect->sY -= m_sprite.m_sRadius;
	pRect->sW = m_sprite.m_sRadius * 2;
	pRect->sH = m_sprite.m_sRadius * 2;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::EditHotSpot(		// Returns nothiing.
	short*	psX,						// Out: X coord of 2D hotspot relative to
											// EditRect() pos.
	short*	psY)						// Out: Y coord of 2D hotspot relative to
											// EditRect() pos.
	{
	// Get rectangle.
	RRect	rc;
	EditRect(&rc);
	// Get 2D hotspot.
	short	sX;
	short	sY;
	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&sX,
		&sY);

	// Get relation.
	*psX	= sX - rc.sX;
	*psY	= sY - rc.sY;
	}

//---------------------------------------------------------------------------
// Useful generic thing3d state-specific functionality.
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Implements basic one-time functionality for each time State_Shot is
// entered.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::OnShot(void)
	{
	}


////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while shot and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CThing3d::WhileShot(void)	// Returns true until state is complete
	{
	bool bStatePersists = true;

	if (m_lAnimTime > m_panimCur->m_psops->TotalTime() || m_stockpile.m_sHitPoints <= 0)
		bStatePersists = false;

	return bStatePersists;
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being blown up and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CThing3d::WhileBlownUp(void)	// Returns true until state is complete.
	{
	bool	bStatePersists	= true;	// Assume not done.
	double	dNewX, dNewY, dNewZ;

	// Get time from last call in seconds.
	long		lCurTime	= m_pRealm->m_time.GetGameTime();
	double	dSeconds	= double(lCurTime - m_lPrevTime) / 1000.0;

	// Update Velocities ////////////////////////////////////////////////////////
	UpdateVelocities(dSeconds, MaxForeVel, MaxBackVel);
	
	// Get New Position /////////////////////////////////////////////////////////
	GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);

	// Validate New Position ////////////////////////////////////////////////////

	// Get attribute at new location.
	// Get height at new position.
	USHORT	usAttrib;
	short		sHeight;
	GetFloorAttributes(dNewX, dNewZ, &usAttrib, &sHeight);

	// If it was above the ground last time and is now below the ground, it must have
	// hit the ground and the blown up state is complete
	if (dNewY < sHeight && m_dY >= sHeight)
	{
		dNewY = sHeight;
		// Make sure its done with current animation also
		if (m_lAnimTime > m_panimCur->m_psops->TotalTime())
			bStatePersists = false;
		
		// No longer above the terrain.
		m_bAboveTerrain	= false;
	}
	else
	{
		// We should be above the terrain.
		m_bAboveTerrain	= true;

		// If the new height is greater than the current and previous height, then it must
		// have hit a wall and should continue to fall against the wall (update Y but not
		// X or Z - which is kind of cheating since it may be free to move in X or Z unless
		// it is in a corner
		if (usAttrib & REALM_ATTR_NOT_WALKABLE || (dNewY < sHeight && m_dY < sHeight))
		{
			// Reset x and z to previous positiion
			dNewX = m_dX;
			dNewZ = m_dZ;
			// Stop moving horizontally.
			m_dVel			= 0.0;
			m_dAcc			= 0.0;
			m_dExtHorzVel	= 0.0;
			m_dExtHorzDrag	= 0.0;
			m_dDrag			= 0.0;
			
			// Make sure it's not underground at this position with the new y position.
			GetFloorAttributes(dNewX, dNewZ, &usAttrib, &sHeight);
			if (dNewY <= sHeight)
				{
				// Get out of the ground.
				dNewY	= sHeight;
				// We are not above the terrain.
				m_bAboveTerrain	= false;
				}
		}
	}

	m_dX = dNewX;
	m_dY = dNewY;
	m_dZ = dNewZ;

	return bStatePersists;
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being on fire and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CThing3d::WhileBurning(void)	// Returns true until state is complete.
	{
	bool	bStatePersists	= true;	// Assume not done.

	// See if it is destroyed/dead yet.
	if (m_stockpile.m_sHitPoints < 0)
		bStatePersists = false;

	return bStatePersists;
	}

////////////////////////////////////////////////////////////////////////////////
// Implements basic functionality while being run over and returns true
// until the state is completed.
////////////////////////////////////////////////////////////////////////////////
bool CThing3d::WhileRunOver(void)	// Returns true until state is complete.
	{
	bool	bStatePersists	= true;	// Assume not done.

	return bStatePersists;
	}

//---------------------------------------------------------------------------
// Useful generic thing3d functionality.
//---------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// This inline updates a specified velocity with a specified drag over a
// specified time.
////////////////////////////////////////////////////////////////////////////////
inline
bool UpdateVelocity(		// Returns true if velocity reaches zero because of the
								// supplied accelration, false otherwise.
	double* pdVel,			// In:  Initial velocity.
								// Out: New velocity.
	double* pdDeltaVel,	// Out: Delta velocity.
	double dAcc,			// In:  Acceleration.
	double dSeconds)		// In:  Elapsed time in seconds.
	{
	bool	bAcceleratedToZero	= false;

	double	dVelPrev	= *pdVel;
	*pdDeltaVel			= dAcc * dSeconds;
	*pdVel				+= *pdDeltaVel;

	// I think this can be consdensed into a subtraction and one or two comparisons,
	// but I'm not sure that's really faster than the max 3 comparisons here.
	// If previously traveling forward . . .
	if (dVelPrev > 0.0)
		{
		// Passing 0 is considered at rest . . .
		if (*pdVel < 0.0)
			{
			// Update delta.
			*pdDeltaVel	-= *pdVel;
			// Zero velocity.
			*pdVel	= 0.0;
			}
		}
	else
		{
		// If previously traveling backward . . .
		if (dVelPrev < 0.0)
			{
			// Passing 0 is considered at rest . . .
			if (*pdVel > 0.0)
				{
				// Update delta.
				*pdDeltaVel	-= *pdVel;
				// Zero velocity.
				*pdVel	= 0.0;
				}
			}
		}

	// If velocity is now zero . . .
	if (*pdVel == 0.0)
		{
		// If drag opposed the previous velocity . . .
		if ((dVelPrev > 0.0 && dAcc < 0.0) || (dVelPrev < 0.0 && dAcc > 0.0))
			{
			// Drag has achieved its goal.
			bAcceleratedToZero = true;
			}
		}

	return bAcceleratedToZero;
	}

////////////////////////////////////////////////////////////////////////////////
// Applies accelerations to velocities keeping them within specified
// limits.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::UpdateVelocities(
	double	dSeconds,		// Seconds since last update.
	double	dMaxForeVel,	// Maximum forward velocity.
	double	dMaxBackVel)	// Maximum backward velocity.
	{
	double	dVelPrev;
	
	///////////////////////// Internal Velocity /////////////////////////////////
	// Note: This does not use UpdateVelocity() b/c there are two accelerations
	// acting on the one velocity.  Have to investigate what problems could
	// arise.
	dVelPrev			= m_dVel;
	m_dDeltaVel		= (m_dAcc + m_dDrag) * dSeconds;
	m_dVel			+= m_dDeltaVel;

	// If previously traveling forward . . .
	if (dVelPrev > 0.0)
		{
		// Passing 0 is considered at rest . . .
		if (m_dVel < 0.0)
			{
			m_dVel	= 0.0;
			}
		else
			{
			if (m_dVel > dMaxForeVel)
				{
				m_dVel	= dMaxForeVel;
				}
			}
		}
	else
		{
		// If previously traveling backward . . .
		if (dVelPrev < 0.0)
			{
			// Passing 0 is considered at rest . . .
			if (m_dVel > 0.0)
				{
				m_dVel	= 0.0;
				}
			else
				{
				if (m_dVel < dMaxBackVel)
					{
					m_dVel	= dMaxBackVel;
					}
				}
			}
		}

	// If velocity is now zero . . .
	if (m_dVel == 0.0)
		{
		// If drag opposed the previous velocity . . .
		if ((dVelPrev > 0.0 && m_dDrag < 0.0) || (dVelPrev < 0.0 && m_dDrag > 0.0))
			{
			// Drag has achieved its goal.
			m_dDrag = 0.0;
			}
		}

	// Update delta (we may have capped or trimmed velocity against dMax*Vel).
	m_dDeltaVel	= m_dVel - dVelPrev;
	
	///////////////////////// External Velocity /////////////////////////////////

	// If velocity is now zero b/c of drag . . .
	if (UpdateVelocity(&m_dExtHorzVel, &m_dExtHorzDeltaVel, m_dExtHorzDrag, dSeconds) == true)
		{
		// Drag has achieved its goal.
		m_dExtHorzDrag = 0.0;
		}
	
	///////////////////////// Vertical Velocity /////////////////////////////////
	UpdateVelocity(&m_dExtVertVel, &m_dExtVertDeltaVel, g_dAccelerationDueToGravity, dSeconds);
	}

////////////////////////////////////////////////////////////////////////////////
// Applies velocities to positions.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::GetNewPosition(	// Returns nothing.
	double*	pdNewX,					// Out: New x position.
	double*	pdNewY,					// Out: New y position.
	double*	pdNewZ,					// Out: New z position.
	double	dSeconds)				// Seconds since last update.
	{
	// Couldn't decide whether this should be in UpdateVelocities() or
	// GetNewPosition().
	///////////////////////////// Rotations /////////////////////////////////////
	m_dRot	= rspMod360(m_dRot + m_dExtRotVelY * dSeconds);
	m_dRotZ	= rspMod360(m_dRotZ + m_dExtRotVelZ * dSeconds);

	// Make sure rotation is w/i bounds for COSQ and SINQ arrays.
	// Note:  If we keep the above adjustments on m_dRot, we won't
	// need this rspMod360().
	short	sRot	= rspMod360(m_dRot);
	m_dRot		= sRot;

	// Make sure external force rotation is w/i bounds for COSQ and SINQ arrays.
	short	sExtHorzRot	= rspMod360(m_dExtHorzRot);
	m_dExtHorzRot		= sExtHorzRot;

	double dDistance;

	// Apply internal velocity.
	dDistance	= (m_dVel - m_dDeltaVel / 2) * dSeconds;
	*pdNewX	= m_dX + COSQ[sRot] * dDistance;
	*pdNewZ	= m_dZ - SINQ[sRot] * dDistance;
	// Apply external velocity.
	dDistance	= (m_dExtHorzVel - m_dExtHorzDeltaVel / 2) * dSeconds;
	*pdNewX	+= COSQ[sExtHorzRot] * dDistance;
	*pdNewZ	+= -SINQ[sExtHorzRot] * dDistance;
	// Apply external vertical velocity.
	dDistance	= (m_dExtVertVel - m_dExtVertDeltaVel / 2) * dSeconds;
	*pdNewY	= m_dY + dDistance;
	}

//#ifdef MOBILE
////////////////////////////////////////////////////////////////////////////////
// Applies velocities to positions with angle
////////////////////////////////////////////////////////////////////////////////
void CThing3d::GetNewPositionAngle(				// Returns nothing.
				double*	pdNewX,					// Out: New x position.
				double*	pdNewY,					// Out: New y position.
				double*	pdNewZ,					// Out: New z position.
				double	dSeconds,               // Seconds since last update.
				double  dAngle
				)
{
	// Couldn't decide whether this should be in UpdateVelocities() or
		// GetNewPosition().
		///////////////////////////// Rotations /////////////////////////////////////
	    //dAngle	= rspMod360(dAngle + m_dExtRotVelY * dSeconds);
		m_dRot	= rspMod360(m_dRot + m_dExtRotVelY * dSeconds);
		m_dRotZ	= rspMod360(m_dRotZ + m_dExtRotVelZ * dSeconds);

		// Make sure rotation is w/i bounds for COSQ and SINQ arrays.
		// Note:  If we keep the above adjustments on m_dRot, we won't
		// need this rspMod360().
		short	sRot	= rspMod360(m_dRot);
		m_dRot		= sRot;


		sRot	= rspMod360(dAngle);

		// Make sure external force rotation is w/i bounds for COSQ and SINQ arrays.
		short	sExtHorzRot	= rspMod360(m_dExtHorzRot);
		m_dExtHorzRot		= sExtHorzRot;

		double dDistance;

		// Apply internal velocity.
		dDistance	= (m_dVel - m_dDeltaVel / 2) * dSeconds;
		*pdNewX	= m_dX + COSQ[sRot] * dDistance;
		*pdNewZ	= m_dZ - SINQ[sRot] * dDistance;
		// Apply external velocity.
		dDistance	= (m_dExtHorzVel - m_dExtHorzDeltaVel / 2) * dSeconds;
		*pdNewX	+= COSQ[sExtHorzRot] * dDistance;
		*pdNewZ	+= -SINQ[sExtHorzRot] * dDistance;
		// Apply external vertical velocity.
		dDistance	= (m_dExtVertVel - m_dExtVertDeltaVel / 2) * dSeconds;
		*pdNewY	= m_dY + dDistance;
}
//#endif

////////////////////////////////////////////////////////////////////////////////
// Determines if supplied position is valid tweaking it if necessary.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
bool CThing3d::MakeValidPosition(	// Returns true, if new position was valid.
												// Returns false, if could not reach new position.
	double*	pdNewX,						// In:  x position to validate.
												// Out: New x position.
	double*	pdNewY,						// In:  y position to validate.
												// Out: New y position.
	double*	pdNewZ,						// In:  z position to validate.
												// Out: New z position.
	short	sVertTolerance /*= 0*/)		// Vertical tolerance.
	{
	bool bValidatedPosition	= false;	// Assume failure.

	// Get attribute at new location.
	// Get height at new position.
	USHORT	usAttrib;
	short		sHeight;
	GetFloorAttributes(*pdNewX, *pdNewZ, &usAttrib, &sHeight);

	// If too big a height difference or completely not walkable . . .
	if (usAttrib & REALM_ATTR_NOT_WALKABLE
		|| (sHeight - *pdNewY > sVertTolerance) )// && m_bAboveTerrain == false && m_dExtHorzVel == 0.0))
		{
		// Restore previous X/Z position.
		*pdNewX	= m_dX;
		*pdNewZ	= m_dZ;
		bValidatedPosition	= true;

		// Get height in that spot.
		GetFloorAttributes(*pdNewX, *pdNewZ, &usAttrib, &sHeight);
		}
	else
		{
		// Note that we succeeded in making new position valid.
		bValidatedPosition	= true;
		}

	// If we're gonna be at or below ground level . . .
	if (sHeight >= *pdNewY)
		{
		// Get outta there!
		*pdNewY	= sHeight;
		// Update vertical delta.
		m_dExtVertDeltaVel	+= -m_dExtVertVel;
		// Reset vertical velocity.
		m_dExtVertVel	= 0.0;
		
		m_bAboveTerrain	= false;
		}
	else
		{
		m_bAboveTerrain	= true;
		}

	return bValidatedPosition;
	}


////////////////////////////////////////////////////////////////////////////////
// Deluxe does all for updating position.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::DeluxeUpdatePosVel(	// Returns nothing.
	double dSeconds)						// In:  Duration since last update in seconds.
	{
	double	dNewX, dNewY, dNewZ;

	// Update Velocities ////////////////////////////////////////////////////////

	UpdateVelocities(dSeconds, MaxForeVel, MaxBackVel);
	
	// Get New Position /////////////////////////////////////////////////////////

	GetNewPosition(&dNewX, &dNewY, &dNewZ, dSeconds);

	// Validate New Position ////////////////////////////////////////////////////

	if (MakeValidPosition(&dNewX, &dNewY, &dNewZ, MaxStepUpThreshold) == true)
		{
		// Update Values /////////////////////////////////////////////////////////

		m_dX	= dNewX;
		m_dY	= dNewY;
		m_dZ	= dNewZ;

		UpdateFirePosition();
		}
	else
		{
		// Restore Values ////////////////////////////////////////////////////////
		
		// Didn't actually move and, therefore, did not actually accelerate.  
		// Restore velocities.
		m_dVel			-= m_dDeltaVel;
//		m_dExtHorzVel	-= m_dExtHorzDeltaVel;
//		m_dExtVertVel	-= m_dExtVertDeltaVel;
		}
	
	}

////////////////////////////////////////////////////////////////////////////////
// Process all messages currently in the message queue through 
// ProcessMessage().
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::ProcessMessages(void)
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
void CThing3d::ProcessMessage(		// Returns nothing.
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

		case typeHelp:
			OnHelpMsg(&(pmsg->msg_Help) );
			break;

		case typePutMeDown:
			OnPutMeDownMsg(&(pmsg->msg_PutMeDown) );
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
void CThing3d::OnShotMsg(	// Returns nothing.
	Shot_Message* pshotmsg)		// In:  Message to handle.
	{
	if (pshotmsg->sDamage > 0)
		{
		// Receive bullet's momentum.
		AddForceVector(
			pshotmsg->sDamage * DAMAGE2VEL_RATIO,	// In:  Magnitude of additional vector.             
			pshotmsg->sAngle);							// In:  Direction (in degrees) of additional vector.
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an Explosion_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::OnExplosionMsg(			// Returns nothing.
	Explosion_Message* pexplosionmsg)	// In:  Message to handle.
	{
	m_dAcc = 0;
	double dX = m_dX - pexplosionmsg->sX;
	double dZ = m_dZ - pexplosionmsg->sZ;
	double dSqDist = (dX*dX) + (dZ*dZ);
	double dMulVert = 0.7;
	double dMulHorz = 0.2;
	if (dSqDist < 1.0)
		dSqDist = 1.0;

	m_dExtHorzRot = rspATan(pexplosionmsg->sZ - m_dZ, m_dX - pexplosionmsg->sX);
	if (dSqDist <= 900)
	{
		dMulVert = 0.8;
		dMulHorz = 0.3;
	}
	if (dSqDist <= 400)
	{
		dMulVert = 0.9;
		dMulHorz = 0.4;
	}
	if (dSqDist <= 100)
	{
		dMulVert = 1.0;
		dMulHorz = 0.5;
	}
	m_dExtHorzVel = pexplosionmsg->sVelocity * dMulHorz;
	m_dExtVertVel = pexplosionmsg->sVelocity * dMulVert;
	m_dExtHorzDrag = -ms_dDefaultAirDrag;

	// If there's an internal velocity . . .
	if (m_dVel != 0.0)
		{
		m_dDrag			= -ms_dDefaultAirDrag;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Burn_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::OnBurnMsg(	// Returns nothing.
	Burn_Message* pburnmsg)		// In:  Message to handle.
	{
	CFire*	pfire;
	// If we don't already have a fire . . .
	if (m_pRealm->m_idbank.GetThingByID((CThing**)&pfire, m_u16IdFire) != 0)
		{
		// Make a fire and remember its ID.
		if (CThing::ConstructWithID(CThing::CFireID, m_pRealm, (CThing**) &pfire) == 0)
			{
			// Store its ID.
			m_u16IdFire	= pfire->GetInstanceID();

			// Put it in the thing3d's midsection.
			pfire->Setup(					
				m_dX, 							// In:  New x coord
				m_dY + m_sprite.m_sRadius, // In:  New y coord
				m_dZ, 							// In:  New z coord
				BURN_DURATION, 				// In:  Number of milliseconds to burn, default 1sec
				false, 							// In:  Use thick fire (more opaque) default = true
				CFire::SmallFire);			// In:  Animation type to use default = LargeFire

			// We use this object as a weapon against others so we are the shooter.
			pfire->m_u16ShooterID		 = m_u16InstanceId;
			// Note though who caused the creation of this fire so we know who's
			// responsible for this thing's damage.
			pfire->m_u16FireStarterID	= pburnmsg->u16ShooterID;

			pfire->m_bIsBurningDude = (GetClassID() == CDudeID);

//			pfire->MessagesOff();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Handles an ObjectDelete_Message.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::OnDeleteMsg(				// Returns nothing.
	ObjectDelete_Message* pdeletemsg)	// In:  Message to handle.
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a Help_Message
// (virtual)
////////////////////////////////////////////////////////////////////////////////
void CThing3d::OnHelpMsg(					// Returns nothing
	Help_Message* phelpmsg)					// In:  Message to handle
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Handles a PutMeDown_Message
// (virtual)
////////////////////////////////////////////////////////////////////////////////
void CThing3d::OnPutMeDownMsg(			// Returns nothing
	PutMeDown_Message* pputmedownmsg)
	{
	}

////////////////////////////////////////////////////////////////////////////////
// Update fire animation's position.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::UpdateFirePosition(void)
	{
	CFire*	pfire;
	// If there is a fire . . .
	if (m_pRealm->m_idbank.GetThingByID((CThing**)&pfire, m_u16IdFire) == 0)
		{
		// Update its position.
		pfire->m_dX	= m_dX;
		pfire->m_dY	= m_dY;
		// Always put fire slightly in front of thing3d so we can see alpha
		// effect.
		pfire->m_dZ	= m_dZ + 1.0;
		// If dead or dying . . .
		if (m_state == State_Die || m_state == State_Dead)
			{
			// Char the guy.
			m_sBrightness	= BURNT_BRIGHTNESS;
			}
		}
	else
		{
		m_u16IdFire	= CIdBank::IdNil;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Get attributes at supplied position (uses m_pap2dAttribCheckPoints).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::GetFloorAttributes(	// Returns nothing.
	short		sX,						// In:  X coord.
	short		sZ,						// In:  Z coord.
	U16*		pu16Attrib,				// Out: Combined attribs, if not NULL.
	short*	psHeight)				// Out: Max height, if not NULL.
	{
	U16	u16CurAttrib;
	U16	u16CombinedAttrib	= 0;
	short	sLightTally			= 0;
	short	sMaxHeight			= -32767;
	short	sCurHeight;

	const Point2D*	p2d;
	for (p2d = m_pap2dAttribCheckPoints; p2d->sX != ATTRIB_CHECK_TERMINATOR; p2d++)
		{
		u16CurAttrib = m_pRealm->GetFloorMapValue((short) sX + p2d->sX,
		                                          (short) sZ + p2d->sZ,
																 REALM_ATTR_NOT_WALKABLE | 0x0000);

		// Combine attributes - and the height for now (since it will be masked out at end)
		u16CombinedAttrib |= u16CurAttrib;

		// Get height.
		sCurHeight	= short(u16CurAttrib & REALM_ATTR_HEIGHT_MASK);
		if (sCurHeight > sMaxHeight)
			{
			sMaxHeight	= sCurHeight;
			}
		}

	// Strip off the height information from the attributes
	u16CombinedAttrib &= REALM_ATTR_FLOOR_MASK;
	
	// If concerned with the height . . .
	if (psHeight)
		{
		// Map into realm.
		m_pRealm->MapAttribHeight(sMaxHeight * 4, psHeight);
		}
	
	SET(pu16Attrib, u16CombinedAttrib);
	}

////////////////////////////////////////////////////////////////////////////////
// Get attributes at supplied position (uses m_pap2dAttribCheckPoints).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::GetEffectAttributes(	// Returns nothing.
	short		sX,							// In:  X coord.
	short		sZ,							// In:  Z coord.
	U16*		pu16Attrib,					// Out: Combined attribs, if not NULL.
	short*	psLightBits)				// Out: Tally of light bits set, if not NULL.
	{
	U16	u16CurAttrib;
	U16	u16CombinedAttrib	= 0;
	short	sLightTally			= 0;

	const Point2D*	p2d;
	for (p2d = m_pap2dAttribCheckPoints; p2d->sX != ATTRIB_CHECK_TERMINATOR; p2d++)
		{
		u16CurAttrib = m_pRealm->GetEffectAttribute(
			(short) sX + p2d->sX,
			(short) sZ + p2d->sZ
			);

		// Combine attributes other than height.
		u16CombinedAttrib |= u16CurAttrib;

		// Get light effect
		if (u16CurAttrib & REALM_ATTR_LIGHT_BIT)
			sLightTally++;
		}

	SET(pu16Attrib, u16CombinedAttrib);
	SET(psLightBits, sLightTally);
	}

////////////////////////////////////////////////////////////////////////////////
// Get the layer based on the attribute points array (uses 
//	m_pap2dAttribCheckPoints).
////////////////////////////////////////////////////////////////////////////////
void CThing3d::GetLayer(	// Returns nothing.
	short  sX,					// In:  X coord.
	short  sZ,					// In:  Z coord.
	short* psLayer)			// Out: Combined layer.
	{
	U16	u16CombinedLayer	= 0;

	const Point2D*	p2d;
	for (p2d = m_pap2dAttribCheckPoints; p2d->sX != ATTRIB_CHECK_TERMINATOR; p2d++)
		{
		u16CombinedLayer	|= m_pRealm->GetLayer(
			(short) sX + p2d->sX,
			(short) sZ + p2d->sZ
			);
		}

	SET(psLayer, m_pRealm->GetLayerViaAttrib(u16CombinedLayer) );
	}

////////////////////////////////////////////////////////////////////////////////
// Get the link point specified by the provided transform.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::GetLinkPoint(	// Returns nothing.
	RTransform*	ptrans,			// In:  Transform specifying point.
	double*	pdX,					// Out: Point speicfied.
	double*	pdY,					// Out: Point speicfied.
	double*	pdZ)					// Out: Point speicfied.
	{
	// Set up translation based on the combined character and rigid body transforms.
	RTransform transChildAbsolute;
	// Apply child and parent to transChildAbs.
	transChildAbsolute.Mul(m_sprite.m_ptrans->T, ptrans->T);
	// Set up pt at origin for weapon.
	RP3d pt3Src = {0, 0, 0, 1};
	RP3d pt3Dst;
	// Get last transition position by mapping origin.
	m_pRealm->m_scene.TransformPtsToRealm(&transChildAbsolute, &pt3Src, &pt3Dst, 1);

	// Output link point.
	*pdX	= pt3Dst.x;
	*pdY	= pt3Dst.y;
	*pdZ	= pt3Dst.z;
	}

////////////////////////////////////////////////////////////////////////////////
// Detach the specified Thing3d.
// (virtual).
////////////////////////////////////////////////////////////////////////////////
CThing3d* CThing3d::DetachChild(	// Returns ptr to the child or NULL, if none.
	U16*		pu16InstanceId,		// In:  Instance ID of child to detach.
											// Out: CIdBank::IdNil.
	RTransform*	ptrans)				// In:  Transform for positioning child.
	{
	CThing3d*	pthing3d;
	if (m_pRealm->m_idbank.GetThingByID((CThing**)&pthing3d, *pu16InstanceId) == 0)
		{
		DetachChild(
			&(pthing3d->m_sprite),		// In:  Child sprite to detach.
			ptrans,							// In:  Transform for positioning child.
			&(pthing3d->m_dX),			// Out: Position of child.
			&(pthing3d->m_dY),			// Out: Position of child.
			&(pthing3d->m_dZ) );			// Out: Position of child.

		// Done with child.
		*pu16InstanceId = CIdBank::IdNil;
		// Child is done with us.
		pthing3d->m_u16IdParent	= CIdBank::IdNil;
		}
	else
		{
		// No longer exists.
		*pu16InstanceId	= CIdBank::IdNil;
		pthing3d				= NULL;
		}

	return pthing3d;
	}

////////////////////////////////////////////////////////////////////////////////
// Detach the specified child sprite (can be any sprite type).
// (virtual)
////////////////////////////////////////////////////////////////////////////////
void CThing3d::DetachChild(	// Returns nothing.
	CSprite*	psprite,				// In:  Child sprite to detach.
	RTransform*	ptrans,			// In:  Transform for positioning child.
	double*	pdX,					// Out: New position of child.
	double*	pdY,					// Out: New position of child.
	double*	pdZ)					// Out: New position of child.
	{
	// Get the link point via the transform.
	GetLinkPoint(ptrans, pdX, pdY, pdZ);

	// Set child position to character's position offset by rigid body's realm offset.
	*pdX += m_dX;
	*pdY += m_dY;
	*pdZ += m_dZ;
	
	// Detatch child's sprite
	m_sprite.RemoveChild(psprite);
	}

////////////////////////////////////////////////////////////////////////////////
// Position the specified child sprite (can be any sprite type).
// (virtual)
////////////////////////////////////////////////////////////////////////////////
void CThing3d::PositionChild(	// Returns nothing.
	CSprite*	psprite,				// In:  Child sprite to position.
	RTransform*	ptrans,			// In:  Transform for positioning child.
	double*	pdX,					// Out: New position of child.
	double*	pdY,					// Out: New position of child.
	double*	pdZ)					// Out: New position of child.
	{
	switch (psprite->GetType())
		{
		case CSprite::Standard3d:
			// Set transform from our rigid body transfanimation for the child
			// sprite.
			((CSprite3*)psprite)->m_ptrans	= ptrans;
			break;

		case CSprite::Standard2d:	// This only works for 1st level children.
			{
			// NOTE: This relies on the child having its Render() call after
			// this object's Render() call.
			// Since we know this object allocated the child (if we know that (it
			// is true in the case of a weapon)), we know that this object was 
			// allocated first.  If it was allocated first, it was added to the 
			// realm list first.  Since the realm always adds to the end of its 
			// list, we know this object's Render() will occur before the child's.
			// It's reliable enough for me, barely.
			// In the case that the Render() for the child occurs before this 
			// object's, the child will lag by, at most, 1 frame.

			// Convert the rigid body transform to some relative coordinates.
			// Note that we don't need to try to extract the rotations from the
			// transform since CSprite2's cannot rotate currently.

			GetLinkPoint(ptrans, pdX, pdY, pdZ);
			
			break;
			}
		default:
			TRACE("PositionChild(): Don't know how to handle this child's sprite type.\n");
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Add a force vector to this thing's external horizontal velocity vector.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::AddForceVector(	// Returns nothing.
	double	dAddVel,					// In:  Magnitude of additional vector.             
	short		sRot)						// In:  Direction (in degrees) of additional vector.
	{
	// Maybe we should make this an inline to add two vectors so we can use
	// this in other places.
	short	sAddAngle		= rspMod360(sRot);
	short sCurAngle		= rspMod360(m_dExtHorzRot);
	// Get our current X and Z component vectors.
	double	dCurVectX	= COSQ[sCurAngle] * m_dExtHorzVel;
	double	dCurVectZ	= -SINQ[sCurAngle] * m_dExtHorzVel;
	double	dAddVectX	= COSQ[sAddAngle] * dAddVel;
	double	dAddVectZ	= -SINQ[sAddAngle] * dAddVel;
	// Determine new combined component vectors.
	dCurVectX	+= dAddVectX;
	dCurVectZ	+= dAddVectZ;
	// Determine new vector and angle.
	m_dExtHorzVel	= rspSqrt(ABS2(dCurVectX, dCurVectZ) );
	m_dExtHorzRot	= rspATan(-dCurVectZ, dCurVectX);

	// Limit the magnitude of the vector.
	if (m_dExtHorzVel > 0.0)
		{
		if (m_dExtHorzVel > MaxForeVel)
			m_dExtHorzVel	= MaxForeVel;
		}
	else
		{
		if (m_dExtHorzVel < MaxBackVel)
			m_dExtHorzVel	= MaxBackVel;
		}

	// Make sure there's some surface drag, if there's a force.
	if (m_dExtHorzDrag == 0.0 && m_dExtHorzVel != 0.0)
		{
		// If above the terrain . . .
		if (m_bAboveTerrain == true)
			{
			m_dExtHorzDrag	= -ms_dDefaultAirDrag;
			}
		else
			{
			m_dExtHorzDrag	= -ms_dDefaultSurfaceDrag;
			}

		// If vector is negative magnitude for some reason . . .
		if (m_dExtHorzVel < 0.0)
			{
			// Negate the drag.
			m_dExtHorzDrag	= -m_dExtHorzDrag;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// PrepareShadow
////////////////////////////////////////////////////////////////////////////////

short CThing3d::PrepareShadow(void)
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
// Plays a sample and purges the resource after playing (as long as nobody
// else has used the same sample resource).  
// Plays a sample with volume adjustment.  This may require load from disk.
// Also, if no initial volume is specified, the distance to the ear is used.
////////////////////////////////////////////////////////////////////////////////
void CThing3d::PlaySample(									// Returns nothing.
																	// Does not fail.
	SampleMasterID	id,										// In:  Identifier of sample you want played.
	SampleMaster::SoundCategory eType,					// In:  Sound Volume Category for user adjustment
	short	sInitialVolume	/*= -1*/,						// In:  Initial Sound Volume (0 - 255)
																	// Negative indicates to use the distance to the
																	// ear to determine the volume.
	SampleMaster::SoundInstance*	psi /*= NULL*/,	// Out: Handle for adjusting sound volume
	long* plSampleDuration /*= NULL*/,					// Out: Sample duration in ms, if not NULL.
	long lLoopStartTime /*= -1*/,							// In:  Where to loop back to in milliseconds.
																	//	-1 indicates no looping (unless m_sLoop is
																	// explicitly set).
	long lLoopEndTime /*= 0*/,								// In:  Where to loop back from in milliseconds.
																	// In:  If less than 1, the end + lLoopEndTime is used.
	bool bPurgeSample /*= false*/)						// In:  Call ReleaseAndPurge rather than Release after playing
	{
	// If negative volume . . .
	if (sInitialVolume < 0)
		{
		// Determine volume based on distance to ear.
		sInitialVolume	= DistanceToVolume(m_dX, m_dY, m_dZ, SoundHalfLife);
		}

	::PlaySample(				// Returns nothing.
									// Does not fail.
		id,						// In:  Identifier of sample you want played.
		eType,					// In:  Sound Volume Category for user adjustment
		sInitialVolume,		// In:  Initial Sound Volume (0 - 255)
									// Negative indicates to use the distance to the
									// ear to determine the volume.
		psi,						// Out: Handle for adjusting sound volume
		plSampleDuration,		// Out: Sample duration in ms, if not NULL.
		lLoopStartTime,		// In:  Where to loop back to in milliseconds.
									//	-1 indicates no looping (unless m_sLoop is
									// explicitly set).
		lLoopEndTime,			// In:  Where to loop back from in milliseconds.
									// In:  If less than 1, the end + lLoopEndTime is used.
		bPurgeSample);			// In:  Call ReleaseAndPurge rather than Release after playing
	}

////////////////////////////////////////////////////////////////////////////////
// Start a CAnimThing.
////////////////////////////////////////////////////////////////////////////////
CAnimThing* CThing3d::StartAnim(		// Returns ptr to CAnimThing on success; NULL otherwise.
	char* pszAnimResName,				// In:  Animation's resource name.
	short	sX,								// In:  Position.
	short	sY,								// In:  Position.
	short	sZ,								// In:  Position.
	bool	bLoop)							// In:  true to loop animation.
	{
	// Create the animator . . .
	CAnimThing*	pat	= NULL;
	if (ConstructWithID(CAnimThingID, m_pRealm, (CThing**)&pat) == 0)
		{
		strcpy(pat->m_szResName, pszAnimResName);

		// Start it up:
		// No looping.
		pat->m_sLoop	= (bLoop == true) ? TRUE : FALSE;
		// No notification necessary.
		pat->Setup(sX, sY, sZ);
		}
	else
		{
		TRACE("StartAnim(): Failed to construct new CAnimThing.\n");
		}

	return pat;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
