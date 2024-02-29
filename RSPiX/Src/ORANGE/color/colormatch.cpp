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
#include <string.h>
#include "Blue.h"
#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "ORANGE/color/colormatch.h"
	#include "GREEN/BLiT/BLIT.H" // ONLY for the debug command... can be disabled if nec
	#include "ORANGE/QuickMath/FixedPoint.h"
#else
	#include "Image.h"
	#include "colormatch.h"
	#include "BLIT.H" // ONLY for the debug command... can be disabled if nec
	#include "fixedpoint.h"
#endif

//short RAlpha::ms_SetPalette(RImage* pimImage);
short RAlpha::ms_IsPaletteSet = FALSE;
U8 RAlpha::ms_red[256] = {0,};
U8 RAlpha::ms_green[256] = {0,};
U8 RAlpha::ms_blue[256] = {0,};

U8 RAlpha::ms_r[256] = {0,};
U8 RAlpha::ms_g[256] = {0,};
U8 RAlpha::ms_b[256] = {0,};
U8 RAlpha::ms_a[256] = {0,};
U8 RAlpha::ms_f[256] = {0,};

short RMultiAlpha::ms_sIsInitialized = FALSE;
UCHAR	RMultiAlpha::ms_aucLiveDimming[65536];

UCHAR rspMatchColorRGB(long r,long g,long b,short sStart,short sNum,
					 UCHAR* pr,UCHAR* pg,UCHAR* pb,long linc)
	{
	long lMatch = 0,i;
	long lMax =  long(16777217),lDist;
	long lOffset = linc * sStart; // array offset

	pr += lOffset;
	pg += lOffset;
	pb += lOffset;
	
	for (i=0;i<sNum;i++)
		{
		lDist = SQR(long(*pr)-r)+SQR(long(*pg)-g)+SQR(long(*pb)-b);
		pr += linc;
		pg += linc;
		pb += linc;
		if (lDist < lMax)
			{
			lMax = lDist;
			lMatch = i;
			}
		}

	return (UCHAR)(lMatch + sStart);
	}


// Currently, I am using an image as the file format for a palette.
// This uses the rsp system standard type RPixel32, which is endian swapped on the PC.
// It counts on RSP SetPalette to go through a gamma correction table on the PC.
// Higher level functions can do actual image conversions.

short RAlpha::Load(char* pszFile)
	{
	RFile file;

	if (file.Open(pszFile,"rb",RFile::LittleEndian))
		{
		TRACE("RAlpha::Load: Error accessing file %s\n",pszFile);
		return -1;
		}
	
	short sRet = Load(&file);
	file.Close();

	return sRet;
	}

short RAlpha::Save(char* pszFile)
	{
	RFile *fp = new RFile;

	if (fp->Open(pszFile,"wb",RFile::LittleEndian))
		{
		TRACE("RAlpha::Load: Error accessing file %s\n",pszFile);
		return -1;
		}
	
	short sRet = Save(fp);
	fp->Close();
	return sRet;
	}

short RAlpha::Save(RFile* fp)
	{
	short sVersion = 1;

	if (!fp)
		{
		TRACE("RAlpha::Save: Null RFile!\n");
		return -1;
		}

	fp->Write("RALPHA");
	fp->Write(&sVersion);
	fp->Write(&m_sAlphaDepth);
	/* inverted 
	for (short i=0; i < m_sAlphaDepth; i++)
		fp->Write(m_pAlphas[i],256);
		*/
	for (short i=0; i < 256; i++)
		fp->Write(m_pAlphas[i],m_sAlphaDepth);

	return 0;
	}

short RAlpha::Load(RFile* fp)
	{
	short sVersion = 1;
	char name[20];

#ifdef _DEBUG

	if (!fp)
		{
		TRACE("RAlpha::Load: Null RFile!\n");
		return -1;
		}

#endif

	fp->Read(name);
	if (strcmp(name,"RALPHA"))
		{
		TRACE("RAlpha::Load: Not an RAlpha!\n");
		return -1;
		}

	fp->Read(&sVersion);
	if (sVersion != 1)
		{
		TRACE("RAlpha::Load: Not a supported version!\n");
		return -1;
		}

	fp->Read(&m_sAlphaDepth);
	Alloc(m_sAlphaDepth);

	/* ionverted
	for (short i=0; i < m_sAlphaDepth; i++)
		fp->Read(m_pAlphas[i],256);
	*/

	for (short i=0; i < 256; i++)
		{
		if (fp->Read(m_pAlphas[i],m_sAlphaDepth)!=m_sAlphaDepth)
			{
			TRACE("RAlpha::Load: read error!\n");
			return -1;
			}
		}

	return 0;
	}

short RAlpha::ms_SetPalette(RImage* pimImage)
	{
#ifdef _DEBUG

	if (!pimImage)
		{
		TRACE("RAlpha::ms_SetPalette: Null image!\n");
		return -1;
		}

	if (!pimImage->m_pPalette)
		{
		TRACE("RAlpha::ms_SetPalette: image has no palette!\n");
		return -1;
		}

	if (!pimImage->m_pPalette->m_pData)
		{
		TRACE("RAlpha::ms_SetPalette: palette has no data!\n");
		return -1;
		}

#endif

	if (pimImage->m_pPalette->GetEntries(0,256,ms_red,ms_green,ms_blue,1)==0)
		{
		ms_IsPaletteSet = TRUE;
		return 0;
		}

	return -1;
	}

 // do alloc first
