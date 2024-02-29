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
// thing.h
// Project: Postal
//
//
//		01/17/97	JMI	Started DoGui(). Checked in so can work at home.  Compiles.
//
//		01/21/97	JMI	Made ms_aClassInfo[] public.
//
//		01/22/97	JMI	Added bEditorCreatable as member of ClassInfo indicating
//							whether the item is creatable in the editor by the user.
//
//		01/29/97	JMI	Added m_u16InstanceId, an identifier unique to each instance
//							of CThing within its Realm.  Also, added the initialization
//							of this ID in the constructor, the release of the ID in the
//							destructor, and the saving and loading of the ID in Save()
//							and Load().  Note that this value is either created by the
//							editor, loaded from the .rlm file, or assigned by the 
//							server (dynamically created CThings (one's not loaded from
//							the .rlm file) will get their IDs assigned somehow at run-
//							time).
//
//		01/30/97	JMI	Had to move Load() into thing.cpp b/c of dependency on
//							CRealm.  Also, added SetInstanceID() proto.
//
//		02/02/97	JMI	Added virtual EditHotSpot() with stub.
//
//		02/07/97	JMI	Added support for new CGameEditThing.
//							Also, Update() was still pure virtual (must've missed b4
//							when we converted to no pure virtuals)...Fixed.
//
//		02/10/97 BRH	Added CNapalmID and CFireID.
//
//		02/11/97	JMI	Moved CAnim3D to thing.h from dude.h.
//
//		02/11/97	JMI	Added ChanTransform (channel of transforms) that will be
//							useful for rigid bodies.
//							Also, made CAnim3D functions virtual for safety purposes.
//
//		02/11/97 BRH	Added CImbecile and changed CDoofus to non-editing (since
//							it is now  base class).  Also added CFirebomb and 
//							CFirefrag objects.
//
//		02/13/97	JMI	Changing RForm3d to RSop.
//
//		02/14/97	JMI	Changed ChanBounds to RP3d (was REAL).
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
//		02/17/97	JMI	Added collision functions to CThing.
//							Added GetSphere(), which fills in a sphere indicating the
//							position and size of a CThing.
//							Also, added sets.  Every item falls into one of currently
//							four sets: Dudes, Enemies, Weapons, Misc.
//
//		02/17/97 BRH	Changed the message queue to use the new message structure
//							which includes a union of the messages.
//
//		02/18/97	JMI	Now uses spherical regions for collision and R3DLine instead
//							or R3DRay.
//
//		02/18/97	JMI	Changed GetSphere() to GetCollisionSphere().
//							Also, removed ThingSet (now use Things instead (they were
//							defined identically)).
//
//		02/19/97	JMI	Added CAnimThingID CThing class ID.
//							Also, removed stuff having to do with collision sets.
//
//		02/23/97 MJR	Added concept of having optional PreLoad() functions for
//							classes derived from CThing.  The idea is that certain
//							kinds of things are only created while the game is being
//							played, but we don't want them to have to load whatever
//							resources they need at that time because it causes the
//							gameplay to momentarily pause.  Instead, such classes can
//							have a PreLoad() function that gets call when the realm is
//							first loaded, and they can load whatever they need then.
//
//		02/24/97 MJR	Added CSoundThing stuff.
//
//		02/25/97	JMI	Added CGunner stuff.
//
//		03/03/97	JMI	Added m_ptransRigid to CAnim3D.
//
//		03/04/97 BRH	Added CBandID.
//
//		03/05/97	JMI	Added ConstructWidthID() to construct an object and assign
//							it an ID, if it does not already have one.
//
//		03/05/97	JMI	The CAnim3D::Get() overload that sets looping was not
//							checking the success/failure status of the other 
//							CAnim3D::Get() call before accessing the pointers.
//							So, if an animation did not load, a crash or bad stuff
//							were pretty much guaranteed.  Fixed.
//
//		03/06/97	JMI	Added CItem3dID.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		03/17/97 BRH	Added CBarrelID.
//
//		03/19/97 BRH	Added CMineID.
//
//		03/19/97	JMI	Added CDispenserID.
//
//		04/16/97 BRH	Added Jon's template CListNode that replaces the old STL
//							lists in CRealm.  The CListNode contains next and previous
//							pointers rather than having the CThing being put into an
//							STL container in CRealm.  
//
//		04/25/97 BRH	Added CFireballID.
//							Added CCopID.
//
//		04/28/97 BRH	Added fake ID's for the fake classes 
//							CPistolID, CMachineGunID, CShotGunID.
//
//		04/28/97 BRH	Added CPersonID
//
//		04/29/97	JMI	Added Get() to CAnim3D that takes a few simple parameters
//							regarding the resource names which it uses to create all
//							7 resource names.  This is less annoying than the one
//							that requires you allocate an array of strings (which we
//							had done b/c we thought we'd combine various things's
//							meshes, sops, etc. but have not yet done).
//
//		04/29/97 BRH	Added Get() to CAnim3D that takes the base name and the 
//							verb to combine to create the filenames for all parts
//							of each animation.  This is the easiest form to use
//							for CPerson which stores the base name, verb, and 
//							rigid body transform name.
//
//		04/30/97	JMI	Changed CMineID to CProximityMineID and added CTimedMineID,
//							CBouncingBettyMineID, and CRemoteControlMineID.
//
//		05/01/97 BRH	Added CPylonID.
//
//		05/01/97	JMI	Made m_everything and m_nodeClass public.
//
//		05/02/97	JMI	Moved m_pRealm into the public section.
//
//		05/04/97 BRH	Removed STL references since the new CListNode method
//							seems to be working.  Took Tkachuk out of the project
//
//		05/05/97 BRH	Had to put placeholder Tkachuk back in to avoid screwing
//							up all of the realm files.
//
//		05/08/97	JMI	Added CPowerUpID.
//
//		05/09/97 BRH	Added COstrichID.
//
//		05/12/97	JRD	Added CTriggerID.
//
//		05/13/97 BRH	Addec CHeatseekerID.
//
//		05/13/97	JMI	Added CChunkID.
//
//		05/17/97	JMI	Removed out of date functions IsColliding() and
//							GetCollisionSphere().
//
//		05/18/97 BRH	Changed CAnim3D's Get function to ignore rigid body
//							transform files with blank names, so that it is easier
//							to specify animations that don't have rigid body 
//							transforms in the personatorium.
//
//		05/23/97	JMI	Moved CAnim3D from here to Anim3D.cpp/h.
//
//		05/26/97	JMI	Finally broke down and added an RHot* so the editor can
//							quickly from CThing* to RHot*.
//
//		05/26/97 BRH	Added CAssault which is the Shot Gun fired rapidly.  
//							This is just another dummy ID like the rest of the guns.
//
//		06/02/97	JMI	Added CLadderID.
//
//		06/02/97 BRH	Added CSentryID and CSentryGunID.
//
//		06/02/97	JMI	Removed CLadder stuff.
//
//		06/03/97	JMI	Made DoGui() static.
//							Also, added CWarpID.
//
//		06/03/97	JMI	Changed Construct() and ConstructWithID() to check to make
//							sure the creation function exists before calling it.
//
//		06/05/97	JMI	Added m_lDoGuiPressedId and GuiPressed() statics.
//
//		06/09/97 BRH	Added CDemonID.
//
//		06/11/97	JMI	Added CCharacterID.
//
//		06/17/97	JMI	Added a section for functions that should be blocked from
//							used w/i CThings.
//							Added some functions that should not be used from CThings:
//							IsSamplePlaying(*), rspGetMilliseconds(), and 
//							rspGetMicroseconds().
//
//		06/26/97	JMI	Added inline aliases to CRealm's Map3DTo2D()s.
//
//		06/27/97	JMI	Added GetSprite() which returns this thing's sprite or 
//							NULL.
//
//		06/30/97 BRH	Added CGoalTimerID, CFlagID, and CFlagbaseID.
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
//		07/09/97	JMI	Changed FuncPreload to take a pointer to the calling realm
//							as a parameter.
//
//		07/14/97	JMI	Moved Construct() definition into thing.cpp.
//							Now checks to make sure id is bounds.
//
//		07/21/97	JMI	Added GetX(), GetY(), and GetZ().	
//
//		07/30/97	JMI	Added CDeathWadID.
//
//		08/06/97	JMI	Added CDoubleBarrelID.
//
//		08/08/97	JMI	Added more weapons for doofuses:
//								CUziID, CAutoRifleID, CSmallPistolID, CDynamiteID.
//
//		08/10/97	JMI	Added CSndRelayID for CSndRelay.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef THING_H
#define THING_H

