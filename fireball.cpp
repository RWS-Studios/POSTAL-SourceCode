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
// fireball.cpp
// Project: Nostril (aka Postal)
//
// This module implements the CFireball weapon class which is a burning flame
//	for several different effects and weapons.
// 
//
// History:
//		01/17/97 BRH	Started this fireball ammo for use in making a flamethrower
//
//		04/25/97	JMI	PreLoad() was not properly assigning into sResult.
//
//		04/29/97	JMI	Changed name of ProcessMessages() to 
//							ProcessFireballMessages() to avoid conflicts with 
//							CWeapon's ProcessMessages().
//							Also, Update() was not moving the m_smash (it was merely
//							set the one time in Init()) which was causing it to not
//							collide with anything (I guess it might've collided with
//							things hovering around the upper, left corner of the realm).
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/11/97 BRH	Passed on shooter ID to the message it sends when it burns
//							someone.
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		07/02/97 BRH	Added CFirestream object that creates several CFireball
//							objects.
//
//		07/04/97 BRH	Added auto alpha based on the time to live.
//
//		07/07/97 BRH	Fixed casing on MIN to compile on both PC and Mac.
//
//		07/08/97	JMI	Put in code in Render() to avoid an unlikely divide by zero.
//							Still have a divide by zero release mode problem though.
//							Also, there was initialization typo in CFireball::Setup().
//
//		07/08/97	JMI	Fixed Render() to distribute the homogeneous alpha level
//							better.  Still needs tuning.
//
//					JMI	Made heavier again.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/09/97	JMI	Changed Preload() to take a pointer to the calling realm
//							as a parameter.
//
//		07/10/97	JMI	Now uses alpha mask and level for animation.
//
//		07/12/97 BRH	Added IsPathClear check to path of fireball since its
//							velocity is so high that it can pass through walls
//							otherwise.
//
//		07/13/97 BRH	Added a check to the Firestream creation of the
//							fireballs to make sure that the iniial position is
//							not in a wall.  If it is, then the other two fireballs
//							with a further offset should not be created, otherwise
//							it will shoot through walls.  
//
//							Also changed the alpha to use 1 mask and tuned the levels
//							so that you can see it at the gun end.
//
//		07/14/97	JMI	Now will detach itself from its parent via 
//							m_sprite.m_psprParent->RemoveChild() in the rare event 
//							that it is still parented going into the Render().
//							Also, threw in some random to the m_dRot before they are
//							separated from the CFirestream.
//
//		07/27/97	JMI	Changed to use Z position (i.e., X/Z plane) instead of
//							Y2 position (i.e., viewing plane) position for draw 
//							priority.
//
//		07/30/97	JMI	Changed several deletes that occurred just before reading
//							instantiable member variables.  Now both 
//							ProcessFireballMessage() functions (there are two (one for
//							CFireballs and one for CFireStreams) ) do NOT delete this
//							when they receive a delete message.  They instead set the
//							state to delete and allow the calling function to do the
//							delete.  The problem here was that the calling function,
//							Update(), needed to look at one more member var, m_eState,
//							before returning, but, at that point (that is, after the
//							delete) this was invalid.  This works find though when
//							using SmartHeap I think b/c of SmartHeap's bounds checking
//							areas to detect overwrites.  The Alpha does not have these
//							areas (no SmartHeap) and, so, crashes easily in these cases.
//							There was also a problem in Update() where it deleted this
//							but did not return right away and one more member var access
//							was done after that point (probably added later).  Fixed.
//
//		07/30/97	JMI	Now uses m_dHorizVel for its velocity which is initially 
//							set to ms_dFireVelocity but can be overridden after the
//							Setup() call.
//
//		08/08/97	JMI	Now CFirestream::ProcessFireballMessages() passes on
//							delete messages to any fireballs it owns.
//
//		08/08/97	JMI	Changed m_pFireball1, 2, & 3 to m_idFireball1, 2, & 3.
//
////////////////////////////////////////////////////////////////////////////////
#define FIREBALL_CPP

#include "RSPiX.h"
#include <math.h>

#include "fireball.h"
#include "game.h"
#include "reality.h"


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_FILE			"tinyfire.aan"

