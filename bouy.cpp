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
// bouy.cpp
// Project: Nostril (aka Postal)
//
// This module implements the bouy marker for use with the network navagation
//	system that will help the enemy guys get around the world.
//
// History:
//		01/28/97 BRH	Added bouy's to the editor which will help with navagation
//
//		02/02/97 BRH	Added NextRouteNode function which will tell you which
//							bouy you should go to next in order to get to your 
//							destination.
//
//		02/03/97 BRH	Added info to the Load and Save functions to save
//							information needed to reconnect the Bouy network after
//							loading it.  
//		
//		02/04/97 BRH	Added GetRouteTableEntry() function that can be called
//							from the CNavigationNet's ping function which will safely
//							return an entry from the routing table.  If the table
//							was not large enough, it expands the the current
//							number of nodes and initializes the data.
//
//		02/04/97	JMI	Changed LoadDib() call to Load() (which now supports
//							loading of DIBs).
//
//		02/23/97 BRH	Changed bouy resource to be under Resource Manager control.
//							Also changed Render to do nothing and moved its 
//							functionality to EditRender so that the Bouys are not
//							drawn during game play but only in the editor.
//
//		02/24/97	JMI	No longer sets the m_type member of the m_sprite b/c it
//							is set by m_sprite's constructor.
//
//		03/06/97 BRH	Changed to the new calling of Ping which doesn't have the
//							maxdepth parameter.  Also added a commented out version of
//							saving the route table but didn't want to include it yet
//							since it would change the format of the realm files.
//
//		03/07/97	JMI	Now draws the bouy number into it's m_pImage.
//
//		03/07/97	JMI	Now sets the color of the text to be safe.
//
//		03/07/97 BRH	Played some with the font for the bouys so it would
//							be easier to see and so it would show 2 digits.
//
//		03/13/97 BRH	Made a few changes to update the routing tables
//							correctly.  I will also be adding a hops table
//							in addition to the route table to cut down on the
//							number of pings required to fill in the tables.  Then
//							the hops tables can be freed since they aren't needed for
//							gameplay.
//
//		03/13/97	JMI	Load now takes a version number.
//
//		04/10/97 BRH	Updated this to work with the new multi layer attribute
//							maps.
//
//		04/11/97 BRH	Adding BuildRoutingTable function which will use a
//							a Breadth-First search of the tree to determine the
//							shortest route to all reachable nodes, and will then
//							use the temporary BSF tree to fill in the routing table.
//
//		04/15/97 BRH	Taking out the old routing method leaving just the new
//							which seems to be working.
//
//		04/20/97 BRH	Added MessageRequest function that will send the
//							bouy's function if it has one.  Also now loads and saves
//							the message using the message's Load and Save functions.
//
//		04/21/97 BRH	Changed to multiple dialogs for each type of message to
//							avoid special code to display the correct fields for each
//							different type of message.
//
//		04/24/97 BRH	Fixed problem in Load with new version number.
//
//		05/01/97 BRH	Removed messages for logic suggestions and put those
//							into the CPylon class instead.
//
//		05/29/97	JMI	Removed ASSERT on m_pRealm->m_pAttribMap which no longer
//							exists.
//
//		06/06/97 BRH	Freed three arrays used in BuildRouteTable that had
//							previously been a source of memory leaks.
//
//		06/25/97 BRH	Took out the STL set "linkset" because it was causing
//							sync problems with the game.  Apparently, the set uses
//							a tree to store its data, and when building that tree, 
//							uses some kind of random function to balance the tree, but
//							not the standard library rand() function, but a different
//							random function that we don't reset from realm to realm.
//							This caused the routing tables to be build differently
//							each time the game was played, and so the demo mode and
//							network mode were out of sync.
//
//		06/29/97 MJR	Removed last trace of STL, replacing vector with RFList.
//
//		06/29/97	JMI	Converted EditRect(), EditRender(), and/or Render() to
//							use Map3Dto2D().
//							Also, moved definitions of EditRect() and EditHotSpot() to
//							here from bouy.h.
//
//		06/30/97	JMI	Now maps the Z to 3D when loading fileversions previous to
//							24.
//
//		07/07/97 BRH	Fixed bug when saving bouy networks where bouys had been
//							deleted.  The Unlink was not correctly decrementing the
//							number of direct links so it was expecting more direct
//							links when the file was reloaded.
//
//		07/09/97	JMI	Now uses m_pRealm->Make2dResPath() to get the fullpath
//							for 2D image components.
//
//		07/25/97 BRH	Fixed the problem of bouys greater than 254 being
//							created in the editor.
//
//		07/30/97 BRH	Added a flag to indicate whether the bouys should be shown
//							or not so that they can be turned off in the editor.
//
//		08/02/97	JMI	Made bouy font smaller (was 22, now 15), made bouy font
//							brighter (was 1 (dark red), now 249 (bright red) ), and
//							widened bouy graphic in an attempt to make IDs more read-
//							able.
//
//		08/05/97	JMI	Changed priority to use Z position rather than 2D 
//							projected Y position.  
//
//		08/05/97 BRH	Defaulted the bouy network to ON.
//
//		08/08/97 BRH	Only the bouys of the current Nav Net are displayed now.
//							(and their hots are disabled when they are not shown).
//							This will help cut down the confusion and prevent
//							users from joining the networks which would be bad.  
//
//		08/08/97	JMI	Now calls GetResources() on Load() and only in edit mode.
//
//		08/08/97	JMI	Now calls GetResources() in Startup() but checks the realm
//							flag indicating whether we're in edit mode first to make
//							sure we are.
//
////////////////////////////////////////////////////////////////////////////////
#define BOUY_CPP

