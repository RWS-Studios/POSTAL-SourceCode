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
//////////////////////////////////////////////////////////////////////
//
// LAYMAGE.CPP
//
// Created on	10/04/96 BRH
// Implemented 10/04/96 BRH
//
//	10/04/96 BRH	Started this class for use in several utilities
//						dealing with multiple layer Photoshop images.
//						This class loads a Photoshop file and creates
//						a CImage for each layer and provides way of 
//						accessing specific layers.
//
// 10/10/96 BRH	Put in basic support which includes loading a
//						layered Photoshop file and converting it to
//						a CLaymage in memory.  GetLayer functions provide
//						access to the layers either by name or number, 
//						and GetLayerName will give the names of a
//						specific layer number.
//
// 10/11/96 BRH	Added SetPSD function which instead of loading
//						all of the layers as LoadPSD does, just reads in
//						the layer names, and saves file positions for
//						the layers header section and layers channel
//						data section.  Then GetLayer is called, if the
//						layers are all in memory because of LoadPSD, 
//						then it will return the pointer to that CImage
//						layer.  If SetPSD was called, then it will read
//						from the Photoshop, the requested layer and
//						create the CImage.  Once the user is done with
//						a layer, they can call FreeLayer() with the layer
//						name or number and it will free that CImage.  
//						These features were added to avoid problems with
//						large Photoshop images which expanded to fill all
//						available RAM.  
//
//	10/22/96	JMI	Moved #include <stdlib.h> and #include <string.h> to
//						before #include "System.h".               
//
//	10/28/96 MJR	Minor tweaks to make it compile correctly and with
//						warnings under CodeWarrior.  No functional changes.
//
//	11/01/96 BRH	Changed CLaymage to RLaymage to conform to new
//						RSPiX class names.
//
//	11/01/96	JMI	Changed:
//						Old label:		New label:
//						=========		=========
//						ENDIAN_BIG		RFile::BigEndian
//						ENDIAN_LITTLE	RFile::LittleEndian
//
//						Also, changed all members referenced in RImage to
//						m_ and all position/dimension members referenced in
//						RImage to type short usage.
//
//	02/13/97 MJR	Added Reset() to reset object back to its initial
//						freshly-constructed state.
//
//						Fixed a problem having to do with single-layer
//						files.  The code was assuming that the layer-related
//						section always exists, but it doesn't for files with
//						just one layer.
//
//	02/27/97 BRH	Fixed bug in ReadLayerInfo where the channel IDs
//						were being read into only the last Channel ID, 
//						causing many channels to be skipped.  Also 
//						ConvertToImage now initializes the image data buffer
//						to white pixels that are transparent (in Photoshop
//						fashion) to match Photoshop layers that may contain
//						a rectangular area that is smaller than the full
//						image size.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "ORANGE/File/file.h"
	#include "ORANGE/Laymage/laymage.h"
	#include "BLUE/Blue.h"
#else
	#include "Image.h"
	#include "FILE.H"
	#include "LAYMAGE.H"
	#include "Blue.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////
//
// Constructor
//
// Description:
//		General constructor for RLaymage for initializing member variables
//
// Parameters:
//		none
//
//	Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RLaymage::RLaymage()
{
	short i;

	for (i = 0; i < LAYMAGE_MAXCHANNELS; i++)
		m_pcChannels[i] = NULL;

	m_sNumLayers = 0;
	m_apImages = NULL;
	m_apszLayerNames = NULL;
	m_lTellLayers = 0;
	m_lTellChannels = 0;
} 

//////////////////////////////////////////////////////////////////////
//
// Destructor
//
// Description:
//		Deallocates memory for RImage layers and layer names
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

RLaymage::~RLaymage()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////
//
// Reset
//
// Description:
//		Resets object back to its freshly-constructed state
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void RLaymage::Reset()
{
	ClearChannelBuffers();	
	FreeAllLayers();
	FreeLayerArrays();

	m_sNumLayers = 0;
	m_apImages = NULL;
	m_apszLayerNames = NULL;
	m_lTellLayers = 0;
	m_lTellChannels = 0;
}

//////////////////////////////////////////////////////////////////////
//
// ClearChannelBuffers
//
// Description:
//		Deallocate all channel buffers and reset the pointers
//
// Parameters:
//		none
// 
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void RLaymage::ClearChannelBuffers(void)
{
	short i;

	for (i = 0; i < LAYMAGE_MAXCHANNELS; i++)
		if (m_pcChannels[i])
		{
			free(m_pcChannels[i]);
			m_pcChannels[i] = NULL;
		}
}

//////////////////////////////////////////////////////////////////////
//
// AllocateChannelBuffers
//
// Description:
//		Allocate the 4 standard channel buffers, Red, Green, Blue, and
//		Alpha.  
//
// Parameters:
//		Number of bytes to be allocated for each buffer
//
// Returns:
//		SUCCESS if all buffers were allocated
//		FAILURE if memory was not allocated for the buffers
//
//////////////////////////////////////////////////////////////////////