#include "RSPiX.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/Channel/channel.h"
	#include "ORANGE/CDT/PQueue.h"
#else
	#include "channel.h"
	#include "pqueue.h"
#endif

#include "game.h"
#include "message.h"
#include "Anim3D.h"
#include "SampleMaster.h"
#include "sprites.h"
#include "smash.h"

// Forward declaration of class to avoid recursive depency of include files
class CRealm;
class CSmash;

// Template node class for linked lists
template <class Owner>
class CListNode
	{
	typedef CListNode Node;

//	protected:
	public:
		CListNode()	{ }	// Do not use.

	public:
		CListNode(Owner* powner)
			{ m_powner	= powner; }

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		Owner* GetNext(void)
			{ return m_pnNext->m_powner; }
		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		Owner* GetPrev(void)
			{ return m_pnPrev->m_powner; }

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		void InsertBefore(
			Node* pn)	// In:  Node to insert before.
			{
			ASSERT(m_pnNext == NULL && m_pnPrev == NULL);
			m_pnNext					= pn;
			m_pnPrev					= pn->m_pnPrev;
			m_pnPrev->m_pnNext	= this;
			pn->m_pnPrev			= this;
			}

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		void AddAfter(
			Node* pn)	// In:  Node to add after.
			{
			ASSERT(m_pnNext == NULL && m_pnPrev == NULL);
			m_pnNext					= pn->m_pnNext;
			m_pnPrev					= pn;
			m_pnNext->m_pnPrev	= this;
			pn->m_pnNext			= this;
			}

		// Note:  This function can only be used with a list that has
		// dummy nodes for head and tail.
		// Note:  Do not call, if already removed.
		void Remove(void)
			{
			m_pnNext->m_pnPrev		= m_pnPrev;
			m_pnPrev->m_pnNext		= m_pnNext;
			m_pnNext						= NULL;
			m_pnPrev						= NULL;
			}

	public:
		Node*		m_pnNext;
		Node*		m_pnPrev;
		Owner*	m_powner;
	};