void RAlpha::StartEffect()
	{
	// clear scratch space
	for (short i=0; i< 256;i++)
		{
		ms_f[i] = ms_r[i]  = ms_g[i] = ms_b[i] = ms_a[i] = 0;
		}
	}

short RAlpha::MarkEffect(short sLev,short sChannel,UCHAR ucLev)
	{
	if (sLev >= m_sAlphaDepth)
		{
		TRACE("RAlpha::MarkEffect: Level out of range!\n");
		return -1;
		}

	if (sChannel >= csLAST_EFFECT) 
		{
		TRACE("RAlpha::MarkEffect: Channel not supported\n");
		return -1;
		}

	ms_f[sLev] |= sChannel;
	switch (sChannel)
		{
	case csALPHA:
		ms_a[sLev] = ucLev;
	break;
	case csRED:
		ms_r[sLev] = ucLev;
	break;
	case csGREEN:
		ms_g[sLev] = ucLev;
	break;
	case csBLUE:
		ms_b[sLev] = ucLev;
	break;

		default:
			TRACE("RAlpha::MarkEffect:: BAD CHANNEL VALUE!\n");
			return -1;
		}

	return 0;
	}

// will create it for you.
// This is designed to interpolate channels independently!
//
void RAlpha::FinishEffect(short sPalStart, short sPalLen)
	{
	// Interpolate each channel freely
	short sChannel = 1;
	// ( THE TWO ENDS of the channels ARE ASSUMED MARKED!!!)
	ms_f[0] = ms_f[m_sAlphaDepth-1] = 255;

	while (sChannel < csLAST_EFFECT)
		{
		UCHAR* pucChannel = NULL;

		switch (sChannel)
			{
		case csALPHA:
			pucChannel = &ms_a[0];
		break;
		case csRED:
			pucChannel = &ms_r[0];
		break;
		case csGREEN:
			pucChannel = &ms_g[0];
		break;
		case csBLUE:
			pucChannel = &ms_b[0];
		break;

			default:
				TRACE("RAlpha::FinishEffect:: BAD CHANNEL VALUE!\n");
			}

		// Interpolate the channel!
		short sIndex = 0;
		short sBaseIndex = 0;
		while (sIndex < m_sAlphaDepth)
			{
			if ((ms_f[sIndex] & sChannel) != 0)
				{
				if (sIndex - sBaseIndex > 1)
					{
					// fill in inbetween stuff
					double dDelta = double(pucChannel[sIndex] - 
						pucChannel[sBaseIndex]) / double(sIndex - sBaseIndex);
					double dVal = double(pucChannel[sBaseIndex])+0.5;

					for (short j = sBaseIndex; j< sIndex;j++)
						{
						pucChannel[j] = UCHAR(dVal);
						dVal += dDelta;
						}
					}
				sBaseIndex = sIndex;
				}
			sIndex++;
			}

		sChannel <<= 1;
		}

	// NOW CREATE THE ACTUAL ALPHAS!
	CreateLightEffectRGB(sPalStart,sPalLen);
	
	}

// Uses the current system palette:
//
short RAlpha::ms_SetPalette()
	{
	rspGetPaletteEntries(0,256,ms_red,ms_green,ms_blue,1);
	ms_IsPaletteSet = TRUE;
	return 0;
	}

short RAlpha::Alloc(short sDepth)
	{
	/* Inverted
	m_pAlphas = (UCHAR**) calloc(m_sAlphaDepth,sizeof (UCHAR*));
	for (short i=0; i < m_sAlphaDepth; i++)
		m_pAlphas[i] = (UCHAR*) calloc(1,256);
	*/
	m_sAlphaDepth = sDepth;

	for (short i=0; i < 256; i++)
		m_pAlphas[i] = (UCHAR*) calloc(1,m_sAlphaDepth);

	return 0;
	}

RAlpha::RAlpha()
	{
	//m_pAlphas = NULL;
	for (short i = 0 ;i<256;i++)
		m_pAlphas[i] = NULL;
	m_sAlphaDepth = 0;
	}

void RAlpha::Erase()
	{
	if (m_pAlphas)
		{
		for (short i=0;i<256 /*m_sAlphaDepth*/;i++)
			{
			if (m_pAlphas[i]) 
				{
				free(m_pAlphas[i]);
				m_pAlphas[i] = NULL;
				}
			}
		//free(m_pAlphas);
		}
	}

RAlpha::~RAlpha()
	{
	Erase();
	}

//************ COMMENT THIS OUT TO REMOVE DEPENDENCY ON BLIT!
// For debugging an alpha, you should set the current palette to the
// default one...
void RAlpha::Dump(RImage* pimDst,short sX,short sY) 
	{
	short i,j;

#ifdef _DEBUG
	if (!pimDst)
		{
		TRACE("RAlpha::Dump: null image passed!\n");
		return;
		}
#endif

	for (j=0;j<m_sAlphaDepth;j++)
		for (i=0;i<255;i++)
			rspPlot((m_pAlphas[i][j]),pimDst,short(sX+i),short(sY+j));
	}

//************ COMMENT THIS OUT TO REMOVE DEPENDENCY ON BLIT!
// For debugging an alpha, you should set the current palette to the
// default one...
void RAlpha::DumpPalette(RImage* pimDst,short sX,short sY) 
	{
	short i;

#ifdef _DEBUG
	if (!pimDst)
		{
		TRACE("RAlpha::DumpPalette: null image passed!\n");
		return;
		}
#endif

	for (i=0;i<255;i++)
		{
		rspLine(UCHAR(i),pimDst,short(sX+i),short(sY),short(sX+i),short(sY+255));
		}
	}

