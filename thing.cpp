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
// thing.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CThing class.
//
// History:
//		12/09/96 MJR	Started.
//
//		01/17/97	JMI	Started DoGui(). Checked in so can work at home.  Compiles.
//
//		01/19/97	JMI	Finished DoGui().
//
//		01/22/97	JMI	Added initialization of bEditorCreatable member of 
//							ms_aClassInfo[] elements.
//
//		01/23/97	JMI	DoGui() now calls functions in GUIs and hots via the this
//							instead of the namespace.
//							Also, now first item in list of children is given the focus
//							instead of OK item.
//							Also, also, RDirtyRect is given a clip rect so we don't
//							have to hear Blue complain about it.	
//
//		01/29/97	CMI	Added m_u16InstanceId, an identifier unique to each instance
//							of CThing within its Realm.  Also, added the initialization
//							of this ID in the constructor, the release of the ID in the
//							destructor, and the saving and loading of the ID in Save()
//							and Load().  Note that this value is either created by the
//							editor, loaded from the .rlm file, or assigned by the 
//							server (dynamically created CThings (one's not loaded from
//							the .rlm file) will get their IDs assigned somehow at run-
//							time).
//
//		01/30/97	JMI	Forgot to reserve the instance ID on load.  Now calls 
//							pRealm->m_idbank.Take(this, m_u16InstanceId).
//							Also, added SetInstanceID().
//
//		02/07/97	JMI	Added support for new CGameEditThing.
//
//		02/10/97 BRH	Added CNapalm and CFire as things to construct
//
//		02/16/97 BRH	Added message.h and the message queue along with the 
//							SendThingMessage function to allow CThings to communicate.
//							Also set the thing base class version of Update to 
//							empty the queue each time.  If you aren't dealing with
//							messages, call the base class Update after yours to 
//							get rid of unused messages.  There should be a better way
//							to have the base class automatically clear the queue
//							so that CThings that aren't aware of message queues at all
//							won't have message queues fill up.
//
//		02/17/97	JMI	CThing() now puts an allocated CThing into the proper set
//							list based on it's ID.  If you want a CThing to be in a
//							particular set list, you must change or add the case in
//							the switch statements in both CThing() (to Add) and 
//							~CThing() (to Remove).
//
//		02/17/97 BRH	Changed the message queue to take the new message structure
//							which contains a union of the other message types.
//
//		02/18/97	JMI	SendThingMessage() now passes a ptr to a GameMessage to
//							EnQ() instead of a ptr to a ptr to a GameMessage.
//
//		02/19/97	JMI	Added CAnimThingID entry to ms_aClassInfo.
//							Also, removed monstrous switch states in con/destructor
//							having to do with old collision detection sets.
//
//		02/20/97	JMI	Now initializes m_sCallStartup and m_sCallShutdown in the
//							CThing constructor.
//
//		02/23/97 MJR	Added Preload members to classinfo struct for those classes
//							that have one, and 0 for those that don't.
//
//		02/24/97 MJR	Added CSoundThing stuff.
//
//		02/25/97	JMI	Added CGunner stuff.
//
//		03/04/97 BRH	Added CBand stuff.
//
//		03/05/97	JMI	Added ConstructWidthID() to construct an object and assign
//							it an ID, if it does not already have one.
//
//		03/06/97 BRH	Fixed SendThingMessage (id) to make sure the thing
//							exists before sending the message.
//
//		03/06/97	JMI	Added CItem3d stuff.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/17/97 BRH	Added CBarrel.
//
//		03/19/97 BRH	Added CMine.
//
//		03/19/97	JMI	Added CDispenser stuff.
//
//		03/20/97	JMI	SetInstanceID() no longer whines about already having an
//							ID before releasing it.
//
//		04/04/97	JMI	Load() now checks to make sure no one else has claimed the
//							loaded ID before taking it.
//
//		04/16/97 BRH	Added Jon's template CListNode that replaces the old STL
//							lists in CRealm.  The CListNode contains next and previous
//							pointers rather than having the CThing being put into an
//							STL container in CRealm.  
//
//		04/25/97 BRH	Added CFireball.
//							Added CCop.
//
//		04/28/97 BRH	Added fake classes CPistol, CMachineGun, CShotGun so that
//							the PrepareWeapon() and ShootWeapon() functions could
//							be made more generic by using the class ID's to identify
//							weapons.  
//
//		04/28/97 BRH	Added CPerson.
//
//		04/30/97	JMI	Changed CMine::Construct to CMine::ConstructProximityMine 
//							and added CMine::ConstructTimed, 
//							CMine::ConstructBouncingBetty, and 
//							CMine::ConstructRemoteControl.
//
//		05/01/97 BRH	Added CPylon::Construct
//
//		05/04/97 BRH	Took Tkachuk out of the project.
//
//		05/05/97 BRH	Had to put Tkachuk placeholder back in to avoid screwing
//							up all of the realm files.
//
//		05/08/97	JMI	Added schtuff for CPowerUp.
//
//		05/09/97 BRH	Added COstrich
//
//		05/12/97 JRD	Added CTrigger
//
//		05/13/97 BRH	Added CHeatseeker.
//
//		05/08/97	JMI	Added schtuff for CChunk.
//
//		05/26/97 BRH	Changed the editor placement flag for several enemies so
//							CPerson will be used instead.
//
//		05/26/97	JMI	Finally broke down and added an RHot* so the editor can
//							quickly from CThing* to RHot*.
//
//		05/26/97 BRH	Added CAssault which is the Shot Gun fired rapidly.  
//							This is just another dummy ID like the rest of the guns.
//
//		06/02/97	JMI	Added schtuff for CLadder.
//
//		06/02/97 BRH	Added CSentry and CSentryGun
//
//		06/02/97	JMI	Removed CLadder stuff.
//
//		06/03/97	JMI	Made DoGui() static.
//							Also, added CWarpID.
//
//		06/03/97	JMI	Removed references to CGrenader, CRocketMan, CCop, CGunner,
//							CImbecile.
//
//		06/05/97	JMI	Added m_lDoGuiPressedId and GuiPressed() statics.
//							Also, made sure DoGui() cleans up the display before 
//							exitting.
//
//		06/09/97 BRH	Added CDemon.
//
//		06/11/97	JMI	Added CCharacter stuff.
//
//		06/15/97 MJR	Now calls Update() instead of rspDoSystem().
//
//		06/24/97	JMI	Now SendThingMessage() ASSERTs that the message priority
//							has been initialized by ASSERTing it's not 0xebeb.
//
//		06/26/97	JMI	Added inline aliases to CRealm's Map3DTo2D()s.
//
//		06/30/97 MJR	Replaced SAFE_GUI_REF with new GuiItem.h-defined macro.
//
//		06/30/97 BRH	Added CGoalTimer, CFlag, and CFlagbase 
//
//		07/02/97 BRH	Added CFirestream as a better fire for the flamethrower.
//
//		07/03/97	JMI	Removed GuiPressed() and added an RProcessGui which is a
//							simple way to process a GUI.  It is nearly the same as the
//							RMsgBox interface but is for Load()ed or otherwise 
//							preprepared GUIs where RMsgBox is a more dynamic method
//							of using a dialog box.  Also, the RProcessGui works for
//							any GUI (not just RDlg).
//
//		07/03/97	JMI	Added callback for RProcessGui so we can use ::Update()
//							instead of its internal update stuff.
//
//		07/14/97	JMI	Moved Construct() definition into thing.cpp.
//							Now checks to make sure id is bounds.	
//
//		07/14/97 BRH	Removed CGoalTimer from the project since the realm
//							is handling this duty.
//
//		07/19/97	JMI	Removed all the 'C's preceding the thing names.
//
//		07/30/97	JMI	Added CDeathWad entry in ms_aClassInfo[].
//
//		08/06/97	JMI	Added CDoubleBarrel entry in ms_aClassInfo[].
//
//		08/08/97	JMI	Added more weapons for doofuses:
//								CUziID, CAutoRifleID, CSmallPistolID, CDynamiteID.
//
//		08/10/97	JMI	Added entries for CSndRelay in ms_aClassInfo.
//
//		08/17/97	JMI	Now sets the Update() call for the GUI processor.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem().
//
//		08/25/97	JMI	Now the editor cannot create CDudes or CBalls.
//
//		08/28/97 BRH	Added Preload for PowerUps and Mines to the table of 
//							preload functions to call.
//
//		09/03/97 BRH	Fixed spelling of Bouy in editor, according to Webster,
//							it really is buoy.  
//
////////////////////////////////////////////////////////////////////////////////
#define THING_CPP

