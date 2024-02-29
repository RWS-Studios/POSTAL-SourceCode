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
#ifndef COLOR_MATCH_H
#define COLOR_MATCH_H
//==================================
#include "System.h"
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "ORANGE/File/file.h"
#else
	#include "Image.h"
	#include "file.h"
#endif
//==================================

extern	UCHAR rspMatchColorRGB(long r,long g,long b,short sStart,short sNum,
					 UCHAR* pr,UCHAR* pg,UCHAR* pb,long linc);

// designed for 2 dimenional input. For example, fog = source color +
// eye distance = dst color.
// The current form of Alpha is 256 x N, i.e., always an array of 256
// pointers to arrays.
// Note that it might be sufficient to use an Alpha as a light effect
// (1d array) just by keeping the depth at one.
//
class RAlpha
	{
public:
	RAlpha();
	~RAlpha();
	//==========================================================
	UCHAR* m_pAlphas[256]; // array of 256 ptrs to UCHAR lists
	short m_sAlphaDepth; // number of 256's
	//==========================================================
	// a fog effect is actually a 256 x n light effect, currently a depth of 256
	// you are sending a depth description of the fog to map to each
	// source pixel color (arrays sAlphaDepth deep)
	// This can also be used to create a light map.
	// This is the simplest colored lighting effect.
	// It is really only of use for fog.
	short CreateLightEffectRGB(UCHAR* pa,UCHAR* pr,UCHAR* pg,UCHAR* pb,long linc = 4,
			short sPalStart=0, short sPalLen = 256, short sAlphaDepth = 256);
	// This uses the built in scratch space and assumes to be already alloc'ed
	short CreateLightEffectRGB(short sPalStart=0, short sPalLen = 256);

	// This is an optional interface for creation of lighting effects:
	void StartEffect(); // do alloc first
	short MarkEffect(short sLev,short sChannel,UCHAR ucLev);
	void FinishEffect(short sPalStart=0, short sPalLen = 256); // will create it for you.

	// This is always 256 x 256, as it maps source to destination
	// You will want an array of these for multiple alpha levels.
	short CreateAlphaRGB(double dOpacity,short sPalStart, short sPalLen);
	void Dump(RImage* pimDst,short sX,short sY); // must be 256 lines high!
	void DumpPalette(RImage* pimDst,short sX,short sY); // must be 256 lines high!
	short Load(RFile* pFile);
	short Save(RFile* pFile);
	short Load(char* pszFile);
	short Save(char* pszFile);
	short Alloc(short sDepth);
	void Erase();
	//==========================================================

	// CHANNELS
	enum { csRED = 1,csBLUE = 2,csGREEN = 4,csALPHA = 8,csLAST_EFFECT = 16};


	//==========================================================
	// scratch space for creating a command list for creation:
	static U8 ms_r[256];
	static U8 ms_g[256];
	static U8 ms_b[256];
	static U8 ms_a[256];
	static U8 ms_f[256];

	static short ms_SetPalette(RImage* pimImage);
	static short ms_SetPalette(); // to system palette
	static short ms_IsPaletteSet;
public:
	// temporary storage for a master palette:
	static U8 ms_red[256];
	static U8 ms_green[256];
	static U8 ms_blue[256];
	};