// dOpacity for now is between 0.0 (background) and 1.0 (foreground)
short RAlpha::CreateAlphaRGB(double dOpacity,short sPalStart, short sPalLen)
	{
#ifdef _DEBUG
	if (!ms_IsPaletteSet)
		{
		TRACE("RAlpha::CreateAlphaRGB: First set your palette with ms_SetPalette\n");
		return -1;
		}
#endif

	// could erase first.
	Alloc(256);

	// If you use a 256 x 256 table for calculating BYTE * OPACITY = BYTE, you may double
	// your net speed
	short lSrc = (long)256 * dOpacity;
	short lDst = 256 - lSrc;

	// If you use a 256 x 256 table for calculating BYTE * OPACITY = BYTE, you may double
	RFixedU16 r,g,b;

	short s,d;
	for (s=0;s<256;s++)
		{
		for (d = 0;d < 256;d++)
			{
			// This can be replaced with a 256K table
			r.val = U16( ms_red[s] * lSrc + ms_red[d] * lDst);
			g.val = U16( ms_green[s] * lSrc + ms_green[d] * lDst);
			b.val = U16( ms_blue[s] * lSrc + ms_blue[d] * lDst);

			m_pAlphas[s][d] = (U8)rspMatchColorRGB(r.mod,g.mod,b.mod,sPalStart,sPalLen,
					ms_red,ms_green,ms_blue,1);					
			}
		}
	
	return 0;
	}

// dOpacity for now is between 0 (background) and 255 (foreground)
// Input description should be arrays at least sAlphaDepth long.
short RAlpha::CreateLightEffectRGB(UCHAR* pa,UCHAR* pr,UCHAR* pg,UCHAR* pb,long linc,
			short sPalStart, short sPalLen, short sAlphaDepth)
	{
#ifdef _DEBUG
	if (!ms_IsPaletteSet)
		{
		TRACE("RAlpha::CreateLightEffectRGB: First set your palette with ms_SetPalette\n");
		return -1;
		}
#endif

	// could erase first.
	Alloc(sAlphaDepth);

	// If you use a 256 x 256 table for calculating BYTE * OPACITY = BYTE, you may double
	RFixedU16 r,g,b;
	long lSrc,lFog;

	short s,f;
	for (s=0;s<256;s++)
		{
		for (f = 0;f < sAlphaDepth;f++)
			{
			lSrc = long(pa[f]);
			lFog = 255 - lSrc;
			// This can be replaced with a 256K table
			r.val = U16( ms_red[s] * lSrc + pr[f] * lFog);
			g.val = U16( ms_green[s] * lSrc + pg[f] * lFog);
			b.val = U16( ms_blue[s] * lSrc + pb[f] * lFog);

			m_pAlphas[s][f] = (U8)rspMatchColorRGB(r.mod,g.mod,b.mod,sPalStart,sPalLen,
					ms_red,ms_green,ms_blue,1);					
			}
		}
	
	return 0;
	}

short RAlpha::CreateLightEffectRGB(short sPalStart, short sPalLen)
	{
#ifdef _DEBUG
	if (!ms_IsPaletteSet)
		{
		TRACE("RAlpha::CreateLightEffectRGB: First set your palette with ms_SetPalette\n");
		return -1;
		}
#endif

	RFixedU16 r,g,b;
	long lSrc,lFog;

	short s,f;
	for (s=0;s<256;s++)
		{
		for (f = 0;f < m_sAlphaDepth;f++)
			{
			lSrc = long(ms_a[f]);
			lFog = 255 - lSrc;

			// This can be replaced with a 256K table
			r.val = U16( ms_red[s] * lSrc + ms_r[f] * lFog);
			g.val = U16( ms_green[s] * lSrc + ms_g[f] * lFog);
			b.val = U16( ms_blue[s] * lSrc + ms_b[f] * lFog);

			m_pAlphas[s][f] = (U8)rspMatchColorRGB(r.mod,g.mod,b.mod,sPalStart,sPalLen,
					ms_red,ms_green,ms_blue,1);					
			}
		}
	
	return 0;
	}


//**************************************************************************
//**************************************************************************
// MultiAlpha!
//**************************************************************************
//**************************************************************************

RMultiAlpha::RMultiAlpha()
	{
	// Handle the one time initiation of the dimmer control:
	if (!ms_sIsInitialized)
		{
		// FILL THE TABLE!
		long lSrcVal,lDimVal,lCurValue,lNumerator;
		UCHAR*	pCur = ms_aucLiveDimming;
		// This is DimVal major!

		for (lDimVal = 0; lDimVal < 256; lDimVal++)
			{
			lNumerator = 127;	// for rounding
			lCurValue = 0;

			for (lSrcVal = 0; lSrcVal < 256; lSrcVal++,pCur++)
				{
				lNumerator += lDimVal;

				if (lNumerator >= 255) 
					{
					lNumerator -= 255; 
					lCurValue++;
					}

				*pCur = UCHAR(lCurValue);
				}
			}
		
		ms_sIsInitialized = TRUE;
		}

	Erase();
	}

RMultiAlpha::~RMultiAlpha()
	{
	for (short i=0;i < m_sNumLevels;i++)
		{
		if (m_pAlphaList[i] != NULL) delete m_pAlphaList[i];
		}

	free (m_pAlphaList);
	free (m_pLevelOpacity);

	Erase();
	}

