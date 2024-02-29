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
// dispenser.cpp
// Project: Nostril (aka Postal)
//
// This module impliments the CDispenser class, which is a simple one animationed
// 3D object.
//
// History:
//		03/19/97	JMI	Started this dispenser item class using CDispenser as a 
//							template.
//							Will not hose you, but not yet functional.  Not recommended
//							for saving with your realm yet.  Might change loading
//							somewhat, but probably not.
//
//		03/20/97	JMI	Now has VERY simple timer logic.
//							Has large problem with statics' sFileCount parameter to
//							Load().
//
//		03/25/97	JMI	Changed EDIT_GUI_FILE to 8.3 name.
//
//		04/04/97	JMI	No longer uses construct with ID in InstantiateDispensee().
//
//		04/08/97	JMI	Although I was checking to make sure I did not mod by 0
//							when determining the next time for dispensage, if it was
//							the case that I would've mod'ed by 0, I was simply not
//							setting a new time causing the dispenser to just pump out
//							one dude per iteration and, indirectly, this run on sen-
//							tence.  Fixed.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/23/97	JMI	Added new logic type Exists and added a max for the number
//							of dispensees and a current number of dispensees dispensed.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/14/97	JMI	Upgraded to use new DoGui() method for sub dialogs 
//							involving GuiPressed().
//
//		06/17/97	JMI	Converted all occurrences of rand() to GetRand() and
//							srand() to SeedRand().
//
//		06/27/97	JMI	Now uses RListBox::EnsureVisible() to make sure the selected
//							dispensee type is visible in EditModify().
//							Also, displays dispensee type via text in EditRender().
//							Also, now updates the dispensee's position in Save() 
//							instead of EditMove() to speed up dragging.
//							Also, now uses Map3Dto2D() in EditRender() and EditRect().
//
//		06/27/97	JMI	Now shows actual dispensee in a deluxe way adding only an
//							icon indicating we're a dispenser.
//
//		06/27/97	JMI	Added temp fix to cause the dispenser m_imRender to be
//							created on load.  But it is very cheesy.
//
//		06/28/97	JMI	Added a function to render the dispensee, 
//							RenderDisipensee() which is now called in the various
//							places that need to update the dispensee's icon.
//
//		06/30/97	JMI	Render() was using m_pim where it should had been using 
//							m_imRender for determining the height for offseting the
//							priority.
//							Now uses the dispensee's actual EditRect() and 
//							EditHotSpot().
//
//		06/30/97	JMI	Now maps the Z to 3D when loading fileversions previous to
//							24.
//
//		07/03/97	JMI	Now uses SetGuiToNotify() to make a button able to end a
//							DoGui() session.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/10/97	JMI	Added GetClosestDude().
//							Also, added new logic type DistanceToDude.
//
//		07/14/97	JMI	Was only instantiating the dispensee in edit mode.  This
//							had the potential of causing the dispensee's ms_sFileCount
//							to be different when loading in edit mode than in non edit
//							mode and, therefore, could cause it to load its statics
//							different amounts of times between modes.  Changed it to
//							be consistent and instantiate the dispensee even when
//							it doesn't need to in non-edit mode.  It just only renders
//							the dispensee in edit mode loads.
//
//		07/21/97	JMI	Now, if it has no thing ptr to use to create the editor 
//							icon, it chooses a size of WIDTH_IF_NO_THING x 
//							HEIGHT_IF_NO_THING.
//
//		07/27/97	JMI	Now m_sMaxDispensees as a negative indicates infinite
//							dispensees.  This is user selectable via a checkbox in the
//							dialog.
//
//		07/28/97	JMI	Now sets m_u16IdDispensee to IdNil after instantiating a
//							thing for renderage.  This was causing a problem with 
//							'Exists' logic.
//							Also, added initial delay for all logics.
//							Changed m_alLogicParms[0],2,3 to m_alLogicParms[4] (adding one
//							logic parm while changing the storage technique).
//							Made dialog more deluxe to handle new parm and 
//							descriptions.
//							Checking in to work at home...may not be up to par.
//
//		07/28/97	JMI	Now uses initial delay to set the next update time.
//							Also, now word wraps description in GUI.
//							Also, now displays a message when no dispensee type chosen.
//
//					JMI	Changed DestroyDispensee() to not ASSERT on a NULL ptr.
//
//		07/29/97	JMI	Changed logic descriptions.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.
//
//		08/10/97	JMI	Now lets you jump right to editting the dispensee by 
//							holding down ALT key when you choose EditModify().
//
////////////////////////////////////////////////////////////////////////////////
#define DISPENSER_CPP

#include "RSPiX.h"
#include <math.h>

#include "dispenser.h"
#include "reality.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define EDIT_GUI_FILE			"res/editor/Dispense.gui"

#define RES_FILE					"dispenser.bmp"

#define THINGS_LIST_BOX_ID				3
#define LOGICS_LIST_BOX_ID				4
#define LOGIC_PARMS_EDIT_ID_BASE		101
#define LOGIC_PARMS_TEXT_ID_BASE		201
#define BTN_MODIFY_DISPENSEE_ID		8
#define MAX_DISPENSEES_EDIT_ID		9
#define INFINITE_DISPENSEES_MB_ID	20
#define DESCRIPTION_TEXT_ID			300

// Font settings for displaying dispensee's that don't return a sprite.
#define FONT_SIZE						15
#define FONT_COLOR					249

// Width and Height if no thing.  Allow enough to read text.
#define WIDTH_IF_NO_THING			60
#define HEIGHT_IF_NO_THING			30

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// Let this auto-init to 0
short CDispenser::ms_sFileCount;
short CDispenser::ms_sDispenseeFileCount;

