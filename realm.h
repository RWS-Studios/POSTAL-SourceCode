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
// realm.h
// Project: Postal
//
//	History:
//		01/22/97	JMI	Added more LayerSpriteN, LayerOpaqueN, LayerAlphaN enums.
//		
//		01/23/97	JMI	Added macro enums LayerAttribMask and LayerAttribShift that
//							can be used on an attribute to get the sprite layer flags
//							indicated by the attribute.  This value can be mapped through
//							the new static member ms_asAttribToLayer.  Or, you can just
//							use the new inline static member CRealm::GetLayerFromAttrib().
//		
//		01/27/97	JMI	Now 4 bits are used from the attribute to determine the proper
//							layer for dudes.
//		
//		01/28/97	JMI	Added ms_apszLayerNames[] to access the names of the layers.
//		
//		01/29/97	JMI	Reordered Layers enum (fixed to the way Steve is doing it
//							(the opaque layers are now before equivalent alpha layers)).
//		
//		02/02/97	JMI	Added m_resmgr to load resources for each realm instance.
//		
//		02/04/97 BRH	Added ms_pCurrentNavNet and GetCurrentNavNet() functiion
//							to the realm.  Previously the current NavNet was stored
//							in the editor which was OK for bouys but difficult to get
//							for other game objects.
//
//		02/13/97	JMI	Added hood pointer to CRealm.
//
//		02/17/97	JMI	Added set lists to CRealm.  Now there is a list for each
//							set of Things.  The sets are enumerated in CThing.
//
//		02/18/97	JMI	Added GetNext*Collision() functions for a sphere and a
//							line segment.
//
//		02/19/97	JMI	Removed stuff having to do with old collision sets and
//							added new Smashatorium for collision detection.
//
//		02/23/97	JMI	Added m_iNext and m_bUpdating so RemoveThing() and 
//							AddThing() can make the necessary adjustments to the next
//							iterator processed in the Update() loop.
//
//		03/13/97	JMI	Upped FileVersion to 2 for new CDude stuff.
//
//		03/14/97	JMI	Upped FileVersion to 3 for new CThing3d base class for
//							CCharacter.
//
//		03/18/97	JMI	Upped FileVersion to 4 for CBand, which now loads info
//							for creating child items.
//
//		04/15/97 BRH	Upped FileVersion to 5 for CMine, which now saves the
//							fuse time for the timed mines.
//
//		04/17/97 BRH	Took timeout check out of Shutdown and Startup (it was
//							already commented out of Startup).  The 1 second timeout
//							when calling shutdown caused large levels like parade
//							to abort before saving everything.
//
//		04/20/97 BRH	Changed FileVersion to 6 for CBouy which now saves
//							a message.
//
//		04/21/97	JMI	Added m_sNumSuspends and IsSuspended().
//
//		04/23/97	JMI	Upped file version to 7 for new CDispenser parameter,
//							m_sMaxDispensees.
//
//		04/24/97	JMI	Upped file version to 8 for newly saved CBarrel parameter,
//							m_dRot.
//
//		04/24/97	JMI	Upped file version to 9 for new CDude parameter,
//							m_sNumShells.
//
//		04/25/97	JMI	Upped file version to 10 for new CDude parameter,
//							m_sNumFuel.
//
//		04/29/97	JMI	Changed GetHeight() to return 0 for the clipped case.
//
//		04/30/97	JMI	Upped file version to 11 since CMine no longer saves its
//							type.  The type (which is now determined by the class ID)
//							is saved by the base class CThing.
//
//		05/01/97 BRH	Updated FileVersion to 12 since its May Day.  (and because
//							on May Day I removed the message saving from the Bouy)
//
//		05/04/97 BRH	Removed the STL version since all of the lists seem to
//							be working fine with the CListNode lists.
//
//		05/09/97 JRD	Added an attribute map for trigger regions and a pylon ID table
//
//		05/13/97	JMI	Upped file version to 13 for new CDude parameters,
//							m_sNumMines and m_sNumHeatseekers.
//
//		05/13/97	JMI	Added IsPathClear() map functions to determine if a path
//							is clear of terrain that cannot be surmounted.
//
//		05/16/97	JMI	Changed layer bits to be 8 bits (instead of 4).  This
//							caused the REALM_ATTR_EFFECT_MASK to lose 4 bits and the
//							REALM_ATTR_LIGHT_BIT to move up 4 bits (0x0010 to 0x0100).
//							Also, the table ms_asAttribToLayer to be increased from
//							16 entries to 256 entries.
//							Also, removed GetLayerFromAttrib() which was left over from
//							the pre RMultiGrid days.
//
//		05/19/97 BRH	Changed file version to 14 to support thought balloon flag
//							for the CPerson.
//
//		05/26/97 BRH	Added population variables to keep track of Numer of 
//							people killed, number of people alive etc.
//
//		05/26/97	JMI	Added IsEndOfLevelGoalMet() based on population vars added 
//							above.
//							Also, added DrawStatus().
//							Also, increased file version to 15 so hood can save end of
//							level goal.
//
//		05/29/97	JMI	Changed REALM_ATTR_LAYER_MASK to 0x7fff (was 0x00ff).
//							Moved REALM_ATTR_EFFECT_MASK and REALM_ATTR_LIGHT_BIT to
//							m_pTerrainMap (was in m_pAttrMap).  This required the light
//							bit to go from 0x0100 to 0x0200 ('no walk' was already 
//							using 0x0100 in m_pTerrainMap).
//							Changed all occurences of m_pAttrMap to m_pLayerMap.
//							Changed all occurences of m_pHeightMap to m_pTerrainMap.
//							Also, added CreateLayerMap() that creates the attribute
//							to layer map now that it is so huge.
//
//		06/05/97	JMI	Upped FileVersion to 16 to accommodate new CStockPile,
//							m_stockpile, in CThing3d.
//
//		06/06/97	JMI	Upped FileVersion to 17 to accommodate new CStockPile,
//							m_stockpile, in CPowerUp.
//
//		06/09/97	JMI	Upped FileVersion to 18 to allow CItem3d to save its
//							rotation velocities.
//
//		06/09/97	JMI	Upped FileVersion to 19 to allow CStockPile to save its
//							new weapon flags.
//
//		06/14/97	JMI	Upped FileVersion to 20 to allow CPowerUp to be descended
//							from CThing3d (instead of CThing).
//
//		06/15/97	JMI	Upped FileVersion to 21 to allow CStockPile to load and
//							save its new m_sKevlarLayers.
//
//		06/15/97	JMI	Upped FileVersion to 22 to allow CStockPile to load and
//							save its new m_sBackpack.
//
//		06/17/97 MJR	Moved some vars that were CPylon statics into the realm
//							so they could be instantiated on a realm-by-realm basis.
//
//		06/26/97 BRH	Changed the population scoring to go down as people get
//							killed.
//
//		06/26/97	JMI	Moved Map2DTo3D from reality.h to here.
//
//		06/28/97	JMI	Moved attribute access functions from realm.h to realm.cpp
//							while we're getting all the conversion from 3D to the X/Z
//							plane stuff right.  Compiling the entire project for any
//							tweak just doesn't sound very fun.  Hopefully, though,
//							there'll won't be many tweaks.
//							Also, upped FileVersion to 23 so CHood can save its new
//							m_sRealmRotX param.
//
//		06/28/97 BRH	Added score variables for separate hostiles and civilians
//							and changed the RegisterBirth, RegisterDeath functions
//							to change the values for each different type.
//
//		06/29/97	JMI	Added version of ScaleZ() that takes shorts.
//							Changed both versions of ScaleZ() to MapZ3DtoY2D()
//							and added two versions of MapY2DtoZ3D().
//
//		06/30/97	JMI	Upped file version to 24 so things can know whether to 
//							convert their cheezy 3D realm coords to real 3D realm 
//							coords.
//
//		06/30/97	JMI	Added GetRealmWidth() and GetRealmHeight() to get the
//							realm's width and height on the X/Z plane.
//
//		06/30/97	JMI	Added bCheckExtents parm to IsPathClear().  See proto for
//							details.
//
//		07/01/97	JMI	Added MapY2DtoY3D() and MapY3DtoY2D().
//							Also, added GetHeightAndNoWalk().
//
//		07/01/97 BRH	Changed the file version for the Sentry gun since it now
//							loads and saves the rotational velocity parameter.
//
//		07/01/97	JMI	Changed the file version for the Hood so it can load and
//							save whether to use the attribute map heights with or
//							without scaling based on the view angle.
//							Also, added function to scale heights that checks the
//							hood value.
//
//		07/04/97 BRH	Added timer for Score to display and CGoalTimer to 
//							manipulate.
//
//		07/07/97 BRH	Added scoring modes to the realm so that they can more
//							easily be set by options at the beginning of the game,
//							changed by CThings, and read by the scoring module.
//							Changed the file version for the realm so that it can
//							save the new properties.
//
//		07/09/97	JMI	Added function to get the full path for a 2D resource
//							based on the current hood setting for 'Use top-view 2Ds'.
//							Also, upped file version the hood can save this setting.
//
//		07/09/97 BRH	Increased the file version to add additional data to
//							the realm file.
//
//		07/09/97	JMI	Moved m_s2dResPathIndex from CHood to CRealm b/c of order
//							issues when loading.  Also, increased the file version so
//							CHood can not load/save this data and CRealm can.
//
//		07/10/97 BRH	Added m_sFlagsCaptured to keep track of the flag count
//							for capture the flag and checkpoint games.  Also moved
//							IsEndOfLevelGoalMet to the cpp file and added cases
//							for the different scoring types.
//
//		07/12/97	JMI	Added m_bMultiplayer, which signifies whether this is
//							a multi or single player game.
//							Also, added Init().  See proto for details.
//
//		07/14/97 BRH	Added m_sFlagbaseCaptured for the levels were the flags
//							must be returned to their proper base in order to count.
//
//		07/15/97 BRH	Added a parameter to IsEndOfLevelGoalMet which now takes
//							a bool telling whether the end of level key has been 
//							hit, indicating that the user wants to go on if the
//							standard play mode goal has been fulfilled.
//
//		07/18/97	JMI	Upped the file version so CSoundThing can load its volume
//							half life.
//
//		07/19/97	JMI	Upped the file version so CWarp can save the initial 
//							rotation for the warped in CDude.
//
//		07/23/97	JMI	Changed file version so CStockPile can save new 
//							m_sNapalmLauncher member.
//
//		07/24/97 MJR	Changed file version so CSoundThing can save more members.
//
//		07/27/97 BRH	Added a variable to save what the score timer was
//							initially set to in order to determine the time
//							elapsed for high score purposes.
//
//		07/28/97	JMI	Upped file version to 35 so CDispenser could save an 
//							additional logic parm.
//
//		07/30/97 BRH	Added a string to hold the name of the realm which
//							will be used for storing high scores for a particular
//							realm.
//
//		07/30/97	JMI	Upped file version so CStockPile can save/load new member.
//
//		07/31/97 BRH	Changed file version so band member can save dest bouy.
//
//		08/01/97	JMI	Upped file version to 38 so CSoundThing can save new 
//							members. 
//
//		08/03/07	JRD	Upped the verion to 39 to save 3d scale.  Note that this is
//							the FINAL 3x version possible.  Soon we will move to 40 and
//							32 bit code.
//
//		08/04/97	JMI	Upped file version to 40 so CSoundThing can save 
//							m_sAmbient.
//
//		08/05/97	JMI	Added CRealm::Flags struct and an instance in CRealm, 
//							m_flags.
//
//		08/06/97	JMI	Upped file version to 41 so CStockPile can save/load new
//							member.
//
//		08/07/97	JMI	Upped file version to 42 so CItem3d can save/load new
//							members.
//
//		08/08/97 BRH	Upped file version to 43 to save extra special bouy data
//							in the Doofus.
//
//		08/09/97	JMI	Added progress callback and components.  There is now a
//							callback member that is called (when non-zero) that passes
//							the number of items processed so far and the total number of
//							items to process.  The return value of the callback dictates
//							whether to proceed with the operation.  Although, this is 
//							meant to be generic so we can use it for any type of 
//							process, it is currently only used by Load() and Save().
//
//		08/10/97	JRD	Upped File Version to 44 to save shadow info.
//
//		08/11/97 BRH	Upped File Version to 45 to save flag color info.
//
//		08/15/97 BRH	Upped File version to save barrel special flag.
//
//		08/20/97	JMI	Made ms_apsz2dResPaths[] a static class member (was just
//							defined at realm.cpp file scope) and added enum macro for
//							number of elements.
//
//		11/21/97	JMI	Added bCoopMode flag indicating whether we're in cooperative
//							or deathmatch mode when in multiplayer.
//
//		12/02/97	JMI	Increased FileVersion to 47 so CDemon could save its new
//							m_sSoundBank.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef REALM_H
#define REALM_H