#define MIN_ALPHA				30
#define MAX_ALPHA				200
// Note the sum of these two values should not exceed 255

// Gets a random between -range / 2 and range / 2.
#define RAND_SWAY(sway)	((GetRand() % sway) - sway / 2)

#define FIREBALL_SWAY		15

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Let this auto-init to 0
short CFirestream::ms_sFileCount;
short CFirestream::ms_sOffset1 = 12;		// pixels from 1st to 2nd fireball
short CFirestream::ms_sOffset2 = 24;	// pixels from 1st to 3rd fireball

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFirestream::Load(										// Returns 0 if successfull, non-zero otherwise
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

			// Load static data.
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
		}
		else
		{
			sResult = -1;
			TRACE("CFirestream::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CFirestream::Load():  CThing::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFirestream::Save(										// Returns 0 if successfull, non-zero otherwise
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
		TRACE("CFirestream::Save(): CThing::Save() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CFirestream::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	return Init();
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CFirestream::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CFirestream::Suspend(void)
{
	m_sSuspend++;
}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CFirestream::Resume(void)
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
void CFirestream::Update(void)
{
	long lThisTime;

	if (!m_sSuspend)
	{
		lThisTime = m_pRealm->m_time.GetGameTime();

		// See if we killed ourselves
		if (ProcessFireballMessages() == State_Deleted)
			{
			delete this;
			return;
			}

		CFireball*	pfireball1	= NULL;
		CFireball*	pfireball2	= NULL;
		CFireball*	pfireball3	= NULL;

		// Update the other fireballs
		if (m_pRealm->m_idbank.GetThingByID((CThing**)&pfireball1, m_idFireball1) == 0)
			{
			pfireball1->m_dX		= m_dX;
			pfireball1->m_dY		= m_dY;
			pfireball1->m_dZ		= m_dZ;
			pfireball1->m_dRot	= rspMod360(m_dRot + RAND_SWAY(FIREBALL_SWAY) );
			}
		else
			{
			m_idFireball1	= CIdBank::IdNil;
			}

		if (m_pRealm->m_idbank.GetThingByID((CThing**)&pfireball2, m_idFireball2) == 0)
			{
			pfireball2->m_dX		= m_dX + COSQ[(short) m_dRot] * ms_sOffset1;
			pfireball2->m_dY		= m_dY;
			pfireball2->m_dZ		= m_dZ - SINQ[(short) m_dRot] * ms_sOffset1;
			pfireball2->m_dRot	= rspMod360(m_dRot + RAND_SWAY(FIREBALL_SWAY) );
			}
		else
			{
			m_idFireball2	= CIdBank::IdNil;
			}

		if (m_pRealm->m_idbank.GetThingByID((CThing**)&pfireball3, m_idFireball3) == 0)
			{
			pfireball3->m_dX		= m_dX + COSQ[(short) m_dRot] * ms_sOffset2;
			pfireball3->m_dY		= m_dY;
			pfireball3->m_dZ		= m_dZ - SINQ[(short) m_dRot] * ms_sOffset2;
			pfireball3->m_dRot	= rspMod360(m_dRot + RAND_SWAY(FIREBALL_SWAY) );
			}
		else
			{
			m_idFireball3	= CIdBank::IdNil;
			}

		if (m_eState == CWeapon::State_Fire)
		{
			if (pfireball1)
				pfireball1->m_eState = CWeapon::State_Fire;
			if (pfireball2)
				pfireball2->m_eState = CWeapon::State_Fire;
			if (pfireball3)
				pfireball3->m_eState = CWeapon::State_Fire;
			delete this;
			return;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CFirestream::Render(void)
{
	// If we have a parent . . .
	if (m_sprite.m_psprParent)
		{
		// Get outta there!
		m_sprite.m_psprParent->RemoveChild(&m_sprite);
		}

	// This should never ever be rendered.
	ASSERT(m_sprite.m_psprParent == NULL);
	ASSERT( (m_sprite.m_sInFlags & CSprite::PrivInserted) == 0);
}

////////////////////////////////////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////////////////////////////////////

short CFirestream::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ,												// In:  New z coord
	short sDir,												// In:  Direction of travel
	long lTimeToLive,										// In:  Number of milliseconds to burn, default 1sec
	U16 u16ShooterID)										// In:  Shooter's ID so you don't hit him
{
	short sResult = 0;
	double dX;
	double dZ;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_dRot = sDir;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_u16ShooterID = u16ShooterID;

	// Make sure that the starting positions are valid before creating
	// fireballs here, otherwise they will shoot through walls.
	dX = m_dX + COSQ[(short) m_dRot] * ms_sOffset2;	// Second interval 
	dZ = m_dZ - SINQ[(short) m_dRot] * ms_sOffset2;	
	if (m_pRealm->IsPathClear(		// Returns true, if the entire path is clear.                 
											// Returns false, if only a portion of the path is clear.     
											// (see *psX, *psY, *psZ).                                    
			(short) m_dX, 				// In:  Starting X.                                           
			(short) m_dY, 				// In:  Starting Y.                                           
			(short) m_dZ, 				// In:  Starting Z.                                           
			3.0, 							// In:  Rate at which to scan ('crawl') path in pixels per    
											// iteration.                                                 
											// NOTE: Values less than 1.0 are inefficient.                
											// NOTE: We scan terrain using GetHeight()                    
											// at only one pixel.                                         
											// NOTE: We could change this to a speed in pixels per second 
											// where we'd assume a certain frame rate.                    
			(short) dX,		 			// In:  Destination X.                                        
			(short) dZ,					// In:  Destination Z.                                        
			0,								// In:  Max traverser can step up.                      
			NULL,							// Out: If not NULL, last clear point on path.                
			NULL,							// Out: If not NULL, last clear point on path.                
			NULL,							// Out: If not NULL, last clear point on path.                
			false) )						// In:  If true, will consider the edge of the realm a path
											// inhibitor.  If false, reaching the edge of the realm    
											// indicates a clear path.                                 
	{
		m_lTimeToLive = m_pRealm->m_time.GetGameTime() + lTimeToLive;

		CFireball*	pfireball;
		if (CThing::ConstructWithID(CThing::CFireballID, m_pRealm, (CThing**) &pfireball) == 0)
		{
			pfireball->Setup(m_dX, m_dY, m_dZ, sDir, lTimeToLive, u16ShooterID);
			m_idFireball1	= pfireball->GetInstanceID();
		}

		dX = m_dX + COSQ[(short) m_dRot] * ms_sOffset1;	// First interval
		dZ = m_dZ - SINQ[(short) m_dRot] * ms_sOffset1;	
		if (CThing::ConstructWithID(CThing::CFireballID, m_pRealm, (CThing**) &pfireball) == 0)
		{
			pfireball->Setup(dX, m_dY, dZ, sDir, lTimeToLive, u16ShooterID);
			m_idFireball2	= pfireball->GetInstanceID();
		}

		dX = m_dX + COSQ[(short) m_dRot] * ms_sOffset2;	// Second interval 
		dZ = m_dZ - SINQ[(short) m_dRot] * ms_sOffset2;	
		if (CThing::ConstructWithID(CThing::CFireballID, m_pRealm, (CThing**) &pfireball) == 0)
		{
			pfireball->Setup(dX, m_dY, dZ, sDir, lTimeToLive, u16ShooterID);
			m_idFireball3	= pfireball->GetInstanceID();
		}

		if (sResult == SUCCESS)
			sResult = Init();
	}

	return sResult;

}

////////////////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////////////////

short CFirestream::Init(void)
{
	short sResult = SUCCESS;
	
	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CFirestream::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_lTimer = GetRand(); //m_pRealm->m_time.GetGameTime() + 1000;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CFirestream::EditModify(void)
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CFirestream::EditMove(									// Returns 0 if successfull, non-zero otherwise
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
void CFirestream::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CFirestream::EditRender(void)
{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
}



////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources for fire
//				 animations before play begins so that when a fire is set for
//				 the first time, there won't be a delay while it loads.
////////////////////////////////////////////////////////////////////////////////

short CFirestream::Preload(
	CRealm* /*prealm*/)			// In:  Calling realm.
{
return 0;
}


////////////////////////////////////////////////////////////////////////////////
// ProcessFireballMessages
////////////////////////////////////////////////////////////////////////////////

CFirestream::CFirestreamState CFirestream::ProcessFireballMessages(void)
{
	CFirestreamState eNewState = State_Idle;

	GameMessage msg;

	if (m_MessageQueue.DeQ(&msg) == true)
	{
		switch(msg.msg_Generic.eType)
		{
			case typeObjectDelete:
				{
				// Pass the message on . . .
				SendThingMessage(&msg, m_idFireball1);
				SendThingMessage(&msg, m_idFireball2);
				SendThingMessage(&msg, m_idFireball3);

				m_MessageQueue.Empty();
				return State_Deleted;
				break;
				}
		}
	}
	// Dump the rest of the messages
	m_MessageQueue.Empty();

	return eNewState;
}



////////////////////////////////// Fireball ////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define SMALL_FILE			"tinyfire.aan"

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Let this auto-init to 0
short CFireball::ms_sFileCount;
short CFireball::ms_sSmallRadius = 8;
long  CFireball::ms_lCollisionTime = 250;			// Check for collisions this often
double CFireball::ms_dFireVelocity = 300;

////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFireball::Load(										// Returns 0 if successfull, non-zero otherwise
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

			// Load static data.
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
			TRACE("CFireball::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CFireball::Load():  CThing::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CFireball::Save(										// Returns 0 if successfull, non-zero otherwise
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
		TRACE("CFireball::Save(): CThing::Save() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CFireball::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	return Init();
}

////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CFireball::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CFireball::Suspend(void)
{
	m_sSuspend++;
}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CFireball::Resume(void)
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
void CFireball::Update(void)
{
	long lThisTime;
	double dSeconds;
	double dDistance;
	double dNewX;
	double dNewZ;

	if (!m_sSuspend)
	{
		lThisTime = m_pRealm->m_time.GetGameTime();
		m_lAnimTime += lThisTime - m_lPrevTime;

		// See if we killed ourselves
		if (ProcessFireballMessages() == State_Deleted)
			{
			delete this;
			return;
			}

		switch (m_eState)
		{
			case State_Fire:
				if (lThisTime < m_lTimeToLive)
				{
					if (m_bMoving)
					{
						// Update position using wind direction and velocity
						dSeconds = ((double) lThisTime - (double) m_lPrevTime) / 1000.0;
						// Apply internal velocity.
						dDistance	= m_dHorizVel * dSeconds;
						dNewX	= m_dX + COSQ[(short) m_dRot] * dDistance;
						dNewZ	= m_dZ - SINQ[(short) m_dRot] * dDistance;

						// Check attribute map for walls, and if you hit a wall, 
						// set the timer so you will die off next time around.
						short sHeight = m_pRealm->GetHeight((short) dNewX, (short) dNewZ);
						// If it hits a wall taller than itself, then it will rotate in the
						// predetermined direction until it is free to move.
						if ((short) m_dY < sHeight ||
							!m_pRealm->IsPathClear(	// Returns true, if the entire path is clear.                 
															// Returns false, if only a portion of the path is clear.     
															// (see *psX, *psY, *psZ).                                    
							(short) m_dX, 				// In:  Starting X.                                           
							(short) m_dY, 				// In:  Starting Y.                                           
							(short) m_dZ, 				// In:  Starting Z.                                           
							3.0, 							// In:  Rate at which to scan ('crawl') path in pixels per    
															// iteration.                                                 
															// NOTE: Values less than 1.0 are inefficient.                
															// NOTE: We scan terrain using GetHeight()                    
															// at only one pixel.                                         
															// NOTE: We could change this to a speed in pixels per second 
															// where we'd assume a certain frame rate.                    
							(short) dNewX, 			// In:  Destination X.                                        
							(short) dNewZ,				// In:  Destination Z.                                        
							0,								// In:  Max traverser can step up.                      
							NULL,							// Out: If not NULL, last clear point on path.                
							NULL,							// Out: If not NULL, last clear point on path.                
							NULL,							// Out: If not NULL, last clear point on path.                
							false) 						// In:  If true, will consider the edge of the realm a path
															// inhibitor.  If false, reaching the edge of the realm    
															// indicates a clear path.                                 
						)
						{
							// Stop moving and fix yourself on a random spot on the wall.
							m_bMoving = false;				
							m_dX += (-3 + GetRand() % 7);
							m_dY += (-3 + GetRand() % 7);
							m_dZ += (-3 + GetRand() % 7);

							// Update sphere
							m_smash.m_sphere.sphere.X = m_dX;
							m_smash.m_sphere.sphere.Y = m_dY;
							m_smash.m_sphere.sphere.Z = m_dZ;
						}
						else
						{
							m_dX = dNewX;
							m_dZ = dNewZ;

							// Update sphere
							m_smash.m_sphere.sphere.X = m_dX;
							m_smash.m_sphere.sphere.Y = m_dY;
							m_smash.m_sphere.sphere.Z = m_dZ;

							// Check for collisions
							CSmash* pSmashed = NULL;
							GameMessage msg;
							msg.msg_Burn.eType = typeBurn;
							msg.msg_Burn.sPriority = 0;
							msg.msg_Burn.sDamage = 10;
							msg.msg_Burn.u16ShooterID = m_u16ShooterID;
							m_pRealm->m_smashatorium.QuickCheckReset(&m_smash, m_u32CollideIncludeBits,
																				  m_u32CollideDontcareBits, 
																				  m_u32CollideExcludeBits);
							while (m_pRealm->m_smashatorium.QuickCheckNext(&pSmashed))
								if (pSmashed->m_pThing->GetInstanceID() != m_u16ShooterID)
									SendThingMessage(&msg, pSmashed->m_pThing);				
						}
					}

				}
				else
				{
					delete this;
					return;
				}
				break;

			default:
				break;
		}
		m_lPrevTime = lThisTime;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CFireball::Render(void)
{
	
	CAlphaAnim* pAnim;

	pAnim = (CAlphaAnim*) m_pAnimChannel->GetAtTime(m_lAnimTime % m_pAnimChannel->TotalTime());

	if (pAnim) // && m_sCurrentAlphaChannel >= 0)
	{
		// No special flags
		m_sprite.m_sInFlags = 0; 

		// Map from 3d to 2d coords
		Map3Dto2D(m_dX, m_dY, m_dZ, &(m_sprite.m_sX2), &(m_sprite.m_sY2) );
		// Offset by animations 2D offsets.
		m_sprite.m_sX2	+= pAnim->m_sX;
		m_sprite.m_sY2	+= pAnim->m_sY;

		// Priority is based on our Z position.
		m_sprite.m_sPriority = m_dZ;

		// Layer should be based on info we get from attribute map.
		m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

//		m_sprite.m_sAlphaLevel = 200;
		if (m_lTotalFlameTime == 0)
			{
			// Safety.
			m_lTotalFlameTime = 500;
			}

//		m_sprite.m_sAlphaLevel = MIN((long)255, (long) (((m_lTimeToLive - m_pRealm->m_time.GetGameTime()) / m_lTotalFlameTime) * 255));

		// Copy the color info and the alpha channel to the Alpha Sprite
		m_sprite.m_pImage = &(pAnim->m_imColor);

		// Now there is only 1 alpha mask.
		m_sCurrentAlphaChannel = 0; //MIN(m_sCurrentAlphaChannel, (short) (m_sTotalAlphaChannels - 1));
		m_sprite.m_pimAlpha = &(pAnim->m_pimAlphaArray[0]);
		// Adjust level between 0 and max so it gets more opaque with time.
		m_sprite.m_sAlphaLevel = MIN_ALPHA + MAX_ALPHA - (MAX_ALPHA * (m_lTimeToLive - m_pRealm->m_time.GetGameTime()) ) / m_lTotalFlameTime ;
		// Keep in range.
		if (m_sprite.m_sAlphaLevel < 0)
			m_sprite.m_sAlphaLevel = 0;
		else if (m_sprite.m_sAlphaLevel > MAX_ALPHA)
			m_sprite.m_sAlphaLevel = MAX_ALPHA;

		// Update sprite in scene
		m_pRealm->m_scene.UpdateSprite(&m_sprite);
		
	}
}

////////////////////////////////////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////////////////////////////////////

short CFireball::Setup(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ,												// In:  New z coord
	short sDir,												// In:  Direction of travel
	long lTimeToLive,										// In:  Number of milliseconds to burn, default 1sec
	U16 u16ShooterID)										// In:  Shooter's ID so you don't hit him
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_dRot = sDir;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();
	m_lCollisionTimer = m_lPrevTime + ms_lCollisionTime;
	m_u16ShooterID = u16ShooterID;
	m_lAnimTime = GetRandom();
	m_dHorizVel	= ms_dFireVelocity; 

	m_lTotalFlameTime = lTimeToLive;
	m_lTimeToLive = m_pRealm->m_time.GetGameTime() + lTimeToLive;
	m_sCurrentAlphaChannel = 0;
	
	// Load resources
	sResult = GetResources();

	if (sResult == SUCCESS)
		sResult = Init();

	return sResult;

}

////////////////////////////////////////////////////////////////////////////////
// Init
////////////////////////////////////////////////////////////////////////////////

short CFireball::Init(void)
{
	short sResult = SUCCESS;
	CAlphaAnim* pAnim = NULL;


	// Update sphere
	m_smash.m_sphere.sphere.X = m_dX;
	m_smash.m_sphere.sphere.Y = m_dY;
	m_smash.m_sphere.sphere.Z = m_dZ;
	m_smash.m_sphere.sphere.lRadius = ms_sSmallRadius;
	m_smash.m_bits = CSmash::Fire;
	m_smash.m_pThing = this;
	// Update the smash
	m_pRealm->m_smashatorium.Update(&m_smash);
	m_eState = CWeapon::State_Idle;

	// Set the collision bits
	m_u32CollideIncludeBits = CSmash::Character | CSmash::Barrel | CSmash::Mine | CSmash::Misc;
	m_u32CollideDontcareBits = CSmash::Good | CSmash::Bad;
	m_u32CollideExcludeBits = 0;

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CFireball::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;
	m_lTimer = GetRand(); //m_pRealm->m_time.GetGameTime() + 1000;
	m_lPrevTime = m_pRealm->m_time.GetGameTime();

	// Load resources
	sResult = GetResources();

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CFireball::EditModify(void)
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CFireball::EditMove(									// Returns 0 if successfull, non-zero otherwise
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
void CFireball::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CFireball::EditRender(void)
{
	// In some cases, object's might need to do a special-case render in edit
	// mode because Startup() isn't called.  In this case it doesn't matter, so
	// we can call the normal Render().
	Render();
}


////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CFireball::GetResources(void)			// Returns 0 if successfull, non-zero otherwise
{
	short sResult = SUCCESS;

	sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(SMALL_FILE), &m_pAnimChannel, RFile::LittleEndian);

	if (sResult != 0)
		TRACE("CFireball::GetResources - Error getting fire animation resource\n");

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CFireball::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	rspReleaseResource(&g_resmgrGame, &m_pAnimChannel);

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Preload - basically trick the resource manager into caching resources for fire
//				 animations before play begins so that when a fire is set for
//				 the first time, there won't be a delay while it loads.
////////////////////////////////////////////////////////////////////////////////

short CFireball::Preload(
	CRealm* prealm)				// In:  Calling realm.
{
	short sResult;
	ChannelAA* pRes;
	sResult = rspGetResource(&g_resmgrGame, prealm->Make2dResPath(SMALL_FILE), &pRes, RFile::LittleEndian);
	rspReleaseResource(&g_resmgrGame, &pRes);
	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// ProcessFireballMessages
////////////////////////////////////////////////////////////////////////////////

CFireball::CFireballState CFireball::ProcessFireballMessages(void)
{
	CFireballState eNewState = State_Idle;

	GameMessage msg;

	if (m_MessageQueue.DeQ(&msg) == true)
	{
		switch(msg.msg_Generic.eType)
		{
			case typeObjectDelete:
				m_MessageQueue.Empty();
				return State_Deleted;
				break;
		}
	}
	// Dump the rest of the messages
	m_MessageQueue.Empty();

	return eNewState;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
