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
// credits.cpp
// Project: Postal
//
// This module deals with displaying the credits.
//
// History:
//		12/05/96 MJR	Started.
//
//		12/19/96	JMI	Scope error in Credits().  Loop was waiting for terminators
//							one of which was lKey.  The lKey was being set via a call
//							to rspGetKey(&lKey).  But there were two lKeys.  One inside
//							and one outside the loop.  The one on the outside was 
//							evaluated and the one on the inside was being set to the
//							current key.
//
//		02/17/97	JMI	Making this obey rspGetQuitStatus().
//
//		07/05/97 MJR	Changed to RSP_BLACK_INDEX instead of 0.
//
//		07/16/97 BRH	Changed the credits screen to the "What's in the
//							release version of postal preview" screen for the
//							demo.  Took out blah blah blah, yada yada yada.
//
//		08/11/97	JRD	Transforming this module into a device for scolling text
//							which is intended to be used both by credits and by story.
//							It wil operate similar to cutscene in that all of it's
//							assets and memory usage is assumed temporary.  It will still
//							use the shell sak for assets.
//
//		08/11/97	JRD	Created a class for the purpose of accessing generic files
//							from within a sak directory.  Used to read in credits metafile.
//
//		08/20/97 BRH	Added music to credits. Unfortunately, I hardwired the music in
//							the function ScrollPage instead of Credits(), so the mustic will
//							play in any part of the game where text is scrolled.  Man, was I
//							"stupid" when I did that.
//
//		08/21/97	JMI	Changed call to Update() to UpdateSystem() and occurrences
//							of rspUpdateDisplay() to UpdateDisplay().
//
//		08/22/97	JRD	Attempted to make the credits locking safe.
//
//		08/22/97	JMI	Changed calls to UpdateDisplay() back to rspUpdateDisplay()
//							since we no longer need UpdateDisplay() now that we are
//							using rspLock/Unlock* functions properly.
//
//		08/26/97	JRD	Moved the credit music into the credits function from the 
//							text scrolling function.
//
//		08/27/97	JRD	Moved end sound out of Scroll Page.  Made Scroll page not clear screen
//							at end.  Added special credit ending.
//
//		08/27/97 JRD	Removed text dependency on windows colors.  Made it so if credits
//							are aborted, ending is not shown.
//
//		08/28/97	JRD	Altered Credits() to allow varying the resources so 
//							different music could play in the final scene than
//							in the normal credits.
//
//		09/08/97 JRD	Had Bill add rspGetQuitStaus() to the list of keys and
//							mouse buttons that abort the credits.  Boy, did I look
//							"stupid" when someone hit Alt-F4 during the ending
//							sequence only to get stuck on the credits.  Thanks for
//							fixing that Bill, I owe you another trip to Las Vegas.
//
//		09/17/97 BRH	Added in a conditional compile optionfor the credits so
//							the shareware version will show the coming soon screen
//							instead of the credits.
//
//		06/24/01 MJR	Changed from obsolete macro to SHOW_EXIT_SCREEN as a way
//							to control what gets displayed when the player exits from
//							the game.  Also renamed macro to EXIT_BG and changed the
//							filename to "exit.bmp".
//
////////////////////////////////////////////////////////////////////////////////
// SCROLL PAGE HACK:  CURRENTLY USES COLOR 244 = {128,0,0}, 245 = {255,0,0} 

#include "RSPiX.h"
#include "credits.h"
#include "game.h"
#include "update.h"
#include "SampleMaster.h"
#include "CompileOptions.h"
#include "ORANGE/Parse/SimpleBatch.h"
#include "GREEN/Mix/MixBuf.h"

// try to hook the sound scaling tables for the palette

////////////////////////////////////////////////////////////////////////////////
// Macros/types/etc.
////////////////////////////////////////////////////////////////////////////////

#define EXIT_BG "credits/exit.bmp"

// default credits parameters... (serve as emergency backups...)
char	g_szBackground[256] = "credits/pile640.bmp";
char	g_szCredits[256] = "credits/credits.txt";
SampleMasterID*	g_psmidMusic = &g_smidCreditsMusak;

#define BG_X			(g_pimScreenBuf->m_sWidth / 2 - pimBackground->m_sWidth / 2)
#define BG_Y			(g_pimScreenBuf->m_sHeight / 2 - pimBackground->m_sHeight / 2)

#define CREDIT_TIME	30000

#define LEFT_X			200
#define SPACE_Y		30

#define MUSAK_START_TIME	0
#define MUSAK_END_TIME		0

#define	MIN_SCOLL_FRAME_MILLI	14		// Cap at 40 fps


extern int wideScreenWidth;


////////////////////////////////////////////////////////////////////////////////
// Variables/data
////////////////////////////////////////////////////////////////////////////////

static SampleMaster::SoundInstance	ms_siMusak;

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
////////////////////////////////////////////////////////////////////////////////
#define MAX_BACKGROUNDS 10