#include "RSPiX.h"
#include "scene.h"
#include "thing.h"
#include "hood.h"
#include "yatime.h"
#include "IdBank.h"
#include "smash.h"
#include "trigger.h"

///////////////////////////////////////////////////////////////////////////////
// If PATHS_IN_INCLUDES macro is defined, we can utilize relative
// paths to a header file.  In this case we generally go off of our
// RSPiX root directory.  System.h MUST be included before this macro
// is evaluated.  System.h is the header that, based on the current
// platform (or more so in this case on the compiler), defines 
// PATHS_IN_INCLUDES.  Blue.h includes system.h so you can include that
// instead.
///////////////////////////////////////////////////////////////////////////////
#ifdef PATHS_IN_INCLUDES
	#include "WishPiX/ResourceManager/resmgr.h"
	#include "ORANGE/MultiGrid/MultiGrid.h"
	#include "ORANGE/MultiGrid/MultiGridIndirect.h"
#else
	#include "resmgr.h"
	#include "multigrid.h"
	#include "multigridindirect.h"
#endif

// The overall "universe" in which a game takes place is represented by one or
// more CRealm's.  A realm is basically a collection of objects plus a handfull
// of functions for managing those objects.  These functions include loading,
// saving, suspending, resuming, updating, and rendering the realm.
//
// All of the objects within a realm must be derived from CThing.
// I'm thinking that the way objects know what realm they belong to is that
// we pass a pointer to the realm to the object's constructor.  All objects
// would need to define such a constructor or some other function that
// indirectly constructs the object.  Actually, I don't think objects need
// to jump through hoops anymore to stay alive because there would now be
// a simplified mechanism by which an object can move from realm to realm!
// If that's the case, then we might be able to heavily simplify the current
// object model.
//
// Is there any way around the requirement that everything derive from a single
// object?  Since we're going to pass the constructor (or whatever) a pointer
// to the realm, couldn't that object register various functions with the
// realm, which would essentially be our own version of virtual functions?
// We're already doing this when it comes to static functions since those can't
// be virtual in C++, so why not extend it to all functions?  The main benefit
// of getting away from a single base class is that it allows more flexibility
// on the part of the implimentation of individual objects, and decouples the
// objects, which always seems like a good idea.  The negative side is that it
// takes a bit more effort on the part of the ll the objects to comply with the
// rules.
//
// A simple game might consist of a number of realm files that are loaded one
// after another as the player "completes" each realm.  In this case, the game
// could utilize a single realm object since only one realm exists at any time.
//
// A more complex game might allow the player to move back and forth between
// certain realms.  This could still be handled , and might want those realms to 
//
// When we talk about loading and saving a realm or an individual thing, there's
// really two issues.  One is saving and loading the actual data related to
// that thing, and the other involves the resources.  The first part is basically
// unvavaoidable and fast -- it usually doesn't involve much data.  It's the
// second part, the resources, that may take a significant amount of time.
// Under the new realm-based scheme, I'm thinking that objects wouldn't free
// their resources until their were destroyed.  If the realm is cleared or
// destroyed, then the objects in it are destroyed.  Very simple, without any
// of the other complexities that the old model had.
//
// To move objects from one realm to another, we could create a temporary
// realm, move the objects into that, then destroy the old realm, then create
// the new one, then transfer the new objects into it.  Or maybe we just create
// the new realm but don't load it, transfer the objects into it, then destroy
// the old one, then load the new one.  Either way, it works pretty well.
//
// Not sure yet how to transfer objects between realms.  I'm thinking it's best
// to call the object and let it handle it, since it may want/need to transfer
// other related objects, or do things prior or after transferring, and so on.
//
// I'm also not sure whether we should offer all objects the chance to transfer
// or just call those we know need to transfer (meaning at the game level).
// I'm leaning towards just calling specific objects, because the other way
// we would need to convey to the objects the reason for the transfer, which 
// seems like it will be kind of vague.  But it would be nicer in the sense that
// we wouldn't have to be carefull to call the correct objects.  This doesn't
// need to be decided just yet.

