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
// realm.cpp
// Project: Postal
//
// This module impliments the CRealm class.
//
// History:
//		12/31/96 MJR	Started.
//
//		01/20/97 MJR	Changed Update loop to save a temporary iterator so
//							objects that delete themselves during the update
//							loop will still have a valid iterator to get the next
//							item in the Things list.
//
//		01/23/97	JMI	Added instantiation of static member ms_asAttribToLayer[].
//
//		01/27/97	JMI	Now 4 bits are used from the attribute to determine the 
//							proper layer for dudes.
//
//		01/28/97	JMI	Added ms_apszLayerNames[] to access the names of the 
//							layers.
//
//		01/29/97	JMI	Reordered ms_apszLayerNames to match the new order in the
//							CRealm::Layers enum (fixed to the way Steve is doing it
//							(the opaque layers are now before alpha equivalents)).
//
//		02/03/97	JMI	The Load(char*) will now Open a SAK with the same base name
//							as the *.rlm file, if one exists.  If it does not exist,
//							m_resmgr's BasePath is set to the No SAK Dir.  Once the
//							load is complete, no more new resources are to be requested
//							from m_resmgr.  To enfore this, m_resmgr's base path is
//							set to a condescending message.  If you are not careful, it
//							may taunt you a second time-a!
//
//		02/04/97 BRH	Temporarily (or permanently) took out timeout check in
//							Startup because it caused problems debugging startup 
//							code.  
//
//		02/04/97 BRH	Added ms_pCurrentNavNet to the realm, moving it from
//							its previous position in gameedit.
//
//		02/10/97	JMI	Now all loops iterate to the next iterator in the STL list
//							before dooing iterative processing just in case the current
//							iterator is invalidated during processing.
//
//		02/13/97	JMI	Added hood pointer to CRealm.
//
//		02/17/97	JMI	Added set lists to CRealm.  Now there is a list for each
//							set of Things.  The sets are enumerated in CThing.  Now
//							new'ed in CRealm() and delete'ed in ~CRealm().
//
//		02/18/97	JMI	Changed uses of CThing::ThingSet to CThing::Things.
//
//		02/19/97	JMI	Got rid of stuff regarding old collision sets and added
//							new CSmashitorium usage.
//
//		02/23/97	JMI	Added m_iNext and m_bUpdating so RemoveThing() and 
//							AddThing() can make the necessary adjustments to the next
//							iterator processed in the Update() loop.
//
//		02/23/97 MJR	Added call to class-based Preload() in CRealm::Load().
//
//		02/24/97	JMI	Added same protection for Render() that we implemented for
//							Update() on 02/23/97.
//
//		02/25/97	JMI	The way I had done the parens for *m_iNext++ for Render()
//							and Update() was scaring me so I separated it out more.
//
//		03/13/97	JMI	Load() now passes the file version number to the CThing
//							Load()'s.
//
//		03/13/97	JMI	Now, instead of the file's version number having to
//							exactly match FileVersion, it must just be less than or
//							equal.  If a file's version is greater than the current
//							version known by the code, the CThing that caused the
//							version number to increase will have more, less, or 
//							different data to load that it cannot possibly know about.
//
//		03/25/97	JMI	Load() no longer opens a SAK with the same title as the
//							.rlm file (This is now opened by the CHood).
//
//		04/09/97 BRH	Added RMultiGrid for the multi layer attribute maps, but
//							haven't taken out the RAttributeMap yet so that the
//							game will continue to work until we completely switch over.
//
//		04/16/97 BRH	Added Jon's template class CListNode head and tail nodes
//							to CRealm to replace the STL containers that provided the
//							linked lists of CThings.  Once the new methods are proven
//							to work, we will get rid of the m_everthing and m_apthings
//							arrays of STL lists.
//
//		04/17/97 BRH	Took timeout check out of Shutdown and Startup (it was
//							already commented out of Startup).  The 1 second timeout
//							when calling shutdown caused large levels like parade
//							to abort before saving everything.
//
//		04/18/97	JMI	Now Suspend() suspends the game time and Resume() resumes
//							it.  This should alleviate the need for the CThing derived
//							objects to do their own time compensation.
//
//		04/21/97	JMI	Added m_sNumSuspends and IsSuspended().
//
//		05/04/97 BRH	Removed STL references since the CListNode lists seem
//							to be working properly.
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
//		05/17/97	JMI	In EditUpdate() no argument list was supplied in the line:
//							pCur->m_powner->EditUpdate;
//							Added a set of parens.
//
//		05/26/97	JMI	Added DrawStatus() function.
//
//		05/27/97	JMI	Changed format of DrawStatus() output string.
//							
//		05/29/97	JMI	Changed occurences of m_pHeightMap to m_pTerrainMap.
//							Changed occurences of m_pAttrMap to m_pLayerMap.
//							Also, removed occurences of m_pAttribMap.
//							Also, added CreateLayerMap() that creates the attribute
//							to layer map now that it is so huge.
//
//		06/04/97 BRH	Turned off drawing lines in IsPathClear function.
//
//		06/09/97	JMI	Changed wording of realm status.
//							Also, Suspend() and Resume() pause and resume the sound.
//
//		06/10/97	JMI	Now Resume() does not let you 'over'-resume.
//
//		06/17/97 MJR	Moved some vars that were CPylon statics into the realm
//							so they could be instantiated on a realm-by-realm basis.
//
//		06/20/97 JRD	Replaced code needed to manage allocation and deallocation
//							for the new CSmashatorium
//
//		06/26/97	JMI	Moved Map2DTo3D from reality.h to here.
//							When converting to 2D, Y is now scaled based on the view
//							angle.  This was not applied to Z.  It could be a bug, but
//							it just looked better this way.  I suspect it is a 
//							bug/feature of the way most of the code was written
//							(probably a product of being more aware of the noticable 
//							difference Y coord has when mapped to 2D when comparing 45
//							to 90 degree type levels while originally designing/coding
//							CThings, the editor, and CScene).
//
//		06/28/97	JMI	Moved attribute access functions from realm.h to realm.cpp
//							while we're getting all the conversion from 3D to the X/Z
//							plane stuff right.  Compiling the entire project for any
//							tweak just doesn't sound very fun.  Hopefully, though,
//							there'll won't be many tweaks.
//							Also, added ScaleZ() functions to scale just Z (useful
//							for attribute map access).
//							Changed references to GetWorldRotX() to GetRealmRotX().
//
//		06/29/97	JMI	Added version of ScaleZ() that takes shorts.
//							Changed both versions of ScaleZ() to MapZ3DtoY2D()
//							and added two versions of MapY2DtoZ3D().
//
//		06/30/97	JMI	Now uses CRealm's new GetRealmWidth() and *Height()
//							for dimensions of realm's X/Z plane.
//
//		06/30/97	JMI	Added bCheckExtents parm to IsPathClear().  See proto for
//							details.
//
//		07/01/97	JMI	Added MapY2DtoY3D() and MapY3DtoY2D().
//							Also, GetHeight() now scales the height into realm 
//							coordinates.
//
//		07/01/97	JMI	IsPathClear() had 'step-up' logic (TM) for land based
//							things (like doofuses) that did not work for hovering
//							things (like missiles).  Fixed.
//
//		07/01/97	JMI	Changed the file version for the Hood so it can load and
//							save whether to use the attribute map heights with or
//							without scaling based on the view angle.
//							Also, added function to scale heights that checks the
//							hood value.
//
//		07/07/97 BRH	Added EditModify function to process the realm properties
//							dialog box where you can select the scoring mode and
//							play mode for the realm.
//
//		07/08/97 BRH	Added loading and saving of properties as of version 27.
//
//		07/09/97	JMI	Added function to get the full path for a 2D resource
//							based on the current hood setting for 'Use top-view 2Ds'.
//
//		07/09/97	JMI	Moved m_s2dResPathIndex from CHood to CRealm b/c of order
//							issues when loading.
//
//		07/10/97 BRH	Added cases to IsEndOfLevelGoalMet for the different 
//							scoring modes.
//
//		07/11/97	JMI	Minor change in IsEndOfLevelGoalMet() to make it so, if
//							there are zero births, the goal is considered met.  A
//							special case for example levels that have no enemies.
//
//		07/11/97 BRH	Added time calculations here for expiration date.  Checked
//							in again to update source safe with correct date.
//
//		07/12/97	JMI	Added m_bMultiplayer, which signifies whether this is
//							a multi or single player game.
//							Also, added Init().  See proto for details.
//							Moved things that were being initialized both in CRealm()
//							and in Clear() to Init() which is called by these two
//							functions.
//							Also, moved the initialization of the population statistics
//							into Init() even though they were only being done on a 
//							Clear().  This might be bad but it did not seem like it 
//							could hurt.
//
//		07/12/97 BRH	Made minor change to the EditModify dialog so that the
//							seconds always appears as two digits.
//
//		07/14/97	JMI	Moved initialization of Pylon stuff into Init() and also
//							no m_ucNextPylonId is 1 instead of 0.
//
//		07/14/97 BRH	Fixed a bug that caused the score timer to always count up.
//							Fixed problems with the goal stopping conditions and added
//							m_sFlagbaseCaptured to keep track of flags that were 
//							successfully returned to their base.
//
//		07/15/97 BRH	Added the end of level key flag as a parameter to 
//							the end of level goal check.
//
//		07/16/97 BRH	Fixed a problem with standard scoring mode on levels
//							that didn't have any enemies, they would end right
//							away using the new method of checking the end of
//							the level.
//
//		07/17/97 BRH	Added the time adjustment to the mac version.
//
//		07/27/97 BRH	Added m_lScoreInitialTime and set it to the same
//							initial value as m_lScoreDisplayTimer.  This was required
//							in order to calculate the time elapsed for the high
//							scores.
//
//		07/30/97 BRH	Added a string to hold the path of the realm which will
//							be used by the high score function to identify the
//							current realm file.
//
//		08/05/97	JMI	Now pauses only active sounds so that new sounds can 
//							continue to play while the realm is suspended.
//
//		08/05/97	JMI	Added CRealm::Flags struct and an instance in CRealm, 
//							m_flags.
//
//		08/08/97	JMI	Now displays a useful message about the thing that failed
//							to load should one do so.
//
//		08/09/97	JMI	Added progress callback and components.  There is now a
//							callback member that is called (when non-zero) that passes
//							the number of items processed so far and the total number of
//							items to process.  The return value of the callback dictates
//							whether to proceed with the operation.  Although, this is 
//							meant to be generic so we can use it for any type of 
//							process, it is currently only used by Load() and Save().
//
//		08/11/97 BRH	Changed both Capture the flag goals to flagbases captured
//							rather than flags captured.  
//
//		08/19/97 BRH	Fixed end of level goal for the checkpoint scoring so
//							if a specific number of flags is not set in the realm
//							file, it will just continue until the time runs out.
//
//		08/20/97	JMI	Made ms_apsz2dResPaths[] a static class member (was just
//							defined at realm.cpp file scope) and added enum macro for
//							number of elements.
//
//		08/28/97 BRH	Changed level goals for challenge scoring modes to use
//							the population numbers rather than just the hostile numbers
//							so that the victims count also.
//
//		08/30/97	JMI	IsEndOfLevelGoalMet() now allows a player to go on in
//							Timed and Checkpoint, if they hit the 'next level' key.
//
//		08/30/97	JMI	Now initializes hostile deaths and population deaths in
//							Startup().
//
//		09/02/97	JMI	Now resets all population statistics in Startup().
//
//		09/04/97 BRH	Realm::Load now attempts several paths.  It first tries
//							the path passed in for the case where the user
//							specified a full path using the open dialog.  Then it
//							tries the HD path and then the CD path if those fail.
//							This would allow us to send updated realm files
//							that could be loaded instead of the ones on the CD 
//							just by putting them in the mirror path on the HD.
//
//		09/07/97 BRH	Changed end of level goal to never end the MPFrag if
//							the kills goal is set to zero.  That will mean no 
//							frag limit.
//
//		09/09/97	JMI	Now checks number of flags against the actual number of
//							flags when using CheckPoint.
//
//		09/11/97 MJR	Added DoesFileExist(), which uses the same logic as Load()
//							to determine if a realm file exists.  Also added Open()
//							as a common function for DoesFileExist() and Load() to use.
//
//		09/12/97	JMI	Now, if ENABLE_PLAY_SPECIFIC_REALMS_ONLY is defined, we
//							try to load the .RLM out of memory using 
//							GetMemFileResource().
//
//		09/12/97 MJR	Now Open() will detect an empty filename as an error,
//							which avoids the ASSERT() that the Microsoft Runtime
//							library does when you pass open() and empty string.
//
//		11/21/97	JMI	Added bCoopMode flag indicating whether we're in cooperative
//							or deathmatch mode when in multiplayer.
//
////////////////////////////////////////////////////////////////////////////////
#define REALM_CPP