class	CBackgroundChange
	{
public:
	RImage*	m_pimPrevBackground;
	RImage*	m_pimNewBackground;
	short	m_sImageCacheIndex;
	char	m_szNewName[64];			// resource name
	bool	m_bActive;					
	// All times in milliseconds...
	long	m_lActivationTime;		// also delay time
	long	m_lFadeOutTime;			//	also in delta time form
	long	m_lBlackTime;				//	also in delta time form
	long	m_lFadeInTime;				//	also in delta time form
	UCHAR	m_TransitionPalette[1024];
	typedef enum { Inactive,FadeOut,Black,FadeIn } BackState;
	BackState	m_eState;
	typedef enum { OnEnter,OnExit } ActivationType;
	ActivationType	m_eType;
		//--------------------------------------------------------
	void Clear()
		{
		m_pimPrevBackground = m_pimNewBackground = NULL;
		m_szNewName[0] = 0;
		m_bActive = false;
		m_lActivationTime = m_lFadeOutTime = m_lBlackTime = m_lFadeInTime = 0;
		m_eState = Inactive;
		m_sImageCacheIndex = -1;
		m_eType = OnEnter;
		}

	CBackgroundChange()
		{
		Clear();
		}

	~CBackgroundChange()
		{
		Clear();
		}
	//--------------------------------------------------------
	void	Activate(RImage* pimCurBack,RImage* pBackgrounds[])
		{
		// set current times as delta from current:
		m_lActivationTime += rspGetMilliseconds();
		m_lFadeOutTime += m_lActivationTime;
		m_lBlackTime += m_lFadeOutTime;
		m_lFadeInTime += m_lBlackTime;

		m_pimPrevBackground = pimCurBack;
		m_pimNewBackground = pBackgrounds[m_sImageCacheIndex];
		if (!m_pimNewBackground) m_pimNewBackground = pimCurBack;

		m_eState = FadeOut;
		}
	};

class	CTextPhrase
	{
public:

	short	m_sColorIndex;
	short	m_sFontSize;
	short	m_sLocalX;
	short m_sLocalY;
	typedef	enum	{Left, Right, Center} Justify;
	Justify	m_eJust;
	char	m_szText[128];
	CTextPhrase*	m_pNext;
	CTextPhrase*	m_pPrev;
	//------------------------------
	CTextPhrase()
		{
		m_pNext = m_pPrev = NULL;
		m_sFontSize = 0;	// means use previous font size
		}

	~CTextPhrase()
		{
		m_pNext = m_pPrev = NULL;;
		}
	};

// Will actually create an image of itself
//
class	CTextChunk
	{
public:
	long	m_lNumPhrases;
	CTextPhrase m_tHead; // bookends
	CTextPhrase m_tTail;
	short	m_sGlobalTopY;
	short	m_sGlobalBottomY;
	RImage* m_pimCache;
	CTextChunk*	m_pNext;
	CTextChunk* m_pPrev;
	CBackgroundChange*	m_pChangeBackground;
	//--------------------------
	CTextChunk()
		{
		m_lNumPhrases = 0;
		m_tHead.m_pNext = &m_tTail;
		m_tTail.m_pPrev = &m_tHead;
		m_sGlobalBottomY = -1;
		m_pimCache = NULL;
		m_pChangeBackground = NULL;
		}

	~CTextChunk()
		{
		CTextPhrase* pCur = m_tHead.m_pNext;
		while (pCur != &m_tTail)
			{
			CTextPhrase* pNext = pCur->m_pNext;
			delete pCur;
			pCur = pNext;
			}

		if (m_pimCache) 
			{
			delete m_pimCache;
			m_pimCache = NULL;
			}
		}
	//--------------------------
	// Must insert at tail to maintain order
	void	AddPhrase(CTextPhrase* pNew)
		{
		m_lNumPhrases++;
		pNew->m_pNext = &m_tTail;
		pNew->m_pPrev = m_tTail.m_pPrev;
		m_tTail.m_pPrev->m_pNext = pNew;
		m_tTail.m_pPrev = pNew;
		}

	// WILL NOT DELETE!
	void	RemovePhrase(CTextPhrase* pGone)
		{
		m_lNumPhrases--;
		pGone->m_pPrev->m_pNext = pGone->m_pNext;
		pGone->m_pNext->m_pPrev = pGone->m_pPrev;

		pGone->m_pNext = pGone->m_pPrev = NULL;
		}
	//------------------------------------
	void	RenderChunk(short sW,RPrint* pPrint)
		{
		if (m_pimCache)
			{
			//TRACE("Already rendered\n");// legal!
			return;
			}

		m_pimCache = new RImage;
		m_pimCache->CreateImage(sW,m_sGlobalBottomY - m_sGlobalTopY+1,RImage::BMP8);

		CTextPhrase* pCur = m_tHead.m_pNext;

		pPrint->SetDestination(m_pimCache);
		while (pCur != &m_tTail)
			{
			//---------------------------------
			pPrint->SetColor(pCur->m_sColorIndex);
			
			// HARD CODED shadow colors (postal specific:)
			if (pCur->m_sColorIndex == 245) 	pPrint->SetColor(pCur->m_sColorIndex, 0 , 244);
			else	pPrint->SetColor(pCur->m_sColorIndex, 0 , 243); // until we have comand support
			
			if (pCur->m_sFontSize) pPrint->SetFont(pCur->m_sFontSize);

			short sTabX = pCur->m_sLocalX;
			if (sTabX < 0) sTabX += sW;	// from the right...

			// each justification needed to select the RPrint cell to use:
			// RPrint doesn't seem to be justifying correctly...
			// I will attempt some of my own:
			int wideScreenOffset = (wideScreenWidth - 640);
			switch (pCur->m_eJust)
				{	
				short sRadius;

				//--------------------------- Left Justify
				case CTextPhrase::Left: 
					pPrint->SetJustifyLeft(); 
					pPrint->SetColumn(0 ,0,sW,m_pimCache->m_sHeight);
					pPrint->print(sTabX + wideScreenOffset/2,pCur->m_sLocalY,"%s",pCur->m_szText);
				break;
				//--------------------------- Right Justify
				case CTextPhrase::Right: 
					pPrint->SetJustifyRight(); 
					pPrint->SetColumn(0,0,sTabX + 1 + wideScreenOffset/2,m_pimCache->m_sHeight);
					// trick print into thinking it's a new line!
					pPrint->print(-1,pCur->m_sLocalY,"%s",pCur->m_szText);
				break;
				//--------------------------- Center Justify (about tab)
				case CTextPhrase::Center: 
					pPrint->SetJustifyCenter(); 
					sRadius = MIN(sTabX,short(sW - sTabX));
					pPrint->SetColumn(sTabX - sRadius + wideScreenOffset,0,
						sRadius<<1, m_pimCache->m_sHeight);
					// trick print into thinking it's a new line!
					pPrint->print(-1,pCur->m_sLocalY,"%s",pCur->m_szText);
				break;
				}

			
			// undo justification:
			pPrint->SetColumn(0,0,sW,m_pimCache->m_sHeight);
			//---------------------------------
			pCur = pCur->m_pNext;
			}

		// Now compress it for speed!
		m_pimCache->Convert(RImage::FSPR8); // for testing.
		}

	void	FreeChunk()
		{
		// Keep 'em for now...
		if (m_pimCache) delete m_pimCache;
		m_pimCache = NULL;
		}

	};