short RMultiAlpha::Alloc(short sDepth)
	{
	// a NULL pointer will be left in the zero position.
	if (m_pAlphaList)
		{
		TRACE("RMultiAlpha::alloc: error, already created!\n");
		return -1;
		}

	m_sNumLevels = sDepth;
	m_pAlphaList = (RAlpha**)calloc(m_sNumLevels,sizeof(RAlpha*));
	m_pLevelOpacity = (UCHAR*)calloc(m_sNumLevels,1);

	return 0;
	}

void RMultiAlpha::Erase()
	{
	m_sNumLevels = 0;
	m_pAlphaList = NULL;
	for (short i=0;i < 256;i++) 
		{
		m_pGeneralAlpha[i] = NULL;
		m_pSaveLevels[i] = UCHAR(0);
		}
	m_sGeneral = TRUE;
	m_pLevelOpacity = NULL;
	}

// Note: this type can be supported with one alphablit
// by just pointing the first n levels to teh shade
// you desire
// SOON TO BE ARCHAIC!!!!!
//
short RMultiAlpha::AddAlpha(RAlpha* pAlpha,short sLev)
	{
	if (sLev > m_sNumLevels)
		{
		TRACE("RMultiAlpha::AddAlpha: level out of range!\n");
		return -1;
		}
	m_pAlphaList[sLev] = pAlpha;
	return 0;
	}

short RMultiAlpha::Load(RFile* pFile)
	{
	char type[50];
	short sVer;

	pFile->Read(type);
	if (strcmp(type,"MALPHA"))
		{
		TRACE("RMultiAlpha::Load: Bad File Type\n");
		return -1;
		}

	pFile->Read(&sVer);

	if (sVer != 2)
		{
		TRACE("RMultiAlpha::Load: Bad Version Number\n");
		return -1;
		}

	pFile->Read(&m_sNumLevels);
	Alloc(m_sNumLevels);

	// General table information:
	pFile->Read(&m_sGeneral);
	// Opacity data
	pFile->Read(m_pLevelOpacity,m_sNumLevels);
	// Level Data
	pFile->Read(m_pSaveLevels,256);

	// Now load the actual sub-alphas
	short i;
	for (i = 0; i < m_sNumLevels; i++)
		{
		m_pAlphaList[i] = new RAlpha;
		m_pAlphaList[i]->Load(pFile);
		}

	// Now the challange... restore the pointer list:
	for (i=0;i<256;i++)
		{
		if ((m_pSaveLevels[i]==0) || (m_pSaveLevels[i] > m_sNumLevels)) 
			m_pGeneralAlpha[i] = NULL;
		else m_pGeneralAlpha[i] = m_pAlphaList[m_pSaveLevels[i] - 1]->m_pAlphas;
		}

	return 0;
	}

short RMultiAlpha::Save(RFile* pFile)
	{
	char type[] = "MALPHA";
	short sVer = 2;

	pFile->Write(type);
	pFile->Write(&sVer);
	pFile->Write(&m_sNumLevels);

	// General table information:
	pFile->Write(&m_sGeneral);
	// Opacity data
	pFile->Write(m_pLevelOpacity,m_sNumLevels);
	// Level Data
	pFile->Write(m_pSaveLevels,256);

	// Now save the actual sub-alphas
	for (short i = 0; i < m_sNumLevels; i++)
		{
		m_pAlphaList[i]->Save(pFile);
		}


	return 0;
	}

short RMultiAlpha::Load(char* pszFile)
	{
	RFile fplocal;
	RFile *fp = &fplocal; // needs to be freed automatically!

	if (m_pAlphaList != NULL)
		{
		TRACE("RMultiAlpha::Load: MultAlpha NOT EMPTY!\n");
		return -1;
		}

	if (fp->Open(pszFile,"rb",RFile::LittleEndian))
		{
		TRACE("RMultiAlpha::Load: Error accessing file %s\n",pszFile);
		return -1;
		}
	
	short sRet = Load(fp);
	fp->Close();
	return sRet;
	}

short RMultiAlpha::Save(char* pszFile)
	{
	RFile *fp = new RFile;

	if (fp->Open(pszFile,"wb",RFile::LittleEndian))
		{
		TRACE("RMultiAlpha::Load: Error accessing file %s\n",pszFile);
		return -1;
		}
	
	short sRet = Save(fp);
	fp->Close();
	return sRet;
	}

// dOpacity for now is between 0.0 (background) and 1.0 (foreground)
// if sGeneral is true, then the LOGICAL layer will be mapped from 
// 0-255, otherwise it wil be mapped to the layer number.
// You must first alloc the amount of colors.
//
short RMultiAlpha::CreateLayer(short sLayerNumber,
												double dOpacity,
												short sPalStart, 
												short sPalLen)
	{
	if (sLayerNumber >= m_sNumLevels)
		{
		TRACE("RMultiAlpha::CreateAlphaLayer: Layer out of range.\n");
		return -1;
		}

	if (m_pAlphaList[sLayerNumber] != NULL)
		{
		TRACE("RMultiAlpha::CreateAlphaLayer: Error: Layer exists.\n");
		return -1;
		}
	// remember how this level was created.
	m_pLevelOpacity[sLayerNumber] = UCHAR(255 * dOpacity);
	m_pAlphaList[sLayerNumber] = new RAlpha;
	m_pAlphaList[sLayerNumber]->CreateAlphaRGB(dOpacity,sPalStart,sPalLen);

	return 0;
	}