// This abstract class is the root of all objects that are part of a CRealm.
// Its primary purpose is to force all derived classes to supply a common set
// of functions so all ojects can be accessed in a generic manner.
class CThing
	{
	// Make CRealm a friend so it can access private stuff
	friend class CRealm;

	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:

		// Typedefs for static functions that all derived classes should have
		typedef short (*FuncConstruct)(CRealm* pRealm, CThing** ppNew);
		typedef short (*FuncPreload)(CRealm* pRealm);
		typedef short (*FuncDestroy)(void);

		// Struct containing info about derived classes
		typedef struct
			{
			FuncConstruct funcConstruct;						// Construct() function pointer
			FuncPreload funcPreload;							// Preload() function pointer
			const char* pszClassName;							// Pointer to class name
			bool bEditorCreatable;								// true indicates the editor can
																		// create this object at user 
																		// request.  false indicates it
																		// cannot.
			} ClassInfo;

		// Typedef for class ID's, required because we want specify the type,
		// whereas the compiler always uses type int for enums.
		typedef unsigned char ClassIDType;

		// Class ID's for all derived classes that need to be loaded/saved.  If
		// these numbers change, it will completely invalidate any world files
		// that were created prior to the change!  Add new ID's after existing
		// ID's so the existing ones don't change.
		typedef enum
			{
			// First entry should start at 0!
			CHoodID = 0,
			CDudeID,
			CDoofusID,
			CTkachukID,
			CRocketManID,
			CGrenaderID,
			CRocketID,
			CGrenadeID,
			CBallID,
			CExplodeID,
			CBouyID,
			CNavigationNetID,
			CGameEditThingID,
			CNapalmID,
			CFireID,
			CImbecileID,
			CFirebombID,
			CFirefragID,
			CAnimThingID,
			CSoundThingID,
			CGunnerID,
			CBandID,
			CItem3dID,
			CBarrelID,
			CProximityMineID,
			CDispenserID,
			CFireballID,
			CCopID,
			CPistolID,
			CMachineGunID,
			CShotGunID,
			CPersonID,
			CTimedMineID,
			CBouncingBettyMineID,
			CRemoteControlMineID,
			CPylonID,
			CPowerUpID,
			COstrichID,
			CTriggerID,
			CHeatseekerID,
			CChunkID,
			CAssaultWeaponID,
			CSentryID,
			CSentryGunID,
			CWarpID,
			CDemonID,
			CCharacterID,
			CGoalTimerID,
			CFlagID,
			CFlagbaseID,
			CFirestreamID,
			CDeathWadID,
			CDoubleBarrelID,
			CUziID,
			CAutoRifleID,
			CSmallPistolID,
			CDynamiteID,
			CSndRelayID,

			// This must be the last entry so it gets set to the total number of ID's
			TotalIDs
			};

		typedef enum	// Macros within CThing namespace.
			{
			InvalidPosition	= -5770321
			} Macros;


	//---------------------------------------------------------------------------
	// Protected static member variables
	//---------------------------------------------------------------------------
	protected:
		// Flag to detect attempts to construct a CGameObj before CGameObj's static
		// member variables are initialized by the C++ startup process.  Aside from
		// the typical problems involving C++ initialization order, it doesn't really
		// make sense to create CGameObj's as global/static objects since they are
		// supposed to be created/destroyed dynamically as worlds are loaded.
		static short ms_sDetectStaticInits;

		// This is used by DoGui() to perform GUI processing.
		static RProcessGui	ms_pgDoGui;

	//---------------------------------------------------------------------------
	// Public static member variables
	//---------------------------------------------------------------------------
	public:
		// Array of class info for each derived class
		static ClassInfo ms_aClassInfo[TotalIDs];

	//---------------------------------------------------------------------------
	// Static functions dealing with common static functions in derived classes.
	// The purpose of these functions is to make it easy to call the appropriate
	// static function in a derived class based on its class ID.  This basically
	// impliments something similar to virtual functions, except these are static
	// functions, which the C++ mechanism doesn't support.
	//---------------------------------------------------------------------------
	public:
		// Construct object
		static short Construct(									// Returns 0 if successfull, non-zero otherwise
			ClassIDType id,										// In:  Class ID
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
 			CThing** ppNew);										// Out: Pointer to new object

		// Construct object and assign it an ID from the Realm's ID bank,
		// if it does not already have one.
		static short ConstructWithID(							// Returns 0 if successfull, non-zero otherwise
			ClassIDType id,										// In:  Class ID
			CRealm* pRealm,										// In:  Pointer to realm this object belongs to
 			CThing** ppNew);										// Out: Pointer to new object

	//---------------------------------------------------------------------------
	// Non-static variables
	//---------------------------------------------------------------------------
	public:
		// Prioritized Message queue for Things to use to communicate with each other
		RPQueue <GameMessage, short> m_MessageQueue;

		// Pointer to the realm this object belongs to
		CRealm* m_pRealm;

		// This is intended for the editor.  It would probably be a bad idea to use
		// this pointer outside of gameedit.cpp.
		RHot*	m_phot;

	protected:
		// Flag indicating whether object wants it's Startup() to be called
		short m_sCallStartup;

		// Flag indicating whether object wants it's Shutdown() to be called
		short m_sCallShutdown;

/*
		// Iterator that specifies this object's position in the realm's container
		// of every object and its class-based container of objects.
#if _MSC_VER >= 1020
		Things::const_iterator m_iterEvery;
		Things::const_iterator m_iterClass;
#else
		Things::iterator m_iterEvery;
		Things::iterator m_iterClass;
#endif
*/

		// Class ID is stored here for instant access
		ClassIDType m_id;

		// Unique ID specific to this instance of CThing (set in constructor,
		// released in destructor).
		U16			m_u16InstanceId;

	//---------------------------------------------------------------------------
	// Public member variables
	//---------------------------------------------------------------------------
	public:
	
		CListNode<CThing>	m_everything;	
		CListNode<CThing> m_nodeClass;	


	//---------------------------------------------------------------------------
	// Constructor(s) / destructor
	//---------------------------------------------------------------------------
	protected:
		// Constructor
		CThing(
			CRealm* pRealm,										// In:  Pointer to realm
			ClassIDType id);										// In:  Class ID

	public:
		// Destructor (must be virtual so derived class destructors are always called!)
		virtual ~CThing();

	//---------------------------------------------------------------------------
	// CThing-only functions
	//---------------------------------------------------------------------------
	public:
		// Get object's class ID (must NOT be virtual!)
		ClassIDType GetClassID(void)							// Returns object's class ID
			{
			return m_id;
			}

		// Get object instance's unique ID.
		U16 GetInstanceID(void)
			{
			return m_u16InstanceId;
			}

		// Set object instance's unique ID.
		void SetInstanceID(	// Returns nothing.
			U16	u16Id);		// New id for this instance.

		// Helper for processing your GUIs.
		// Will be made visible by calling pguiRoot->SetVisible(TRUE).
		// GUI will be run with focus-awareness until a GUI with ID 1 or 2 is
		// clicked.  Typically, 1 should be an 'OK' equivalent and 2 'Cancel'.
		// Return value indicates which item was clicked (1, 2, or by the ID
		// of a GUI that was previously passed to SetGuiToNotify() ).
		// Processing involves using queued RSPiX user input via
		// rspGetNextInputEvent().
		static							// Static for your usage pleasure.
		long DoGui(						// Returns ID of item that terminated looping.
											// Returns 0 if rspGetQuitStatus() is nonzero.
											// Returns negative on error.
			RGuiItem*	pguiRoot);	// Root of GUI items to process through user.

		// Call this for any GUIs besides the standard OK (ID 1) and Cancel (ID 2)
		// to set the callback (for on 'pressed') for any GUI you want to end
		// a DoGui().
		static								// Static.
		void SetGuiToNotify(				// Returns nothing.
			RGuiItem* pguiNotifier);	// In:  The pressed GUI.

		// Callback from ms_pgDoGui for system update.
		static								// Static for use as a callback.
		long SysUpdate(					// Returns a non-zero ID to abort or zero
												// to continue.                          
			RInputEvent*	pie);			// Out: Next input event to process.     

		short SendThingMessage(pGameMessage pMessage, U16 u16ID)
			{
				return SendThingMessage(pMessage, pMessage->msg_Generic.sPriority, u16ID);
			}

		short SendThingMessage(pGameMessage pMessage, CThing* pThing)
			{
				return SendThingMessage(pMessage, pMessage->msg_Generic.sPriority, pThing);
			}

		short SendThingMessage(pGameMessage pMessage, short sPriority, U16 u16ID);

		short SendThingMessage(pGameMessage pMessage, short sPriority, CThing* pThing);

		// Maps a 3D coordinate onto the viewing plane.
		void Map3Dto2D(		// Returns nothing.
			short sX,			// In.
			short	sY,			// In.
			short	sZ,			// In.
			short* psX,			// Out.
			short* psY);		// Out.

		// Maps a 3D coordinate onto the viewing plane.
		void Map3Dto2D(		// Returns nothing.
			double	dX,		// In.
			double	dY,		// In.
			double	dZ,		// In.
			double* pdX,		// Out.
			double* pdY);		// Out.

	//---------------------------------------------------------------------------
	// Virtual functions that should be overloaded for additional functionality.
	// The default implementations simply return success.
	//---------------------------------------------------------------------------
	public:
		// Load object (should call base class version!)
		virtual short Load(										// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode,										// In:  True for edit mode, false otherwise
			short sFileCount,										// In:  File count (unique per file, never 0)
			ULONG	ulFileVersion);								// In:  File version being loaded.

		// Save object (should call base class version!)
		virtual short Save(										// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to save to
			short /*sFileCount*/)								// In:  File count (unique per file, never 0)
			{
			// Save this thing's ID.  The ID is unique to this 'thing' within its realm
			// (i.e., no other CThing or derived class has this same ID within this realm).
			// This is assigned by the editor via a call to the realm's m_idbank.Get().
			pFile->Write(m_u16InstanceId);

			return pFile->Error();
			}

		// Startup object
		virtual short Startup(void)							// Returns 0 if successfull, non-zero otherwise
			{
			return 0;
			}

		// Shutdown object
		virtual short Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
			{
			return 0;
			}

		// Suspend object
		virtual void Suspend(void)
			{
			}

		// Resume object
		virtual void Resume(void)
			{
			}

		// Update object
		virtual void Update(void)
			{
			}

		// Render object
		virtual void Render(void)
			{
			}

		// Called by editor to init new object at specified position
		virtual short EditNew(									// Returns 0 if successfull, non-zero otherwise
			short /*sX*/,											// In:  New x coord
			short /*sY*/,											// In:  New y coord
			short /*sZ*/)											// In:  New z coord
			{
			return 0;
			}

		// Called by editor to modify object
		virtual short EditModify(void)						// Returns 0 if successfull, non-zero otherwise
			{
			return 0;
			}

		// Called by editor to move object to specified position
		virtual short EditMove(									// Returns 0 if successfull, non-zero otherwise
			short /*sX*/,											// In:  New x coord
			short /*sY*/,											// In:  New y coord
			short /*sZ*/)											// In:  New z coord
			{
			return 0;
			}

		// Called by editor to get the clickable pos/area of an object in 2D.
		virtual	// If you override this, do NOT call this base class.
		void EditRect(				// Returns nothiing.
			RRect*	prc)			// Out: Clickable pos/area of object.
			{
			// Default implementation makes the object unclickable.
			prc->sX	= 0;
			prc->sY	= 0;
			prc->sW	= 0;
			prc->sH	= 0;
			}

		// Called by editor to get the hotspot of an object in 2D.
		virtual	// If you override this, do NOT call this base class.
		void EditHotSpot(			// Returns nothiing.
			short*	psX,			// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
			short*	psY)			// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
			{
			// Default implementation puts hotspot in upper left corner of
			// EditRect().
			*psX	= 0;
			*psY	= 0;
			}

		// Called by editor to update object
		virtual void EditUpdate(void)
			{
			}

		// Called by editor to render object
		virtual void EditRender(void)
			{
			}

		// Get the sprite for this thing.  If there's more than one, pick one
		// or none to return.
		virtual	// If you override this, do NOT call this base class.
		CSprite* GetSprite(void)	// Returns the sprite for this thing or NULL.
			{ return NULL; }

		// Get the coordinates of this thing.  This implementation returns 
		// InvalidPosition to indicate that it is not implemented for this 
		// class type.  Override these functions for your class type to 
		// enable this feature.
		virtual					// Override to implement this functionality.
		double GetX(void)	{ return InvalidPosition; }

		virtual					// Override to implement this functionality.
		double GetY(void)	{ return InvalidPosition; }

		virtual					// Override to implement this functionality.
		double GetZ(void)	{ return InvalidPosition; }

		// Get the smash - for normal CThings that don't have a smash, it
		// will return NULL, CThing3d's though always have a smash.
		virtual 
		CSmash* GetSmash(void) {return NULL;}

		//////////////////////////////////////////////////////////////////////////
		// These are defined merely to discourage their use within CThings.
		// They can cause problems when they are used in a chain of events that
		// leads up to a rand() b/c they do not act the same from run to run and/or
		// from machine to machine.
		//
		// DO NOT USE THE THESE FUNCTIONS!!! DO __NOT__ SIMPLY PUT A '::' IN FRONT
		// OF YOUR REF TO THESE FUNCTIONS!!!!
		//////////////////////////////////////////////////////////////////////////

		// Never rely on sounds finishing w/i a CThing.  This varies greatly from
		// machine to machine (and, in many cases, from run to run).
		bool IsSamplePlaying(	 
										
			SampleMasterID	/*id*/)	
			{
			ASSERT(0);
			return false;
			}

		bool IsSamplePlaying(	
			RSnd*	/*psnd*/,				
			SampleMasterID /*id*/)	
			{
			ASSERT(0);
			return false;
			}

		long rspGetMilliseconds(void)
			{
			ASSERT(0);
			return 0;
			}
		
		long rspGetMicroseconds(
			short /*sReset	= FALSE*/)
			{
			ASSERT(0);
			return 0;
			}

	};


#endif // THING_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