char*	pct = g_szCredits;
extern void SetAll();

// The highest level:
class CScrollMaster
	{
public:
	long			m_lGlobalHeight;
	long			m_lCurrentTopY;
	long			m_lCurrentBottomY;
	long			m_lTotalChunks;
	CTextChunk*	m_pTopActiveChunk;
	CTextChunk*	m_pBottomActiveChunk;
	long			m_lActivationTime;
	RRect			m_rDisplay;
	double		m_dScrollRate;	// screens/sec

	CTextChunk	m_cHead;	// bookends:
	CTextChunk	m_cTail;

	short	m_sNumBackgrounds;
	char	m_szBackgroundNames[MAX_BACKGROUNDS][64];
	RImage*	m_pimBackgrounds[MAX_BACKGROUNDS];

	CBackgroundChange* m_pCurSceneChange;
	//--------------------------------------
	CScrollMaster()
		{
		m_lGlobalHeight = 0;
		m_lCurrentBottomY = 0;
		m_lTotalChunks = 0;
		m_pTopActiveChunk = NULL;
		m_pBottomActiveChunk = NULL;
		m_lActivationTime = 0;
		m_cHead.m_pNext = &m_cTail;
		m_cTail.m_pPrev = &m_cHead;
		m_rDisplay = RRect(0,40,wideScreenWidth,360);
		m_dScrollRate = 0.1;	// 100 seconds per screen
		m_sNumBackgrounds = 0;
		m_pCurSceneChange = NULL;
		}

	~CScrollMaster()
		{
		CTextChunk* pCur = m_cHead.m_pNext;
		while (pCur != &m_cTail)
			{
			CTextChunk* pNext = pCur->m_pNext;
			delete pCur;
			pCur = pNext;
			}

		// release all of the backgrounds:
		short i;

		for (i = 0; i < m_sNumBackgrounds;i++)
			{
			if (m_pimBackgrounds[i]) g_resmgrShell.Release(m_pimBackgrounds[i]);
			m_pimBackgrounds[i] = NULL;
			}
		}

	void	Configure(double dScrollRate,RRect* prWindow = NULL)
		{
		if (dScrollRate > 0.0) m_dScrollRate = dScrollRate;
		if (prWindow) m_rDisplay = *prWindow;
		}

	//--------------------------------
	void	AddChunk(CTextChunk* pNew)
		{
		m_lTotalChunks++;
		pNew->m_pNext = &m_cTail;
		pNew->m_pPrev = m_cTail.m_pPrev;
		m_cTail.m_pPrev->m_pNext = pNew;
		m_cTail.m_pPrev = pNew;

		m_lGlobalHeight = pNew->m_sGlobalBottomY + 1;
		}
	//--------------------------------
	void	Start(RPrint* pPrint)
		{
		// Try to load all the extra bmps from memory:
		short i;
		for (i=0; i < m_sNumBackgrounds;i++)
			{
			if (rspGetResource(&g_resmgrShell,m_szBackgroundNames[i],&m_pimBackgrounds[i])
				!= SUCCESS)
				{
				TRACE("Couldn't load resource %s\n",m_szBackgroundNames[i]);
				m_pimBackgrounds[i] = NULL;
				}
			}

		// Activate first chunk:
		ASSERT(m_lTotalChunks >= 1);

		m_pTopActiveChunk = m_pBottomActiveChunk = m_cHead.m_pNext;
		m_pBottomActiveChunk->RenderChunk(m_rDisplay.sW,pPrint);
		m_pBottomActiveChunk = m_pBottomActiveChunk->m_pNext; // chunk 2

		// ATTEMPT TO LOAD ALL THE BACKGROUND RESOURCES FOR SCENE CHANGES!

		m_lActivationTime = rspGetMilliseconds();
		}

	// will return FAILURE if scroll is over!
	short Update(RPrint* pPrint)
		{
		// calculate current global scroll location:
		m_lCurrentBottomY = long(double(rspGetMilliseconds() - m_lActivationTime)
			* m_dScrollRate * m_rDisplay.sH / 1000.0);
		m_lCurrentTopY = (m_lCurrentBottomY - m_rDisplay.sH);

		if (m_lCurrentTopY > m_lGlobalHeight) return FAILURE; // scroll over!

		// see if we need to activate a new chunk:
		if (m_pBottomActiveChunk != &m_cTail)
			{
			if (m_pBottomActiveChunk->m_sGlobalTopY <= m_lCurrentBottomY)
				{
				m_pBottomActiveChunk->RenderChunk(m_rDisplay.sW,pPrint);
				m_pBottomActiveChunk = m_pBottomActiveChunk->m_pNext;

				// CHECK for scene change
				}
			}

		// see if we need to deactivate a new chunk:
		if (m_pTopActiveChunk != &m_cTail)
			{
			if (m_pTopActiveChunk->m_sGlobalBottomY <= m_lCurrentTopY)
				{
				m_pTopActiveChunk->FreeChunk();
				m_pTopActiveChunk = m_pTopActiveChunk->m_pNext;

				// CHECK for exit scene change
				}
			}

		return SUCCESS;
		}

	// will overlay the text onto your bitmap with the designated clip rectangle:
	// NOTE: script coordinates are relative to upper left clipping corner...
	void	Render(RImage* pimDst)
		{
		// Draw each chunk separately:
		CTextChunk* pCur;

		// all active chunks should be pre-rendered:
		for (pCur = m_pTopActiveChunk; pCur != m_pBottomActiveChunk->m_pNext;
				pCur = pCur->m_pNext)
			{
			if (pCur != &m_cTail)
				{
				// Draw the pre-rendered chunk...
				if (pCur->m_pimCache)
					{
					rspBlit(pCur->m_pimCache,pimDst,m_rDisplay.sX,
						pCur->m_sGlobalTopY - m_lCurrentTopY
						+ m_rDisplay.sY,&m_rDisplay);
					}
				}
			}
		}
	//-------------------------------------------------------------
	// returns the index number for the resource bmp:
	short	AddBackground(char* pszName)
		{
		ASSERT(pszName);
		ASSERT(m_sNumBackgrounds < MAX_BACKGROUNDS);

		strcpy(m_szBackgroundNames[m_sNumBackgrounds],pszName);
		m_pimBackgrounds[m_sNumBackgrounds] = NULL;

		return m_sNumBackgrounds++;
		}
	};