#include "RSPiX.h"
#include "thing.h"
#include "realm.h"

#include "IdBank.h"

// Need these to initialize array of class info (until better method is developed)
#include "ball.h"
#include "hood.h"
#include "dude.h"
#include "doofus.h"
#include "rocket.h"
#include "grenade.h"
#include "explode.h"
#include "bouy.h"
#include "navnet.h"
#include "gameedit.h"
#include "napalm.h"
#include "fire.h"
#include "firebomb.h"
#include "AnimThing.h"
#include "SoundThing.h"
#include "band.h"
#include "item3d.h"
#include "barrel.h"					
#include "mine.h"
#include "dispenser.h"
#include "fireball.h"
#include "person.h"
#include "pylon.h"
#include "PowerUp.h"
#include "ostrich.h"
#include "trigger.h"
#include "heatseeker.h"
#include "chunk.h"
#include "sentry.h"
#include "warp.h"
#include "demon.h"
#include "update.h"
#include "flag.h"
#include "flagbase.h"
#include "deathWad.h"
#include "SndRelay.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Init this flag to a non-zero value so we can detect whether a CThing is
// created before these static member variables are initialized.
short CThing::ms_sDetectStaticInits = 1;

// This is used by DoGui() to perform GUI processing.
RProcessGui	CThing::ms_pgDoGui;