// This interpolates the general table information
//
short RMultiAlpha::Finish(short sGeneral)
	{
	short i;

	// At this level, might as well using floating point math for now.
	if (sGeneral == FALSE) // make a level based identity mapping:
		{
		m_sGeneral = FALSE;

		m_pGeneralAlpha[0] = NULL; // shift things up one.
		for (i=0;i<m_sNumLevels;i++) // make it look like (m_sNumLevels+2)
			{
			m_pSaveLevels[i] = i;
			// Use NULL as a code for opacity, 0 is hooked as transparent
			if (m_pLevelOpacity[i] == 255) m_pGeneralAlpha[i+1] = NULL;
			else
				m_pGeneralAlpha[i+1] = m_pAlphaList[i]->m_pAlphas;
			}
		// make everything off the end opaque
		m_pSaveLevels[m_sNumLevels] = m_sNumLevels;
		for (i = m_sNumLevels + 1; i < 256;i++) 
			{
			m_pGeneralAlpha[i] = NULL;
			m_pSaveLevels[i] = m_sNumLevels + 1;
			}
		}
	else // spread out the values from 0 to 255:
		{
		m_sGeneral = TRUE;

		for (i=0;i<256;i++) m_pSaveLevels[i] = UCHAR(0);

		for (i=0;i<m_sNumLevels;i++)
			{
			// Use NULL as a code for opacity, 0 is hooked as transparent
			m_pSaveLevels[m_pLevelOpacity[i]] = (i+1); // the zeroth level is implied!
			}

		// set ends as a default:
		if (m_pSaveLevels[255] == 0) m_pSaveLevels[255] = m_sNumLevels+1;

		short sIndex = 0;
		short sBaseIndex = 0;
		while (sIndex < 256)
			{
			if (m_pSaveLevels[sIndex] != 0)
				{
				if (sIndex - sBaseIndex > 1)
					{
					// fill in inbetween stuff
					double dDelta = double(m_pSaveLevels[sIndex] - 
						m_pSaveLevels[sBaseIndex]) / double(sIndex - sBaseIndex);
					double dVal = double(m_pSaveLevels[sBaseIndex])+0.5;

					for (short j = sBaseIndex; j< sIndex;j++)
						{
						m_pSaveLevels[j] = UCHAR(dVal);
						dVal += dDelta;
						}
					}
				sBaseIndex = sIndex;
				}
			sIndex++;
			}
		// Now, link it up!
		for (i=0;i<256;i++) 
			{
			if ((m_pSaveLevels[i]==0) || (m_pSaveLevels[i] > m_sNumLevels)) 
				m_pGeneralAlpha[i] = NULL;
			else m_pGeneralAlpha[i] = m_pAlphaList[m_pSaveLevels[i] - 1]->m_pAlphas;
			}
		}

	return 0;
	}

///////////////////////////////////////////////////////////////////////////
// This is designed for getting an entire multialpha table into L2 cache.
// It does not behave entirely within the official MultiAlpha genre, and
// so it is best to create an uncompressed MultiAlpha First, and then to 
// create a compact one.  
//
// The compressed Multialpha does more than merely use less memory - it
// ASSURES a single contiguous block of memory is used, making it easier
// to ensure caching.
//
// The main memory savings is that you only need to store alpha lists for 
// source olors you anticipate.  If your source only has 64 colors, than
// your alpha table needs only 1/4 the memory as if it realied on 256 colors.
// NOTE:  In release mode, using such a table incorrectly will crash the 
// machine.
//
// Other than these points, it will behave very closely to a normal
// multialpha.
///////////////////////////////////////////////////////////////////////////
//  UINPUT:  the source color range (Start Index, Len) and destination
//				range (start index and len)
///////////////////////////////////////////////////////////////////////////
//  OUPUT:  the actual FastMultiAlpha pointer in the form of a (UCHAR***)
//          optional:  the size of the data in bytes.
///////////////////////////////////////////////////////////////////////////
UCHAR*** RMultiAlpha::pppucCreateFastMultiAlpha(
		short sStartSrc,short sNumSrc,	// color indices
		short sStartDst,short sNumDst,
		long*	plAlignedSize)
	{
	// Assumes a fully correct this pointer!
	long lTotSize = 1024 + long(m_sNumLevels) * sNumSrc * (4L + sNumDst);
	// align it to 4096 to save cache memory!
	UCHAR*** pppucFastMem = (UCHAR***) calloc(1,lTotSize + 4095);
	// WARNING:  these arrays of pointers are 4 bytes in size, so be careful
	// how you seek!

	// I am adding on in BYTES, not words...
	UCHAR*** pppucFastAligned = (UCHAR***) ((ULONG(pppucFastMem)+4095) & (~4095) );

	if (!pppucFastMem) return NULL;
	UCHAR* pInfo = (UCHAR*)pppucFastAligned;
	// For freeing:
	long lMemOffset = ULONG(pppucFastAligned) - ULONG(pppucFastMem);

	// Remember offsets for each main memory structure:

	// Jump over the array of pointers to alpha levels
	// I AM COUNTING ON 4 byte array increments! (+ 1024 bytes)
	UCHAR** ppucFirstSrcArray = (UCHAR**)(pppucFastAligned + 256);
	UCHAR** ppucCurSrc = ppucFirstSrcArray;
	// Jump over the source arrays of pointers destination lists:

	// I AM COUNTING ON 4 byte array increments!
	// ( + 4LS bytes)
	UCHAR* pucData = (UCHAR*) (ppucFirstSrcArray + long(m_sNumLevels) * sNumSrc);
	UCHAR* pData = pucData;

	//------------------------------------------------------------------------
	// Create the needed pointers into the normal MultiAlpha:
	//------------------------------------------------------------------------
	UCHAR** ppucLevel = NULL;

	// Copy the abridged data from the current MultiAlpha in 
	// level majorest, source major, destination minor form:
	short a = 0,l,s,d;
	short sOldL = 0;

	while (a < 255)
		{
		a++;
		//------------------------- Find unique alphas
		l = m_pSaveLevels[a];
		pppucFastAligned[a] = pppucFastAligned[a-1]; // repeat the pointers
		if (l == sOldL) continue;
		sOldL = l;
		//-------------------------
		// Start a new Alpha Level:
		pppucFastAligned[a] = ppucCurSrc - sStartSrc; // sStartSrc Biased!

		ppucLevel = m_pGeneralAlpha[a];
		if (!ppucLevel) {pppucFastAligned[a] = NULL; continue;} // Final 100% alphas are not stored

		for (s = 0; s < sNumSrc; s++,ppucCurSrc++)
			{
			*ppucCurSrc = pData - sStartDst; // sStartDst Biased!
			for (d = 0; d < sNumSrc; d++)
				{
				*pData++ = ppucLevel[s + sStartSrc][d + sStartDst];
				}
			}
		}

	// Create the minimum possible data header in BYTES:
	// Set the minimum cut off at one half the minimum alpha level. (MUST ROUND UP!)
	pInfo[0] = UCHAR((*m_pLevelOpacity + 1)>>1); // The minimum non-fully-transparent alpha value
	pInfo[1] = UCHAR(sNumSrc);  // from this you can calculate remaining values
	pInfo[2] = UCHAR(lMemOffset & 0xff); // low order byte of mem offset
	pInfo[3] = UCHAR((lMemOffset & 0xff00) >> 8); // high order byte of mem offset
	
	if (plAlignedSize) *plAlignedSize = lTotSize;
	return pppucFastAligned;
	}