// Descriptions of logic types and their parameters.
CDispenser::LogicInfo	CDispenser::ms_aliLogics[NumLogicTypes]	=
	{
		////////////////////////// Timed ////////////////////////////////////////
		{ 
		"Timed",		// Logic Name.
			{			// Parm descriptions.  NULL for each entry that does not exist.
			"Min delay (ms):", 
			"Max delay (ms):", 
			"Initial delay (ms):",
			NULL,
			},
		// Description of logic.
#if 1
		"\"Initial delay\" is the amount of time before dispensing begins.\n"
		"\"Min delay\" is the minimum amount of time before the next item is dispensed.\n"
		"\"Max delay\" is the maximum amount of time before the next item is dispensed.\n",
#else
		"After the initial delay, dispenses a dispensee between \"Min delay\" "
		"and \"Max delay\" milliseconds.",
#endif
		},

		////////////////////////// Exists ///////////////////////////////////////
		{ 
		"Exists",	// Logic Name
			{			// Parm descriptions.  NULL for each entry that does not exist.
			"Min delay (ms):", 
			"Max delay (ms):", 
			"Initial delay (ms):",
			NULL,
			},
		// Description of logic.
#if 1
		"\"Initial delay\" is the amount of time before dispensing begins.\n"
		"\"Min delay\" is the minimum amount of time after the previously dispensed item "
		"is destroyed or killed that the next item will be dispensed.\n"
		"\"Max delay\" is the maximum amount of time after the previously dispensed item "
		"is destroyed or killed that the next item will be dispensed.\n",
#else
		"After the initial delay, dispesenses a dispensee.  Does not dispense the "
		"next dispensee until between \"Min delay\" and \"Max delay\" milliseconds "
		"after the previous one has been destroyed/killed.",
#endif
		},

		///////////////////// Distance To Dude ///////////////////////////////////
		{ 
		"Distance to Dude",	// Logic Name.
			{						// Parm descriptions.  NULL for each entry that does not exist.
			"Min distance (0 = none):", 
			"Max distance (0 = none):", 
			"How often to check (ms):", 
			"Initial delay (ms):",
			},
		// Description of logic.
#if 1
		"\"Initial delay\" is the amount of time before dispensing begins.\n"
		"\"How often to check\" is the amount of time between checking the distance "
		"from the dispenser to the main dude.\n"
		"\"Min distance\" to \"Max distance\" is the range of distances to the closest "
		"main dude that will cause the next item to be dispensed.\n",
#else
		"After the initial delay, checks the closest main dude every \"How often "
		"to check\" milliseconds and dispenses a new dispensee if the main dude "
		"is within \"Min distance\" to \"Max distance\" pixels at that time.  Not "
		"for use with other toys.",
#endif
		},
	};