// Array of class info
// This sucks right now.  I need to add the info for each class into this
// array, but I don't like this method because it means this file has to
// include the headers for each class.  Sounds like an RImage-style solution
// might work, but I'm not sure exactly how.
CThing::ClassInfo CThing::ms_aClassInfo[CThing::TotalIDs] =
	{	// Object Allocator					Preload function			Object Name				User can create in Editor
		// =====================			=====================	================
		{ CHood::Construct,					0,								"Hood",					false	},
		{ CDude::Construct,					0,								"Dude",					false	},
		{ CDoofus::Construct,				0,								"Doofus",				false	},
		{ NULL, /* Tkachuk */				0,								"Tkachuk",				false },
		{ NULL, /* CRocketMan */			0,								"RocketMan",			false	},
		{ NULL, /* CGrenader */				0,								"Grenader",				false	},
		{ CRocket::Construct,				CRocket::Preload,			"Rocket",				false	},
		{ CGrenade::Construct,				CGrenade::Preload,		"Grenade",				false	},
		{ CBall::Construct,					0,								"Ball",					false	},
		{ CExplode::Construct,				CExplode::Preload,		"Explode",				false	},
		{ CBouy::Construct,					0,								"Buoy",					true  },
		{ CNavigationNet::Construct,		0,								"NavNet",				true  },
		{ CGameEditThing::Construct,		0,								"GameEditThing",		false },
		{ CNapalm::Construct,				CNapalm::Preload,			"Napalm",				false },
		{ CFire::Construct,					CFire::Preload,			"Fire",					false },
		{ NULL, /* CImbecile */				0,								"Imbecile",				false	},
		{ CFirebomb::Construct,				CFirebomb::Preload,		"Firebomb",				false },
		{ CFirefrag::Construct,				0,								"Firefrag",				false },
		{ CAnimThing::Construct,			0,								"AnimThing",			true	},
		{ CSoundThing::Construct,			0,								"SoundThing",			true	},
		{ NULL, /* CGunner */				0,								"Gunner",				false	},
		{ CBand::Construct,					0,								"Band",					true  },
		{ CItem3d::Construct,				0,								"Item3d",				true	},
		{ CBarrel::Construct,				0,								"Barrel",				true	},
		{ CMine::ConstructProximity,		0,								"ProximityMine",		true  },
		{ CDispenser::Construct,			0,								"Dispenser",			true	},
		{ CFireball::Construct,				CFireball::Preload,		"Fireball",				false },
		{ NULL, /* CCop */					0,								"Cop",					false	},
		{ NULL, /* CPistol */				0,								"Pistol",				false },
		{ NULL, /* CMachineGun */			0,								"MachineGun",			false },
		{ NULL, /* CShotGun */				0,								"ShotGun",				false },
		{ CPerson::Construct,				0,								"Person",				true	},
		{ CMine::ConstructTimed,			CMine::Preload,			"TimedMine",			true	},
		{ CMine::ConstructBouncingBetty,	0,/*CMine::Preload*/		"BouncingBettyMine",	true	},
		{ CMine::ConstructRemoteControl,	0,/*CMine::Preload*/		"RemoteControlMine",	false	},
		{ CPylon::Construct,					0,								"Pylon",					true  },
		{ CPowerUp::Construct,				CPowerUp::Preload,		"PowerUp",				true  },
		{ COstrich::Construct,				0,								"Ostrich",				true  },
		{ CTrigger::Construct,				0,								"Trigger",				false },
		{ CHeatseeker::Construct,			CHeatseeker::Preload,	"Heatseeker",			false },
		{ CChunk::Construct,					0,								"Chunk",					false },
		{ NULL, /* CAssault */				0,								"AssaultWeapon",		false },
		{ CSentry::Construct,				0,								"Sentry",				true	},
		{ NULL, /* CSentryGun */			0,								"CentryGun",			false	},
		{ CWarp::Construct,					0,								"Warp",					true	},
		{ CDemon::Construct,					CDemon::Preload,			"Demon",					true  },
		{ NULL, /* CCharacter */			CCharacter::Preload,		"Character",			false	},
		{ NULL, /*CGoalTimer */				0,								"GoalTimer",			false },
		{ CFlag::Construct,					0,								"Flag",					true  },
		{ CFlagbase::Construct,				0,								"Flagbase",				true  },
		{ CFirestream::Construct,			0,								"Firestream",			false },
		{ CDeathWad::Construct,				CDeathWad::Preload,		"DeathWad",				false	},
		{ NULL, /*CDoubleBarrel */			0,								"DoubleBarrel",		false	},
		{ NULL, /*CUziID */			0,										"Uzi",					false	},
		{ NULL, /*CAutoRifleID */			0,								"AutoRifle",			false	},
		{ NULL, /*CSmallPistolID */		0,								"SmallPistol",			false	},
		{ CGrenade::ConstructDynamite,	0,								"Dynamite",				false	},
		{ CSndRelay::Construct,				0,								"SndRelay",				true	},
	};


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Default (and only) constructor
////////////////////////////////////////////////////////////////////////////////
CThing::CThing(
	CRealm* pRealm,										// In:  Pointer to realm
	ClassIDType id)										// In:  Class ID
	{
	// Make sure CThing static's have been initialized by C++ runtime
	if (ms_sDetectStaticInits != 1)
		TRACE("CThing::CThing(): Can't create global/static objects based on CThing!\n");

	// Save class id so we have it quickly avaiable in destructor
	m_id = id;

	// Default to calling startup and shutdown.  What could be the harm?!
	m_sCallStartup		= TRUE;
	m_sCallShutdown	= TRUE;

	// Save realm
	m_pRealm = pRealm;

	m_everything.m_powner = this;
	m_everything.m_pnNext = NULL;
	m_everything.m_pnPrev = NULL;
	m_nodeClass.m_powner = this;
	m_nodeClass.m_pnNext = NULL;
	m_nodeClass.m_pnPrev = NULL;

	// Add this object to realm and save its assigned position in realm's container
//	pRealm->AddThing(this, id, &m_iterEvery, &m_iterClass);
	pRealm->AddThing(this, id);

	// Start out with no ID.
	m_u16InstanceId	= CIdBank::IdNil;

	// Clear editor's RHot*.
	m_phot				= NULL;
	}