///////////////////////////////////////////////////////////////////////////
//  Delete Fast MultiAlpha ============  IMPORTANT!!!!!!!
//
//  This is the ONLY way a Fast MultiAlpha can be deleted!
//  Returns FAILURE or SUCCESS
///////////////////////////////////////////////////////////////////////////
short	RMultiAlpha::DeleteFastMultiAlpha(UCHAR ****pfmaDel)
	{
	ASSERT(*pfmaDel);

	UCHAR ***pAligned = *pfmaDel;
	long lDelta = 0;
	UCHAR* pByteAligned = (UCHAR*)pAligned;
	lDelta = pByteAligned[2] + (pByteAligned[3] << 8);

	ASSERT(lDelta < 4096); // best error checking I can do.

	free(pByteAligned - lDelta);
	*pfmaDel = NULL;

	return SUCCESS;
	}

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//  
//  Query Fast MultiAlpha -	determines optimum alpha layers for a given
//										cache.
//
///////////////////////////////////////////////////////////////////////////
//  Input:  # of source colors, # of dest colors, desired # of total bytes
//
//  Ouput:  Max # of layers (= levels - 2)
//				optional:  separate sizes for header and data
///////////////////////////////////////////////////////////////////////////
short RMultiAlpha::QueryFastMultiAlpha(short sNumSrcCol, short sNumDstCol,
													long lTotMem, long* plHeaderSize,
													long* plDataSize)
	{
	// This is for version I, naturally.
	long lLevMem;
	long lDataPerLev;
	long lHeaderPerLev;
	short sNumLev;
	
	long lHeaderMem = 1024; // ucClear & deltaMemFree16, + 255 pointer array (UCHAR**)
	lDataPerLev = long(sNumSrcCol) * sNumDstCol; // Src * Dst BYTE table
	lHeaderPerLev = 4L * sNumSrcCol; // UCHAR* array of len Src
	lLevMem = lDataPerLev + lHeaderPerLev;

	sNumLev = (lTotMem - lHeaderMem) / lLevMem;

	if (plHeaderSize) *plHeaderSize = lHeaderPerLev * sNumLev +  lHeaderMem;
	if (plDataSize) *plDataSize = lDataPerLev * sNumLev;

	return sNumLev;
	}

///////////////////////////////////////////////////////////////////////////
//	Here is a class wrapper for the Fast Multi Alpha:
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//  
//	RFastMulitAlphaWrapper
//
///////////////////////////////////////////////////////////////////////////

void	RFastMultiAlphaWrapper::Clear()
	{
	m_sAccess = FALSE;

	m_sStartSrc = 
	m_sStartDst =
	m_sNumSrc = 
	m_sNumDst = 
	m_sNumLayers = 0;
	m_sDelete = FALSE; // wil not delete data unless loaded
	}

RFastMultiAlphaWrapper::RFastMultiAlphaWrapper()
	{
	m_pppucFastMultiAlpha = NULL;
	Clear();
	}

void	RFastMultiAlphaWrapper::Erase()
	{
	if (m_sDelete == TRUE) // if I have responsibility for the data
		{
		if (m_pppucFastMultiAlpha) 
			RMultiAlpha::DeleteFastMultiAlpha(&m_pppucFastMultiAlpha);
		}

	Clear();
	}

RFastMultiAlphaWrapper::~RFastMultiAlphaWrapper()
	{
	Erase();
	}