#include "RSPiX.h"
#include "realm.h"
#include "game.h"
#include "reality.h"
#include "score.h"
#include <time.h>
#include "MemFileFest.h"

//#define RSP_PROFILE_ON


#include "ORANGE/Debug/profile.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

// Sets the specified value into the data pointed, if the ptr is not NULL.
#define SET(ptr, val)	( (ptr != NULL) ? *ptr = val : val)

// Time, in ms, between status updates.
#define STATUS_UPDATE_INTERVAL	1000

#define STATUS_PRINT_X					0
#define STATUS_PRINT_Y					0

#define STATUS_FONT_SIZE				19
#define STATUS_FONT_FORE_INDEX		2
#define STATUS_FONT_BACK_INDEX		0
#define STATUS_FONT_SHADOW_INDEX		0

// Determines the number of elements in the passed array at compile time.
#define NUM_ELEMENTS(a)		(sizeof(a) / sizeof(a[0]) )

#define MAX_SMASH_DIAMETER				20

#define REALM_DIALOG_FILE				"res/editor/realm.gui"

#define TIMER_MIN_EDIT_ID				201
#define TIMER_SEC_EDIT_ID				202
#define KILLS_NUM_EDIT_ID				203
#define KILLS_PCT_EDIT_ID				204
#define FLAGS_NUM_EDIT_ID				205
#define SCORE_MODE_LB_ID				99
#define SCORE_MODE_LIST_BASE			100



////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// File counter
short CRealm::ms_sFileCount;

// Maps the layer portion of an attribute to the appropriate
// layer.
// Now that this table is 32K, we generate it table at run time to avoid adding
// an extra 32K of uncompressable space to the exe.
short CRealm::ms_asAttribToLayer[CRealm::LayerAttribMask + 1];

// Names of layers.  Use Layer enum values to index.
char* CRealm::ms_apszLayerNames[TotalLayers]	=
	{
	"Background",

 	"Sprite1",
	"Opaque1",

	"Sprite2",
	"Alpha1",

	"Sprite3",
	"Opaque2",

	"Sprite4",
	"Alpha2",

	"Sprite5",
	"Opaque3",

	"Sprite6",
	"Alpha3",

	"Sprite7",
	"Opaque4",

	"Sprite8",
	"Alpha4",

	"Sprite9",
	"Opaque5",

	"Sprite10",
	"Alpha5",

	"Sprite11",
	"Opaque6",

	"Sprite12",
	"Alpha6",

	"Sprite13",
	"Opaque7",

	"Sprite14",
	"Alpha7",

	"Sprite15",
	"Opaque8",

	"Sprite16",
	};

// These are the various 2d paths that we currently support.  Eventually, if
// there's more than two, this can be presented in listbox form (instead of
// checkbox form).
char*	CRealm::ms_apsz2dResPaths[Num2dPaths]	=
	{
	"2d/Top/",
	"2d/Side/",
	"2d/SideBright/",
	};

// Used for CRealm oriented drawing tasks.
static RPrint	ms_print;


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Non-member functions.
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Maps a 3D coordinate onto the viewing plane provided the view angle
// (~angle of projection).
///////////////////////////////////////////////////////////////////////////////
template <class TIn, class TOut>
void Map3Dto2D(	// Returns nothing.
	TIn tX,				// In.
	TIn tY,				// In.
	TIn tZ,				// In.
	TOut* ptX,			// Out.
	TOut* ptY,			// Out.
	short	sViewAngle)	// In:  View angle in degrees.
	{
	*ptX	= tX;
	*ptY	= SINQ[sViewAngle] * tZ - COSQ[sViewAngle] * tY;
	}

///////////////////////////////////////////////////////////////////////////////
// Scales a Z coordinate onto the viewing plane provided the 
// view angle (~angle of projection).
///////////////////////////////////////////////////////////////////////////////
template <class TIn, class TOut>
void MapZ3DtoY2D(		// Returns nothing.
	TIn	tZIn,			// In.
	TOut* ptYOut,		// Out.
	short	sViewAngle)	// In:  View angle in degrees.
	{
	ASSERT(sViewAngle >= 0 && sViewAngle < 360);

	*ptYOut	= SINQ[sViewAngle] * tZIn;
	}

///////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate from the viewing plane provided the 
// view angle (~angle of projection).
///////////////////////////////////////////////////////////////////////////////
template <class TIn, class TOut>
void MapY2DtoZ3D(		// Returns nothing.
	TIn	tYIn,			// In.
	TOut* ptZOut,		// Out.
	short	sViewAngle)	// In:  View angle in degrees.
	{
	ASSERT(sViewAngle >= 0 && sViewAngle < 360);

	REAL	rSin	= SINQ[sViewAngle];
	if (rSin != 0.0)
		{
		*ptZOut	= tYIn / rSin;
		}
	else
		{
		*ptZOut	= 0;
		}
	}

///////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate onto the viewing plane provided the 
// view angle (~angle of projection).
///////////////////////////////////////////////////////////////////////////////
template <class TIn, class TOut>
void MapY3DtoY2D(		// Returns nothing.
	TIn	tYIn,			// In.
	TOut* ptYOut,		// Out.
	short	sViewAngle)	// In:  View angle in degrees.
	{
	ASSERT(sViewAngle >= 0 && sViewAngle < 360);

	*ptYOut	= COSQ[sViewAngle] * tYIn;
	}