short RLaymage::AllocateChannelBuffers(ULONG ulSize)
{
	short sReturn = SUCCESS;

	ClearChannelBuffers();

	m_pcChannels[LAYMAGE_RED] = (char*) calloc(ulSize, 1);
	m_pcChannels[LAYMAGE_GREEN] = (char*) calloc(ulSize, 1);
	m_pcChannels[LAYMAGE_BLUE] = (char*) calloc(ulSize, 1);
	m_pcChannels[LAYMAGE_ALPHA] = (char*) calloc(ulSize, 1);

	if (m_pcChannels[LAYMAGE_RED] &&
	    m_pcChannels[LAYMAGE_GREEN] &&
		 m_pcChannels[LAYMAGE_BLUE] &&
		 m_pcChannels[LAYMAGE_ALPHA])
		sReturn = SUCCESS;
	else
	{
		sReturn = FAILURE;
		ClearChannelBuffers();
	}
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// SetPSD
//
// Description:
//		Opens the given Photoshop file and reads it layer names and
//		saves poistions to key points in the Photoshop file.  It 
//		does not load any layers.  When the user requests a layer
//		using GetLayer(), it will load that layer from this Photoshop
//		file.
//
// Parameters:
//		pszFilename = filename of Photoshop file (.PSD) to be set
//
// Returns:
//		SUCCESS if the file was loaded and set up
//		FAILURE otherwise - TRACE information gives failure
//
//////////////////////////////////////////////////////////////////////

short RLaymage::SetPSD(char* pszFilename)
{
	RFile cf;
	RFile cfChannel;
	short sReturn = SUCCESS;
	short i;

	strcpy(m_szPhotoshopFilename, pszFilename);

	if (ReadPSDHeader(pszFilename) != SUCCESS)
	{
		TRACE("RLaymage::SetPSD - Failed - header could not be read\n");
		goto error;		
	}

	if (cf.Open(pszFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::SetPSD - Error cannot open file %s\n", pszFilename);
		goto error;
	}

	if (cfChannel.Open(pszFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::SetPSD - Error cannot open channel file %s\n", pszFilename);
		goto error;
	}	

	cf.Seek(m_lTellLayers, SEEK_SET);
	cfChannel.Seek(m_lTellChannels, SEEK_SET);

	i = 0;
	while (i < m_sNumLayers)
		ReadLayerName(i++, &cf);

	cf.Close();
	cfChannel.Close();
	return SUCCESS;

error:
	cf.Close();
	cfChannel.Close();
	return FAILURE;
}

//////////////////////////////////////////////////////////////////////
//
// LoadPSD
//
// Description:
//		Load the given Photoshop file and convert its layers into a
//		series of RImages.
//
// Parameters:
//		pszFilename = filename of Photoshop file (.PSD) to be loaded
//
// Returns:
//		SUCCESS if the file was loaded and converted
//		
//////////////////////////////////////////////////////////////////////

short RLaymage::LoadPSD(char* pszFilename)
{
	RFile cf;
	RFile cfChannel;
	short sReturn = SUCCESS;
	short i;

	strcpy(m_szPhotoshopFilename, pszFilename);

	if (ReadPSDHeader(pszFilename) != SUCCESS)
	{
		TRACE("RLaymage::LoadPSD - Failed - header could not be read\n");
		goto error;		
	}

	if (cf.Open(pszFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::LoadPSD - Error cannot open file %s\n", pszFilename);
		goto error;
	}

	if (cfChannel.Open(pszFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::LoadPSD - Error cannot open channel file %s\n", pszFilename);
		goto error;
	}	

	cf.Seek(m_lTellLayers, SEEK_SET);
	cfChannel.Seek(m_lTellChannels, SEEK_SET);

	i = 0;
	while (i < m_sNumLayers)
		ReadLayerInfo(i++, &cf, &cfChannel);

	cf.Close();
	cfChannel.Close();
	return SUCCESS;

error:
	cf.Close();
	cfChannel.Close();
	return FAILURE;
}

//////////////////////////////////////////////////////////////////////
//
// ReadPSDHeader
//
// Description:
//		Reads in the header for the Photoshop file and sets the 
//		width, height, number of layers and the file positions to
//		the beginning of the layers header information, and the 
//		beginning of the channel data.  This routine can be called
//		from either LoadPSD or SetPSD to get this information.  SetPSD
//		will then just read the names of the layers, and LoadPSD
//		will call the ReadLayerInfo for each layer.
//
// Parameters:
//		pszFilename = name of the Photoshop header to read
//
// Returns:
//		SUCCESS if the file was opened and header was read successfully
//		FAILURE if there was an error - TRACE messages will help
//				  pinpoint the error.
//
//////////////////////////////////////////////////////////////////////

short RLaymage::ReadPSDHeader(char* pszFilename)
{
	RFile cf;
	RFile cfChannel;
	short sReturn = SUCCESS;
	ULONG ulData;
	USHORT usData;

	if (cf.Open(pszFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::ReadPSDHeader - Error cannot open file %s\n", pszFilename);
		goto error;
	}

	if (cfChannel.Open(pszFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::ReadPSDHeader - Error cannot open channel file %s\n", pszFilename);
		goto error;
	}	
	
	//-----------------------------------------------------------------
	// Read in Photoshop header
	//-----------------------------------------------------------------
	
	// Read file signature to make sure it is a Photoshop file
	cf.Read(&ulData);
	if (ulData != LAYMAGE_PHOTOSHOP_SIGNATURE)
	{
		TRACE("RLaymage::ReadPSDHeader - Error - File signature is not 8BPS\n");
		goto error;
	}			

	// Read version number and make sure it matches
	cf.Read(&usData);
	if (usData != 1)
	{
		TRACE("RLaymage::ReadPSDHeader - Error - File version is not 1\n");
		goto error;
	}

	// Skip over reserved space
	cf.Seek(6, SEEK_CUR);

	// Read number of channels
	cf.Read(&m_sSpecialNumChannels);

	// Read image size
	cf.Read(&m_lHeight);
	cf.Read(&m_lWidth);

	// Skip Depth of channels field
	cf.Seek(2, SEEK_CUR);

	// Read mode and make sure it is RGB
	cf.Read(&usData);
	if (usData != 3)
	{
		TRACE("RLaymage::ReadPSDHeader - Error - file not in RGB color mode\n");
		goto error;
	}	

	// Read the size of the color mode data section and skip it
	cf.Read(&ulData);
	cf.Seek(ulData, SEEK_CUR);

	// Read the size of the image resource section and skip it
	cf.Read(&ulData);
	cf.Seek(ulData, SEEK_CUR);

	// Read the size of the misc info section.  If the section exists,
	// then the image probably has more than one layer.  If it doesn't,
	// then the image has just one layer.
	cf.Read(&ulData);
	if (ulData > 0)
		{
		// This file has layer info
		m_sHasLayerInfo = 1;

		// Read the size of the layers section
		cf.Read(&ulData);

		// Get the number of layers (if negative, get absolute value)
		cf.Read(&m_sNumLayers);
		if (m_sNumLayers < 0)
			m_sNumLayers = -m_sNumLayers;

		// Read through the Layer info once to find the end of the
		// headers and the beginning of the channel data.  
		cfChannel.Seek(cf.Tell(), SEEK_SET);
		SetChannelPointer(m_sNumLayers, &cfChannel);
		}
	else
		{
		// This file has no layer info
		m_sHasLayerInfo = 0;

		// There's only one layer
		m_sNumLayers = 1;

		// The misc info section is empty, and what follows is basically
		// the channel data for the one and only layer.
		cfChannel.Seek(cf.Tell(), SEEK_SET);
		}
	m_lTellLayers = cf.Tell();
	m_lTellChannels = cfChannel.Tell();

	// Allocate RImage and name arrays for each layer
	if (AllocateLayerArrays(m_sNumLayers) != SUCCESS)
		{
		TRACE("RLaymage::ReadPSDHeader - Error allocating layer arrays\n");
		goto error;
		}

	cf.Close();
	cfChannel.Close();
	return SUCCESS;

error:
	cf.Close();
	cfChannel.Close();
	return FAILURE;
}

//////////////////////////////////////////////////////////////////////
//
// ReadLayer
//
// Description:
//		This function is called by GetLayer when the layer requested
//		is not currently loaded into memory and needs to be retrieved
//		from the Photoshop file.  It interfaces with ReadLayerInfo
//		by opening the Layer and Channel files and setting them to the
//		saved locations.  Then it loops through the layers, reading each
//		one in order to parse the file.  If the layer it just read
//		in the loop was not the requested layer, then it frees that
//		layer and reads the next one until it finds the one it wants.
//
// Parameters
//		sRequestedLayer = The number of the layer to be set in memory
//
// Returns:
//		SUCCESS if the layer was found and read
//		FAILURE if there was an error
//
//////////////////////////////////////////////////////////////////////

short RLaymage::ReadLayer(short sRequestedLayer)
{
	RFile cf;
	RFile cfChannel;
	short sReturn = SUCCESS;
	short i;

	if (cf.Open(m_szPhotoshopFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::ReadLayer - Error cannot open file %s\n", m_szPhotoshopFilename);
		goto error;
	}

	if (cfChannel.Open(m_szPhotoshopFilename, "rb", RFile::BigEndian) != SUCCESS)
	{
		TRACE("RLaymage::ReadPSDHeader - Error cannot open channel file %s\n", m_szPhotoshopFilename);
		goto error;
	}	
	
	cf.Seek(m_lTellLayers, SEEK_SET);
	cfChannel.Seek(m_lTellChannels, SEEK_SET);

	i = 0;
	while (i < sRequestedLayer)
	{
		if (ReadLayerInfo(i, &cf, &cfChannel) != SUCCESS)
			goto error;
		FreeLayer(i);
		i++;
	}
	if (i == sRequestedLayer)
		if (ReadLayerInfo(i, &cf, &cfChannel) != SUCCESS)
			goto error;

	cf.Close();
	cfChannel.Close();
	return SUCCESS;

error:
	cf.Close();
	cfChannel.Close();
	return FAILURE;
}

//////////////////////////////////////////////////////////////////////
//
// SetChannelPointer
//
// Description:
//		Private function to set a second file pointer to the 
//		beginning of the Channel data so that two pointers 
//		working together can read both the layer header information
//		and the data.  This routine gets an open RFile pointer 
//		at the start of the layer header information and it will
//		skip through the header information for all of the layers
//		and will be left at the beginning of the channel data for
//		the first layer.  
//
// Parameters:
//		sNumLayers = number of layers to be skipped
//		pcfChannel = RFile pointer to open Photoshop file at
//						 the beginning of the Layers header section
//						 which will be moved to the beginning of the
//						 channel data for the layers.
//
// Returns:
//		SUCCESS if the pointer was skipped successfully
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RLaymage::SetChannelPointer(short sNumLayers, RFile* pcfChannel)
{
	ULONG ulData;
	USHORT usNumChannels = 0;
	USHORT i;
	UCHAR ucData;
	short sReturn = FAILURE;
	short sLayer;

	if (pcfChannel && pcfChannel->IsOpen())
	{
		pcfChannel->ClearError();

		for (sLayer = 0; sLayer < sNumLayers; sLayer++)
		{
			// Skip over 4 longs of layer bounding rectangle
			pcfChannel->Seek(16, SEEK_CUR);

			pcfChannel->Read(&usNumChannels);

			// For each channel skip over chennel ID and length of data
			for (i = 0; i < usNumChannels; i++)
				pcfChannel->Seek(6, SEEK_CUR);

			// Skip over blend mode signature, blend info, opacity, clip,
			// flags, and extra data size
			pcfChannel->Seek(16, SEEK_CUR);

			// Get size of layer mask data and skip over it
			pcfChannel->Read(&ulData);
			pcfChannel->Seek(ulData, SEEK_CUR);

			// Get size of blending ranges and skip over it
			pcfChannel->Read(&ulData);
			pcfChannel->Seek(ulData, SEEK_CUR);

			// Get the pascal formatted layer name string
			// and skip over it
			pcfChannel->Read(&ucData);
			char* pszName = (char*) malloc(ucData);
			pcfChannel->Read(pszName, ucData);
			free (pszName);

			// The name should be padded to a multiple of 4 bytes
			// so skip the pad bytes if any.
			if ((ucData+1) % 4)
				pcfChannel->Seek((4-((ucData+1) % 4)), SEEK_CUR);
		}

		if (!pcfChannel->Error())	
			sReturn = SUCCESS;
	}

	return sReturn;	
}

//////////////////////////////////////////////////////////////////////
//
// ReadLayerInfo
//
// Description:
//		This is called for each layer.  It creates a new RImage
//		layer, reads the Photoshop data for the layer, and
//		converts it to a 32-bit ARGB format RImage
//
// Parameters:
//		sLayerNum = current layer number
//		pcfLayer = pointer to layer header portion of file
//		pcfChannel = pointer to channel data portion of file
//
// Returns:
//		SUCCESS if layer was loaded and converted successfully
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RLaymage::ReadLayerInfo(short sLayerNum, RFile* pcfLayer, 
                              RFile* pcfChannel)
{
	ULONG ulData;
	USHORT usData;
	UCHAR ucData;
	short asChannelID[LAYMAGE_MAXCHANNELS];
	USHORT usNextChannel = 0;
	USHORT usNumChannels = 0;
	ULONG ulTop;
	ULONG ulBottom;
	ULONG ulLeft;
	ULONG ulRight;
	USHORT k;
	USHORT i;
	short sReturn = SUCCESS;

	pcfLayer->ClearError();
	pcfChannel->ClearError();
	
	if (m_sHasLayerInfo)
		{
		// Read the bounding rectangle for this layer
		pcfLayer->Read(&ulTop);
		pcfLayer->Read(&ulLeft);
		pcfLayer->Read(&ulBottom);
		pcfLayer->Read(&ulRight);

		// Read number of channels
		pcfLayer->Read(&usNumChannels);

		// Get channel ID's (and skip channel sizes)
		for (i = 0; i < usNumChannels; i++)
			{
			pcfLayer->Read(&asChannelID[i]);
			pcfLayer->Read(&ulData);
			}

		// Skip blend mode signature, blend info, opacity, clip, flags and extra data size
		pcfLayer->Seek(16, SEEK_CUR);

		// Get size of layer mask data and skip over it
		pcfLayer->Read(&ulData);
		pcfLayer->Seek(ulData, SEEK_CUR);

		// Get size of layer blending ranges and skip over it
		pcfLayer->Read(&ulData);
		pcfLayer->Seek(ulData, SEEK_CUR);

		// Get the pascal formatted layer name string
		// Get the length of the string
		pcfLayer->Read(&ucData);
		// If the layer name has already been read, seek over it and
		// leave the string alone.
		if (m_apszLayerNames[sLayerNum])
			pcfLayer->Seek(ucData, SEEK_CUR);
		else
			{
			m_apszLayerNames[sLayerNum] = new char[ucData+1];
			pcfLayer->Read(m_apszLayerNames[sLayerNum], ucData);
			m_apszLayerNames[sLayerNum][ucData] = 0;
			}
		// Skip to multiple of 4 following string
		if ((ucData+1) % 4)
			pcfLayer->Seek((4 - (ucData+1) % 4), SEEK_CUR);

		// Allocate 4 channel buffers for Alpha, R, G, and B channels
		if (AllocateChannelBuffers((ulBottom-ulTop)*(ulRight-ulLeft)) == SUCCESS)
			{

			// Read the channel data into buffers
			short sChannelSel;
			for (k = 0; k < usNumChannels; k++)
				{
				// If the channel is Alpha or R G or B, read the data
				if (asChannelID[k] > -2 && asChannelID[k] < 3)
					{
					switch (asChannelID[k])
						{
						case -1:
							sChannelSel = LAYMAGE_ALPHA;
							break;
						case 0:
							sChannelSel = LAYMAGE_RED;
							break;
						case 1:
							sChannelSel = LAYMAGE_GREEN;
							break;
						case 2:
							sChannelSel = LAYMAGE_BLUE;
							break;
						default:
							sChannelSel = LAYMAGE_IGNORE;
							break;
						}

					// Read compression type
					pcfChannel->Read(&usData);
					if (usData == 0)
						{
						// Read RAW uncompressed data
						if (!m_pcChannels[sChannelSel])
							TRACE("RLaymage::ReadLayerInfo - Error allocating memory for channel buffer\n");
						else
							pcfChannel->Read(m_pcChannels[sChannelSel], (ulBottom-ulTop)*(ulRight-ulLeft));
						}
					else
						{
						// Read RLE compressed data
						USHORT m;
						ULONG ulCompSize = 0;
						for (m = 0; m < ulBottom-ulTop; m++)
							{
							pcfChannel->Read(&usData);
							ulCompSize += usData;
							}
						if (!m_pcChannels[sChannelSel])
							TRACE("RLaymage::ReadLayerInfo - Error allocating memory for channel buffer\n");
						else
							RLE_Decompress(m_pcChannels[sChannelSel], ulCompSize, pcfChannel);
						}
					}
				else
					{
					// discard the channel

					// Read compression type
					pcfChannel->Read(&usData);
					if (usData == 0)
						{
						// If uncompressed, skip the size of the image given by
						// the bounding rectangle
						pcfChannel->Seek((ulBottom-ulTop)*(ulRight-ulLeft), SEEK_CUR);
						}
					else
						{
						// If compressed, the length of each line is stored at
						// this position in the file.  Get the total size of 
						// compressed data and then skip over it.
						USHORT j;
						ULONG ulSkip = 0;
						for (j = 0; j < ulBottom - ulTop; j++)
							{
							pcfChannel->Read(&usData);
							ulSkip += usData;
							}
						pcfChannel->Seek(ulSkip, SEEK_CUR);
						}						
					}
				}

			// Convert color planes into a 32-bit ARGB RImage
			ConvertToImage(sLayerNum, ulTop, ulBottom, ulLeft, ulRight);
			}
		else
			{
			sReturn = FAILURE;
			TRACE("RLaymage::ReadLayerInfo - Error allocating channel buffers\n");
			}
		}
	else
		{
		// If the layer name hasn't been set, set it to "background" to match the photoshop default name
		if (m_apszLayerNames[sLayerNum] == 0)
			{
			m_apszLayerNames[sLayerNum] = new char[20+1];
			strcpy(m_apszLayerNames[sLayerNum], "background");
			}

		// Allocate 4 channel buffers for Alpha, R, G, and B channels
		if (AllocateChannelBuffers(m_lWidth * m_lHeight) == SUCCESS)
			{

			// Read compression type
			USHORT usComp;
			pcfChannel->Read(&usComp);

			// Read the channel data into buffers.  The order of the channels seems to be fixed in this
			// type of file: RED, GREEN, BLUE, followed by any user-defined channels, and we assume the
			// first one after BLUE is ALPHA.
			if (usComp == 0)
				{
				// Read RAW uncompressed data for each channel
				pcfChannel->Read(m_pcChannels[LAYMAGE_RED], m_lWidth * m_lHeight);
				pcfChannel->Read(m_pcChannels[LAYMAGE_GREEN], m_lWidth * m_lHeight);
				pcfChannel->Read(m_pcChannels[LAYMAGE_BLUE], m_lWidth * m_lHeight);
				pcfChannel->Read(m_pcChannels[LAYMAGE_ALPHA], m_lWidth * m_lHeight);
				}
			else
				{
				// Read RLE compressed data.  This starts with a byte count for every row of every
				// channel (in other words, there are row * channel number of byte counts) followed
				// by the compressed data for all the rows.  Our compression is oriented towards
				// doing each channel separately, so we have to jump around in the file alot.
				ULONG ulCompSize[4] = { 0, 0, 0, 0 };
				for (k = 0; k < 4; k++)
					{
					for (USHORT m = 0; m < m_lHeight; m++)
						{
						pcfChannel->Read(&usData);
						ulCompSize[k] += usData;
						}
					}

				RLE_Decompress(m_pcChannels[LAYMAGE_RED], ulCompSize[0], pcfChannel);
				RLE_Decompress(m_pcChannels[LAYMAGE_GREEN], ulCompSize[1], pcfChannel);
				RLE_Decompress(m_pcChannels[LAYMAGE_BLUE], ulCompSize[2], pcfChannel);
				RLE_Decompress(m_pcChannels[LAYMAGE_ALPHA], ulCompSize[3], pcfChannel);
				}

			// Convert color planes into a 32-bit ARGB RImage
			ConvertToImage(sLayerNum, 0, m_lHeight, 0, m_lWidth);
			}
		else
			{
			sReturn = FAILURE;
			TRACE("RLaymage::ReadLayerInfo - Error allocating channel buffers\n");
			}
		}

	ClearChannelBuffers();

	return sReturn;
	}

//////////////////////////////////////////////////////////////////////
//
// ReadLayerName
//
// Description:
//		Read the name of the given layer.  This will be called in
//		a loop much like ReadLayerInfo, but it only gets the names, 
//		it does not read the channel data.  
//
// Parameters:
//		sLayer = layer number to read
//		pcfLayer = pointer to open RFile at layer header
//
// Returns:
//		SUCCESS if read successfully
//		FAILURE if there was an error
//
//////////////////////////////////////////////////////////////////////

short RLaymage::ReadLayerName(short sLayerNum, RFile* pcfLayer)
{
	ULONG ulData;
	USHORT usData;
	UCHAR ucData;
	ULONG* pulChannelLength = NULL;
	short* psChannelID = NULL;
	USHORT usNextChannel = 0;
	USHORT usNumChannels = 0;
	ULONG ulTop;
	ULONG ulBottom;
	ULONG ulLeft;
	ULONG ulRight;
	USHORT i;
	short sReturn = SUCCESS;

	pcfLayer->ClearError();
	
	// Read the bounding rectangle for this layer
	pcfLayer->Read(&ulTop);
	pcfLayer->Read(&ulLeft);
	pcfLayer->Read(&ulBottom);
	pcfLayer->Read(&ulRight);
	pcfLayer->Read(&usNumChannels);

	pulChannelLength = (ULONG*) calloc(usNumChannels, sizeof(ULONG));
	psChannelID = (short*) calloc(usNumChannels, sizeof(short));
	if (pulChannelLength == NULL || psChannelID == NULL)
	{
		TRACE("RLaymage::ReadLayerInfo - Error allocating buffers for channel data\n");
		sReturn = FAILURE;
	}

	for (i = 0; i < usNumChannels; i++)
	{
		// Get channel ID
		pcfLayer->Read(&usData);
		psChannelID[i] = usData;
		pcfLayer->Read(&ulData);
		pulChannelLength[usNextChannel++] = ulData;
	}

	// Read the blend mode signature which is always 8BIM
	pcfLayer->Read(&ulData);

	// Skip Blend info, opacity, clip, flags and extra data size
	pcfLayer->Seek(12, SEEK_CUR);

	// Get size of layer mask data and skip over it
	pcfLayer->Read(&ulData);
	pcfLayer->Seek(ulData, SEEK_CUR);

	// Get size of layer blending ranges and skip over it
	pcfLayer->Read(&ulData);
	pcfLayer->Seek(ulData, SEEK_CUR);

	// Get the pascal formatted layer name string
	// Get the length of the string
	pcfLayer->Read(&ucData);
	m_apszLayerNames[sLayerNum] = new char[ucData+1];
	pcfLayer->Read(m_apszLayerNames[sLayerNum], ucData);
	m_apszLayerNames[sLayerNum][ucData] = 0;
	// Skip to multiple of 4 following string
	if ((ucData+1) % 4)
		pcfLayer->Seek((4 - (ucData+1) % 4), SEEK_CUR);

	// Allocate 4 channel buffers for Alpha, R, G, and B channels
	if (AllocateChannelBuffers((ulBottom-ulTop)*(ulRight-ulLeft)) != SUCCESS)
		TRACE("RLaymage::ReadLayerInfo - Error allocating channel buffers\n");

	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// RLE_Decompress
//
// Description:
//		Decompress the RLE data using the algroithm from the Mac's
//		PackBits/UnpackBits routines.  This function gets the file
//		pointer at the beginning of the RLE data, the size of the
//		compressed data to read (so it knows when to stop), and the
//		buffer that is already allocated large enough to hold the
//		decompressed data.
//
// Input:
//		char* pcBuffer = pointer to buffer large enough for the
//							  decompressed data.
//		ulCompSize = size of compressed data
//		pcf = RFile pointer at the start of compressed data
//
// Output:
//		pcBuffer is filled with decompressed data
//
// Returns:
//		SUCCESS if decompressed successfully
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RLaymage::RLE_Decompress(char* pcBuffer, ULONG ulCompSize, RFile* pcfRLE)
{
	short sReturn;
	ULONG ulRead = 0;
	ULONG ulBufferPos = 0;
	ULONG ulBufferFill = 0;
	signed char cData;
	signed char cFlag;
	UCHAR ucRun;
	USHORT i;

	if (pcfRLE && pcfRLE->IsOpen())
	{
		pcfRLE->ClearError();

		while (ulRead < ulCompSize)
		{
			pcfRLE->Read(&cFlag);
			ulRead++;
			if (cFlag < 0)
			{
				if (cFlag == 0x80)
				{
					pcfRLE->Read(&cData);
					ulRead++;
				}
				else
				{
					// Calculate number of a single byte pattern
					ucRun = -cFlag + 1;
					// Read the pattern
					pcfRLE->Read(&cData);
					ulRead++;
					for (i = 0; i < ucRun; i++)
						pcBuffer[ulBufferPos++] = cData;
					ulBufferFill += ucRun;
				}
			}
			else
			{
				// Read number of discrete bytes that follow
				ucRun = cFlag + 1;
				pcfRLE->Read(&(pcBuffer[ulBufferPos]), ucRun);
				ulBufferPos += ucRun;
				ulRead += ucRun;
				ulBufferFill += ucRun;			
			}	
		}

		if (pcfRLE->Error())
			sReturn = FAILURE;
		else
			sReturn = SUCCESS;
	}
	else
		sReturn = FAILURE;


	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// ConvertToImage
//
// Description:
//		Takes the 4 channels of Photoshop data and converts it to a
//		32-bit ARGB RImage.  The Photoshop layers are saved not as
//		the whole image size, but a bounding rectangle that contains
//		information.  So the bounding rectangle is passed in and will
//		be mapped on to a full size RImage layer.
//
// Parameters:
//		ulTop, ulBottom, ulLeft, ulRight = bounding rectangle for
//													  the Photoshop data
//
// Returns:
//		SUCCESS if converted successfully
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RLaymage::ConvertToImage(short sLayerNum, ULONG ulTop, ULONG ulBottom, 
                               ULONG ulLeft, ULONG ulRight)
{
	short sReturn = SUCCESS;
	RImage* pImage;
	ULONG row;
	ULONG col;
	ULONG i;
	ULONG ulPixel;

	pImage = m_apImages[sLayerNum] = new RImage(m_lWidth*m_lHeight*4);
	if (pImage)
	{
		// Initialize buffer to Photoshop's style of blank which is 
		// white pixels that are totally transparent.  This way if we read
		// in a layer with a rectangle smaller than the total picture size, 
		// the outer regions will match.
		U32* pu32 = (U32*) pImage->m_pData;
		for (i = 0; i < pImage->m_ulSize / 4; i++)
			pu32[i] = 0x00ffffff;
		pImage->m_type = pImage->m_typeDestination = RImage::SCREEN32_ARGB;
		pImage->m_sWidth = (short)m_lWidth;
		pImage->m_sHeight = (short)m_lHeight;	
		pImage->m_sDepth = 32;
		pImage->m_lPitch = (long)pImage->m_sWidth * (pImage->m_sDepth/8);

		ULONG* ulp32 = (ULONG*) pImage->m_pData;
		long lDestPitch = pImage->m_lPitch / 4;

		i = 0;
		for (row = ulTop; row < ulBottom; row++)
			for (col = ulLeft; col < ulRight; col++)
			{
				ulPixel = ((UCHAR) m_pcChannels[LAYMAGE_ALPHA][i]) * 0x01000000 |
							 ((UCHAR) m_pcChannels[LAYMAGE_RED][i])   * 0x00010000 |
							 ((UCHAR) m_pcChannels[LAYMAGE_GREEN][i]) * 0x00000100 |
							 ((UCHAR) m_pcChannels[LAYMAGE_BLUE][i]);
				i++;

				ulp32[row*lDestPitch + col] = ulPixel;
			}
	}


	return sReturn;
}
//////////////////////////////////////////////////////////////////////
//
// Load
//
// Description:
//		The load functions take either a filename or an open RFile 
//		pointer and begin loading a CLamage file (,IML)
//
// Parameters:
//		pszFilename = filename of the .IML file to be loaded - OR - 
//		CFile* pcf = pointer to open RFile where image loading starts
//
// Returns:
//		SUCCESS if the file was loaded successfully
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RLaymage::Load(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "rb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RLaymage::Load - could not open file %s\n", pszFilename);
		return FAILURE;
	}

	sReturn = Load(&cf);

	cf.Close();

	return sReturn;
}

short RLaymage::Load(RFile* pcf)
{
	short sReturn = SUCCESS;
	ULONG ulFileType = 0;
	ULONG ulVersion = 0;

	if (pcf && pcf->IsOpen())
	{
		if (pcf->Read(&ulFileType) == 1)
		{
			if (ulFileType == LAYMAGE_COOKIE)
			{
				if (pcf->Read(&ulVersion) == 1)
				{
					if (ulVersion == LAYMAGE_CURRENT_VERSION)
					{
						// Read number of layers and allocate memory

						// Read layer names

						// Use RImage to load the images
					}
					else
					{
						TRACE("RLaymage::Load - Error: File version is %d Current code version is %d\n", ulVersion, LAYMAGE_CURRENT_VERSION);
						sReturn = FAILURE;
					}
				}
				else
				{
					TRACE("RLaymage::Load - Error reading file version\n");
					sReturn = FAILURE;
				}
			}
			else
			{
				TRACE("RLaymage::Load - Error: File typs is not \"IML \"\n");
				sReturn = FAILURE;
			}
		}
		else
		{
			TRACE("RLaymage::Load - Error reading file type\n");
			sReturn = FAILURE;
		}
	}
	else
	{
		TRACE("RLaymage::Load - Error RFile pointer does not refer to an open file\n");
		sReturn = FAILURE;
	}

	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// Save
//
// Description:
//		These functions save the RLaymage as its own file type (.IML)
//		They save the layer names and use the RImage save to save each
//		Image layer.  One version takes a filename to save and the other
//		version takes a pointer to an open RFile and writes the data where
//		it is.
//
// Parameters:
//		char* pszFilename = filename of the .IML file you wish to save - OR -
//		RFile* pcf = pointer to an open RFile where the data will be saved
//
// Returns:
//		SUCCESS if the file was saved
//		FAILURE is there was an error - 
//				  TRACE message will help pinpoint failure
//
//////////////////////////////////////////////////////////////////////

short RLaymage::Save(char* pszFilename)
{
	RFile cf;
	short sReturn = SUCCESS;

	if (cf.Open(pszFilename, "wb", RFile::LittleEndian) != SUCCESS)
	{
		TRACE("RLaymage::Save - could not open file %s for output\n", pszFilename);
		return FAILURE;
	}	

	sReturn = Save(&cf);

	cf.Close();

	return sReturn;
}

short RLaymage::Save(RFile* /*pcf*/)
{
	return FAILURE;
}

//////////////////////////////////////////////////////////////////////
//
// GetLayer
//
// Description:
//		Returns a pointer to the RImage for the requested layer.  
//		This version of the function takes a string and tries to
//		find a layer with the same name.  If it finds it, it returns
//		a pointer to the RImage for that layer.
//
// Parameters:
//		pszLayerName = requested layer
//
// Returns:
//		pointer to RImage of the requested layer if found
//		NULL if not found
//
//////////////////////////////////////////////////////////////////////

RImage* RLaymage::GetLayer(char* pszLayerName)
{
	short i = 0;
	RImage* pLayerImage = NULL;

	while (i < m_sNumLayers && pLayerImage == NULL)
	{
		if (strcmp(pszLayerName, m_apszLayerNames[i]) == 0)
		{
			pLayerImage = m_apImages[i];
			if (pLayerImage == NULL)
			{
				ReadLayer(i);
				pLayerImage = m_apImages[i];
			}
		}
		else
			i++;
	}

	return pLayerImage;
}

//////////////////////////////////////////////////////////////////////
//
// GetLayer
//
// Description:
//		Returns a pointer to the RImage for the reuested layer.
//		This version of the function takes a layer number and
//		retuns a pointer to that layer (as long as it is a valid
//		layer number).
//
// Parameters:
//		sLayerNumber = number of the requested layer
//
// Returns:
//		pointer to RImage of the requested layer if found
//		NULL if not found
//
//////////////////////////////////////////////////////////////////////

RImage* RLaymage::GetLayer(short sLayerNumber)
{
	RImage* pLayerImage = NULL;

	if (sLayerNumber >= 0 && sLayerNumber < m_sNumLayers)
	{
		pLayerImage = m_apImages[sLayerNumber];
		// If this layer is not in memory, load it in now
		if (pLayerImage == NULL)
		{
			ReadLayer(sLayerNumber);
			pLayerImage = m_apImages[sLayerNumber];
		}
	}

	return pLayerImage;
}

//////////////////////////////////////////////////////////////////////
//
// FreeLayer
//
// Description:
//		Frees the RImage for the specified layer.  This version of the
//		function takes the name of a layer and frees it as long as
//		that layer exists.
//
// Parameters
//		pszLayerName = name of layer to be freed
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void RLaymage::FreeLayer(char* pszLayerName)
{
	short i = 0;
	short bFound = FALSE;

	while (i < m_sNumLayers && !bFound)
	{
		if (strcmp(pszLayerName, m_apszLayerNames[i]) == 0)
			bFound = TRUE;
		else
			i++;
	}

	if (bFound && m_apImages[i] != NULL)
	{
			delete m_apImages[i];
			m_apImages[i] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
//
// FreeLayer
//
// Description:
//		Frees the RImage for the specified layer.  This version of the
//		function takes the layer number and frees it as long as that
//		layer exists.
//
// Parameters
//		sLayer = number of layer to be freed
//
//	Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void RLaymage::FreeLayer(short sLayerNumber)
{
	if (sLayerNumber >= 0 && sLayerNumber < m_sNumLayers)
		if (m_apImages[sLayerNumber])
		{
			delete m_apImages[sLayerNumber];
			m_apImages[sLayerNumber] = NULL;
		}
}

//////////////////////////////////////////////////////////////////////
//
// FreeAllLayers
//
// Description:
//		Frees all of the RImage layers that have been allocated
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void RLaymage::FreeAllLayers(void)
{
	short i;

	for (i = 0; i < m_sNumLayers; i++)
	{
		if (m_apImages[i])
		{
			delete m_apImages[i];
			m_apImages[i] = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////
//
// GetLayerName
//
// Description:
//		Get the name of the requested layer
//
// Parameters:
//		sLayer = Number of the layer
//		pszNameBuffer = a buffer in which the name will be copied
//
// Returns:
//		SUCCESS if the layer number given was valid
//		FAILURE if that layer number does not exist or if the
//				  buffer passed in was NULL
//
//////////////////////////////////////////////////////////////////////

short RLaymage::GetLayerName(short sLayer, char* pszNameBuffer)
{
	short sReturn = FAILURE;
	
	if (pszNameBuffer && sLayer >= 0 && sLayer < m_sNumLayers)
	{
		strcpy(pszNameBuffer, m_apszLayerNames[sLayer]);	
		sReturn = SUCCESS;
	}
		
	return sReturn;
}

//////////////////////////////////////////////////////////////////////
//
// FreeLayerArrays
//
// Description:
//		Frees the array of pointers to RImage and layer names along
//		with the things they were pointing to.  This is called to 
//		clean up the memory either by the destructor or if a new
//		file is set or loaded.
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void RLaymage::FreeLayerArrays(void)
{
	if (m_apImages)
	{
		FreeAllLayers();
		delete []m_apImages;
		m_apImages = NULL;
	}

	if (m_apszLayerNames)
	{
		short i;
		for (i = 0; i < m_sNumLayers; i++)
			if (m_apszLayerNames[i])
				delete []m_apszLayerNames[i];
		delete []m_apszLayerNames;
		m_apszLayerNames = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
//
// AllocateLayerArrays
//
// Description:
//		Allocates the arrays of RImage pointers and string pointers
//		for the layer names.  It first calls FreeLayerArrays in
//		case a file had already been loaded.
//
// Parameters:
//		sNumLayers = number of pointers to allocate
//
// Returns:
//		SUCCESS if the arrays were allocated successfully
//		FAILURE otherwise
//
//////////////////////////////////////////////////////////////////////

short RLaymage::AllocateLayerArrays(short sNumLayers)
{
	short sReturn = SUCCESS;
	short i;

	FreeLayerArrays();
	
	m_apImages = new RImage*[m_sNumLayers];
	if (m_apImages)
	{
		m_apszLayerNames = new char*[m_sNumLayers];
			if (!m_apszLayerNames)
				sReturn = FAILURE;
	}
	else
		sReturn = FAILURE;

	if (sReturn == SUCCESS)
	{
		for (i = 0; i < sNumLayers; i++)
		{
			m_apImages[i] = NULL;
			m_apszLayerNames[i] = NULL;
		}
	}

	return sReturn;		
}

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////