// Masks and bits for m_pTerrainMap:
// The height map contains the height, the 'no walk', and the light attributes.
#define REALM_ATTR_HEIGHT_MASK	0x00ff
#define REALM_ATTR_FLOOR_MASK		0xff00
#define REALM_ATTR_EFFECT_MASK	0xff00
#define REALM_ATTR_LIGHT_BIT		0x0200
#define REALM_ATTR_NOT_WALKABLE	0x0100

// Masks and bits for m_pLayerMap:
// The attribute map contains only the layer bits.
#define REALM_ATTR_LAYER_MASK		0x7fff

#define REALM_NONSTL 1

class CNavigationNet;

class CRealm
	{
	friend class CNavigationNet;
	//---------------------------------------------------------------------------
	// Types, enums, etc.
	//---------------------------------------------------------------------------
	public:
		// Miscellaneous enums
		enum
			{
			FileID = 0x44434241,									// File ID
			FileVersion = 49,										// File version
			Num2dPaths	= 3										// Number of 2D res paths
			};

		enum	// Macros.
			{
			LayerAttribMask	= REALM_ATTR_LAYER_MASK	// Mask against attribute to 
																	// get appropriate sprite
																	// layer.
			};

		// Layer enums
		typedef enum
			{
			// First entry must start at 0!
			LayerBg = 0,

 			LayerSprite1,
			LayerOpaque1,
 			LayerSprite2,
			LayerAlpha1,

			LayerSprite3,
			LayerOpaque2,
 			LayerSprite4,
			LayerAlpha2,

			LayerSprite5,
			LayerOpaque3,
 			LayerSprite6,
			LayerAlpha3,

 			LayerSprite7,
			LayerOpaque4,
 			LayerSprite8,
			LayerAlpha4,

			LayerSprite9,
			LayerOpaque5,
 			LayerSprite10,
			LayerAlpha5,

			LayerSprite11,
			LayerOpaque6,
			LayerSprite12,
			LayerAlpha6,

			LayerSprite13,
			LayerOpaque7,
			LayerSprite14,
			LayerAlpha7,

			LayerSprite15,
			LayerOpaque8,

			LayerSprite16,

			// This must be the last entry so it gets set to the total number of layers
			TotalLayers
			} Layer;

		// Scoring modes
		typedef unsigned short ScoringMode;
		typedef enum
			{
			Standard = 0,		// Standard single player showing population, hostiles, kills and kill %
			Timed,				// Score as many kills as possible in set amount of time
			TimedGoal,			// Kill the set number of enemies before time expires
			TimedFlag,			// Capture the flag before time expires
			Goal,					// Kill the set number of enemies as quickly as possible
			CaptureFlag,		// Capture the flag - no time limit
			Checkpoint,			// Grab all of the flags before time runs out, each flag gives bonus time
			MPTimed,				// Multiplayer Timed - Score the most frags in the given amount of time
			MPFrag,				// First person to Frag Limit wins
			MPLastMan,			// Last man standing wins - scoring shows who's dead & alive
			MPCaptureFlag,		// Capture your flag and return to base.
			MPTimedFlag,		// Capture your flag before time runs out
			MPTimedFrag,		// Time limit and frag limit for MP games
			MPLastManFrag,		// No rejuvination with frag limit
			MPLastManTimed,	// No rejuvination with time limit
			MPLastManTimedFrag,//No rejuvination with time and frag limit

			TotalScoringModes
			};

		// Realm flags.
		typedef struct
			{
			bool		bMultiplayer;				// true, if multiplayer game; false, if
														// single player game.
														// Note that this has nothing to do with
														// whether the level was designed or in-
														// tended as a single or multiplayer
														// level.
			bool		bCoopMode;					// true, if in cooperative mode; false, if
														// in deathmatch mode.  N/A if bMultiplayer
														// is false.
			bool		bEditing;					// In editor mode.  That is, in the mode
														// where one can create new objects, modify,
														// move them, etc.
			bool		bEditPlay;					// Playing a realm from within the editor.
			short		sDifficulty;				// Difficulty level.
			} Flags;

		// Callback called by various processes in Realm (such as Load and Save)
		// to indicate current progress and allow a hook that can abort the process.
		typedef bool (*ProgressCall)(			// Returns true to continue; false to
														// abort operation.
			short	sLastItemProcessed,			// In:  Number of items processed so far.
			short	sTotalItemsToProcess);		// In:  Total items to process.

	//---------------------------------------------------------------------------
	// Static variables
	//---------------------------------------------------------------------------
	private:
		static short ms_sFileCount;

	//---------------------------------------------------------------------------
	// Public static variables
	//---------------------------------------------------------------------------
	public:
		// Maps the layer portion of an attribute to the appropriate
		// layer.
		static short ms_asAttribToLayer[LayerAttribMask + 1];
		
		// Names of layers.  Use Layer enum values to index.
		static char* ms_apszLayerNames[TotalLayers];

		// 2D resource paths.
		static char* ms_apsz2dResPaths[Num2dPaths];


	//---------------------------------------------------------------------------
	// Non-static variables
	//---------------------------------------------------------------------------
	public:

		CListNode<CThing>* m_pNext;

		// This flag indicates (by true) that we are in the Update() loop.
		// It is false at all other times.
		bool	m_bUpdating;
		
		// The scene, which is basically the visual representation of the realm
		CScene m_scene;

		// Head and tail pointers for all CThings.
		CListNode<CThing> m_everythingHead;
		CListNode<CThing> m_everythingTail;
		short m_sNumThings;

		// Array of Head and Tail pointers to the various lists of derived CThings
		// in the game.  The array is indexed by the class ID, and each list contains
		// only objects of that type.		
		CListNode<CThing> m_aclassHeads[CThing::TotalIDs];
		CListNode<CThing> m_aclassTails[CThing::TotalIDs];
		short m_asClassNumThings[CThing::TotalIDs];

		// Pointer to the attribute map.  The CHood is expected to set these
		// as soon as it can so that other obects can use it.  This is really
		// just a shortcut to the attribute map since so many objects need to use
		// it so often.
		// The CHood should also clear these when the map is destroyed.

		RMultiGrid* m_pTerrainMap;
		RMultiGrid* m_pLayerMap;
		RMultiGridIndirect* m_pTriggerMap; // This is a shadow reference
		CTrigger* m_pTriggerMapHolder;	// This points to the CThing holding the actual map

		// Pointer to the CHood.  The CHood is expected to set this as soon as it
		// is allocated so that other objects can use this to access it.  Since
		// there is only one and it is often access every iteration, this'll make
		// it a bit quicker.
		// The CHood should also clear this when it is destroyed.
		CHood*			m_phood;

		// Time object, which manages game time
		CTime m_time;

		// Bank of IDs so each item in Realm can be uniquely identified.
		CIdBank	m_idbank;

		// Resource manager.  All resources loaded by CThings in this realm should
		// be loaded through this.
		// The base path and/or SAK, will be set via the CHood.
		RResMgr	m_resmgr;

		// Smashitorium.  Used for collision detection.  Add your CSmash to this
		// CSmashitorium to be included in collision detection for this CRealm.
		CSmashatorium	m_smashatorium;

		// Number of Suspend() calls that have occurred without corresponding 
		// Resume() calls.
		// If 0, we are not suspended.
		short m_sNumSuspends;

		// Pylon stuff
		USHORT	m_asPylonUIDs[256]; // Complete Cheese!
		short		m_sNumPylons;
		UCHAR		m_ucNextPylonID;

		// Path index for 2D assets.
		short		m_s2dResPathIndex;

		// Process progress callback.  See ProgressCall typedef for details.
		ProgressCall	m_fnProgress;

	protected:
		CNavigationNet* m_pCurrentNavNet;

	public:

		// Population variables to keep track of those alive and dead.
		short m_sPopulationBirths;			// Number of enemies & victims born in this level
		short m_sPopulation;					// Current population
		short m_sPopulationDeaths;			// Number of enemies & victims killed in this level
		short m_sHostileBirths;				// Number of hostiles that have been born in this level
		short m_sHostileKills;				// Number of hostiles that have been killed
		short m_sHostiles;					// Number of hostiles remaining alive

		// Timer values for different scoring methods.
		bool m_bScoreTimerCountsUp;		// Which direction the timer runs, up or down
		long m_lScoreInitialTime;			// What timer was initially set to.
		long m_lScoreTimeDisplay;			// Time in ms that is used for the Score display
													// (score converts it to minutes:seconds)
		long m_lPrevTime;						// Previous read of the clock
		long m_lElapsedTime;
		long m_lThisTime;
		

		long	m_lLastStatusDrawTime;		// Last time the status was drawn.

		// Goals for different modes of play.
		ScoringMode m_ScoringMode;			// Scoring mode that score module uses
													// to determine what to display.
		double	m_dKillsPercentGoal;		// Kill Percent needed to end the level.
		short		m_sKillsGoal;				// Kill number needed to end the level
		short		m_sFlagsGoal;				// Number of flags needed to end the level

		short		m_sFlagsCaptured;			// Number of flags captured.
		short		m_sFlagbaseCaptured;		// Number of flags returned to proper base

		RString	m_rsRealmString;			// Name of realm (currently full path of file)

		Flags		m_flags;						// Realm flags.

		bool	m_bPressedEndLevelKey;	// True if the player pressed the key or controller button bound to "End Map"
										// Different from the hardcoded F1 key

	//---------------------------------------------------------------------------
	// Non-static functions
	//---------------------------------------------------------------------------
	public:
	
		// Default (and only) constructor
		CRealm();

		// Destructor
		~CRealm();

		// Register a birth for this realm - Each enemy and victim should call
		// this when they are created.
		void RegisterBirth(bool bCivilian = false)
		{
			m_sPopulationBirths++;
			m_sPopulation++;
			if (!bCivilian)
			{
				m_sHostiles++;
				m_sHostileBirths++;
			}
		};

		// Register a death for this realm
		void RegisterDeath(bool bCivilian = false, bool bPlayerKill = true)
		{
			// Added bPlayerKill to make it easier to get the Boondock Saints achievement.
			if (StatsAreAllowed && bPlayerKill)
			{
				// need to unlock achievement in here, too, since this fires when starting a level without you killing anything.
				UnlockAchievement(ACHIEVEMENT_KILL_FIRST_VICTIM);

				Stat_TotalKilled++;
				if (Stat_TotalKilled >= 100)
					UnlockAchievement(ACHIEVEMENT_KILL_100);
				if (Stat_TotalKilled >= 1000)
					UnlockAchievement(ACHIEVEMENT_KILL_1000);
				if (Stat_TotalKilled >= 10000)
					UnlockAchievement(ACHIEVEMENT_KILL_10000);

				if (!bCivilian)
					Stat_KilledHostiles++;
				else
				{
					Flag_Achievements &= ~FLAG_KILLED_ONLY_HOSTILES;
					Stat_KilledCivilians++;
				}
			}

			m_sPopulationDeaths++;
			// Added this to make the population display go down.
			m_sPopulation--;

			if (!bCivilian)
			{
				m_sHostileKills++;
				m_sHostiles--;
			}
		}

		// Check if kills percent goal has been met.
		bool IsEndOfLevelGoalMet(bool bEndLevelKey);	// Returns true, if end of level goal
																	// has been met.

/*
		// Add thing to realm
		void AddThing(
			CThing* pThing,										// In:  Pointer to thing to be added
			CThing::ClassIDType id,								// In:  Thing's class ID
#if _MSC_VER >= 1020
			CThing::Things::const_iterator* piterEvery,	// Out: Iterator into everything container
			CThing::Things::const_iterator* piterClass)	// Out: Iterator into class container
#else
			CThing::Things::iterator* piterEvery,			// Out: Iterator into everything container
			CThing::Things::iterator* piterClass)			// Out: Iterator into class container
#endif
			{
			*piterEvery = m_everything.insert(m_everything.end(), pThing);
			*piterClass = m_apthings[id]->insert(m_apthings[id]->end(), pThing);

			// If in the update loop . . .
			if (m_bUpdating == true)
				{
				// If the next is the end . . .
				if (m_iNext == m_everything.end())
					{
					// Get the added thing.
					m_iNext	= *piterEvery;
					}
				}
			}
*/
		// Add thing to realm
		void AddThing(
			CThing* pThing, 
			CThing::ClassIDType id)
			{
			pThing->m_everything.InsertBefore(&m_everythingTail);
			pThing->m_nodeClass.InsertBefore(&(m_aclassTails[id]));			

			m_sNumThings++;
			m_asClassNumThings[id]++;

			// If in the update loop...
			if (m_bUpdating == true)
				{
				// If the next is the end...
				if (m_pNext == &m_everythingTail)
					{
					// Set next to the newly added thing
					m_pNext = &(pThing->m_everything);
					}
				}
			}

		// Remove thing from realm
/*
		void RemoveThing(
			CThing::ClassIDType id,								// In:  Thing's class ID
#if _MSC_VER >= 1020
			CThing::Things::const_iterator iterEvery,		// In: Thing's iterator into everything container
			CThing::Things::const_iterator iterClass)		// In: Thing's iterator into class container
#else
			CThing::Things::iterator iterEvery,				// In: Thing's iterator into everything container
			CThing::Things::iterator iterClass)				// In: Thing's iterator into class container
#endif
			{
			// If in the update loop . . .
			if (m_bUpdating == true)
				{
				// If the next is thing being removed . . .
				if (m_iNext == iterEvery)
					{
					// This is safe b/c the end is always available and will
					// never be removed.
					m_iNext++;
					}
				}

			m_everything.erase(iterEvery);
			m_apthings[id]->erase(iterClass);
			}
*/
		// Remove thing from realm
		void RemoveThing(
			CThing* pThing)
			{
			// If in the update loop...
			if (m_bUpdating == true)
				{
				// If the next is the thing being removed...
				if (m_pNext == &(pThing->m_everything))
					{
					// This is save b/c the end is always available and will never be removed
					m_pNext = pThing->m_everything.m_pnNext;
					}
				}

			pThing->m_everything.Remove();
			pThing->m_nodeClass.Remove();
			m_sNumThings--;
			m_asClassNumThings[pThing->GetClassID()]--;
			}

		// Clear
		void Clear(void);

		// Determine if specified file exists according to same rules used by Load()
		static
		bool DoesFileExist(										// Returns true if file exists, false otherwise
			const char* pszFileName);							// In:  Name of file

		// Open the specified realm file (primarily for internal use, but who knows, it might be usefull)
		static
		short Open(													// Returns 0 if successfull, non-zero otherwise
			const char* pszFileName,							// In:  Name of file to load from
			RFile* pfile);											// I/O: RFile to be used

		// Load
		short Load(													// Returns 0 if successfull, non-zero otherwise
			const char* pszFileName,							// In:  Name of file to load from
			bool bEditMode);										// In:  Use true for edit mode, false otherwise

		// Load
		short Load(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile,											// In:  File to load from
			bool bEditMode);										// In:  Use true for edit mode, false otherwise

		// Save
		short Save(													// Returns 0 if successfull, non-zero otherwise
			const char* pszFileName);							// In:  Name of file to save to

		// Save
		short Save(													// Returns 0 if successfull, non-zero otherwise
			RFile* pFile);											// In:  File to save to

		// Startup
		short Startup(void);										// Returns 0 if successfull, non-zero otherwise

		short Shutdown(void);									// Returns 0 if successfull, non-zero otherwise

		// Suspend
		void Suspend(void);

		// Resume
		void Resume(void);

		bool IsSuspended(void)	// Returns true, if suspended; false, otherwise.
			{ return (m_sNumSuspends == 0) ? false : true; }

		// Update
		void Update(void);

		// Render
		void Render(void);

		// Edit-mode update
		void EditUpdate(void);

		// Edit-mode render
		void EditRender(void);

		// Edit-Modify dialog - set properties for the realm like scoring & play mode.
		void EditModify(void);

		// Give the current network for this realm
		CNavigationNet* GetCurrentNavNet(void)
			{ return m_pCurrentNavNet; }

		short GetHeight(short sX, short sZ);

		short GetHeightAndNoWalk(	// Returns height at new location.
			short sX,					// In:  X position to check on map.
			short	sZ,					// In:  Z position to check on map.
			bool* pbNoWalk);			// Out: true, if 'no walk'.

		short GetTerrainAttributes(short sX, short sZ);

		short GetFloorAttribute(short sX, short sZ);

		short GetFloorMapValue(short sX, short sZ, short sMask = 0x007f);

		short GetLayer(short sX, short sZ);

		short GetEffectAttribute(short sX, short sZ);

		short GetEffectMapValue(short sX, short sZ);
			
		// Determine if a path is clear of terrain.
		bool IsPathClear(						// Returns true, if the entire path is clear.
													// Returns false, if only a portion of the path is clear.
													// (see *psX, *psY, *psZ).
			short sX,							// In:  Starting X.
			short	sY,							// In:  Starting Y.
			short sZ,							// In:  Starting Z.
			short sRotY,						// In:  Rotation around y axis (direction on X/Z plane).
			double dCrawlRate,				// In:  Rate at which to scan ('crawl') path in pixels per
													// iteration.
													// NOTE: Values less than 1.0 are inefficient.
													// NOTE: We scan terrain using GetHeight()
													// at only one pixel.
													// NOTE: We could change this to a speed in pixels per second
													// where we'd assume a certain frame rate.
			short	sDistanceXZ,				// In:  Distance on X/Z plane.
			short sVerticalTolerance = 0,	// In:  Max traverser can step up.
			short* psX = NULL,				// Out: If not NULL, last clear point on path.
			short* psY = NULL,				// Out: If not NULL, last clear point on path.
			short* psZ = NULL,				// Out: If not NULL, last clear point on path.
			bool bCheckExtents = true);	// In:  If true, will consider the edge of the realm a path
													// inhibitor.  If false, reaching the edge of the realm
													// indicates a clear path.


		// Determine if a path is clear of terrain.
		bool IsPathClear(						// Returns true, if the entire path is clear.
													// Returns false, if only a portion of the path is clear.
													// (see *psX, *psY, *psZ).
			short sX,							// In:  Starting X.
			short	sY,							// In:  Starting Y.
			short sZ,							// In:  Starting Z.
			double dCrawlRate,				// In:  Rate at which to scan ('crawl') path in pixels per
													// iteration.
													// NOTE: Values less than 1.0 are inefficient.
													// NOTE: We scan terrain using GetHeight()
													// at only one pixel.
													// NOTE: We could change this to a speed in pixels per second
													// where we'd assume a certain frame rate.
			short	sDstX,						// In:  Destination X.
			short	sDstZ,						// In:  Destination Z.
			short sVerticalTolerance = 0,	// In:  Max traverser can step up.
			short* psX = NULL,				// Out: If not NULL, last clear point on path.
			short* psY = NULL,				// Out: If not NULL, last clear point on path.
			short* psZ = NULL,				// Out: If not NULL, last clear point on path.
			bool bCheckExtents = true);	// In:  If true, will consider the edge of the realm a path
													// inhibitor.  If false, reaching the edge of the realm
													// indicates a clear path.

		// Gives this realm an opportunity and drawing surface to display its 
		// current status.
		void DrawStatus(	// Returns nothing.
			RImage*	pim,	// In:  Image in which to draw status.
			RRect*	prc);	// In:  Rectangle in which to draw status.  Clips to.

		// Maps a 3D coordinate onto the viewing plane.
		void Map3Dto2D(	// Returns nothing.
			short sX,		// In.
			short	sY,		// In.
			short	sZ,		// In.
			short* psX,		// Out.
			short* psY);	// Out.

		// Maps a 3D coordinate onto the viewing plane.
		void Map3Dto2D(		// Returns nothing.
			double	dX,		// In.
			double	dY,		// In.
			double	dZ,		// In.
			double* pdX,		// Out.
			double* pdY);		// Out.

		// Scales a Z coordinate onto the viewing plane using the 
		// view angle (~angle of projection).
		void MapZ3DtoY2D(		// Returns nothing.
			double	dZIn,		// In.
			double*	pdYOut);	// Out.

		// Scales a Z coordinate onto the viewing plane using the 
		// view angle (~angle of projection).
		void MapZ3DtoY2D(		// Returns nothing.
			short		sZIn,		// In.
			short*	psYOut);	// Out.

		// Scales a Y coordinate from the viewing plane using the 
		// view angle (~angle of projection).
		void MapY2DtoZ3D(		// Returns nothing.
			double	dYIn,		// In.
			double*	pdZOut);	// Out.

		// Scales a Y coordinate from the viewing plane using the 
		// view angle (~angle of projection).
		void MapY2DtoZ3D(		// Returns nothing.
			short		sYIn,		// In.
			short*	psZOut);	// Out.

		// Scales a Y coordinate onto the viewing plane using the 
		// view angle (~angle of projection).
		void MapY3DtoY2D(		// Returns nothing.
			double	dYIn,		// In.
			double*	pdYOut);	// Out.

		// Scales a Y coordinate onto the viewing plane using the 
		// view angle (~angle of projection).
		void MapY3DtoY2D(		// Returns nothing.
			short		sYIn,		// In.
			short*	psYOut);	// Out.

		// Scales a Y coordinate from the viewing plane using the 
		// view angle (~angle of projection).
		void MapY2DtoY3D(		// Returns nothing.
			double	dYIn,		// In.
			double*	pdYOut);	// Out.

		// Scales a Y coordinate from the viewing plane using the 
		// view angle (~angle of projection).
		void MapY2DtoY3D(		// Returns nothing.
			short		sYIn,		// In.
			short*	psYOut);	// Out.

		// If enabled, scales the specified height based on the view angle.
		void MapAttribHeight(	// Returns nothing.
			short		sHIn,			// In.
			short*	psHOut);		// Out.

		// Get dimension of realm on X/Z plane.
		short GetRealmWidth(void)	// Returns width of realm's X/Z plane.
			{
			short	sRealmW;
			MapY2DtoZ3D(m_phood->GetWidth(), &sRealmW);
			return sRealmW;
			}

		// Get dimension of realm on X/Z plane.
		short GetRealmHeight(void)	// Returns height of realm's X/Z plane.
			{
			short	sRealmH;
			MapY2DtoZ3D(m_phood->GetHeight(), &sRealmH);
			return sRealmH;
			}

		// Makes a 2D path based on the current hood setting for 'Use top-view 2Ds'.
		// Note that this function returns to you a ptr to its one and only static
		// string of length RSP_MAX_PATH.  Do not write to this string and do not
		// store this string.  It is best to just use this call to pass a string to
		// a function that will just use it right away (i.e., will not store it or
		// modify it).
		const char* Make2dResPath(		// Returns a ptr to an internal static buffer
												// containing the passed string, pszResName,
												// preceded by the appropriate directory based
												// on the current hood settings.
			const char* pszResName);	// In:  Resource name to prepend path to.

	//---------------------------------------------------------------------------
	// Protected (Internal) functions
	//---------------------------------------------------------------------------
	protected:

		// This will set all values that are to be set on construction and during
		// a Clear().  This is called by CRealm() and Clear().  This gives us one 
		// spot to implement these, rather than having to do it twice.
		void Init(void);		// Returns nothing.  Cannot fail.

	//---------------------------------------------------------------------------
	// Public static functions
	//---------------------------------------------------------------------------
	public:

		static short GetLayerViaAttrib(	// Returns the sprite layer indicated by
													// the specified attribute.             
			U16 u16Attrib)
			{
			return ms_asAttribToLayer[u16Attrib & REALM_ATTR_LAYER_MASK];
			}

		// Creates the layer map, if it has not already been done.
		// Now that the layer map needs to be 32K of uncompressable data, we create it
		// at run time.
		static void CreateLayerMap(void);

	//---------------------------------------------------------------------------
	// Protected static functions
	//---------------------------------------------------------------------------
	protected:

	};


#endif // REALM_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