////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
CThing::~CThing()
	{
	// Remove this object from realm
//	m_pRealm->RemoveThing(m_id, m_iterEvery, m_iterClass);
	m_pRealm->RemoveThing(this);
	
	// Release this fellow's ID.
	m_pRealm->m_idbank.Release(m_u16InstanceId);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Call this for any GUIs besides the standard OK (ID 1) and Cancel (ID 2)
// to set the callback (for on 'pressed') for any GUI you want to end
// a DoGui().
// (static).
//
////////////////////////////////////////////////////////////////////////////////
// static
void CThing::SetGuiToNotify(	// Returns nothing.
	RGuiItem* pguiNotifier)		// In:  The pressed GUI.
	{
	ms_pgDoGui.SetGuiToNotify(pguiNotifier);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Callback from ms_pgDoGui for system update.
//
////////////////////////////////////////////////////////////////////////////////
// static							// Static for use as a callback.
long CThing::SysUpdate(			// Returns a non-zero ID to abort or zero
										// to continue.                          
	RInputEvent*	pie)			// Out: Next input event to process.     
	{
	UpdateSystem();
	rspGetNextInputEvent(pie);

	return 0;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Helper for processing your GUIs.
// Will be made visible by calling pguiRoot->SetVisible(TRUE).
// GUI will be run with focus-awareness until a GUI with ID 1 or 2 is
// clicked.  Typically, 1 should be an 'OK' equivalent and 2 'Cancel'.
// Return value indicates which item was clicked (1, 2, or by the ID
// of a GUI that was previously passed to SetGuiToNotify() ).
// Processing involves using queued RSPiX user input via
// rspGetNextInputEvent().
// (static).
//
////////////////////////////////////////////////////////////////////////////////
long CThing::DoGui(			// Returns ID of item that terminated looping.
									// Returns 0 if rspGetQuitStatus() is nonzero.
									// Returns negative on error.
	RGuiItem*	pguiRoot)	// Root of GUI items to process through user.
	{
	// Get two controls that can end the processing.
	RGuiItem*	pguiOk		= pguiRoot->GetItemFromId(1);
	RGuiItem*	pguiCancel	= pguiRoot->GetItemFromId(2);

	// Use the update function.
	ms_pgDoGui.m_fnUpdate	= SysUpdate;

	return ms_pgDoGui.DoModal(pguiRoot, pguiOk, pguiCancel);
	}

////////////////////////////////////////////////////////////////////////////////
//
// Load object (should call base class version!)
// (virtual).
//
////////////////////////////////////////////////////////////////////////////////
short CThing::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  File version being loaded.
	{
	// Switch on the file version.
	switch (ulFileVersion)
		{
		default:	// Newer versions where this format has not changed.
		case 1:
			// Load this thing's ID.  The ID is unique to this 'thing' within its realm
			// (i.e., no other CThing or derived class has this same ID within this realm).
			// This is assigned by the editor via a call to the realm's m_idbank.Get().
			pFile->Read(&m_u16InstanceId);
			break;
		}

	// If this ID is not yet claimed . . .
	CThing*	pthing;
	if (m_pRealm->m_idbank.GetThingByID(&pthing, m_u16InstanceId) != 0)
		{
		// Reserve ID.
		m_pRealm->m_idbank.Take(this, m_u16InstanceId);
		}
	else
		{
		// No ID for you!  Currently the only way I know this happens is via the
		// dispenser.  In that case, this is fine b/c the dispenser never uses the
		// id from the file.
		m_u16InstanceId	= CIdBank::IdNil;
		}

	return pFile->Error();
	}

////////////////////////////////////////////////////////////////////////////////
//
// Set object instance's unique ID.
//
////////////////////////////////////////////////////////////////////////////////
void CThing::SetInstanceID(	// Returns nothing.
	U16	u16Id)					// New id for this instance.
	{
	// Safety.
	if (m_u16InstanceId != CIdBank::IdNil)
		{
//		TRACE("SetInstanceID(): This thing already had an ID!\n");
		// Release existing ID.
		m_pRealm->m_idbank.Release(m_u16InstanceId);
		}

	m_u16InstanceId	= u16Id;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Put a message in another CThing's message queue
//
////////////////////////////////////////////////////////////////////////////////

short CThing::SendThingMessage(pGameMessage pMessage, short sPriority, U16 u16ID)
	{
	short sResult = SUCCESS;
	CThing* pThing = NULL;

	m_pRealm->m_idbank.GetThingByID(&pThing, u16ID);
	if (pThing)
		return SendThingMessage(pMessage, sPriority, pThing);
	else
		return -1;
	}

short CThing::SendThingMessage(pGameMessage pMessage, short sPriority, CThing* pThing)
	{
	short sResult = SUCCESS;

	// Make sure this has been initialized.
	// If you were using 0xebeb as your priority, stop that.
	ASSERT(sPriority != 0xebeb);

	if (pThing != NULL)
		pThing->m_MessageQueue.EnQ(pMessage, &sPriority);
	else
		sResult = -1;

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Maps a 3D coordinate onto the viewing plane.
////////////////////////////////////////////////////////////////////////////////
void CThing::Map3Dto2D(	// Returns nothing.
	short sX,				// In.
	short	sY,				// In.
	short	sZ,				// In.
	short* psX,				// Out.
	short* psY)				// Out.
	{
	m_pRealm->Map3Dto2D(sX, sY, sZ, psX, psY); 
	}

////////////////////////////////////////////////////////////////////////////////
// Maps a 3D coordinate onto the viewing plane.
////////////////////////////////////////////////////////////////////////////////
void CThing::Map3Dto2D(	// Returns nothing.
	double	dX,			// In.
	double	dY,			// In.
	double	dZ,			// In.
	double* pdX,			// Out.
	double* pdY)			// Out.
	{
	m_pRealm->Map3Dto2D(dX, dY, dZ, pdX, pdY); 
	}

////////////////////////////////////////////////////////////////////////////////
// Construct object
// (static).
////////////////////////////////////////////////////////////////////////////////
short CThing::Construct(								// Returns 0 if successfull, non-zero otherwise
	ClassIDType id,										// In:  Class ID
	CRealm* pRealm,										// In:  Pointer to realm this object belongs to
 	CThing** ppNew)										// Out: Pointer to new object
	{
	// If in bounds (note ClassIDType is unsigned) . . .
	if (id < TotalIDs)
		{
		// If there is a Construct func . . .
		if (ms_aClassInfo[id].funcConstruct)
			return (*(ms_aClassInfo[id].funcConstruct))(pRealm, ppNew);
		else
			return -1;
		}
	else
		{
		TRACE("Construct(): id %d is out of bounds.\n", id);
		return -2;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Construct object and assign it an ID from the Realm's ID bank,
// if it does not already have one.
// (static).
////////////////////////////////////////////////////////////////////////////////
short CThing::ConstructWithID(						// Returns 0 if successfull, non-zero otherwise
	ClassIDType id,										// In:  Class ID
	CRealm* pRealm,										// In:  Pointer to realm this object belongs to
 	CThing** ppNew)										// Out: Pointer to new object
	{
	short	sResult	= Construct(id, pRealm, ppNew);
	if (sResult == 0)
		{
		// If new thing has no ID . . .
		if ((*ppNew)->m_u16InstanceId == CIdBank::IdNil)
			{
			sResult = pRealm->m_idbank.Get(*ppNew, &((*ppNew)->m_u16InstanceId) );
			if (sResult != 0)
				{
				delete *ppNew;
				*ppNew	= NULL;
				}
			}
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