///////////////////////////////////////////////////////////////////////////
//  
//	Attach:  After creating an FMA, give it this wrapper:
// IMPORTANT:  If you ATTACH an FMA, the wrapper will NOT delete it.
//					Only if you LOAD one will the FMA take responsibility.
//
// NOTE:  The information MUST MATCH the FMA, as there is no way to 
//			verify it!
//
///////////////////////////////////////////////////////////////////////////

short RFastMultiAlphaWrapper::Attach(UCHAR	***pppucFMA,short sStartSrc,
		short sNumSrc,short sStartDst,short sNumDst,
		short sNumLayers)
	{
	ASSERT(m_pppucFastMultiAlpha == NULL);
	ASSERT(sStartSrc >= 0);
	ASSERT(sNumSrc > 0);
	ASSERT(sStartDst >= 0);
	ASSERT(sNumDst > 0);
	ASSERT(sNumLayers > 0);

	m_pppucFastMultiAlpha = pppucFMA;

	m_sStartSrc = sStartSrc;
	m_sNumSrc = sNumSrc;
	m_sStartDst = sStartDst;
	m_sNumDst = sNumDst;
	m_sNumLayers = sNumLayers;

	m_sAccess = TRUE;
	m_sDelete = FALSE;

	return SUCCESS;
	}

///////////////////////////////////////////////////////////////////////////
//
//	pppucGetFMA:  This returns your actual FMA.  You MUST refer to it
//						directly to get the benefits.  To avoid the temptation
//						to refer to this wrapper INSTEAD of the true FMA, you
//						are only permitted to grab it ONCE.
//
///////////////////////////////////////////////////////////////////////////