extern short sLoaded;

////////////////////////////////////////////////////////////////////////////////
// This is cheesy, but right now I'm using a global stream to load into.
////////////////////////////////////////////////////////////////////////////////
//CScrollMaster*	gpCurStream = NULL;

// For Res managing an ANSI file:
class	CFileTextInput
	{
public:

	RBatch	m_bf;
	//-----------------------
	CFileTextInput() 
		{
		m_pStream = NULL;
		};

	~CFileTextInput() 
		{
		m_bf.clear();	// don't let it close the file!
		if (m_pStream) delete m_pStream;
		};
	//-----------------------
	short	ParseTextInput(FILE* fp);
	//-----------------------
	short	Load(RFile* pFile) // so res manager can hook it!
		{
		FILE* fp = pFile->m_fs;
		//--------------------- do my load:

		ParseTextInput(fp);

		return SUCCESS;
		}

	short	Load(FILE* fp) // so res manager can hook it!
		{
		//--------------------- do my load:

		ParseTextInput(fp);

		return SUCCESS;
		}

	CScrollMaster* m_pStream;
	};

////////////////////////////////////////////////////////////////////////////////
//	Tokenizing the script file into absolute strips:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  ASSUME A VALID FILE STREAM
//
//	 This will parse and tokenize the incoming text...
//  It will not actually render it.
//
////////////////////////////////////////////////////////////////////////////////
short	CFileTextInput::ParseTextInput(FILE* fp)
	{
	if (m_pStream)
		{
		TRACE("CFileTextInput::ParseTextInput: Stream in progress.\n");
		return FAILURE;
		}
	else
		m_pStream = new CScrollMaster;	// needs to be freed!

	// We need to be a little low level so we can force feed a 
	// file pointer to it.
	m_bf.clear();
	m_bf.m_fp = fp;
	m_bf.configure(" \t,;=({[",";/)}]",'`','/');
	char* pszToken;
	//gpCurStream = new CTextStream;

	short sGlobalY = 0;
	short sLocalY = 0;
	short sCurStripTop = 0;
	short	sMaxH = 0;
	bool	bNewChunk = true;
 
	short sCurFontSize = 0;
	short	sCurTabX = 0;
	CTextPhrase::Justify eCurJust = CTextPhrase::Left;
	short sCurColor = 255;

	CTextChunk*	pChunk = new CTextChunk;
	CTextPhrase* pCurPhrase = new CTextPhrase;

	pChunk->m_sGlobalTopY = sCurStripTop;
	pChunk->m_sGlobalBottomY = sCurStripTop;

	if (sLoaded) SetAll();

	while ((pszToken = m_bf.NextToken()) != NULL)
		{
		//TRACE("TOKEN = '%s'\n",pszToken);

		//-----------------------------------------------------
		if (!strcmp(pszToken,"color"))
			{
			sCurColor = atoi(m_bf.NextToken());

			continue;
			}
		//-----------------------------------------------------
		if (!strcmp(pszToken,"size"))
			{
			pCurPhrase->m_sFontSize = 0;

			short sNewSize = atoi(m_bf.NextToken());

			// If we are starting a new chunk, we MUST give a font
			// size!

			if (bNewChunk || (sCurFontSize != sNewSize))
				{
				pCurPhrase->m_sFontSize = sNewSize;
				}

			sCurFontSize = sNewSize;
			bNewChunk = false;

			sMaxH = MAX(sMaxH,short(sLocalY + sNewSize));

			continue;
			}
		//-----------------------------------------------------
		if (!strcmp(pszToken,"back"))
			{
			sLocalY -= atoi(m_bf.NextToken());

			if (sLocalY < 0)
				{
				TRACE("ERROR - you backed up above the strip!\n");
				sLocalY = 0;
				}
			continue;
			}
		//-----------------------------------------------------
		if (!strcmp(pszToken+1,"tab"))
			{
			pCurPhrase->m_sLocalX = sCurTabX = atoi(m_bf.NextToken());

			switch	(*pszToken)
				{
				case 'l': eCurJust = pCurPhrase->m_eJust = CTextPhrase::Left; break;
				case 'r': eCurJust = pCurPhrase->m_eJust = CTextPhrase::Right; break;
				case 'c': eCurJust = pCurPhrase->m_eJust = CTextPhrase::Center; break;
				}
			continue;
			}
		//-----------------------------------------------------
		if (!strcmp(pszToken,";")) // means new line (& should benew phrase)
			{
			sLocalY += sCurFontSize;
			sMaxH = MAX(sMaxH,short(sLocalY + sCurFontSize));

			continue;
			}
		//-----------------------------------------------------
		if (!strcmp(pszToken,"end")) //  if in the sak, no EOF!
			{

			break; // done
			}
		//-----------------------------------------------------
		if (!strcmp(pszToken,"skip")) // signals a new chunk!
			{
			//TRACE("New chunk!\n");
			bNewChunk = true;

			sMaxH = MAX(sMaxH,short(sLocalY + sCurFontSize));

			// Figure out how big chunk was:
			sGlobalY += MAX(sMaxH,sLocalY);
			pChunk->m_sGlobalBottomY = sGlobalY - 1;

			if (pChunk->m_sGlobalBottomY > pChunk->m_sGlobalTopY)
				{
				m_pStream->AddChunk(pChunk);
				}

			// create a new chunk
			short sVal = atoi(m_bf.NextToken());
			sGlobalY += sVal;
			pChunk = new CTextChunk;
			pChunk->m_sGlobalTopY = pChunk->m_sGlobalBottomY = sGlobalY;
			sMaxH = 0;	// resetting stuff like this!
			sLocalY = 0;

			continue;
			}
		//-----------------------------------------------------
		if (!strcmp(pszToken,"scene")) // describe a scene change:
			{
			// first, parse through it all, then validate the information:

			// Good standards: (in milliseconds)
			short sDelay = 0,sFadeInTime = 1000,sFadeOutTime = 1000;
			short sBlackTime = 0, sOnExit = FALSE;
			char	szName[64] = {0,};

			// The order doesn't matter - go until ')'
			pszToken = m_bf.NextToken();

			while (strcmp(pszToken,")")) // until we hit a right parenthesis
				{
				if (!strcmp(pszToken,"delay"))
					{
					sDelay = atoi(m_bf.NextToken());
					}
				else if (!strcmp(pszToken,"black"))
					{
					sBlackTime = atoi(m_bf.NextToken());
					}
				else if (!strcmp(pszToken,"out"))
					{
					sFadeOutTime = atoi(m_bf.NextToken());
					}
				else if (!strcmp(pszToken,"in"))
					{
					sFadeInTime = atoi(m_bf.NextToken());
					}
				else if (!strcmp(pszToken,"exit"))
					{
					sOnExit = TRUE;
					}
				else if (!strcmp(pszToken,"name"))
					{
					strcpy(szName,m_bf.NextToken());
					}

				pszToken = m_bf.NextToken();
				}
			//........... Now put the data into a useful form 
			// and log it:
			CBackgroundChange* pNew = new CBackgroundChange;
			pNew->m_lActivationTime = sDelay;
			pNew->m_lFadeOutTime = sFadeOutTime;
			pNew->m_lBlackTime = sBlackTime;
			pNew->m_lFadeInTime = sFadeInTime;

			strcpy(pNew->m_szNewName,szName);
			if (sOnExit) pNew->m_eType = CBackgroundChange::OnExit;

			// Log into the stream:
			pNew->m_sImageCacheIndex = m_pStream->AddBackground(szName);

			// Add into chunk:
			ASSERT(!pChunk->m_pChangeBackground);

			pChunk->m_pChangeBackground = pNew;

			continue;
			}

		//-----------------------------------------------------
		// Assume default case is text to be printed, which 
		// also signals the creation of a new phrase...
		//-----------------------------------------------------
		if (*pCurPhrase->m_szText)
			{
			TRACE("Error - already text\n");
			}

		// Set latest states
		pCurPhrase->m_sLocalY = sLocalY;
		pCurPhrase->m_sColorIndex = sCurColor;

		strcpy(pCurPhrase->m_szText,pszToken);
		// Create a brand new phrase:

		pChunk->AddPhrase(pCurPhrase);
		CTextPhrase* pNewPhrase = new CTextPhrase;
		*pNewPhrase = *pCurPhrase;	// struct copy
		*pNewPhrase->m_szText = 0;
		pCurPhrase = pNewPhrase;

		continue;
		}

	if (pChunk) 
		{
		// Figure out how big the chunk was
		sGlobalY += MAX(sMaxH,sLocalY);
		pChunk->m_sGlobalBottomY = sGlobalY - 1;

		m_pStream->AddChunk(pChunk);
		}

	if (pCurPhrase)
		{
		delete pCurPhrase;
		pCurPhrase = NULL;
		}

	return SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////
//
//	General scrolling text screens: Returns FAILURE if assets couldn't load
// OR returns FAILURE if use aborts!
//
//	All resources are assumed from the shell sak.
//
////////////////////////////////////////////////////////////////////////////////
short	ScrollPage(char* pszBackground,char* pszScrollScript,double dScrollRate,RRect *prWindow)
	{
	// Try to load resources:
	short sResult;

	rspLockBuffer();
	// clear background BEFORE doing a palette swap:
	rspRect(RSP_BLACK_INDEX,g_pimScreenBuf,0,0,g_pimScreenBuf->m_sWidth,
		g_pimScreenBuf->m_sHeight);
	rspUnlockBuffer();

	rspUpdateDisplay();

	// Load background
	RImage*	pimBackground;
	if (rspGetResource(&g_resmgrShell, pszBackground, &pimBackground) != SUCCESS)
		{
		TRACE("ScrollPage: Couldn't load background %s\n",pszBackground);

		return FAILURE; // couldn't load background
		}

	// Set palette
	ASSERT(pimBackground->m_pPalette != NULL);
	ASSERT(pimBackground->m_pPalette->m_type == RPal::PDIB);
	rspSetPaletteEntries(
		0, //10,
		256, // 236,
		pimBackground->m_pPalette->Red(0), //10
		pimBackground->m_pPalette->Green(0), //10
		pimBackground->m_pPalette->Blue(0), //10
		pimBackground->m_pPalette->m_sPalEntrySize);
	rspUpdatePalette();

	// Load text script:
	CFileTextInput*	pScript = NULL;	// must free it myself!

	// Try to override with our own file, because I'm too lazy to figure out how to regenerate .sak files.
	FILE *nonsak = fopen(FindCorrectFile("res/credits.txt", "rb"), "rb");
	if (nonsak != NULL)
		{
		pScript = new CFileTextInput;
		pScript->Load(nonsak);
		}

	if (pScript == NULL)
		{
		sResult = rspGetResource(&g_resmgrShell, pszScrollScript, &pScript);
		if (sResult != SUCCESS)
			{
			// try another sak:
			sResult = rspGetResource(&g_resmgrSamples, pszScrollScript, &pScript);
			if (sResult != SUCCESS)
				{
				TRACE("ScrollPage: Couldn't load script %s\n",pszScrollScript);
				g_resmgrShell.Release(pimBackground); // dom't need it anymore
				return FAILURE; // couldn't load background
				}
			}
		}

	// Set standard font...
	RPrint print;
	print.SetFont(24, &g_fontPostal);
	print.SetColor(255, 0 , 0);
	print.SetEffectAbs(RPrint::SHADOW_X,1);
	print.SetEffectAbs(RPrint::SHADOW_Y,1);

	// Clear mouse and keyboard events
	rspClearKeyEvents();
	rspClearMouseEvents();

	// Clear screen
	rspLockBuffer();
	rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);
	rspUnlockBuffer();

	pScript->m_pStream->Configure(dScrollRate,prWindow);
	pScript->m_pStream->Start(&print);

	// Wait until user input
	long lKey = 0;
	short sButtons = 0;
	short sJoyPress = 0;

//******************  STATISTICAL ANALYSIS!
/*
long lTimeCount[256] = {0,}; // bucket sort
*/
long lRunningTime,lPrevTime;
lRunningTime = lPrevTime = rspGetMilliseconds();

	while ( (pScript->m_pStream->Update(&print) == SUCCESS) && !lKey && !sButtons && !(rspGetQuitStatus()) && !sJoyPress)
		{
		// Show title image.

		rspLockBuffer();
		rspBlit(
			pimBackground, 
			g_pimScreenBuf, 
			0,0,
			BG_X ,//+ (rspGetMilliseconds()/100)&15, 
			BG_Y, 
			pimBackground->m_sWidth, 
			pimBackground->m_sHeight);

		pScript->m_pStream->Render(g_pimScreenBuf);
		rspUnlockBuffer();

		rspUpdateDisplay();
		UpdateSystem();

		// Get key and mouse button inputs
		rspGetMouse(NULL, NULL, &sButtons);
		rspGetKey(&lKey);
		sJoyPress = IsXInputButtonPressed();

		// Clear mouse events to avoid overflowing the queue
		rspClearMouseEvents();

		// Restrict the frame rate to MAX_SCOLL_FRAME_MILLI
		while ( (lRunningTime - lPrevTime) < MIN_SCOLL_FRAME_MILLI)
			{
			lRunningTime = rspGetMilliseconds();
			}
//*******************8 Update timing statistics:
//lTimeCount[lRunningTime - lPrevTime]++;

		lPrevTime = lRunningTime;
		}

//************* Report statistics
/*
FILE* fp = fopen("speed.out","a");
for (short i=0;i < 256;i++) fprintf(fp,"%hd = %ld\n",i,lTimeCount[i]);
*/

	//------------------------------------------------------------------------------
	// palette fade out ? ...


	// Free resources:
	g_resmgrShell.Release(pimBackground);

	if (nonsak != NULL)
		{
		delete pScript;
		fclose(nonsak);
		}
	else
		{
		g_resmgrShell.Release(pScript);
		}

	// Clean Up:

	// Clear mouse and keyboard events
	rspClearKeyEvents();
	rspClearMouseEvents();

	// DO NOT Clear screen!
	/*
	rspLockBuffer();
	rspRect(RSP_BLACK_INDEX, g_pimScreenBuf, 0, 0, g_pimScreenBuf->m_sWidth, g_pimScreenBuf->m_sHeight);
	rspUnlockBuffer();
	*/

	rspUpdateDisplay();

	if (lKey || sButtons || sJoyPress || rspGetQuitStatus()) return FAILURE;

	return SUCCESS;
	}

