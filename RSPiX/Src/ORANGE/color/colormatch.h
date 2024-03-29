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

extern	uint8_t rspMatchColorRGB(int32_t r,int32_t g,int32_t b,int16_t sStart,int16_t sNum,
					 uint8_t* pr,uint8_t* pg,uint8_t* pb,int32_t linc);

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
	uint8_t* m_pAlphas[256]; // array of 256 ptrs to uint8_t lists
	int16_t m_sAlphaDepth; // number of 256's
	//==========================================================
	// a fog effect is actually a 256 x n light effect, currently a depth of 256
	// you are sending a depth description of the fog to map to each
	// source pixel color (arrays sAlphaDepth deep)
	// This can also be used to create a light map.
	// This is the simplest colored lighting effect.
	// It is really only of use for fog.
	int16_t CreateLightEffectRGB(uint8_t* pa,uint8_t* pr,uint8_t* pg,uint8_t* pb,int32_t linc = 4,
			int16_t sPalStart=0, int16_t sPalLen = 256, int16_t sAlphaDepth = 256);
	// This uses the built in scratch space and assumes to be already alloc'ed
	int16_t CreateLightEffectRGB(int16_t sPalStart=0, int16_t sPalLen = 256);

	// This is an optional interface for creation of lighting effects:
	void StartEffect(); // do alloc first
	int16_t MarkEffect(int16_t sLev,int16_t sChannel,uint8_t ucLev);
	void FinishEffect(int16_t sPalStart=0, int16_t sPalLen = 256); // will create it for you.

	// This is always 256 x 256, as it maps source to destination
	// You will want an array of these for multiple alpha levels.
	int16_t CreateAlphaRGB(double dOpacity,int16_t sPalStart, int16_t sPalLen);
	void Dump(RImage* pimDst,int16_t sX,int16_t sY); // must be 256 lines high!
	void DumpPalette(RImage* pimDst,int16_t sX,int16_t sY); // must be 256 lines high!
	int16_t Load(RFile* pFile);
	int16_t Save(RFile* pFile);
	int16_t Load(char* pszFile);
	int16_t Save(char* pszFile);
	int16_t Alloc(int16_t sDepth);
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

	static int16_t ms_SetPalette(RImage* pimImage);
	static int16_t ms_SetPalette(); // to system palette
	static int16_t ms_IsPaletteSet;
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
	int16_t Alloc(int16_t sDepth);
	void Erase();
	//==========================================================
	int16_t m_sNumLevels; // 0 = 1st non transparent level
	RAlpha** m_pAlphaList; // store ACTUAL alpha tables...
	uint8_t* m_pLevelOpacity; // for each layer
	// This goes from a 0-255 Alpha channel DIRECTLY to an alpha matrix
	uint8_t** m_pGeneralAlpha[256];

	// For live dimming, this takes a source pixel alpha level and
	// translates it to a dimmed alpha value, where 255 = source
	// brightness, and 0 equals dimmed to black!
	// It is selected at a higher level by the dimming parameter
	// Sadly, this is a 64K hit, but I don't see a way around it.

	static	uint8_t	ms_aucLiveDimming[65536];	// must be initialized!
	static	int16_t ms_sIsInitialized;

	// This stores the alpha levels so m_pGeneralAlpha can be
	// restored after load / save
	// THIS contains TWO more levels than you: 
	// 0 = transparent, and m_sNumLevels = OPAQUE
	uint8_t	m_pSaveLevels[256];
	int16_t m_sGeneral; // a flag, TRUE = general type
	//==========================================================
	// Newest Format: FastMultiAlpha Support
	//==========================================================
	// MultiAlpha does NOT support access by layers and is NOT
	// very flexible.

	// Find optimum # of alpha level for your cache
	static int16_t QueryFastMultiAlpha(
		int16_t sNumSrcCol, int16_t sNumDstCol,int32_t lTotMem, 
		int32_t* plHeaderSize = NULL,int32_t* plDataSize = NULL);

	// Create a FastMultiAlpha which MUST be freed BY the USER
	// USING the DeleteFastMultiAlpha command ising THIS MALPHA:
	uint8_t*** pppucCreateFastMultiAlpha(
		int16_t sStartSrc,int16_t sNumSrc,	// color indices
		int16_t sStartDst,int16_t sNumDst,
		int32_t*	plAlignedSize = NULL);

	// USER MUST call this to free the fast multi alpha
	static int16_t DeleteFastMultiAlpha(uint8_t ****pfmaDel);

	//==========================================================
	// archaic - old format - will go away
	int16_t  AddAlpha(RAlpha* pAlpha,int16_t sLev);

	//
	int16_t CreateLayer(int16_t sLayerNumber,
							double dOpacity,
							int16_t sPalStart = 0, 
							int16_t sPalLen = 256);

	// After all layers are in, this creates the logical mapping
	// if general, logically map the layer as 0-255,
	// Otherwise, just map layer to layer.
	int16_t Finish(int16_t sGeneral = TRUE);
	//==========================================================
	// This 
	int16_t Load(RFile* pFile);
	int16_t Save(RFile* pFile);
	int16_t Load(char* pszFile);
	int16_t Save(char* pszFile);
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
	int16_t	Load(RFile* prfFile);
	uint8_t ***pppucGetFMA(); // can only access ONCE!
	int16_t	IsSrcCompatible(RImage* pimSrc);
	int16_t	IsDstCompatible(RImage* pimDst);
	//------------------------ Creating the wrapper
	int16_t	Attach(uint8_t ***pppucFMA,int16_t sStartSrc,
		int16_t sNumSrc,int16_t sStartDst,int16_t sNumDst,
		int16_t sNumLayers);
	int16_t	Save(RFile* prfFile);
	//------------------------ Validating use
	void	GetSrcRange(int16_t *sStartIndex,int16_t *sFinalIndex)
		{
		*sStartIndex = m_sStartSrc;
		*sFinalIndex = m_sStartSrc + m_sNumSrc - 1;
		}

	void	GetDstRange(int16_t *sStartIndex,int16_t *sFinalIndex)
		{
		*sStartIndex = m_sStartDst;
		*sFinalIndex = m_sStartDst + m_sNumDst - 1;
		}

	int16_t	IsSrcValid(RImage* pimCheck);

	//-----------------------------------------------
private:
	// You can't touch this directly!
	uint8_t ***m_pppucFastMultiAlpha;
	int16_t	m_sStartSrc;
	int16_t m_sStartDst;
	int16_t m_sNumSrc;
	int16_t m_sNumDst;
	int16_t m_sNumLayers;

	int16_t m_sDelete; // should I delete the data?

	int16_t m_sAccess; // to guide the programmer
	};

//==================================
#endif