UCHAR***	RFastMultiAlphaWrapper::pppucGetFMA()
	{
	ASSERT(m_pppucFastMultiAlpha);

	if (m_sAccess == TRUE) 
		{
		m_sAccess = FALSE;
		return m_pppucFastMultiAlpha;
		}

	TRACE("YOU MAY NOT EVER TRY TO GET THE FAST MULTI ALPHA MOE THAN ONCE!\n");
	TRACE("YOU MUST INSTEAD STORE THE POINTER SOMEWHERE AND REUSE IT!\n");

	ASSERT(FALSE);
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////
//
//	Save:  Save a File version of the FMA
//
///////////////////////////////////////////////////////////////////////////

short RFastMultiAlphaWrapper::Save(RFile* pf)
	{
	ASSERT(pf);
	ASSERT(m_pppucFastMultiAlpha);

	pf->Write("__FMA__");
	short sVer = 1;
	pf->Write(sVer);

	pf->Write(m_sNumLayers);
	pf->Write(m_sStartSrc);
	pf->Write(m_sNumSrc);
	pf->Write(m_sStartDst);
	pf->Write(m_sNumDst);

	// RESERVED:
	pf->Write(short(0)); 
	pf->Write(short(0));
	pf->Write(short(0));
	pf->Write(short(0));
	pf->Write(long(0));

	//======================= Write out the secret header from the FMA:
	UCHAR	*pucHeader = (UCHAR*)m_pppucFastMultiAlpha;
	pf->Write(*pucHeader); // save ucMin

	//========================== Write out the LAYER distribution list: 
	void* pvTemp = (void*)-1;
	short sLevel = -1;
	short i;

	// Assume the FIRST level has a value of ZERO!
	pf->Write(UCHAR(0));

	for (i=1;i < 256;i++) // SKIPPING the zeroth element!
		{
		if (m_pppucFastMultiAlpha[i] != pvTemp) // there's a change!
			{
			pvTemp = m_pppucFastMultiAlpha[i];
			sLevel++;
			}

		pf->Write(UCHAR(sLevel));
		}

	//==========  CALCULATE THE START OF THE DATA  ===============

	UCHAR *pucData = &(m_pppucFastMultiAlpha[*pucHeader][m_sStartSrc][m_sStartDst]);
	long lDataLen = long(m_sNumLayers) * long(m_sNumSrc) * long(m_sNumDst);

	//==========  Write out the DATA
	pf->Write(pucData,lDataLen);

	return SUCCESS;
	}

///////////////////////////////////////////////////////////////////////////
//
//	Load:  Load a File version of the FMA and CREATE a memory Version!
// NOTE:  Only by loading an FMA will the wrapper delete it on destruction.
//
///////////////////////////////////////////////////////////////////////////

short RFastMultiAlphaWrapper::Load(RFile* pf)
	{
	ASSERT(pf);
	ASSERT(m_pppucFastMultiAlpha == NULL);

	short sVer = 1;
	char	szType[32];

	pf->Read(szType);
	if (strcmp(szType,"__FMA__"))
		{
		TRACE("RFastMulitAlphaWrapper::Load:  Wrong file type!\n");
		return FAILURE;
		}

	pf->Read(&sVer);
	if (sVer != 1)
		{
		TRACE("RFastMulitAlphaWrapper::Load:  Can't load FMA version %hd!\n",sVer);
		return FAILURE;
		}

	pf->Read(&m_sNumLayers);
	pf->Read(&m_sStartSrc);
	pf->Read(&m_sNumSrc);
	pf->Read(&m_sStartDst);
	pf->Read(&m_sNumDst);

	// RESERVED:
	short sRes;
	long lRes;

	pf->Read(&sRes); 
	pf->Read(&sRes);
	pf->Read(&sRes);
	pf->Read(&sRes);
	pf->Read(&lRes);

	//---------------------- HEADER ------------------
	// Read in the secret header from the FMA:
	UCHAR	ucHeader;
	pf->Read(&ucHeader);

	//-------------------- LAYER INFO ----------------
	// Read in the LEVEL list:
	UCHAR ucLevels[256];  
	pf->Read(ucLevels,256); // we never save element zero!

	//==========  Allocate the FMA  ===============
	long lDataLen = long(m_sNumLayers) * long(m_sNumSrc) * long(m_sNumDst);

	// Assumes a fully correct this pointer!
	long lTotSize = 1024 + long(m_sNumLayers) * m_sNumSrc * (4L + m_sNumDst);
	// align it to 4096 to save cache memory!
	UCHAR*** pppucFastMem = (UCHAR***) calloc(1,lTotSize + 4095);
	if (!pppucFastMem) return FAILURE;

	// WARNING:  these arrays of pointers are 4 bytes in size, so be careful
	// how you seek!

	// I am adding on in BYTES, not words...
	UCHAR*** pppucFastAligned = (UCHAR***) ((ULONG(pppucFastMem)+4095) & (~4095) );

	// For freeing (in bytes):
	long lMemOffset = ULONG(pppucFastAligned) - ULONG(pppucFastMem);

	//==============  Idenitfy the different data sections  ===============
	// NOTE:  once debugged, remove ppucFirstSrcArray,pucData
	
	UCHAR** ppucFirstSrcArray = (UCHAR**)(pppucFastAligned + 256); // in 4 byte units!
	UCHAR** ppucCurSrc = ppucFirstSrcArray;

	// ( + 4LS bytes)
	UCHAR* pucData = (UCHAR*) (ppucFirstSrcArray + long(m_sNumLayers) * m_sNumSrc);
	UCHAR* pData = pucData;

	//=============  Insert the actual data  ==================

	pf->Read(pData,lDataLen); // one big contiguous chunck

	//=============  Populate the source table  ===============
	short sL,sS,sD;
	UCHAR	**ppucLayers[256]; // only need m_lNumLayers, but this way is fine

	for (sL = 0; sL < m_sNumLayers; sL++)
		{
		ppucLayers[sL] = ppucCurSrc - m_sStartSrc;
		for (sS = 0; sS < m_sNumSrc; sS++)
			{
			*ppucCurSrc++ = pData - m_sStartDst;// Hack test!
			for (sD = 0; sD < m_sNumDst; sD++)
				{
				pf->Read(pData++);
				}
			}
		}

	// Now populate the alpha table based on the stored level numbers:
	for (short i=1;i < 256; i++)
		{
		UCHAR ucLev = ucLevels[i];
		if ((ucLev > 0) && (ucLev <= m_sNumLayers))
			{
			pppucFastAligned[i] = ppucLayers[ucLev-1];
			}
		}

	//=============  Populate the index tables  ===============

	/*
	long lNumSrcTable = m_sNumLayers * m_sNumSrc;
	long lSrcPtrOffset = m_sNumDst;

	UCHAR* pSrcTableValue = pData - m_sStartDst; // offset base dest

	for (long i=0;i < lNumSrcTable;i++,pSrcTableValue += lSrcPtrOffset)
		{
		*ppucCurSrc++ = pSrcTableValue;
		}

	//============  Populate the alpha table  ==================
	long lAlphaPtrOffset = m_sNumSrc; // SHOULD advance by 4
	UCHAR** ppucCurPtrVal = ppucFirstSrcArray - m_sStartSrc; // offset source
	UCHAR*** pppAlphaEntry = pppucFastAligned;

	for (i=0; i < m_sNumLayers; i++,ppucCurPtrVal += lAlphaPtrOffset)
		{
		*pppAlphaEntry++ = ppucCurSrc;
		}

	*/
	// Create the internal header!

	UCHAR* pInfo = (UCHAR*)pppucFastAligned;
	// Remember offsets for each main memory structure:

	// Create the minimum possible data header in BYTES:
	pInfo[0] = ucHeader; // The minimum non-fully-transparent alpha value
	pInfo[1] = UCHAR(m_sNumSrc);  // from this you can calculate remaining values
	pInfo[2] = UCHAR(lMemOffset & 0xff); // low order byte of mem offset
	pInfo[3] = UCHAR(lMemOffset / 256); // high order byte of mem offset

	m_pppucFastMultiAlpha = pppucFastAligned;  // INSTALL!
	m_sAccess = TRUE;
	m_sDelete = TRUE; // NOW the wrapper will delete itself

	return SUCCESS;
	}

///////////////////////////////////////////////////////////////////////////
//
//	IsValid:  Check the source Image's color Range to see if it can be used
// (not a real time operation)
//
///////////////////////////////////////////////////////////////////////////

short RFastMultiAlphaWrapper::IsSrcValid(RImage* pimSrc)
	{
	ASSERT(pimSrc);
	ASSERT(pimSrc->m_sWidth);
	ASSERT(pimSrc->m_sHeight);

	if (pimSrc->m_type != RImage::BMP8) return FALSE;

	short i,j,sStartSrc,sLastSrc;
	GetSrcRange(&sStartSrc,&sLastSrc);

	// The ever so standard 2d memory loop:
	UCHAR* pDst,*pDstLine = pimSrc->m_pData;
	long lP = pimSrc->m_lPitch;
	short	sW = pimSrc->m_sWidth;

	for (j = pimSrc->m_sHeight;j; j--,pDstLine += lP)
		{
		pDst = pDstLine;
		for (i = sW;i;i--,pDst++)
			{
			UCHAR ucPix = *pDst;
			if ((ucPix < sStartSrc) || (ucPix > sLastSrc)) return FALSE;
			}
		}

	return TRUE;
	}