////////////////////////////////////////////////////////////////////////////////
//
// Display the credits - now with variable input
//
////////////////////////////////////////////////////////////////////////////////

// Returns 0 if successfull, non-zero otherwise
short Credits(SampleMasterID* pMusic,
							char*	pszBackground,
							char* pszCredits)					
{
	short sResult = 0;

#ifdef SHOW_EXIT_SCREEN
	
	RImage*	pimBackground;
	sResult = rspGetResource(&g_resmgrShell, EXIT_BG, &pimBackground);
	if (sResult == 0)
		{
		// Set palette
		ASSERT(pimBackground->m_pPalette != NULL);
		ASSERT(pimBackground->m_pPalette->m_type == RPal::PDIB);
		rspSetPaletteEntries(
			0, //10,
			256, // 236,
			pimBackground->m_pPalette->Red(0), //10
			pimBackground->m_pPalette->Green(0), //10
			pimBackground->m_pPalette->Blue(0), //10
			pimBackground->m_pPalette->m_sPalEntrySize);
		rspUpdatePalette();

		// Show title image.
		rspLockBuffer();
		rspBlit(
			pimBackground, 
			g_pimScreenBuf, 
			0, 0, 
			BG_X, 
			BG_Y, 
			pimBackground->m_sWidth, 
			pimBackground->m_sHeight);
		rspUnlockBuffer();
		}

	rspUpdateDisplay();

	// Wait a while or until user input
	long lKey = 0;
	short sButtons = 0;
	do	{
		UpdateSystem();

		// Get key and mouse button inputs
		short sButtons;
		rspGetMouse(NULL, NULL, &sButtons);
		rspGetKey(&lKey);

		// Clear mouse events to avoid overflowing the queue
		rspClearMouseEvents();

		} while (!lKey && !sButtons && rspGetQuitStatus() == FALSE);

	// Release bg
	g_resmgrShell.Release(pimBackground);

#else

	SampleMasterID*	psmidMusic;
	char	szBackground[256];
	char	szCredits[256];

	// Set defaults:
	psmidMusic = g_psmidMusic;
	strcpy(szBackground,g_szBackground);
	strcpy(szCredits,g_szCredits);

	// Set input parameters:
	if (pMusic) psmidMusic = pMusic;
	if (pszBackground) strcpy(szBackground,pszBackground);
	if (pszCredits) strcpy(szCredits,pszCredits);

	// Start credits music
	PlaySample(										// Returns nothing.
														// Does not fail.
		*psmidMusic,								// In:  Identifier of sample you want played.
		SampleMaster::Unspecified,				// In:  Sound Volume Category for user adjustment
		255,											// In:  Initial Sound Volume (0 - 255)
		&ms_siMusak,								// Out: Handle for adjusting sound volume
		NULL,											// Out: Sample duration in ms, if not NULL.
		MUSAK_START_TIME,							// In:  Where to loop back to in milliseconds.
														//	-1 indicates no looping (unless m_sLoop is
														// explicitly set).
		MUSAK_END_TIME,							// In:  Where to loop back from in milliseconds.
														// In:  If less than 1, the end + lLoopEndTime is used.
		true);										// In:  Call ReleaseAndPurge rather than Release after playing
	
	//------------------------------------------------------------------------------
	// Begin scrolling loop....
    RRect rect(0,80,wideScreenWidth,320);
	if (ScrollPage(szBackground,szCredits,0.12,&rect) != SUCCESS)
		{
		// USER aborted!

		// Stop the musak
		if (ms_siMusak)
			{
			// Don't just allow it to finish.
			StopLoopingSample(ms_siMusak);
			// Cut it off.
			AbortSample(ms_siMusak);
			// Clear.
			ms_siMusak	= 0;
			// Play final sample that completes the cut off sound. ***
			}

		return SUCCESS;
		}

	// fade out the musak: cross fade the laughter
	SampleMaster::SoundInstance siLaughter;

	UnlockAchievement(ACHIEVEMENT_WATCH_ALL_CREDITS);

	// Start silent laughter looping:
	PlaySample(
		g_smidDemonLaugh2,
		SampleMaster::Unspecified,
		0,
		&siLaughter,
		NULL,
		0,
		0,
		true);


	long	lTimeToFade = 15000;
	long	lStartTime = rspGetMilliseconds();

	// Copy the palette:
	UCHAR PaletteCopy[256 * 3] = {0,};
	
	rspGetPaletteEntries(10,236,PaletteCopy+30,PaletteCopy + 31,PaletteCopy + 32,3);

	//====================================================================
	long lCurTime;

	while ( (lCurTime = rspGetMilliseconds() - lStartTime) < lTimeToFade)
		{

		short sByteLevel = short((255.0 * lCurTime) / lTimeToFade);
		if (sByteLevel > 255) sByteLevel = 255;

		// Update the sound volumes:
		SetInstanceVolume(ms_siMusak,255 - sByteLevel);
		SetInstanceVolume(siLaughter,sByteLevel);

		// Update the Palette:
		short i=0;

		short sCurPal = 10 * 3;

		for (i=10; i < 246; i++, sCurPal += 3)
			{
			rspSetPaletteEntry( i,
					(PaletteCopy[sCurPal + 0] * (256 - sByteLevel)) / 256,
					(PaletteCopy[sCurPal + 1] * (256 - sByteLevel)) / 256,
					(PaletteCopy[sCurPal + 2] * (256 - sByteLevel)) / 256
					);
			}

		rspUpdatePalette();

		UpdateSystem();
		}

	//====================================================================

	// Stop the musak
	if (ms_siMusak)
		{
		// Don't just allow it to finish.
		StopLoopingSample(ms_siMusak);
		}

	// Stop the musak
	if (siLaughter)
		{
		// Don't just allow it to finish.
		StopLoopingSample(siLaughter);
		}

	// Stop the musak
	if (ms_siMusak)
		{
		// Cut it off.
		AbortSample(ms_siMusak);
		// Clear.
		ms_siMusak	= 0;
		// Play final sample that completes the cut off sound. ***
		}

	if (siLaughter)
		{
		while (IsSamplePlaying(siLaughter) == true)
			{
			UpdateSystem();
			}
		}

	//===========================================================================
	// Stop the musak
	if (siLaughter)
		{
		// Cut it off.
		AbortSample(siLaughter);
		// Clear.
		siLaughter	= 0;
		// Play final sample that completes the cut off sound. ***
		}

#endif

	return SUCCESS;
}



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