////////////////////////////////////////////////////////////////////////////////
// Local function prototypes
////////////////////////////////////////////////////////////////////////////////
void LogicItemCall(
	RGuiItem*	pguiLogicItem);	// In:  Logic item that was pressed.


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDispenser::Load(		// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,				// In:  File to load from
	bool bEditMode,			// In:  True for edit mode, false otherwise
	short sFileCount,			// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)		// In:  Version of file format to load.
	{
	short sResult = 0;

	// In most cases, the base class Load() should be called.
	sResult	= CThing::Load(pFile, bEditMode, sFileCount, ulFileVersion);
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
			case 35:
				pFile->Read(&m_sMaxDispensees);
				pFile->Read(&m_sX);
				pFile->Read(&m_sY);
				pFile->Read(&m_sZ);
				pFile->Read(&m_idDispenseeType);
				U16	u16LogicType;
				pFile->Read(&u16LogicType);
				m_logictype	= (LogicType)u16LogicType;
				pFile->Read(m_alLogicParms, 4);
				pFile->Read(&m_ulFileVersion);
				long	lSize;
				if (pFile->Read(&lSize) == 1)
					{
					// Open memory file to receive the clone data . . .
					if (m_fileDispensee.Open(lSize, 1L, (RFile::Endian)pFile->GetEndian()) == 0)
						{
						// Put 'er there.
						pFile->Read(m_fileDispensee.GetMemory(), lSize);
						}
					}

				// Had to start the whole format over b/c the number of parms changed and I
				// didn't want to read the parms all over the place.  The unfortunate thing
				// is that I end up with two versions of reading the dispensee which could
				// likely need to be changed at some point (so it'd have to be change in two
				// locations).  Perhaps I could make that part a separate inline.  Not sure
				// now though.
				
				// Break out here intentionally.
				break;

			// Older format support with less parms.  Make sure to init unused new parms
			// to zero.
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
				pFile->Read(&m_sMaxDispensees);

			case 6:
			case 5:
			case 4:
			case 3:
			case 2:
			case 1:
				{
				pFile->Read(&m_sX);
				pFile->Read(&m_sY);
				pFile->Read(&m_sZ);
				pFile->Read(&m_idDispenseeType);
				U16	u16LogicType;
				pFile->Read(&u16LogicType);
				m_logictype	= (LogicType)u16LogicType;
				pFile->Read(m_alLogicParms + 0);
				pFile->Read(m_alLogicParms + 1);
				pFile->Read(m_alLogicParms + 2);

				short i;
				for (i = 3; i < NumParms; i++)
					{
					m_alLogicParms[i]	= 0;
					}

				pFile->Read(&m_ulFileVersion);
				long	lSize;
				if (pFile->Read(&lSize) == 1)
					{
					// Open memory file to receive the clone data . . .
					if (m_fileDispensee.Open(lSize, 1L, (RFile::Endian)pFile->GetEndian()) == 0)
						{
						// Put 'er there.
						pFile->Read(m_fileDispensee.GetMemory(), lSize);
						}
					}
				break;
				}
			}
		
		// If the file version is earlier than the change to real 3D coords . . .
		if (ulFileVersion < 24)
			{
			// Convert to 3D.
			m_pRealm->MapY2DtoZ3D(
				m_sZ,
				&m_sZ);
			}

		// Make sure there were no file errors or format errors . . .
		if (!pFile->Error() && sResult == 0)
			{
			// Init dispenser
			sResult = Init(bEditMode);
			}
		else
			{
			sResult = -1;
			TRACE("CDispenser::Load(): Error reading from file!\n");
			}
		}
	else
		{
		TRACE("CDispenser::Load(): CThing::Load() failed.\n");
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CDispenser::Save(		// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,				// In:  File to save to
	short sFileCount)			// In:  File count (unique per file, never 0)
	{
	short sResult = 0;

	// In most cases, the base class Save() should be called.
	sResult	= CThing::Save(pFile, sFileCount);
	if (sResult == 0)
		{
		// Save common data just once per file (not with each object)
		if (ms_sFileCount != sFileCount)
			{
			ms_sFileCount = sFileCount;

			// Save static data
			}

		pFile->Write(m_sMaxDispensees);
		pFile->Write(m_sX);
		pFile->Write(m_sY);
		pFile->Write(m_sZ);
		pFile->Write(m_idDispenseeType);
		pFile->Write((U16)m_logictype);
		pFile->Write(m_alLogicParms, 4);

		// We do this here instead of on EditMove() b/c EditMove() can
		// be slow when on every iteration, it allocates a CWhatever,
		// loads it, sets the new position, saves it and deletes it.
		// Update position . . .
		CThing*	pthing;
		if (InstantiateDispensee(&pthing, false) == 0)
			{
			// Update position.
			pthing->EditMove(m_sX, m_sY, m_sZ);
			// Resave.
			SaveDispensee(pthing);
			// Get rid of.
			DestroyDispensee(&pthing);
			}

		pFile->Write(m_ulFileVersion);
		pFile->Write(m_fileDispensee.GetSize());
		pFile->Write(m_fileDispensee.GetMemory(), m_fileDispensee.GetSize());

		sResult	= pFile->Error();
		}
	else
		{
		TRACE("CDispenser::Save(): CThing::Save() failed.\n");
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CDispenser::Startup(void)						// Returns 0 if successfull, non-zero otherwise
	{
	switch (m_logictype)
		{
		case Timed:
		case Exists:
			m_lNextUpdate = m_pRealm->m_time.GetGameTime() + m_alLogicParms[2];
			break;
		case DistanceToDude:
			m_lNextUpdate = m_pRealm->m_time.GetGameTime() + m_alLogicParms[3];
			break;
		default:
			m_lNextUpdate = m_pRealm->m_time.GetGameTime();
			break;
		}

	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CDispenser::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
	{
	return 0;
	}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CDispenser::Suspend(void)
	{
	if (m_sSuspend == 0)
		{
		// Store current delta so we can restore it.
		long	lCurTime				= m_pRealm->m_time.GetGameTime();
		m_lNextUpdate				= lCurTime - m_lNextUpdate;
		}

	m_sSuspend++;
	}

////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CDispenser::Resume(void)
	{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
	if (m_sSuspend == 0)
		{
		long	lCurTime				= m_pRealm->m_time.GetGameTime();
		m_lNextUpdate				= lCurTime - m_lNextUpdate;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CDispenser::Update(void)
	{
	if (!m_sSuspend)
		{
		long	lCurTime	= m_pRealm->m_time.GetGameTime();

		if (m_sNumDispensees < m_sMaxDispensees || m_sMaxDispensees < 0)
			{
			// Logic on when to dispense next.
			switch (m_logictype)
				{
				case Timed:
					if (lCurTime >= m_lNextUpdate)
						{
						// Create a thing . . .
						CThing*	pthing;
						if (InstantiateDispensee(&pthing, false) == 0)
							{
							// Wahoo.
							m_sNumDispensees++;
							}

						if (m_alLogicParms[1] - m_alLogicParms[0] > 0)
							{
							// Next update will be in a min of m_alLogicParms[0] and a max of m_alLogicParms[1]
							// milliseconds.
							m_lNextUpdate	= lCurTime + m_alLogicParms[0] + (GetRand() % (m_alLogicParms[1] - m_alLogicParms[0]));
							}
						else
							{
							m_lNextUpdate	= lCurTime + m_alLogicParms[0];
							}
						}
					break;
				case Exists:
					{
					// If we don't have one . . .
					if (m_u16IdDispensee == CIdBank::IdNil)
						{
						if (lCurTime >= m_lNextUpdate)
							{
							// Create a thing . . .
							CThing*	pthing;
							if (InstantiateDispensee(&pthing, false) == 0)
								{
								// Wahoo.
								m_sNumDispensees++;
								}
							}
						}
					else
						{
						// If the last one no longer exists . . .
						CThing* pthing;
						if (m_pRealm->m_idbank.GetThingByID(&pthing, m_u16IdDispensee) != 0)
							{
							// Clear our ID.
							m_u16IdDispensee	= CIdBank::IdNil;
							// Set the next update time.
							if (m_alLogicParms[1] - m_alLogicParms[0] > 0)
								{
								// Next update will be in a min of m_alLogicParms[0] and a max of m_alLogicParms[1]
								// milliseconds.
								m_lNextUpdate	= lCurTime + m_alLogicParms[0] + (GetRand() % (m_alLogicParms[1] - m_alLogicParms[0]));
								}
							else
								{
								m_lNextUpdate	= lCurTime + m_alLogicParms[0];
								}
							}
						}
					break;
					}
				case DistanceToDude:
					{
					if (lCurTime >= m_lNextUpdate)
						{
						// If in range . . .
						long	lDudeDist;
						if (GetClosestDudeDistance(&lDudeDist) == 0)
							{
							if ( (lDudeDist >= m_alLogicParms[0] || m_alLogicParms[0] == 0) && (lDudeDist <= m_alLogicParms[1] || m_alLogicParms[1] == 0) )
								{
								// Create a thing . . .
								CThing*	pthing;
								if (InstantiateDispensee(&pthing, false) == 0)
									{
									// Wahoo.
									m_sNumDispensees++;
									}
								}
							}

						m_lNextUpdate	= lCurTime + m_alLogicParms[2];
						}
					break;
					}
				}
			}
		else
			{
			// Should it destroy itself?
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CDispenser::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	// Initialize for edit mode.
	short sResult	= Init(true);
	if (sResult == 0)
		{
		sResult	= EditModify();
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Set text for and recompose item.
////////////////////////////////////////////////////////////////////////////////
inline void SetLogicText(	// Returns nothing.
	RGuiItem*	pguiRoot,	// In:  Root item.
	long			lId,			// In:  ID of item to update text.
	char*			pszText,		// In:  New text or NULL for none and to disable lIdEdit.
	long			lIdEdit)		// In:  Item to enable or disable.
	{
	RGuiItem*	pguiEdit	= pguiRoot->GetItemFromId(lIdEdit);
	RGuiItem*	pguiText	= pguiRoot->GetItemFromId(lId);
	if (pszText != NULL)
		{
		RSP_SAFE_GUI_REF_VOID(pguiText, SetText("%s", pszText) );
		RSP_SAFE_GUI_REF_VOID(pguiText, Compose() );
		RSP_SAFE_GUI_REF_VOID(pguiEdit, SetVisible(TRUE) );
		}
	else
		{
		RSP_SAFE_GUI_REF_VOID(pguiText, SetText("") );
		RSP_SAFE_GUI_REF_VOID(pguiText, Compose() );
		RSP_SAFE_GUI_REF_VOID(pguiEdit, SetVisible(FALSE) );
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Setup parms for specified logic type.
////////////////////////////////////////////////////////////////////////////////
inline void SetupLogicParms(	// Returns nothing.
	RGuiItem*	pguiRoot,		// In:  Root item.
	short			sType)			// In:  Logic type to setup parms for.
	{
	short	i;
	for (i = 0; i < CDispenser::NumParms; i++)
		{
		SetLogicText(
			pguiRoot, 
			LOGIC_PARMS_TEXT_ID_BASE + i, 
			CDispenser::ms_aliLogics[sType].apszParms[i], 
			LOGIC_PARMS_EDIT_ID_BASE + i);
		}

	// Update description.
	RGuiItem*	pguiDescription	= pguiRoot->GetItemFromId(DESCRIPTION_TEXT_ID);
	if (pguiDescription)
		{
		// Remember if word wrap was on . . .
		short	sWasWordWrap	= FALSE;
		if (pguiDescription->m_pprint->m_eModes & RPrint::WORD_WRAP)
			{
			sWasWordWrap	= TRUE;
			}

		// Guarantee word wrap status.
		pguiDescription->m_pprint->SetWordWrap(TRUE);

		pguiDescription->SetText("%s", CDispenser::ms_aliLogics[sType].pszDescription);
		pguiDescription->Compose();

		// Restore word wrap status.
		pguiDescription->m_pprint->SetWordWrap(sWasWordWrap);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Set selection and update parms via pressed callback from GUI.
////////////////////////////////////////////////////////////////////////////////
void LogicItemCall(
	RGuiItem*	pguiLogicItem)	// In:  Logic item that was pressed.
	{
	ASSERT(pguiLogicItem->m_ulUserInstance != NULL);
	RListBox*	plb	= (RListBox*)pguiLogicItem->m_ulUserInstance;
	ASSERT(plb->m_type == RGuiItem::ListBox);

	// Make item the selection.
	plb->SetSel(pguiLogicItem);

	short	sType	= pguiLogicItem->m_ulUserData;

	// Update parms.
	SetupLogicParms(plb->GetParent(), sType);
	}

////////////////////////////////////////////////////////////////////////////////
// Updates the GUI items impacted by the current state of the max dispensees
// 'Infinite' checkbox.
////////////////////////////////////////////////////////////////////////////////
static void UpdateMaxDispensees(
	RGuiItem*	pgui_pmb)			// In:  Multibtn that was pressed.
	{
	ASSERT(pgui_pmb->m_type == RGuiItem::MultiBtn);
	RMultiBtn*	pmb	= (RMultiBtn*)pgui_pmb;
	REdit*		pedit	= (REdit*)pmb->m_ulUserInstance;
	ASSERT(pedit->m_type == RGuiItem::Edit);

	// If infinite dispensees . . .
	if (pmb->m_sState == 2)
		{
		// No need for edit field, then.
		pedit->SetVisible(FALSE);
		}
	else
		{
		// We'll be needing edit field, then.
		pedit->SetVisible(TRUE);
		// If negative . . .
		if (pedit->GetVal() < 0)
			{
			// Set to default val.
			pedit->SetText("%d", 10);
			pedit->Compose();
			}
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CDispenser::EditModify(void)					// Returns 0 if successfull, non-zero otherwise
	{
	// Get key status array.
	U8*	pau8KeyStatus	= rspGetKeyStatusArray();

	short	sResult	= 0;
	ClassIDType	idNewThingType;

	// Set up to modify dispensee.
	bool	bModifyDispensee	= false;

	// If not yet setup or key to jump straight to modifying dispensee is not held . . .
	if (m_idDispenseeType == TotalIDs || (pau8KeyStatus[RSP_SK_ALT] & 1) == 0)
		{
		idNewThingType	= TotalIDs;

		RGuiItem*	pguiRoot	= RGuiItem::LoadInstantiate(FullPathVD(EDIT_GUI_FILE));
		if (pguiRoot != NULL)
			{
			// Get items.
			RListBox*	plbThings					= (RListBox*)pguiRoot->GetItemFromId(THINGS_LIST_BOX_ID);
			RListBox*	plbLogics					= (RListBox*)pguiRoot->GetItemFromId(LOGICS_LIST_BOX_ID);
			RBtn*			pbtnModify					= (RBtn*)pguiRoot->GetItemFromId(BTN_MODIFY_DISPENSEE_ID);
			REdit*		peditMaxDispensees		= (REdit*)pguiRoot->GetItemFromId(MAX_DISPENSEES_EDIT_ID);
			RMultiBtn*	pmbInfiniteDispensees	= (RMultiBtn*)pguiRoot->GetItemFromId(INFINITE_DISPENSEES_MB_ID);
			if (	plbThings
				&& plbLogics 
				&& pbtnModify 
				&& peditMaxDispensees 
				&& pmbInfiniteDispensees)
				{
				ASSERT(plbThings->m_type == RGuiItem::ListBox);
				ASSERT(plbLogics->m_type == RGuiItem::ListBox);
				ASSERT(pbtnModify->m_type == RGuiItem::Btn);
				ASSERT(pmbInfiniteDispensees->m_type == RGuiItem::MultiBtn); 

				short	i;
				for (i = 0; i < NumParms; i++)
					{
					RGuiItem*	pgui	= pguiRoot->GetItemFromId(LOGIC_PARMS_EDIT_ID_BASE + i);
					if (pgui)
						{
						pgui->SetText("%ld", m_alLogicParms[i]);
						pgui->Compose();
						}
					}

				peditMaxDispensees->SetText("%ld", m_sMaxDispensees);
				peditMaxDispensees->Compose();

				// Point instance data at the max dispensees edit so it can show and
				// hide it.
				pmbInfiniteDispensees->m_ulUserInstance	= (ULONG)peditMaxDispensees;
				pmbInfiniteDispensees->m_sState				= (m_sMaxDispensees < 0) ? 2 : 1;
				pmbInfiniteDispensees->m_bcUser				= UpdateMaxDispensees;
				pmbInfiniteDispensees->Compose();

				UpdateMaxDispensees(pmbInfiniteDispensees);

				// Set a callback for the button to end the DoGui().
				SetGuiToNotify(pbtnModify);

				RGuiItem*	pguiItem;
				RGuiItem*	pguiSel	= NULL;
				for (i = 0; i < NumLogicTypes; i++)
					{
					pguiItem	= plbLogics->AddString(ms_aliLogics[i].pszName);
					if (pguiItem != NULL)
						{
						// Set item number.
						pguiItem->m_ulUserData		= i;
						// Set listbox ptr.
						pguiItem->m_ulUserInstance	= (ULONG)plbLogics;
						// Set callback.
						pguiItem->m_bcUser			= LogicItemCall;
						// If this item is the current logic type . . .
						if (m_logictype == i)
							{
							pguiSel	= pguiItem;
							// Select it.
							plbLogics->SetSel(pguiItem);
							// Set up parms.
							SetupLogicParms(pguiRoot, i);
							}
						}
					}

				plbLogics->AdjustContents();
				// If there's a selected item (there should be) . . .
				if (pguiSel != NULL)
					{
					plbLogics->EnsureVisible(pguiSel);
					}

				pguiSel	= NULL;

				// Add available objects to listbox.
				CThing::ClassIDType	idCur;
				for (idCur	= 0; idCur < CThing::TotalIDs; idCur++)
					{
					// If item is editor creatable . . .
					if (CThing::ms_aClassInfo[idCur].bEditorCreatable == true)
						{
						// Add string for each item to listbox.
						pguiItem	= plbThings->AddString((char*)CThing::ms_aClassInfo[idCur].pszClassName);
						if (pguiItem != NULL)
							{
							pguiItem->m_ulUserData	= (ULONG)idCur;

							// If this is the current type . . .
							if (m_idDispenseeType == idCur)
								{
								pguiSel	= pguiItem;
								// Select it.
								plbThings->SetSel(pguiItem);
								}
							}
						}
					}

				// Format list items.
				plbThings->AdjustContents();
				// If there's a selected item (there may not be) . . .
				if (pguiSel != NULL)
					{
					plbThings->EnsureVisible(pguiSel);
					}

				while (sResult == 0 && idNewThingType == TotalIDs)
					{
					bModifyDispensee	= false;

					switch (DoGui(pguiRoot) )
						{
						case BTN_MODIFY_DISPENSEE_ID:
							bModifyDispensee	= true;
							// Intentional fall through.
						case 1:
							{
							// Get logic selection.  Required.
							RGuiItem*	pguiSel	= plbLogics->GetSel();
							if (pguiSel != NULL)
								{
								m_logictype	= (LogicType)pguiSel->m_ulUserData;
								}
							else
								{
								sResult	= 1;
								}

							// Get dispensee type selection.  Required.
							pguiSel	= plbThings->GetSel();
							if (pguiSel != NULL)
								{
								idNewThingType	= (ClassIDType)pguiSel->m_ulUserData;
								}
							else
								{
								rspMsgBox(
									RSP_MB_ICN_INFO | RSP_MB_BUT_OK,
									"Dispenser",
									g_pszDispenserNoDispenseeTypeChosen);
								}

							short i;
							for (i = 0; i < NumParms; i++)
								{
								m_alLogicParms[i]	= pguiRoot->GetVal(LOGIC_PARMS_EDIT_ID_BASE + i);
								}

							if (pmbInfiniteDispensees->m_sState == 1)
								{
								m_sMaxDispensees	= peditMaxDispensees->GetVal();
								}
							else
								{
								m_sMaxDispensees	= -1;
								}
							
							break;
							}
						
						case 2:
						default:
							sResult	= 1;
							break;
						}
					}
				}
			else
				{
				TRACE("EditModify(): Missing GUI items in  %s.\n", EDIT_GUI_FILE);
				sResult	= -2;
				}

			// Done with GUI.
			delete pguiRoot;
			pguiRoot	= NULL;
			}
		else
			{
			TRACE("EditModify(): Failed to load %s.\n", EDIT_GUI_FILE);
			sResult	= -1;
			}
		}
	else
		{
		// Go right to modifying dispensee.
		bModifyDispensee	= true;
		// Use same thing type.
		idNewThingType	= m_idDispenseeType;
		}

	// If successful so far . . .
	if (sResult == 0)
		{
		// If we have a dispensee . . .
		CThing*	pthing	= NULL;
		// Instantiate it so we get its current settings . . . 
		if (InstantiateDispensee(&pthing, true) == 0)
			{
			// If the current dispensee has a different type than the desired one . . .
			if (pthing->GetClassID() != idNewThingType)
				{
				// Be done with this one.
				DestroyDispensee(&pthing);
				m_fileDispensee.Close();
				}
			}

		m_idDispenseeType	= idNewThingType;

		// If no current thing . . .
		if (pthing == NULL)
			{
			// Allocate the desired thing . . .
			sResult	= ConstructWithID(m_idDispenseeType, m_pRealm, &pthing);
			if (sResult == 0)
				{
				// New it in the correct location.
				sResult	= pthing->EditNew(m_sX, m_sY, m_sZ);
				if (sResult == 0)
					{
					// Success.
					}
				else
					{
					TRACE("EditModify(): EditNew() failed for new dispensee.\n");
					}

				// If any errors occurred after allocation . . .
				if (sResult != 0)
					{
					DestroyDispensee(&pthing);
					}
				}
			else
				{
				TRACE("EditModify(): Failed to allocate new %s.\n",
					CThing::ms_aClassInfo[m_idDispenseeType].pszClassName);
				}
			}

		if (pthing != NULL)
			{
			// If editing was specified . . .
			if (bModifyDispensee == true)
				{
				// Modify it.
				pthing->EditModify();
				}

			// Render 'im:
			RenderDispensee(pthing);

			// Dump 'im:
			SaveDispensee(pthing);

			// Now that we have one in cold storage, we're done with this one.
			DestroyDispensee(&pthing);
			}
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
short CDispenser::EditMove(							// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
	{
	short	sResult	= 0;	// Assume success.

	m_sX	= sX;
	m_sY	= sY;
	m_sZ	= sZ;

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CDispenser::EditRender(void)
	{
	// Map from 3d to 2d coords
	Map3Dto2D(
		(short) m_sX, 
		(short) m_sY, 
		(short) m_sZ, 
		&m_sprite.m_sX2, 
		&m_sprite.m_sY2);

	// Priority is based on hotspot of sprite
	m_sprite.m_sPriority = m_sZ;

	// Center on dispensee's hotspot.
	m_sprite.m_sX2	-= m_sDispenseeHotSpotX;
	m_sprite.m_sY2	-= m_sDispenseeHotSpotY;

	// Layer should be based on info we get from attribute map.
	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer(m_sX, m_sZ));

	// Image would normally animate, but doesn't for now
	m_sprite.m_pImage = &m_imRender;

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
	}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CDispenser::EditRect(RRect* prc)
	{
	// Map from 3d to 2d coords
	Map3Dto2D(
		(short) m_sX, 
		(short) m_sY, 
		(short) m_sZ, 
		&(prc->sX), 
		&(prc->sY) );

#if 0
	// Center on image.
	prc->sX	-= m_imRender.m_sWidth / 2;
	prc->sY	-= m_imRender.m_sHeight;

	prc->sW = m_imRender.m_sWidth;
	prc->sH = m_imRender.m_sHeight;
#else
	prc->sX	-= m_sDispenseeHotSpotX;
	prc->sY	-= m_sDispenseeHotSpotY;
	prc->sW	= m_rcDispensee.sW;
	prc->sH	= m_rcDispensee.sH;
#endif
	}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
// (virtual (Overridden here)).
////////////////////////////////////////////////////////////////////////////////
void CDispenser::EditHotSpot(	// Returns nothiing.
	short*	psX,					// Out: X coord of 2D hotspot relative to
										// EditRect() pos.
	short*	psY)					// Out: Y coord of 2D hotspot relative to
										// EditRect() pos.
	{
	// Base of dispenser is hotspot.
	*psX	= m_sDispenseeHotSpotX;
	*psY	= m_sDispenseeHotSpotY;
	}

////////////////////////////////////////////////////////////////////////////////
// Init dispenser
////////////////////////////////////////////////////////////////////////////////
short CDispenser::Init(	// Returns 0 if successfull, non-zero otherwise
	bool	bEditMode)		// true, if in edit mode; false, otherwise.
	{
	short sResult = 0;

	// Remember.
	m_bEditMode		= bEditMode;

	// Only need resources in edit mode . . .
	if (bEditMode == true)
		{
		// Get resources
		sResult = GetResources();
		}

	if (m_idDispenseeType < TotalIDs && sResult == 0)
		{
		// Instantiate dispensee so we can create its icon.
		// NOTE:  We MUST do this in both edit mode and non-edit mode
		// b/c it affects the dispensee's ms_sFileCount which can affect
		// the load process; therefore, we must be consistent.
		CThing*	pthing	= NULL;
		InstantiateDispensee(&pthing, false);

		// If in edit mode . . .
		if (bEditMode == true)
			{
			// Render 'im:
			RenderDispensee(pthing);
			}

		DestroyDispensee(&pthing);
		}

	// No special flags.
	m_sprite.m_sInFlags = 0;

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Kill dispenser
////////////////////////////////////////////////////////////////////////////////
void CDispenser::Kill(void)
	{
	// Remove sprite from scene (this is safe even if it was already removed!)
	m_pRealm->m_scene.RemoveSprite(&m_sprite);

	// Free resources.
	FreeResources();

	if (m_fileDispensee.IsOpen() != FALSE)
		{
		m_fileDispensee.Close();
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CDispenser::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;

	if (m_pim == NULL)
		{
		sResult	= rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(RES_FILE), &m_pim, RFile::LittleEndian);
		if (sResult == 0)
			{
			if (m_pim->Convert(RImage::FSPR8) == RImage::FSPR8)
				{
				}
			else
				{
				TRACE("GetResources(): Error converting to FSPR8.\n");
				}
			}
		}
		
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
void CDispenser::FreeResources(void)
	{
	if (m_pim != NULL)
		{
		// Release resources for animations.
		rspReleaseResource(&g_resmgrGame, &m_pim);
		}
	}

////////////////////////////////////////////////////////////////////////////////
// Create a dispensee from the memfile, if open.
////////////////////////////////////////////////////////////////////////////////
short CDispenser::InstantiateDispensee(	// Returns 0 on success.
	CThing**	ppthing,								// Out: New thing loaded from m_fileDispensee.
	bool		bEditMode)							// In:  true if in edit mode.
	{
	short	sResult	= 0;	// Assume success.

	// If we even have a dispensee type . . .
	if (m_idDispenseeType > 0 && m_idDispenseeType < TotalIDs)
		{
		// Allocate the desired thing . . .
		if (CThing::Construct(m_idDispenseeType, m_pRealm, ppthing) == 0)
			{
			if (m_fileDispensee.IsOpen() != FALSE)
				{
				m_fileDispensee.Seek(0, SEEK_SET);
				if ((*ppthing)->Load(
					&m_fileDispensee, 
					bEditMode, 
					--ms_sDispenseeFileCount,	// Always load statics for these.
					m_ulFileVersion) == 0)
					{
					U16	idInstance;
					if (m_pRealm->m_idbank.Get(*ppthing, &idInstance) == 0)
						{
						// Release file's ID (cannot have all the dispensee's
						// using the same ID) and set new one.
						(*ppthing)->SetInstanceID(idInstance);
						
						// Success.  
						m_u16IdDispensee	= idInstance;

						// If in edit mode . . .
						if (bEditMode == true)
							{
							// Update position.
							(*ppthing)->EditMove(m_sX, m_sY, m_sZ);
							}

						// Startup, if requested.  We only give one chance
						// UNlike CRealm::Startup().
	//					if ((*ppthing)->m_sCallStartup != 0)
							{
	//						(*ppthing)->m_sCallStartup	= 0;
							(*ppthing)->Startup();
							}
						}
					else
						{
						TRACE("InstantiateDispensee(): Could not get an instance ID from the idbank.\n");
						sResult	= -3;
						}
					}
				else
					{
					TRACE("InstantiateDispensee(): Load() failed for dispensee.\n");
					sResult	= -2;
					}
				}
			else
				{
				sResult	= 1;
				}

			// If any errors after allocation . . .
			if (sResult != 0)
				{
				DestroyDispensee(ppthing);
				}
			}
		else
			{
			TRACE("InstantiateDispensee(): Failed to allocate new %s.\n",
				CThing::ms_aClassInfo[m_idDispenseeType].pszClassName);
			sResult	= -1;
			}
		}
	else
		{
		sResult	= 1;
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Write dispensee to the memfile.
////////////////////////////////////////////////////////////////////////////////
short CDispenser::SaveDispensee(		// Returns 0 on success.
	CThing*	pthing)						// In:  Instance of Dispensee to save.
	{
	short	sResult	= 0;	// Assume success.

	// If we already have a mem file . . .
	if (m_fileDispensee.IsOpen() != FALSE)
		{
		m_fileDispensee.Close();
		}

	if (m_fileDispensee.Open(1, 1, RFile::LittleEndian) == 0)
		{
		if (pthing->Save(&m_fileDispensee, --ms_sDispenseeFileCount) == 0)
			{
			m_ulFileVersion	= CRealm::FileVersion;
			}
		else
			{
			TRACE("SaveDispensee(): pthing->Save() failed.\n");
			sResult	= -2;
			}
		}
	else
		{
		TRACE("SaveDispensee(): m_fileDispensee->Open() failed.\n");
		sResult	= -1;
		}

	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Render dispensee to m_imRender.
////////////////////////////////////////////////////////////////////////////////
short CDispenser::RenderDispensee(	// Returns 0 on success.
	CThing*	pthing)						// In:  Instance of Dispensee to render.
	{
	short	sResult	= 0;	// Assume success.
	
	// If in edit mode . . .
	if (m_bEditMode == true)
		{
		// Redo display image.  This is a waste if we're not in edit mode.

		m_imRender.DestroyData();

		CSprite*	psprite	= NULL;

		// If there is an instance . . .
		if (pthing)
			{
			// Prepare item to be rendered.
			pthing->Render();

			// Get size of dispensee.
			pthing->EditRect(&m_rcDispensee);

			pthing->EditHotSpot(&m_sDispenseeHotSpotX, &m_sDispenseeHotSpotY);

			psprite	= pthing->GetSprite();
			}
		else
			{
			// Map from 3d to 2d coords
			Map3Dto2D(
				(short) m_sX, 
				(short) m_sY, 
				(short) m_sZ, 
				&(m_rcDispensee.sX), 
				&(m_rcDispensee.sY) );

			m_rcDispensee.sW	= WIDTH_IF_NO_THING;
			m_rcDispensee.sH	= HEIGHT_IF_NO_THING;

			m_sDispenseeHotSpotX	= m_rcDispensee.sW / 2;
			m_sDispenseeHotSpotY	= m_rcDispensee.sH;
			}

		// Create image . . .
		if (m_imRender.CreateImage(	// Return 0 on success.
			m_rcDispensee.sW,				// Width of new buffer.
			m_rcDispensee.sH,				// Height of new buffer.
			RImage::BMP8)					// Type of new buffer.
			== 0)
			{
			// Clear.
			rspRect(
				0,								// In:  Not black, but transparent.
				&m_imRender,
				0,
				0,
				m_imRender.m_sWidth,
				m_imRender.m_sHeight,
				NULL);

			// If we could get the thing's sprite . . .
			if (psprite != NULL)
				{
				// Determine offset that would put the dispensee's hotspot in the center
				// of our image.
				short	sOffX	= -m_rcDispensee.sX;
				short	sOffY	= -m_rcDispensee.sY;

				RRect	rcClip(0, 0, m_imRender.m_sWidth, m_imRender.m_sHeight);

				// Render dispensee into image.
				m_pRealm->m_scene.Render(	// Returns nothing.          
					&m_imRender,				// Destination image.        
					sOffX,						// Destination 2D x coord.   
					sOffY,						// Destination 2D y coord.   
					psprite,						// Tree of sprites to render.
					m_pRealm->m_phood,		// Da hood, homey.           
					&rcClip,						// Dst clip rect.            
					NULL);						// XRayee, if not NULL.      
				}
			else
				{
				// Use text alternative.
				RPrint	print;
				print.SetFont(FONT_SIZE, &g_fontBig);
				print.SetColor(FONT_COLOR, 0, 0);
				print.print(
					&m_imRender,
					0,
					0,
					"%s",
					ms_aClassInfo[m_idDispenseeType].pszClassName);
				}
#if 1
			// Draw dispenser icon on top.
			rspBlit(
				m_pim,															// Src image.
				&m_imRender,													// Dst image.
				m_imRender.m_sWidth / 2 - m_pim->m_sWidth / 2,		// Dst x.
				m_imRender.m_sHeight / 2 - m_pim->m_sHeight / 2,	// Dst y.
				NULL);															// Dst clip.
#endif
			}
		else
			{
			TRACE("RenderDispensee(): m_imRender.CreateImage() failed.\n");
			sResult	= -1;
			}
		}

	return sResult;
	}

////////////////////////////////////////////////////////////////////////////////
// Get the distance to the closest dude.
////////////////////////////////////////////////////////////////////////////////
short CDispenser::GetClosestDudeDistance(	// Returns 0 on success.  Fails, if no dudes.
	long* plClosestDistance)					// Out:  Distance to closest dude.
	{
	short	sRes	= 1;	// Assume no dude found.

	ULONG	ulSqrDistance;
	ULONG	ulCurSqrDistance	= 0xFFFFFFFF;
	ULONG	ulDistX;
	ULONG	ulDistZ;
	CDude*	pdude;

	CListNode<CThing>* pDudeList = m_pRealm->m_aclassHeads[CThing::CDudeID].m_pnNext;
	
	// While we have a node and that node is owned (the head and tail are not owned).
	while (pDudeList && pDudeList->m_powner)
		{
		// Get current owner.
		pdude = (CDude*) pDudeList->m_powner;
		// Must be Dude.
		ASSERT(pdude->GetClassID() == CDudeID);

		// If this dude is not dead . . .
		if (pdude->m_state != CThing3d::State_Dead)
			{
			// Determine square distance on X/Z plane.
			ulDistX	= pdude->m_dX - m_sX;
			ulDistZ	= pdude->m_dZ - m_sZ;
			ulSqrDistance	= ulDistX * ulDistX + ulDistZ * ulDistZ;
			// If closer than the last guy . . .
			if (ulSqrDistance < ulCurSqrDistance)
				{
				// This one is closer.
				ulCurSqrDistance	= ulSqrDistance;

				// Definitely going to have a dude to return.
				sRes	= 0;
				}
			}

		// Next node please.
		pDudeList = pDudeList->m_pnNext;
		}

	if (sRes == 0)
		{
		*plClosestDistance	= rspSqrt(ulSqrDistance);
		}
	else
		{
		*plClosestDistance	= 0;
		}
	
	return sRes;
	}

////////////////////////////////////////////////////////////////////////////////
// Destroy an instantiated dispensee.
////////////////////////////////////////////////////////////////////////////////
void CDispenser::DestroyDispensee(	// Returns nothing.
	CThing**	ppthing)						// In:  Ptr to the instance.
	{
	ASSERT(ppthing);

	if (*ppthing)
		{
		// If this one is the one indicated by the ID . . .
		if ( (*ppthing)->GetInstanceID() == m_u16IdDispensee)
			{
			m_u16IdDispensee	= CIdBank::IdNil;
			}

		// Destroy the dispensee.
		delete *ppthing;

		// Clear user's pointer.
		*ppthing	= NULL;
		}
	}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