#include "RSPiX.h"
#include <math.h>

#include "bouy.h"
#include "navnet.h"

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define IMAGE_FILE			"bouy.bmp"

#define BOUY_ID_FONT_HEIGHT	15
#define BOUY_ID_FONT_COLOR		249

////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

// These are default values -- actually values are set using the editor!

// Let this auto-init to 0
short CBouy::ms_sFileCount;
bool  CBouy::ms_bShowBouys = true;


////////////////////////////////////////////////////////////////////////////////
// Load object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CBouy::Load(										// Returns 0 if successfull, non-zero otherwise
	RFile* pFile,											// In:  File to load from
	bool bEditMode,										// In:  True for edit mode, false otherwise
	short sFileCount,										// In:  File count (unique per file, never 0)
	ULONG	ulFileVersion)									// In:  Version of file format to load.
{
	GameMessage msg;
	// Call the base load to get the u16InstanceID
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

		// Load object data
		pFile->Read(&m_dX);
		pFile->Read(&m_dY);
		pFile->Read(&m_dZ);

		U16 u16Data;
		U16 u16NumLinks;
		U16 i;
		// Get number of links to be read
		pFile->Read(&u16NumLinks);
		for (i = 0; i < u16NumLinks; i++)
		{
			pFile->Read(&u16Data);
			m_LinkInstanceID.InsertTail(u16Data);
		}

		// Get the instance ID for the NavNet
		pFile->Read(&u16Data);
		m_u16ParentInstanceID = u16Data;

		// Switch on the parts that have changed
		switch (ulFileVersion)
		{
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
				msg.Load(pFile);
				break;

			default:
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
#if 0
		// If you were thinking of doing something like this, stop.
		// It is too early to create the image for displaying this
		// bouy (we don't yet have the bouy ID).
			// ONLY IN EDIT MODE . . .
			if (bEditMode == true)
				{
				// Get resources
				sResult = GetResources();
				}
#endif
		}
		else
		{
			sResult = -1;
			TRACE("CBouy::Load(): Error reading from file!\n");
		}
	}
	else
	{
		TRACE("CBouy::Load(): CThing::Load() failed.\n");
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Save object (should call base class version!)
////////////////////////////////////////////////////////////////////////////////
short CBouy::Save(										// Returns 0 if successfull, non-zero otherwise
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

	U16 u16Data = m_sNumDirectLinks;
	// Save the number of links that will follow in the file
	pFile->Write(&u16Data);
	CBouy* pLinkedBouy = m_aplDirectLinks.GetHead();
	while (pLinkedBouy != NULL)
	{
		u16Data = pLinkedBouy->GetInstanceID();
		pFile->Write(&u16Data);
		pLinkedBouy = m_aplDirectLinks.GetNext();
	}

	// Save the instance ID for the parent NavNet so it can be connected
	// again after load
	u16Data = m_pParentNavNet->GetInstanceID();
	pFile->Write(&u16Data);

	if (pFile->Error())
		return FAILURE;
	else
		return SUCCESS;
}


////////////////////////////////////////////////////////////////////////////////
// Startup object
////////////////////////////////////////////////////////////////////////////////
short CBouy::Startup(void)								// Returns 0 if successfull, non-zero otherwise
{
	short sResult = 0;

	// At this point we can assume the CHood was loaded, so we init our height
	m_dY = m_pRealm->GetHeight((short) m_dX, (short) m_dZ);

	// Init other stuff
	// Get pointer to Navigation Net
	// If we don't have a pointer to the nav net yet, get it from the ID
	if (m_pParentNavNet == NULL)
	{
		m_pRealm->m_idbank.GetThingByID((CThing**) &m_pParentNavNet, m_u16ParentInstanceID); 
		// Re-register yourself with the network.
		m_pParentNavNet->AddBouy(this);
	

		linkinstanceid::Pointer i;
		CBouy* pBouy = NULL;
		for (i = m_LinkInstanceID.GetHead(); i != 0; i = m_LinkInstanceID.GetNext(i))
		{
			m_pRealm->m_idbank.GetThingByID((CThing**) &pBouy, m_LinkInstanceID.GetData(i));		
			// If its not already linked, then add it.
			if (!m_aplDirectLinks.Find(pBouy))
			{
				m_aplDirectLinks.AddTail(pBouy);
				m_sNumDirectLinks++;
			}
		}

		// Only in edit mode . . .
		if (m_pRealm->m_flags.bEditing == true)
		{
			sResult = GetResources();
		}
	}

	return sResult;
}


////////////////////////////////////////////////////////////////////////////////
// Shutdown object
////////////////////////////////////////////////////////////////////////////////
short CBouy::Shutdown(void)							// Returns 0 if successfull, non-zero otherwise
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Suspend object
////////////////////////////////////////////////////////////////////////////////
void CBouy::Suspend(void)
{
	m_sSuspend++;
}


////////////////////////////////////////////////////////////////////////////////
// Resume object
////////////////////////////////////////////////////////////////////////////////
void CBouy::Resume(void)
{
	m_sSuspend--;

	// If we're actually going to start updating again, we need to reset
	// the time so as to ignore any time that passed while we were suspended.
	// This method is far from precise, but I'm hoping it's good enough.
}


////////////////////////////////////////////////////////////////////////////////
// Update object
////////////////////////////////////////////////////////////////////////////////
void CBouy::Update(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// AddLink - Add a 1 hop direct link to the routing table
////////////////////////////////////////////////////////////////////////////////

short CBouy::AddLink(CBouy* pBouy)
{
	short sReturn = SUCCESS;
	
	if (!m_aplDirectLinks.Find(pBouy))
	{
		m_aplDirectLinks.AddTail(pBouy);
		m_sNumDirectLinks++;
	}
	
	return sReturn;	
}

////////////////////////////////////////////////////////////////////////////////
// Render object
////////////////////////////////////////////////////////////////////////////////
void CBouy::Render(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to init new object at specified position
////////////////////////////////////////////////////////////////////////////////
short CBouy::EditNew(									// Returns 0 if successfull, non-zero otherwise
	short sX,												// In:  New x coord
	short sY,												// In:  New y coord
	short sZ)												// In:  New z coord
{
	short sResult = 0;
	
	// Use specified position
	m_dX = (double)sX;
	m_dY = (double)sY;
	m_dZ = (double)sZ;

	// Since we were created in the editor, set our Nav Net
	m_pParentNavNet = m_pRealm->GetCurrentNavNet();
	if (m_pParentNavNet->AddBouy(this) == 0)
		sResult = FAILURE;
	else
		// Load resources
		sResult = GetResources();

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to modify object
////////////////////////////////////////////////////////////////////////////////
short CBouy::EditModify(void)
{
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to move object to specified position
////////////////////////////////////////////////////////////////////////////////
short CBouy::EditMove(									// Returns 0 if successfull, non-zero otherwise
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
void CBouy::EditUpdate(void)
{
}


////////////////////////////////////////////////////////////////////////////////
// Called by editor to render object
////////////////////////////////////////////////////////////////////////////////
void CBouy::EditRender(void)
{
	// No special flags
	if (ms_bShowBouys)
	{
		if (m_pParentNavNet == m_pRealm->GetCurrentNavNet())
		{
			m_sprite.m_sInFlags = 0;
			m_phot->SetActive(TRUE);
		}
		else
		{
			m_sprite.m_sInFlags = CSprite::InHidden;
			m_phot->SetActive(FALSE);
		}
	}
	else
		m_sprite.m_sInFlags = CSprite::InHidden;

	// Map from 3d to 2d coords
	Map3Dto2D(
		(short) m_dX, 
		(short) m_dY, 
		(short) m_dZ, 
		&m_sprite.m_sX2, 
		&m_sprite.m_sY2);

	// Center on image.
	m_sprite.m_sX2	-= m_pImage->m_sWidth / 2;
	m_sprite.m_sY2	-= m_pImage->m_sHeight;

	// Priority is based on bottom edge of sprite
	m_sprite.m_sPriority = m_dZ;

	// Layer should be based on info we get from attribute map.
	m_sprite.m_sLayer = CRealm::GetLayerViaAttrib(m_pRealm->GetLayer((short) m_dX, (short) m_dZ));

	// Image would normally animate, but doesn't for now
	m_sprite.m_pImage = m_pImage;

	// Update sprite in scene
	m_pRealm->m_scene.UpdateSprite(&m_sprite);
}

////////////////////////////////////////////////////////////////////////////////
// Give Edit a rectangle around this object
////////////////////////////////////////////////////////////////////////////////
void CBouy::EditRect(RRect* pRect)
{
	Map3Dto2D(
		m_dX,
		m_dY,
		m_dZ,
		&(pRect->sX),
		&(pRect->sY) );

	pRect->sW	= 10;	// Safety.
	pRect->sH	= 10;	// Safety.

	if (m_pImage != NULL)
		{
		pRect->sW	= m_pImage->m_sWidth;
		pRect->sH	= m_pImage->m_sHeight;
		}

	pRect->sX	-= pRect->sW / 2;
	pRect->sY	-= pRect->sH;
}

////////////////////////////////////////////////////////////////////////////////
// Called by editor to get the hotspot of an object in 2D.
////////////////////////////////////////////////////////////////////////////////
void CBouy::EditHotSpot(	// Returns nothiing.
	short*	psX,				// Out: X coord of 2D hotspot relative to
									// EditRect() pos.
	short*	psY)				// Out: Y coord of 2D hotspot relative to
									// EditRect() pos.
	{
	// Base of bouy is hotspot.
	*psX	= (m_pImage->m_sWidth / 2);
	*psY	= m_pImage->m_sHeight;
	}

////////////////////////////////////////////////////////////////////////////////
// Get all required resources
////////////////////////////////////////////////////////////////////////////////
short CBouy::GetResources(void)						// Returns 0 if successfull, non-zero otherwise
	{
	short sResult = 0;
	
	if (m_pImage == 0)
		{
		RImage*	pimBouyRes;
		sResult = rspGetResource(&g_resmgrGame, m_pRealm->Make2dResPath(IMAGE_FILE), &pimBouyRes);
		if (sResult == 0)
			{
			// Allocate image . . .
			m_pImage	= new RImage;
			if (m_pImage != NULL)
				{
				// Allocate image data . . .
				if (m_pImage->CreateImage(
					pimBouyRes->m_sWidth,
					pimBouyRes->m_sHeight,
					RImage::BMP8) == 0)
					{
					// Blt bouy res.
					rspBlit(
						pimBouyRes,		// Src.
						m_pImage,		// Dst.
						0,					// Dst.
						0,					// Dst.
						NULL);			// Dst clip.

					// Put in ID.
					RPrint	print;
					print.SetFont(BOUY_ID_FONT_HEIGHT, &g_fontBig);
					print.SetColor(BOUY_ID_FONT_COLOR);
					print.SetJustifyCenter();
					print.print(
						m_pImage,												// Dst.
						0,															// Dst.
						m_pImage->m_sHeight - BOUY_ID_FONT_HEIGHT,	// Dst.
						"%d",														// Format.
						(short)m_ucID);										// Src.
																					
					// Convert to efficient transparent blit format . . .
					if (m_pImage->Convert(RImage::FSPR8) != RImage::FSPR8)
						{
						sResult = -3;
						TRACE("CBouy::GetResource() - Couldn't convert to FSPR8\n");
						}
					}
				else
					{
					sResult	= -2;
					TRACE("CBouy::GetResource() - m_pImage->CreateImage() failed.\n");
					}

				// If an error occurred after allocation . . .
				if (sResult != 0)
					{
					delete m_pImage;
					m_pImage	= NULL;
					}
				}
			else
				{
				sResult	= -1;
				TRACE("CBouy::GetResource(): Failed to allocate RImage.\n");
				}
			
			rspReleaseResource(&g_resmgrGame, &pimBouyRes);
			}
		}
	
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// Free all resources
////////////////////////////////////////////////////////////////////////////////
short CBouy::FreeResources(void)						// Returns 0 if successfull, non-zero otherwise
{
	if (m_pImage != NULL)
		{
		delete m_pImage;
		m_pImage	= NULL;
		}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Unlink - Visit your direct links and unlink yourself from them, then free
//				your own links.
////////////////////////////////////////////////////////////////////////////////

void CBouy::Unlink(void)
{
	// Follow all of this bouy's direct links and unlink this bouy
	CBouy* pLinkedBouy = m_aplDirectLinks.GetHead();
	while (pLinkedBouy)
	{
		pLinkedBouy->m_aplDirectLinks.Remove(this);
		pLinkedBouy->m_sNumDirectLinks--;
		pLinkedBouy = m_aplDirectLinks.GetNext();
	}

	// Erase all of your own links to other bouys
	m_aplDirectLinks.Reset();
	m_sNumDirectLinks = 0;

	// Remove this bouy from the network
	m_pParentNavNet->RemoveBouy(m_ucID);
}


////////////////////////////////////////////////////////////////////////////////
// BuildRoutingTable - Fills in the routing table by building a BSF tree and
//							  using the hop counts and parent tree, fills in the
//							  routing table.
////////////////////////////////////////////////////////////////////////////////

short CBouy::BuildRoutingTable(void)
{
	short sResult = SUCCESS;
	UCHAR* aVisited = NULL;
	UCHAR* aDistance = NULL;
	UCHAR* aParent = NULL;
	UCHAR* pucCurrentNode = NULL;
	UCHAR* pucAdjNode = NULL;
	CBouy* pTraverseBouy = NULL;

	ASSERT(m_pParentNavNet != NULL);
	short sCurrentNumNodes = m_pParentNavNet->GetNumNodes();
	RQueue <UCHAR, 256> bfsQueue;

	// Make sure there is enough space in the routing table, or
	// reallocate it if there isn't enough.
	if (m_sRouteTableSize < sCurrentNumNodes)
	{
		if (m_paucRouteTable != NULL)
			free(m_paucRouteTable);
		m_paucRouteTable = (UCHAR*) malloc(sCurrentNumNodes);
		m_sRouteTableSize = sCurrentNumNodes;
	}

	// Allocate memory for use in building the BSF tree
	aVisited = (UCHAR*) malloc(sCurrentNumNodes);
	aDistance = (UCHAR*) malloc(sCurrentNumNodes);
	aParent = (UCHAR*) malloc(sCurrentNumNodes);

	if (m_paucRouteTable != NULL &&
	    aVisited != NULL &&
		 aDistance != NULL &&
		 aParent != NULL)
	{
		// Initialize the table to unreachable and initialize the
		// BSF data structures.
		memset(m_paucRouteTable, 255, m_sRouteTableSize);
		memset(aVisited, 0, sCurrentNumNodes);
		memset(aDistance, 255, sCurrentNumNodes);
		memset(aParent, 0, sCurrentNumNodes);

		// Breadth-First Search
		aVisited[m_ucID] = TRUE;
		aDistance[m_ucID] = 0;
		bfsQueue.EnQ(&m_ucID);

		while (!bfsQueue.IsEmpty())
		{
			pucCurrentNode = bfsQueue.DeQ();
			pTraverseBouy = m_pParentNavNet->GetBouy(*pucCurrentNode);

			CBouy* pLinkedBouy = pTraverseBouy->m_aplDirectLinks.GetHead();
			while (pLinkedBouy)
			{
				pucAdjNode = &(pLinkedBouy->m_ucID);
				if (aVisited[*pucAdjNode] == FALSE)
				{
					aVisited[*pucAdjNode] = TRUE;
					aParent[*pucAdjNode] = *pucCurrentNode;
					aDistance[*pucAdjNode] = aDistance[*pucCurrentNode] + 1;
					bfsQueue.EnQ(pucAdjNode);
				}
				pLinkedBouy = pTraverseBouy->m_aplDirectLinks.GetNext();
			}
		}

		// Breadth-First Search complete.

		// Now the aDistance contains the hop count to all other connected nodes
		// and aParent provides a way to build the routing table by traversing
		// backwards.

		UCHAR ucCurrentDistance;
		UCHAR curr;
		short j;

		for (j = 1; j < sCurrentNumNodes; j++)
		{
			ucCurrentDistance = aDistance[j];
			// If distance is 255 (infinite flag) then mark that node
			// as unreachable in the routing table using 255 as the flag
			if (ucCurrentDistance == 255)
			{
				m_paucRouteTable[j] = 255;
			}
			else
			{
				// If distance is 0, then this is the node, and mark it in the
				// routing table as 0, meaning you have reached your final destination
				if (j == m_ucID)
				{
					m_paucRouteTable[j] = 0;
				}
				else
				{
					// If the distance is 1, it is a directly connected node, so put
					// its ID in the routing table as the node to go to.
					if (ucCurrentDistance == 1)
					{
						m_paucRouteTable[j] = j;
					}
					else
					{
						// Else, its a child of a directly connected node, so find out which
						// one by tracing back along the parent tree.
						curr = j;

						while (aParent[curr] != m_ucID)
							curr = aParent[curr];

						m_paucRouteTable[j] = curr;
					}
				}
			}
		}
	}
	else
	{
		TRACE("CBouy::BuildRoutingTable: Error allocating memory for tables for bouy %d\n", m_ucID);
		sResult = -1;
	}

	if (aVisited)
		free(aVisited);
	if (aDistance)
		free(aDistance);
	if (aParent)
		free(aParent);

	return sResult;
}

////////////////////////////////////////////////////////////////////////////////
// NextRouteNode - Tells you which node to go to next to get to your destination
////////////////////////////////////////////////////////////////////////////////

UCHAR CBouy::NextRouteNode(UCHAR dst)
{
	if (dst >= m_pParentNavNet->GetNumNodes())
		return 255;
	else
		return m_paucRouteTable[dst];	
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