///////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate from the viewing plane provided the 
// view angle (~angle of projection).
///////////////////////////////////////////////////////////////////////////////
template <class TIn, class TOut>
void MapY2DtoY3D(		// Returns nothing.
	TIn	tYIn,			// In.
	TOut* ptYOut,		// Out.
	short	sViewAngle)	// In:  View angle in degrees.
	{
	ASSERT(sViewAngle >= 0 && sViewAngle < 360);

	REAL	rCos	= COSQ[sViewAngle];
	if (rCos != 0.0)
		{
		*ptYOut	= tYIn / rCos;
		}
	else
		{
		*ptYOut	= 0;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Default (and only) constructor
////////////////////////////////////////////////////////////////////////////////
CRealm::CRealm()
	{
	time_t lTime;
	time(&lTime);
#ifndef WIN32
	// Mac version time adjusment back to UTC time.
	lTime -= ((365 * 70UL) + 17) * 24 * 60 * 60; // time_fudge 1900->1970
#endif
	g_lRegValue = lTime - g_lRegTime;
	g_lExpValue = g_lExpTime - lTime; 

	CreateLayerMap();
	
	// Setup render object (it's constructor was automatically called)
	m_scene.SetLayers(TotalLayers);

	// Set attribute map to a safe (but invalid) value
	m_pTerrainMap = 0;
	m_pLayerMap = 0;
	m_pTriggerMap = 0;
	m_pTriggerMapHolder = 0;

	// Set Hood ptr to a safe (but invalid) value.
	m_phood			= NULL;

/*
	// Create a container of things for each element in the array
	short	s;
	for (s = 0; s < CThing::TotalIDs; s++)
		m_apthings[s] = new CThing::Things;
*/

	// Initialize current Navigation Net pointer
	m_pCurrentNavNet = NULL;

	// Not currently updating.
	m_bUpdating		= false;

	// Initialize dummy nodes for linked lists of CThings
	m_everythingHead.m_pnNext = &m_everythingTail;
	m_everythingHead.m_pnPrev = NULL;
	m_everythingHead.m_powner = NULL;
	m_everythingTail.m_pnNext = NULL;
	m_everythingTail.m_pnPrev = &m_everythingHead;
	m_everythingTail.m_powner = NULL;

	short i;
	for (i = 0; i < CThing::TotalIDs; i++)
		{
		m_aclassHeads[i].m_pnNext = &(m_aclassTails[i]);
		m_aclassHeads[i].m_pnPrev = NULL;
		m_aclassHeads[i].m_powner = NULL;
		m_aclassTails[i].m_pnNext = NULL;
		m_aclassTails[i].m_pnPrev = &(m_aclassHeads[i]);
		m_aclassTails[i].m_powner = NULL;
		m_asClassNumThings[i] = 0;
		}

	m_sNumThings = 0;

	m_sNumSuspends	= 0;

	// Setup print.
	ms_print.SetFont(STATUS_FONT_SIZE, &g_fontBig);
	ms_print.SetColor(
		STATUS_FONT_FORE_INDEX, 
		STATUS_FONT_BACK_INDEX, 
		STATUS_FONT_SHADOW_INDEX);

	// Initialize flags to defaults for safety.
	// This might be a bad idea if we want to guarantee they get set in which
	// case we should maybe set them to absurd values.
	m_flags.bMultiplayer	= false;
	m_flags.bCoopMode		= false;
	m_flags.bEditing		= false;
	m_flags.bEditPlay		= false;
	m_flags.sDifficulty	= 5;

	m_fnProgress			= NULL;

	m_bPressedEndLevelKey = false;

	// Initialize.
	Init();
	}


////////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////////
CRealm::~CRealm()
	{
	// Clear the realm (in case this hasn't been done yet)
	Clear();

	// Double-check to be sure there's nothing left
	if (m_everythingHead.m_pnNext != &m_everythingTail)
		TRACE("CRealm::~CRealm(): There are still %d CThing's in this realm!\n", m_sNumThings);
	}


////////////////////////////////////////////////////////////////////////////////
// This will set all values that are to be set on construction and during
// a Clear().  This is called by CRealm() and Clear().  This gives us one 
// spot to implement these, rather than having to do it twice.
////////////////////////////////////////////////////////////////////////////////
void CRealm::Init(void)		// Returns nothing.  Cannot fail.
	{
	m_dKillsPercentGoal = 80.0;
	m_sKillsGoal = 0;
	m_sFlagsGoal = 0;
	m_lScoreTimeDisplay = 0;
	m_lScoreInitialTime = 0;
	m_ScoringMode = Standard;
	m_sFlagsCaptured = 0;
	m_sFlagbaseCaptured = 0;

	// Reset the Population statistics
	m_sPopulationBirths = 0;
	m_sPopulation = 0;
	m_sPopulationDeaths = 0;
	m_sHostiles = 0;
	m_sHostileBirths = 0;
	m_sHostileKills = 0;

	// Initial index for 2D resoruce paths array.
	m_s2dResPathIndex = 1;

	// Reset timer.
	m_lLastStatusDrawTime	= -STATUS_UPDATE_INTERVAL;

	// Pylon stuff
	short i;
	for (i=0;i < 256;i++)
		m_asPylonUIDs[i] = 0; // clear the Pylon UIDs!
	m_sNumPylons = 0;
	m_ucNextPylonID = 1;
	}

////////////////////////////////////////////////////////////////////////////////
// Clear the realm
////////////////////////////////////////////////////////////////////////////////
void CRealm::Clear()
	{
	// Shutdown the realm (in case this hasn't been done yet)
	Shutdown();

	// Destroy all the objects.  We use a copy of the iterator to avoid being
	// stuck with an invalid iterator once the object is gone.
	CListNode<CThing>* pCur;
	CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
	while (pNext->m_powner != NULL)
	{
		pCur = pNext;
		pNext = pNext->m_pnNext;
		delete (pCur->m_powner);
	}

	// Clear out any sprites that didn't already remove themselves
	m_scene.RemoveAllSprites();

	// Clear out any residue IDs.  Shouldn't need to, but . . .
	m_idbank.Reset();

	// Reset smashatorium.

#ifdef NEW_SMASH // need to become final at some point...
	m_smashatorium.Destroy();
#else
	m_smashatorium.Reset();
#endif

	// Re-Initialize.
	Init();
	}


////////////////////////////////////////////////////////////////////////////////
// Determine if specified file exists according to same rules used by Load()
////////////////////////////////////////////////////////////////////////////////
// static
bool CRealm::DoesFileExist(							// Returns true if file exists, false otherwise
	const char* pszFileName)							// In:  Name of file
	{
	bool bResult = false;
	RFile file;
	if (Open(pszFileName, &file) == 0)
		{
		file.Close();
		bResult = true;
		}
	return bResult;
	}



////////////////////////////////////////////////////////////////////////////////
// Open the specified realm file
////////////////////////////////////////////////////////////////////////////////
// static
short CRealm::Open(										// Returns 0 if successfull, non-zero otherwise
	const char* pszFileName,							// In:  Name of file to load from
	RFile* pfile)											// I/O: RFile to be used
	{
	short sResult = 0;
	
	if (strlen(pszFileName) > 0)
		{

		#if !defined(ENABLE_PLAY_SPECIFIC_REALMS_ONLY)
			// Try the given path first since it may already have a full path in the
			// case of loading a level, then try the path with the HD path prepended, 
			// then try the CD path.
			sResult = pfile->Open(rspPathToSystem((char*)pszFileName), "rb", RFile::LittleEndian);
			if (sResult != 0)
				{
				char pszFullPath[RSP_MAX_PATH];
				strcpy(pszFullPath, FullPathHD((char*) pszFileName));
				sResult = pfile->Open((char*)pszFullPath, "rb", RFile::LittleEndian);
				if (sResult != 0)
					{
					strcpy(pszFullPath, FullPathCD((char*) pszFileName));
					sResult = pfile->Open((char*)pszFullPath, "rb", RFile::LittleEndian);
					}
				}
		#else
			// There's only one place it can possibly be and, if it's not there,
			// no realm for you!
			sResult	= GetMemFileResource(pszFileName, RFile::LittleEndian, pfile);
		#endif	// ENABLE_PLAY_SPECIFIC_REALMS_ONLY
		}
	else
		{
		sResult = -1;
		TRACE("CRealm::Open(): Empty file name!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Load the realm
////////////////////////////////////////////////////////////////////////////////
short CRealm::Load(										// Returns 0 if successfull, non-zero otherwise
	const char* pszFileName,							// In:  Name of file to load from
	bool bEditMode)										// In:  Use true for edit mode, false otherwise
	{
	short sResult = 0;

	// Copy the name to use later for high score purposes
	m_rsRealmString = pszFileName;

	// Open file
	RFile file;
	sResult = Open(pszFileName, &file);
	if (sResult == 0)
		{
		// Use alternate load to do most of the work
		sResult = Load(&file, bEditMode);

		file.Close();
		}
	else
		{
		sResult = -1;
		TRACE("CRealm::Load(): Couldn't open file: %s !\n", pszFileName);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Load realm
////////////////////////////////////////////////////////////////////////////////
short CRealm::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode)										// In:  Use true for edit mode, false otherwise
	{
	short sResult = 0;
	
	// Clear the realm before loading this new stuff
	Clear();

	// Increment file count
	ms_sFileCount++;

	// Read & validate file ID
	ULONG ulFileID;
	if (pFile->Read(&ulFileID) == 1)
		{
		if (ulFileID == CRealm::FileID)
			{

			// Read & validate file version
			ULONG ulFileVersion;
			if (pFile->Read(&ulFileVersion) == 1)
				{
				// If a known version . . .
				if (ulFileVersion <= CRealm::FileVersion)
					{
					// Read properties for the realm
					switch (ulFileVersion)
					{
						default:
						case 30:
							pFile->Read(&m_s2dResPathIndex);

						case 29:
							pFile->Read(&m_ScoringMode);
						case 28:
						case 27:
						{
							short sUp;
							pFile->Read(&m_lScoreTimeDisplay);
							m_lScoreInitialTime = m_lScoreTimeDisplay;
							pFile->Read(&sUp);
							if (sUp == 1)
								m_bScoreTimerCountsUp = true;
							else
								m_bScoreTimerCountsUp = false;
							pFile->Read(&m_sKillsGoal);
							pFile->Read(&m_sFlagsGoal);
							pFile->Read(&m_dKillsPercentGoal);
							break;
						}
						case 26:
						case 25:
						case 24:
						case 23:
						case 22:
						case 21:
						case 20:
						case 19:
						case 18:
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
						case 0:
							break;
					}


					// Scan through class info structs and for each non-0 preload func,
					// call it to give that class a chance to preload stuff.  The intention
					// is to give classes whose objects don't exist at the start of a level
					// a chance to preload resources now rather than during gameplay.
					for (short sPre = 0; sPre < CThing::TotalIDs; sPre++)
						{
						CThing::FuncPreload func = CThing::ms_aClassInfo[sPre].funcPreload;
						if (func != 0)
							{
							sResult = (*func)(this);
							if (sResult != 0)
								{
								TRACE("CRealm::Load(): Error reported by Preload() for CThing class ID = %hd\n", (short)sPre);
								break;
								}
							}
						}
					if (sResult == 0)
						{

						// Read number of things that were written to file (could be 0!)
						short sCount;
						if (pFile->Read(&sCount) == 1)
							{

							CThing::ClassIDType	idLastThingLoaded	= CThing::TotalIDs;

							// If there's a callback . . .
							if (m_fnProgress)
								{
								// Call it . . .
								if (m_fnProgress(0, sCount) == true)
									{
									// Callback is happy to continue.
									}
								else
									{
									// Callback has decided to end this operation.
									sResult	= 1;
									}
								}

							// Load each object that was written to the file (could be 0!)
							for (short s = 0; (s < sCount) && !sResult; s++)
								{

								// Read class ID of next object in file
								CThing::ClassIDType id;
								if (pFile->Read(&id) == 1)
									{

									// Create object based on class ID
									CThing* pThing;
									sResult = CThing::Construct(id, this, &pThing);
									if (!sResult)
										{

										// Load object assocated with this class ID
										sResult = pThing->Load(pFile, bEditMode, ms_sFileCount, ulFileVersion);

										// If successful . . .
										if (sResult == 0)
											{
											// Store last thing to successfully load.
											idLastThingLoaded	= id;
											// If there's a callback . . .
											if (m_fnProgress)
												{
												// Call it . . .
												if (m_fnProgress(s + 1, sCount) == true)
													{
													// Callback is happy to continue.
													}
												else
													{
													// Callback has decided to end this operation.
													sResult	= 1;
													}
												}
											}
										else
											{
											TRACE("CRealm::Load(): Load() failed for thing of type %s; ",
												CThing::ms_aClassInfo[id].pszClassName);
											if (idLastThingLoaded != CThing::TotalIDs)
												{
												STRACE("The last thing to successfully loaded was a %s.\n",
													CThing::ms_aClassInfo[idLastThingLoaded].pszClassName);
												}
											else
												{
												STRACE("This was the first thing to load.\n");
												}
											}
										}

									}
								else
									{
									sResult = -1;
									TRACE("CRealm::Load(): Error reading class ID!\n");
									}
								}

							// Check for I/O errors (only matters if no errors were reported so far)
							if (!sResult && pFile->Error())
								{
								sResult = -1;
								TRACE("CRealm::Load(): Error reading file!\n");
								}

							// If any errors occurred . . .
							if (sResult)
								{
								// Better clean up stuff that did load.
								Clear();
								}
							}
						else
							{
							sResult = -1;
							TRACE("CRealm::Load(): Error reading count of objects in file!\n");
							}
						}
					}
				else
					{
					sResult = -1;
					TRACE("CRealm::Load(): Incorrect file version (should be 0x%lx or less, was 0x%lx)!\n", CRealm::FileVersion, ulFileVersion);
					}
				}
			else
				{
				sResult = -1;
				TRACE("CRealm::Load(): Error reading file version!\n");
				}
			}
		else
			{
			sResult = -1;
			TRACE("CRealm::Load(): Incorrect file ID (should be 0x%lx, was 0x%lx)!\n", CRealm::FileID, ulFileID);
			}
		}
	else
		{
		sResult = -1;
		TRACE("CRealm::Load(): Error reading file ID!\n");
		}

#ifdef NEW_SMASH
	if (sResult == 0) // a success....
		{
		/* For now, let's see if this is necessary...

		// Allocate the Smashatorium:
		// Kill old...*
		short	sOldW = m_smashatorium.m_sWorldW;
		short	sOldH = m_smashatorium.m_sWorldH;
		short sOldTileW = m_smashatorium.m_sTileW;
		short sOldTileH = m_smashatorium.m_sTileH;


		if (m_smashatorium.m_pGrid) m_smashatorium.Destroy();

		if (m_smashatorium.Alloc(sOldW,sOldH,sOldTileW,sOldTileH) != SUCCESS)
			{
			TRACE("CRealm::Load(): Error reallocating the smashatorium!\n");
			sResult = -1;
			}
		*/
		}
#endif

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save the realm
////////////////////////////////////////////////////////////////////////////////
short CRealm::Save(										// Returns 0 if successfull, non-zero otherwise
	const char* pszFile)									// In:  Name of file to save to
	{
	short sResult = 0;

	// Open file
	RFile file;
	sResult = file.Open((char*)pszFile, "wb", RFile::LittleEndian);
	if (sResult == 0)
		{

		// Use alternate save to do most of the work
		sResult = Save(&file);

		file.Close();

		// Would this be an appropriate time to build the SAK file???
		}
	else
		{
		sResult = -1;
		TRACE("CRealm::Save(): Couldn't open file: %s !\n", pszFile);
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save the realm
////////////////////////////////////////////////////////////////////////////////
short CRealm::Save(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile)											// In:  File to save to
	{
	short sResult = 0;

	// Increment file count
	ms_sFileCount++;

	// Write out file ID and version
	pFile->Write((unsigned long)CRealm::FileID);
	pFile->Write((unsigned long)CRealm::FileVersion);

	// Save properties for the realm
	pFile->Write(m_s2dResPathIndex);
	pFile->Write(&m_ScoringMode);
	pFile->Write(&m_lScoreTimeDisplay);
	short sUp = 0;
	if (m_bScoreTimerCountsUp)
		sUp = 1;
	pFile->Write(&sUp);
	pFile->Write(&m_sKillsGoal);
	pFile->Write(&m_sFlagsGoal);
	pFile->Write(&m_dKillsPercentGoal);

	// Write out number of objects
	pFile->Write(m_sNumThings);

	// If there's a callback . . .
	if (m_fnProgress)
		{
		// Call it . . .
		if (m_fnProgress(0, m_sNumThings) == true)
			{
			// Callback is happy to continue.
			}
		else
			{
			// Callback has decided to end this operation.
			sResult	= 1;
			}
		}

	// Do this for all of the objects
	CListNode<CThing>* pCur;
	CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
	short	sCurItemNum	= 0;
	while (pNext->m_powner != NULL && !sResult)
		{
		pCur = pNext;
		pNext = pNext->m_pnNext;

		// Write out object's class ID (so we know waht kind it is when we load it)
		pFile->Write(pCur->m_powner->GetClassID());

		// Let object save itself
		sResult = pCur->m_powner->Save(pFile, ms_sFileCount);
		if (sResult)
			break;
		else
			{
			sCurItemNum++;

			// If there's a callback . . .
			if (m_fnProgress)
				{
				// Call it . . .
				if (m_fnProgress(sCurItemNum, m_sNumThings) == true)
					{
					// Callback is happy to continue.
					}
				else
					{
					// Callback has decided to end this operation.
					sResult	= 1;
					}
				}
			}
		}

	// Check for I/O errors (only matters if no errors were reported so far)
	if (!sResult && pFile->Error())
		{
		sResult = -1;
		TRACE("CRealm::Save(): Error writing file!\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Startup the realm
////////////////////////////////////////////////////////////////////////////////
short CRealm::Startup(void)							// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// Initialize Population statistics b/c anyone killed already was not done so 
	// by the player.
	m_sPopulationBirths = 0;
	m_sPopulation = 0;
	m_sPopulationDeaths = 0;
	m_sHostiles = 0;
	m_sHostileBirths = 0;
	m_sHostileKills = 0;


	// The idea is to only call Startup() for those objects that were Load()'ed,
	// and NOT for any other objects in the realm.  The m_sCallStartup flags are
	// set during the CRealm::Load() process to ensure that only those objects
	// are called here.  I'm no longer sure I like this idea, so perhaps we
	// should do what Shutdown() does, which is to call Startup() for every
	// object.  The original reasoning was that once the game gets going, any
	// newly created objects will NOT have their Startup() called, so Startup()
	// was seen as a special service for Load()'ed objects so they could interact
	// with other objects once all the objects are Load()'ed.  Actually, that
	// sounds pretty good!  I guess I'll keep it this way pending suggestions.

	// This loop is specifically designed so that it will not end until all of
	// the objects have been scanned in a single pass and none have their flags
	// set.  Keep in mind that since objects may be interacting with one another,
	// calling one object may result in indirectly changing another's flag!
	short sDone;
	long lPassNum = 0;
	do	{
		// Always assume this will be the last pass.  If it isn't, this flag will
		// be reset to 0, and we'll do the whole thing again.
		sDone = 1;

		// Do this for all the objects.  We use a copy of the iterator to avoid being
		// stuck with an invalid iterator once the object is gone.
		CListNode<CThing>* pCur;
		CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
		// Go through all objects, calling Startup() for those that have the flag set
		while (pNext->m_powner != NULL && !sResult)
		{
			pCur = pNext;
			pNext = pNext->m_pnNext;

			if (pCur->m_powner->m_sCallStartup)
			{
				sDone = 0;
				pCur->m_powner->m_sCallStartup = 0;
				sResult = pCur->m_powner->Startup();
			}
		}

		// Increment pass number for testing/debugging
		lPassNum++;

		// Setup the previous time for the start of the game.														
		m_lPrevTime = m_time.GetGameTime();

		} while (!sDone && !sResult); 


	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown the realm
////////////////////////////////////////////////////////////////////////////////
short CRealm::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	// This loop is specifically designed so that it will not end until all of
	// the objects have been scanned in a single pass and none have their flags
	// set.  Keep in mind that since objects may be interacting with one another,
	// calling one object may result in indirectly changing another's flag!
	short sDone;
	short sFirstTime = 1;
	long lPassNum = 0;
	do	{
		// Always assume this will be the last pass.  If it isn't, this flag will
		// be reset to 0, and we'll do the whole thing again.
		sDone = 1;

		// Do this for all the objects.  We use a copy of the iterator to avoid being
		// stuck with an invalid iterator once the object is gone.
		CListNode<CThing>* pCur;
		CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
		while (pNext->m_powner != NULL && !sResult)
		{
			pCur = pNext;
			pNext = pNext->m_pnNext;

			if (pCur->m_powner->m_sCallShutdown || sFirstTime)
			{
				sDone = 0;
				pCur->m_powner->m_sCallShutdown = 0;
				sResult = pCur->m_powner->Shutdown();
			}
		}

		// Clear first-time flag
		sFirstTime = 0;

		// Increment pass number for testing/debugging
		lPassNum++;

		} while (!sDone && !sResult);


	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend the realm
////////////////////////////////////////////////////////////////////////////////
void CRealm::Suspend(void)
	{
	m_sNumSuspends++;

	// Do this for all the objects.  We use a copy of the iterator to avoid being
	// stuck with an invalid iterator once the object is gone.
	CListNode<CThing>* pCur;
	CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
	while (pNext->m_powner != NULL)
	{
		pCur = pNext;
		pNext = pNext->m_pnNext;

		pCur->m_powner->Suspend();
	}

	// Suspend the game time.  I don't think it matters whether this is done
	// before or after the CThing->Suspend() calls since game time never actually
	// advances unless CTime->Update() is called (when not suspended, of course).
	m_time.Suspend();

	// Suspend active sounds.
	PauseAllSamples();
	}

////////////////////////////////////////////////////////////////////////////////
// Resume the realm
////////////////////////////////////////////////////////////////////////////////
void CRealm::Resume(void)
	{
	if (m_sNumSuspends > 0)
		{
		// Do this for all the objects.  We use a copy of the iterator to avoid being
		// stuck with an invalid iterator once the object is gone.
		CListNode<CThing>* pCur;
		CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
		while (pNext->m_powner != NULL)
		{
			pCur = pNext;
			pNext = pNext->m_pnNext;

			pCur->m_powner->Resume();
		}	

		// Resume the game time.  I don't think it matters whether this is done
		// before or after the CThing->Resume() calls since game time never actually
		// advances unless CTime->Update() is called (when not suspended, of course).
		m_time.Resume();

		m_sNumSuspends--;

		ASSERT(m_sNumSuspends >= 0);

		// Resume active sounds.
		ResumeAllSamples();
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Update the realm
////////////////////////////////////////////////////////////////////////////////
void CRealm::Update(void)
	{
	// We want to do this for every CThing in the realm.  We use iNext to get
	// the next iterator before calling the current item to avoid
	// a problem that could otherwise occur where iCur becomes invalidated during
	// the (*iCur)->Update() (like when a CThing deletes itself).
	// This causes an additional problem which is relatively easy to fix.  Since
	// we always insert at the end, if iCur is the last thing (meaning iNext is the
	// .end()), and that call adds a new thing at the end, that thing will never
	// get called b/c iNext already points to .end().  We effectively skip the 
	// newly added item.
	// To avoid this, we simply, at the end, check to make sure iNext-- produces
	// iCur.  If not, we process iNext--.
	// Further, if the last item added two things, we need to go back two.

	rspStartProfile("Realm Update");

	// Entering update loop.
	m_bUpdating	= true;

	// Do this for everything.
	CThing* pthing;
	m_pNext = m_everythingHead.m_pnNext;
	while (m_pNext->m_powner != NULL)
	{
		pthing = m_pNext->m_powner;
		m_pNext = m_pNext->m_pnNext;
		pthing->Update();
	}

	// Update the display timer
	m_lThisTime = m_time.GetGameTime();
	m_lElapsedTime = m_lThisTime - m_lPrevTime;
	if (m_bScoreTimerCountsUp)
		m_lScoreTimeDisplay += m_lElapsedTime;
	else
		m_lScoreTimeDisplay -= m_lElapsedTime;
	m_lPrevTime = m_lThisTime;
 
	// Leaving update loop.
	m_bUpdating	= false;

	rspEndProfile("Realm Update");
	}


////////////////////////////////////////////////////////////////////////////////
// Render the realm
////////////////////////////////////////////////////////////////////////////////
void CRealm::Render(void)
	{
	// Entering update loop.
	m_bUpdating	= true;

	// Do this for everything.
	CThing* pthing;
	m_pNext = m_everythingHead.m_pnNext;
	while (m_pNext->m_powner != NULL)
	{
		pthing = m_pNext->m_powner;
		m_pNext = m_pNext->m_pnNext;
		pthing->Render();
	}

	// Leaving update loop.
	m_bUpdating	= false;
	}

// This old way probably doesn't make sense any more since we're going to allow
// for multiple views of a realm.  I don't think we'd want to tell each object
// to render itself for each different view -- not unless the enter concept of
// what each item does to "render" itself changes.  Right now, objects merely
// update their representations in the scene, which really doesn't take too
// long, and needs to get done no matter what view we're talking about.
/*
void CRealm::Render(
	short sViewX,											// In:  X coord of view
	short sViewY,											// In:  Y coord of view
	short sViewW,											// In:  Width of view
	short sViewH,											// In:  Height of view
	RImage* pimDst,										// In:  Image to render to
	short sDstX,											// In:  X coord to draw to
	short sDstY)											// In:  Y coord to draw to
	{
	// It may turn out that some objects want to do their own clipping to see if
	// they need to add themselves to the scene.  I would guess this would be
	// the exception rather than the rule, so when (or if) such a requirement
	// comes up, we shouldn't pass the view info to each object, but instead
	// should create a function that objects can call to get the view info, the
	// idea being that this would be faster overall than passing all that data
	// to each object only to have it ignored most of the time.

	// Do this for everything
	for (CThing::Things::iterator i = m_everything.begin(); i != m_everything.end(); i++)
		(*i)->Render();

	// Render specified view to specified position in specified image
	m_scene.Render(
		sViewX,
		sViewY,
		sViewW,
		sViewH,
		pimDst,
		sDstX,
		sDstY);
	}
*/


////////////////////////////////////////////////////////////////////////////////
// Edit mode: Update the realm
////////////////////////////////////////////////////////////////////////////////
void CRealm::EditUpdate(void)
	{
	// Do this for all the objects.  We use a copy of the iterator to avoid being
	// stuck with an invalid iterator once the object is gone.
	CListNode<CThing>* pCur;
	CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
	while (pNext->m_powner != NULL)
		{
		pCur = pNext;
		pNext = pNext->m_pnNext;

		pCur->m_powner->EditUpdate();
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Edit mode: Render the realm
////////////////////////////////////////////////////////////////////////////////
void CRealm::EditRender(void)
	{
	// Do this for all the objects.  We use a copy of the iterator to avoid being
	// stuck with an invalid iterator once the object is gone.
	CListNode<CThing>* pCur;
	CListNode<CThing>* pNext = m_everythingHead.m_pnNext;
	while (pNext->m_powner != NULL)
		{
		pCur = pNext;
		pNext = pNext->m_pnNext;

		pCur->m_powner->EditRender();
		}
	}

// This old way probably doesn't make sense any more since we're going to allow
// for multiple views of a realm.  I don't think we'd want to tell each object
// to render itself for each different view -- not unless the enter concept of
// what each item does to "render" itself changes.  Right now, objects merely
// update their representations in the scene, which really doesn't take too
// long, and needs to get done no matter what view we're talking about.
/*
void CRealm::EditRender(
	short sViewX,											// In:  X coord of view
	short sViewY,											// In:  Y coord of view
	short sViewW,											// In:  Width of view
	short sViewH,											// In:  Height of view
	RImage* pimDst,										// In:  Image to render to
	short sDstX,											// In:  X coord to draw to
	short sDstY)											// In:  Y coord to draw to
	{
	// It may turn out that some objects want to do their own clipping to see if
	// they need to add themselves to the render.  I would guess this would be
	// the exception rather than the rule, so when (or if) such a requirement
	// comes up, we shouldn't pass the view info to each object, but instead
	// should create a function that objects can call to get the view info, the
	// idea being that this would be faster overall than passing all that data
	// to each object only to have it ignored most of the time.

	// Do this for everything
	for (CThing::Things::iterator i = m_everything.begin(); i != m_everything.end(); i++)
		(*i)->EditRender();

	// Render specified view to specified position in specified image
	m_scene.Render(
		sViewX,
		sViewY,
		sViewW,
		sViewH,
		pimDst,
		sDstX,
		sDstY);
	}
*/

////////////////////////////////////////////////////////////////////////////////
// EditModify - Run dialog for realm scoring and play options
////////////////////////////////////////////////////////////////////////////////
void CRealm::EditModify(void)
{
	RGuiItem*	pguiRoot	= RGuiItem::LoadInstantiate(FullPathVD(REALM_DIALOG_FILE));
	RProcessGui	guiDialog;

	if (pguiRoot != NULL)
	{
		RGuiItem*	pguiOk		= pguiRoot->GetItemFromId(1);
		RGuiItem*	pguiCancel	= pguiRoot->GetItemFromId(2);

		REdit* peditMinutes = (REdit*) pguiRoot->GetItemFromId(TIMER_MIN_EDIT_ID);
		REdit* peditSeconds = (REdit*) pguiRoot->GetItemFromId(TIMER_SEC_EDIT_ID);
		REdit* peditKillsNum = (REdit*) pguiRoot->GetItemFromId(KILLS_NUM_EDIT_ID);
		REdit* peditKillsPct = (REdit*) pguiRoot->GetItemFromId(KILLS_PCT_EDIT_ID);
		REdit* peditFlagsNum = (REdit*) pguiRoot->GetItemFromId(FLAGS_NUM_EDIT_ID);
		RListBox* plbScoreModes = (RListBox*) pguiRoot->GetItemFromId(SCORE_MODE_LB_ID);
		RGuiItem* pguiItem = NULL;
		long lMinutes;
		long lSeconds;

		if (peditMinutes != NULL && peditSeconds != NULL && peditKillsNum != NULL &&
		    peditKillsPct != NULL && peditFlagsNum != NULL && plbScoreModes != NULL)
		{
			ASSERT(peditMinutes->m_type == RGuiItem::Edit);
			ASSERT(peditSeconds->m_type == RGuiItem::Edit);
			ASSERT(peditKillsNum->m_type == RGuiItem::Edit);
			ASSERT(peditKillsPct->m_type == RGuiItem::Edit);
			ASSERT(peditFlagsNum->m_type == RGuiItem::Edit);
			ASSERT(plbScoreModes->m_type == RGuiItem::ListBox);

			lMinutes = m_lScoreTimeDisplay / 60000;
			lSeconds = (m_lScoreTimeDisplay / 1000) % 60;

			peditMinutes->SetText("%ld", lMinutes);
			peditSeconds->SetText("%2.2ld", lSeconds);
			peditKillsNum->SetText("%d", m_sKillsGoal);
			peditKillsPct->SetText("%3.1f", m_dKillsPercentGoal);
			peditFlagsNum->SetText("%d", m_sFlagsGoal);
			peditMinutes->Compose();
			peditSeconds->Compose();
			peditKillsNum->Compose();
			peditKillsPct->Compose();
			peditFlagsNum->Compose();
			
			pguiItem = plbScoreModes->GetItemFromId(SCORE_MODE_LIST_BASE + m_ScoringMode);
			if (pguiItem != NULL)
			{
				plbScoreModes->SetSel(pguiItem);
				plbScoreModes->AdjustContents();
				plbScoreModes->EnsureVisible(pguiItem);

			}
			
			if (guiDialog.DoModal(pguiRoot, pguiOk, pguiCancel) == 1)
			{
				lMinutes = peditMinutes->GetVal();
				lSeconds = peditSeconds->GetVal() % 60;
				m_lScoreInitialTime = m_lScoreTimeDisplay = (lMinutes * 60000) + (lSeconds * 1000);
				if (m_lScoreTimeDisplay == 0)
					m_bScoreTimerCountsUp = true;
				else
					m_bScoreTimerCountsUp = false;
				m_sKillsGoal = (short) peditKillsNum->GetVal();
				m_sFlagsGoal = (short) peditFlagsNum->GetVal();
				m_dKillsPercentGoal = (double) peditKillsPct->GetVal();

				pguiItem = plbScoreModes->GetSel();
				if (pguiItem != NULL)
					m_ScoringMode = pguiItem->m_lId - SCORE_MODE_LIST_BASE;
			}
		}
	}
}

#ifdef MOBILE
extern "C"
{
#include "android/android.h"
}
#endif
////////////////////////////////////////////////////////////////////////////////
// IsEndOfLevelGoalMet - check to see if level is complete based on the
//								 scoring and game play mode.
////////////////////////////////////////////////////////////////////////////////
bool CRealm::IsEndOfLevelGoalMet(bool bEndLevelKey)
{
#ifdef MOBILE
	bool showAndroidKey = true;
	switch (m_ScoringMode)
	{
	case Standard:
		if (m_sHostileBirths != 0)
			if (((m_sHostileKills * 100) / m_sHostileBirths < m_dKillsPercentGoal))
				showAndroidKey = false;
		break;
	default: //Hide the next key for anything else for the moment
		showAndroidKey = false;
	}
	AndroidSetShowEndLevelKey(showAndroidKey);
#endif

	bool bEnd = true;

	if (m_bPressedEndLevelKey)
	{
		m_bPressedEndLevelKey = false;
		// Hack: don't let the level end immediately if the player is using the debug level skip
		if (m_time.GetGameTime() > 1000)
			bEndLevelKey = true;
	}

	switch (m_ScoringMode)
	{
		// In a standard level, the user is done when the percentage of hostiles killed
		// is greater than the minimum set in the level and the user presses the
		// 'next level' key.
		case Standard:
			if (m_sHostileBirths != 0)
				{
				if (((m_sHostileKills * 100) / m_sHostileBirths < m_dKillsPercentGoal) || !bEndLevelKey)
					bEnd = false;
				}
				else
				{
					if (!bEndLevelKey)
						bEnd = false;
				}
			break;

		// In a timed level, the user is done when the time runs out, the population
		// runs out, or the user presses the 'next level' key.
		case Timed:
			if (m_lScoreTimeDisplay > 0 && m_sPopulation > 0 && !bEndLevelKey)
				bEnd = false;
			break;

		// In a timed goal level, the user must meet the goal within the specified
		// time.
		case TimedGoal:
			if (m_lScoreTimeDisplay > 0 && m_sPopulationDeaths < m_sKillsGoal)
				bEnd = false;
			break;

		// In a timed flag level, the user must get the flag to a base before
		// the goal is considered met.
		case TimedFlag:
		case MPTimedFlag:
//			if (m_lScoreTimeDisplay > 0 && m_sFlagsCaptured < m_sFlagsGoal)
			if (m_lScoreTimeDisplay > 0 && m_sFlagbaseCaptured < m_sFlagsGoal)
				bEnd = false;
			break;

		// In a capture the flag level, a user must capture a flag and return it
		// to a base to complete the level.
		case CaptureFlag:
		case MPCaptureFlag:
			if (m_sFlagbaseCaptured < m_sFlagsGoal)
				bEnd = false;
			break;

		// In a goal level, the user can only be done when they meet the goal.
		case Goal:
			if (m_sPopulationDeaths < m_sKillsGoal)
				bEnd = false;
			break;

		// In a checkpoint level, the user collects as many flags as possible and
		// can choose to end the level whenever they want (they'll just get a lower
		// score, if they have not gotten all the flags).
		case Checkpoint:
			if (m_sFlagsGoal == 0)
			{
				if (m_lScoreTimeDisplay > 0 && m_sFlagsCaptured < m_asClassNumThings[CThing::CFlagID])
					bEnd = false;
			}
			else
			{
				if (m_lScoreTimeDisplay > 0 && m_sFlagsCaptured < m_sFlagsGoal && !bEndLevelKey)
					bEnd = false;
			}
			break;

		case MPFrag:
			// Get highest number of kills from score module and 
			if ((m_sKillsGoal < 1) || (ScoreHighestKills(this) < m_sKillsGoal))
				bEnd = false;
			break;

		case MPTimedFrag:
			if (m_lScoreTimeDisplay > 0 && ScoreHighestKills(this) < m_sKillsGoal)
				bEnd = false;
			break;

		case MPLastMan:
			// if (ScorePlayersRemaining() > 1)
				bEnd = false;
			break;

		case MPTimed:
			if (m_lScoreTimeDisplay > 0)
				bEnd = false;
			break;
	}

#if defined(DEBUG_LEVEL_CHEAT)
	bEnd = bEndLevelKey;
#endif

	return bEnd;
}


////////////////////////////////////////////////////////////////////////////////
// Determine if a path is clear of terrain.
////////////////////////////////////////////////////////////////////////////////
bool CRealm::IsPathClear(			// Returns true, if the entire path is clear.
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
	short sVerticalTolerance /*= 0*/,	// In:  Max traverser can step up.
	short* psX /*= NULL*/,			// Out: If not NULL, last clear point on path.
	short* psY /*= NULL*/,			// Out: If not NULL, last clear point on path.
	short* psZ /*= NULL*/,			// Out: If not NULL, last clear point on path.
	bool bCheckExtents /*= true*/)	// In:  If true, will consider the edge of the realm a path
												// inhibitor.  If false, reaching the edge of the realm
												// indicates a clear path.
	{
	bool	bEntirelyClear	= false;	// Assume entire path is not clear.

	////////////////////////// Traverse path ///////////////////////////////////

	// Get most efficient increments that won't miss any attributes.
	// For the rates we use trig with a hypotenuse of 1 which will give
	// us a rate <= 1.0 and then multiply by the the crawl for
	// a reasonable increase in the speed of this alg.
	
	// sAngle must be between 0 and 359.
	sRotY	= rspMod360(sRotY);

	float	fRateX		= COSQ[sRotY] * dCrawlRate;
	float	fRateZ		= -SINQ[sRotY] * dCrawlRate;
	float	fRateY		= 0.0;	// If we ever want vertical movement . . .

	// Set initial position to first point to check (NEVER checks original position).
	float	fPosX			= sX + fRateX;
	float	fPosY			= sY + fRateY;
	float	fPosZ			= sZ + fRateZ;

	// Determine amount traveled per iteration on X/Z plane just once.
	float	fIterDistXZ		= rspSqrt(ABS2(fRateX, fRateZ) );

	float	fTotalDistXZ	= 0.0F;

	// Store extents.
	short	sMaxX			= GetRealmWidth();
	short	sMaxZ			= GetRealmHeight();

	short	sMinX			= 0;
	short	sMinZ			= 0;

	short	sCurH;

	bool	bInsurmountableHeight	= false;

	// Scan while in realm.
	while (
			fPosX > sMinX 
		&& fPosZ > sMinZ 
		&& fPosX < sMaxX 
		&& fPosZ < sMaxZ
		&& fTotalDistXZ < sDistanceXZ)
		{
		sCurH	= GetHeight((short)fPosX, (short)fPosZ);
		// If too big a height difference . . .
		if (sCurH - fPosY > sVerticalTolerance)
			{
			bInsurmountableHeight	= true;
			break;
			}

		// Update position.
		fPosX	+= fRateX;
		fPosY	=	MAX(fPosY, (float)sCurH);
		fPosZ	+= fRateZ;
		// Update distance travelled on X/Z plane.
		fTotalDistXZ	+= fIterDistXZ;
		}

	// Set end pt.
	SET(psX, fPosX);
	SET(psY, fPosY);
	SET(psZ, fPosZ);

	// If we made it the whole way . . .
	if (fTotalDistXZ >= sDistanceXZ)
		{
		bEntirelyClear	= true;
		}
	// Else, if we didn't hit any terrain . . .
	else if (bInsurmountableHeight == false)
		{
		// Only clear if we are not checking extents.
		bEntirelyClear	= !bCheckExtents;
		}

#if 0
	// FEEDBACK.
	// Create a line sprite.
	CSpriteLine2d*	psl2d	= new CSpriteLine2d;
	if (psl2d != NULL)
		{
		Map3Dto2D(
			sX, 
			sY, 
			sZ, 
			&(psl2d->m_sX2), 
			&(psl2d->m_sY2) );
		Map3Dto2D(
			fPosX, 
			fPosY, 
			fPosZ, 
			&(psl2d->m_sX2End), 
			&(psl2d->m_sY2End) );
		psl2d->m_sPriority	= sZ;
		psl2d->m_sLayer		= GetLayerViaAttrib(GetLayer(sX, sZ));
		psl2d->m_u8Color		= (bEntirelyClear == false) ? 249 : 250;
		// Destroy when done.
		psl2d->m_sInFlags	= CSprite::InDeleteOnRender;
		// Put 'er there.
		m_scene.UpdateSprite(psl2d);
		}
#endif

	return bEntirelyClear;
	}

////////////////////////////////////////////////////////////////////////////////
// Determine if a path is clear of terrain.
////////////////////////////////////////////////////////////////////////////////
bool CRealm::IsPathClear(			// Returns true, if the entire path is clear.
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
	short sVerticalTolerance /*= 0*/,	// In:  Max traverser can step up.
	short* psX /*= NULL*/,			// Out: If not NULL, last clear point on path.
	short* psY /*= NULL*/,			// Out: If not NULL, last clear point on path.
	short* psZ /*= NULL*/,			// Out: If not NULL, last clear point on path.
	bool bCheckExtents /*= true*/)	// In:  If true, will consider the edge of the realm a path
												// inhibitor.  If false, reaching the edge of the realm
												// indicates a clear path.
	{
	short	sDistanceXZ	= rspSqrt(ABS2(sDstX - sX, sZ - sDstZ) );
	short	sRotY			= rspATan(sZ - sDstZ, sDstX - sX);

	return IsPathClear(		// Returns true, if the entire path is clear.
									// Returns false, if only a portion of the path is clear.
									// (see *psX, *psY, *psZ).
		sX,						// In:  Starting X.
		sY,						// In:  Starting Y.
		sZ,						// In:  Starting Z.
		sRotY,					// In:  Rotation around y axis (direction on X/Z plane).
		dCrawlRate,				// In:  Rate at which to scan ('crawl') path in pixels per
									// iteration.
									// NOTE: Values less than 1.0 are inefficient.
									// NOTE: We scan terrain using GetHeight()
									// at only one pixel.
									// NOTE: We could change this to a speed in pixels per second
									// where we'd assume a certain frame rate.
		sDistanceXZ,			// In:  Distance on X/Z plane.
		sVerticalTolerance,	// In:  Max traverser can step up.
		psX,						// Out: If not NULL, last clear point on path.
		psY,						// Out: If not NULL, last clear point on path.
		psZ,						// Out: If not NULL, last clear point on path.
		bCheckExtents);		// In:  If true, will consider the edge of the realm a path
									// inhibitor.  If false, reaching the edge of the realm
									// indicates a clear path.
	}

////////////////////////////////////////////////////////////////////////////////
// Gives this realm an opportunity and drawing surface to display its 
// current status.
////////////////////////////////////////////////////////////////////////////////
void CRealm::DrawStatus(	// Returns nothing.
	RImage*	pim,				// In:  Image in which to draw status.
	RRect*	prc)				// In:  Rectangle in which to draw status.  Clips to.
	{
	long	lCurTime	= m_time.GetGameTime();
	if (lCurTime > m_lLastStatusDrawTime + STATUS_UPDATE_INTERVAL)
		{
		// Set print/clip to area.
		RRect	rcDst;
		rcDst.sX	= prc->sX + STATUS_PRINT_X;
		rcDst.sY = prc->sY + STATUS_PRINT_Y;
		rcDst.sW = prc->sW - STATUS_PRINT_X;
		rcDst.sH	= prc->sH - STATUS_PRINT_Y;
		// Clear.
		rspRect(RSP_BLACK_INDEX, pim, rcDst.sX, rcDst.sY, rcDst.sW, rcDst.sH);

		ms_print.SetDestination(pim, &rcDst);
		ms_print.print(
			pim, 
			rcDst.sX, 
			rcDst.sY, 
			"      Population %d                 Body Count %d (%d%%)                       Goal %d%%",
			m_sPopulationBirths,
			m_sPopulationDeaths,
			m_sPopulationDeaths * 100 / ((m_sPopulationBirths != 0) ? m_sPopulationBirths : 1),
			(short)m_dKillsPercentGoal
			);

		m_lLastStatusDrawTime	= lCurTime;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Maps a 3D coordinate onto the viewing plane provided the view angle
// (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::Map3Dto2D(	// Returns nothing.
	short sX,				// In.
	short	sY,				// In.
	short	sZ,				// In.
	short* psX,				// Out.
	short* psY)				// Out.
	{
	::Map3Dto2D(sX, sY, sZ, psX, psY, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Maps a 3D coordinate onto the viewing plane provided the view angle
// (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::Map3Dto2D(	// Returns nothing.
	double	dX,			// In.
	double	dY,			// In.
	double	dZ,			// In.
	double* pdX,			// Out.
	double* pdY)			// Out.
	{
	::Map3Dto2D(dX, dY, dZ, pdX, pdY, m_phood->GetRealmRotX() );
	}


////////////////////////////////////////////////////////////////////////////////
// Scales a Z coordinate onto the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapZ3DtoY2D(	// Returns nothing.
	double	dZIn,				// In.
	double*	pdYOut)			// Out.
	{
	::MapZ3DtoY2D(dZIn, pdYOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Scales a Z coordinate onto the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapZ3DtoY2D(	// Returns nothing.
	short		sZIn,				// In.
	short*	psYOut)			// Out.
	{
	::MapZ3DtoY2D(sZIn, psYOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate from the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapY2DtoZ3D(	// Returns nothing.
	double	dYIn,				// In.
	double*	pdZOut)			// Out.
	{
	::MapY2DtoZ3D(dYIn, pdZOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate from the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapY2DtoZ3D(	// Returns nothing.
	short		sYIn,				// In.
	short*	psZOut)			// Out.
	{
	::MapY2DtoZ3D(sYIn, psZOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate onto the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapY3DtoY2D(	// Returns nothing.
	double	dYIn,				// In.
	double*	pdYOut)			// Out.
	{
	::MapY3DtoY2D(dYIn, pdYOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate onto the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapY3DtoY2D(	// Returns nothing.
	short		sYIn,				// In.
	short*	psYOut)			// Out.
	{
	::MapY3DtoY2D(sYIn, psYOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate from the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapY2DtoY3D(	// Returns nothing.
	double	dYIn,				// In.
	double*	pdYOut)			// Out.
	{
	::MapY2DtoY3D(dYIn, pdYOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// Scales a Y coordinate from the viewing plane using the 
// view angle (~angle of projection).
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapY2DtoY3D(	// Returns nothing.
	short		sYIn,				// In.
	short*	psYOut)			// Out.
	{
	::MapY2DtoY3D(sYIn, psYOut, m_phood->GetRealmRotX() );
	}

////////////////////////////////////////////////////////////////////////////////
// If enabled, scales the specified height based on the view angle.
////////////////////////////////////////////////////////////////////////////////
void CRealm::MapAttribHeight(	// Returns nothing.
	short		sHIn,					// In.
	short*	psHOut)				// Out.
	{
	// If scaling attrib map heights . . .
	if (m_phood->m_sScaleAttribHeights != FALSE)
		{
		short	sRotX	= m_phood->GetRealmRotX();

		// Scale into realm.
		::MapY2DtoY3D(sHIn, psHOut, sRotX);
		}
	else
		{
		*psHOut	= sHIn;
		}
	}

////////////////////////////////////////////////////////////////////////////////
//// Terrrain map access functions /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Note these had no comments describing their function so I made some very
// vague comments that I hope were accurate -- JMI	06/28/97.

// Get the terrain height at an x/z position.
// Zero, if off map.
short CRealm::GetHeight(short sX, short sZ)
	{
	short	sRotX	= m_phood->GetRealmRotX();
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, sRotX);

	short	sH = 4 * (m_pTerrainMap->GetVal(sX, sZ, 0x0000) & REALM_ATTR_HEIGHT_MASK); 

	// Scale into realm.
	MapAttribHeight(sH, &sH);

	return sH;
	}

// Get the height and 'not walkable' status at the specified location.
// 'No walk', if off map.
short CRealm::GetHeightAndNoWalk(	// Returns height at new location.
	short sX,								// In:  X position to check on map.
	short	sZ,								// In:  Z position to check on map.
	bool* pbNoWalk)						// Out: true, if 'no walk'.
	{
	short	sRotX	= m_phood->GetRealmRotX();
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, sRotX);

	U16	u16Attrib	= m_pTerrainMap->GetVal(sX, sZ, REALM_ATTR_NOT_WALKABLE);

	short	sH = 4 * (u16Attrib & REALM_ATTR_HEIGHT_MASK); 

	// Scale into realm.
	MapAttribHeight(sH, &sH);

	// Get 'no walk'.
	if (u16Attrib & REALM_ATTR_NOT_WALKABLE)
		{
		*pbNoWalk	= true;
		}
	else
		{
		*pbNoWalk	= false;
		}

	return sH;
	}

// Get the terrain attributes at an x/z position.
// 'No walk', if off map.
short CRealm::GetTerrainAttributes(short sX, short sZ)
	{
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, m_phood->GetRealmRotX() );

	return m_pTerrainMap->GetVal(sX, sZ, REALM_ATTR_NOT_WALKABLE); 
	}

// Get the floor attributes at an x/z position.
// Zero, if off map.
short CRealm::GetFloorAttribute(short sX, short sZ)
	{
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, m_phood->GetRealmRotX() );

	return m_pTerrainMap->GetVal(sX, sZ, 0) & REALM_ATTR_FLOOR_MASK; 
	}

// Get the floor value at an x/z position.
// sMask, if off map.
short CRealm::GetFloorMapValue(short sX, short sZ, short sMask/* = 0x007f*/)
	{
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, m_phood->GetRealmRotX() );

	return m_pTerrainMap->GetVal(sX, sZ, sMask); 
	}

// Get the all alpha and opaque layer bits at an x/z position.
// Zero, if off map.
short CRealm::GetLayer(short sX, short sZ)
	{
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, m_phood->GetRealmRotX() );

	return m_pLayerMap->GetVal(sX, sZ, 0) & REALM_ATTR_LAYER_MASK; 
	}

// Get effect attributes at an x/z position.
// Zero, if off map.
short CRealm::GetEffectAttribute(short sX, short sZ)
	{
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, m_phood->GetRealmRotX() );

	return m_pTerrainMap->GetVal(sX, sZ, 0) & REALM_ATTR_EFFECT_MASK; 
	}

// Get effect value at an x/z position.
// Zero, if off map.
short CRealm::GetEffectMapValue(short sX, short sZ)
	{
	// Scale the Z based on the view angle.
	::MapZ3DtoY2D(sZ, &sZ, m_phood->GetRealmRotX() );

	return m_pTerrainMap->GetVal(sX, sZ, 0); 
	}
	
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Makes a 2D path based on the current hood setting for 'Use top-view 2Ds'.
// Note that this function returns to you a ptr to its one and only static
// string of length RSP_MAX_PATH.  Do not write to this string and do not
// store this string.  It is best to just use this call to pass a string to
// a function that will just use it right away (i.e., will not store it or
// modify it).
////////////////////////////////////////////////////////////////////////////////
const char* CRealm::Make2dResPath(	// Returns a ptr to an internal static buffer
												// containing the passed string, pszResName,
												// preceded by the appropriate directory based
												// on the current hood settings.
	const char* pszResName)				// In:  Resource name to prepend path to.
	{
	static char	szFullPath[RSP_MAX_PATH];

	ASSERT(m_s2dResPathIndex < NUM_ELEMENTS(ms_apsz2dResPaths) );

	// Get resource path.
	char*	pszPath	= ms_apsz2dResPaths[m_s2dResPathIndex];
	
	ASSERT(strlen(pszPath) + strlen(pszResName) < sizeof(szFullPath) );

	strcpy(szFullPath, pszPath);
	strcat(szFullPath, pszResName);

	return szFullPath;
	}

////////////////////////////////////////////////////////////////////////////////
// Creates the layer map, if it has not already been done.
// Now that the layer map needs to be 32K of uncompressable data, we create it
// at run time.
// (static)
////////////////////////////////////////////////////////////////////////////////
void CRealm::CreateLayerMap(void)
	{
	// If table needs to be built . . .
	if (ms_asAttribToLayer[0] != LayerSprite16)
		{
		long	l;
		for (l = 0; l < NUM_ELEMENTS(ms_asAttribToLayer); l++)
			{
			if (l & 0x0001)
				ms_asAttribToLayer[l]	= LayerSprite1;
			else if (l & 0x0002)
				ms_asAttribToLayer[l]	= LayerSprite2;
			else if (l & 0x0004)
				ms_asAttribToLayer[l]	= LayerSprite3;
			else if (l & 0x0008)
				ms_asAttribToLayer[l]	= LayerSprite4;
			else if (l & 0x0010)
				ms_asAttribToLayer[l]	= LayerSprite5;
			else if (l & 0x0020)
				ms_asAttribToLayer[l]	= LayerSprite6;
			else if (l & 0x0040)
				ms_asAttribToLayer[l]	= LayerSprite7;
			else if (l & 0x0080)
				ms_asAttribToLayer[l]	= LayerSprite8;
			else if (l & 0x0100)
				ms_asAttribToLayer[l]	= LayerSprite9;
			else if (l & 0x0200)
				ms_asAttribToLayer[l]	= LayerSprite10;
			else if (l & 0x0400)
				ms_asAttribToLayer[l]	= LayerSprite11;
			else if (l & 0x0800)
				ms_asAttribToLayer[l]	= LayerSprite12;
			else if (l & 0x1000)
				ms_asAttribToLayer[l]	= LayerSprite13;
			else if (l & 0x2000)
				ms_asAttribToLayer[l]	= LayerSprite14;
			else if (l & 0x4000)
				ms_asAttribToLayer[l]	= LayerSprite15;
			else
				ms_asAttribToLayer[l]	= LayerSprite16;
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