// This concept will be refined later, but allows pixel selected alpha effects:
//
class RMultiAlpha
	{
public:
	RMultiAlpha();
	~RMultiAlpha();
	short Alloc(short sDepth);
	void Erase();
	//==========================================================
	short m_sNumLevels; // 0 = 1st non transparent level
	RAlpha** m_pAlphaList; // store ACTUAL alpha tables...
	UCHAR* m_pLevelOpacity; // for each layer
	// This goes from a 0-255 Alpha channel DIRECTLY to an alpha matrix
	UCHAR** m_pGeneralAlpha[256];

	// For live dimming, this takes a source pixel alpha level and
	// translates it to a dimmed alpha value, where 255 = source
	// brightness, and 0 equals dimmed to black!
	// It is selected at a higher level by the dimming parameter
	// Sadly, this is a 64K hit, but I don't see a way around it.

	static	UCHAR	ms_aucLiveDimming[65536];	// must be initialized!
	static	short ms_sIsInitialized;

	// This stores the alpha levels so m_pGeneralAlpha can be
	// restored after load / save
	// THIS contains TWO more levels than you: 
	// 0 = transparent, and m_sNumLevels = OPAQUE
	UCHAR	m_pSaveLevels[256];
	short m_sGeneral; // a flag, TRUE = general type
	//==========================================================
	// Newest Format: FastMultiAlpha Support
	//==========================================================
	// MultiAlpha does NOT support access by layers and is NOT
	// very flexible.

	// Find optimum # of alpha level for your cache
	static short QueryFastMultiAlpha(
		short sNumSrcCol, short sNumDstCol,long lTotMem, 
		long* plHeaderSize = NULL,long* plDataSize = NULL);

	// Create a FastMultiAlpha which MUST be freed BY the USER
	// USING the DeleteFastMultiAlpha command ising THIS MALPHA:
	UCHAR*** pppucCreateFastMultiAlpha(
		short sStartSrc,short sNumSrc,	// color indices
		short sStartDst,short sNumDst,
		long*	plAlignedSize = NULL);

	// USER MUST call this to free the fast multi alpha
	static short DeleteFastMultiAlpha(UCHAR ****pfmaDel);

	//==========================================================
	// archaic - old format - will go away
	short  AddAlpha(RAlpha* pAlpha,short sLev);

	//
	short CreateLayer(short sLayerNumber,
							double dOpacity,
							short sPalStart = 0, 
							short sPalLen = 256);

	// After all layers are in, this creates the logical mapping
	// if general, logically map the layer as 0-255,
	// Otherwise, just map layer to layer.
	short Finish(short sGeneral = TRUE);
	//==========================================================
	// This 
	short Load(RFile* pFile);
	short Save(RFile* pFile);
	short Load(char* pszFile);
	short Save(char* pszFile);
	//==========================================================
private:

	};

//===========================================================
//  This is to provide class like functionality to the
//  Fast MultiAlpha.  It is not designed to be referred to
//  in normal use - merely to manage the actual data.
//
//  It CANNOT create RFastMultiAlphaData - use an RMultiAlpha
//  for that.
//===========================================================

class	RFastMultiAlphaWrapper
	{
public:
	//-----------------------------------------------
	RFastMultiAlphaWrapper();
	void	Clear();
	~RFastMultiAlphaWrapper();
	void	Erase();
	//------------------------ Using the wrapper
	short	Load(RFile* prfFile);
	UCHAR ***pppucGetFMA(); // can only access ONCE!
	short	IsSrcCompatible(RImage* pimSrc);
	short	IsDstCompatible(RImage* pimDst);
	//------------------------ Creating the wrapper
	short	Attach(UCHAR ***pppucFMA,short sStartSrc,
		short sNumSrc,short sStartDst,short sNumDst,
		short sNumLayers);
	short	Save(RFile* prfFile);
	//------------------------ Validating use
	void	GetSrcRange(short *sStartIndex,short *sFinalIndex)
		{
		*sStartIndex = m_sStartSrc;
		*sFinalIndex = m_sStartSrc + m_sNumSrc - 1;
		}

	void	GetDstRange(short *sStartIndex,short *sFinalIndex)
		{
		*sStartIndex = m_sStartDst;
		*sFinalIndex = m_sStartDst + m_sNumDst - 1;
		}

	short	IsSrcValid(RImage* pimCheck);

	//-----------------------------------------------
private:
	// You can't touch this directly!
	UCHAR ***m_pppucFastMultiAlpha;
	short	m_sStartSrc;
	short m_sStartDst;
	short m_sNumSrc;
	short m_sNumDst;
	short m_sNumLayers;

	short m_sDelete; // should I delete the data?

	short m_sAccess; // to guide the programmer
	};

//==================================
#endif
