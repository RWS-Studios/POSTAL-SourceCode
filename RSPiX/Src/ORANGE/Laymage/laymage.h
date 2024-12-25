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
#ifndef LAYMAGE_H
#define LAYMAGE_H

//////////////////////////////////////////////////////////////////////
//
// LAYMAGE.H
//
// Created on	10/04/96 BRH
// Implemented 10/04/96 BRH
//
// 10/04/96 BRH	Started the CLaymage (Layered CImage) class for
//						use in several utilities for Postal which involve
//						getting information from Photoshop files and
//						specific layers.  Once a layered Photoshop file
//						has been converted to CLaymage other utilities
//						can use the image types rather than having to 
//						work with the Photoshop file.  Some of the 
//						Photoshop information will be retained such as
//						the layer names, sizes and probably Alpha 
//						channel information.
//
//	11/01/96 BRH	Changed CLaymage to RLaymage to conform to new
//						RSPiX class names.
//
//	11/12/96 MJR	Removed include of "imagecon.h", which is obsolete.
//
//////////////////////////////////////////////////////////////////////

#include "Blue.h"
#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "GREEN/Image/Image.h"
	#include "ORANGE/File/file.h"
#else
	#include "Image.h"
	#include "file.h"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////


#define LAYMAGE_COOKIE	0x204c4d49	// Looks like "IML " in the file
#define LAYMAGE_CURRENT_VERSION 1	// Current file version.  Change
												// This number and die!  No, not
												// really, change this number if
												// you change the .IML file
												// format.  It will be compared to
												// the version number in the file
												// as a check when loaded.
#define LAYMAGE_PHOTOSHOP_SIGNATURE 0x38425053 //"8BPS"	
#define LAYMAGE_MAXCHANNELS	10
#define LAYMAGE_MAXLAYERS		24

#define LAYMAGE_IGNORE	-1
#define LAYMAGE_RED		0
#define LAYMAGE_GREEN	1
#define LAYMAGE_BLUE		2
#define LAYMAGE_ALPHA	3




//////////////////////////////////////////////////////////////////////
//
// RLaymage class
//
// Layered RImage which can read in Photoshop PSD files and convert
//	them to several RImages which are stored together.  It preserves
//	the layer information from the Photoshop file but makes the data
//	easier to work with.
//
//////////////////////////////////////////////////////////////////////

class RLaymage
{
	public:

		// Set to 1 if current file has layer info, 0 if not.
		int16_t m_sHasLayerInfo;

		// This number-of-channels is used only when the file has no layer info
		int16_t m_sSpecialNumChannels;

		int16_t m_sNumLayers;
		int32_t	m_lWidth;
		int32_t	m_lHeight;

		// General Constructor
		RLaymage();

		// General Destructor
		~RLaymage();

		// Resets object back to its freshly-constructed state
		void Reset();

		// Load funtion for loading Photoshop files
		int16_t LoadPSD(char* pszFilename);

		// Set a Photoshop file from which to get information
		// on demand.  Similar to load, but it does not load the
		// the layer information at this time, only when a layer
		// is requested by calling GetLayer()
		int16_t SetPSD(char* pszFilename);

		// Load function for loading RLaymage
		int16_t Load(char* pszFilename);

		// Load function for loading RLaymage.  This version of
		// the function reads from an open RFile pointer so
		// that a RLaymage could be included within another 
		// file
		int16_t Load(RFile* pcf);

		// Save function for saving RLaymage (.IML)
		int16_t Save(char* pszFilename);

		// Save function for saving RLaymage.  This version of
		// the function writes to an open RFile pointer so
		// that a RLaymage could be included in another file
		int16_t Save(RFile* pcf);

		// Function to return a pointer to a specified layer
		RImage* GetLayer(char* pszLayerName);

		// Function to return a pointer to a specified layer
		RImage* GetLayer(int16_t sLayer);

		// Function to free the RImage for the specified layer
		void FreeLayer(char* pszLayerName);

		// Function to free the RImage for the specified layer
		void FreeLayer(int16_t sLayer);

		// Function to free all currently allocated layers
		void FreeAllLayers(void);

		// Query functions
		int16_t GetNumLayers(void)	{return m_sNumLayers;};

		// Get the name of the specified layer
		// given the sLayer number, fill in the char buffer
		// pszNameBuffer with the name of this layer
		int16_t GetLayerName(int16_t sLayer, char* pszNameBuffer, size_t bufferSize);

	private:
		// Array of pointers to images
		RImage** m_apImages;
		// Array of strings of layer names
		char** m_apszLayerNames;
//		char* m_apszLayerNames[LAYMAGE_MAXLAYERS];

		// File position to Layer Headers
		int32_t m_lTellLayers;

		// file position to Channel data
		int32_t m_lTellChannels;

		// save the Photoshop filename for virtual loading
		char m_szPhotoshopFilename[1024];

		// Array of channel pointers for temporarily storing the 
		// Photoshop channels before converting to RImage
		char* m_pcChannels[LAYMAGE_MAXCHANNELS];

		// Read the header of the Photoshop file and set the
		//	width, height, number of layers and positions
		// to the layer info and channel data.  This routine
		//	is used by LoadPSD and SavePSD to read the common info
		// that they both need.
		int16_t ReadPSDHeader(char* pszFilename);

		// Skip a RFile pointer down to the channel data section
		// of the file.  This is called by LoadPSD just before it
		// begins to read the layers information.
		int16_t SetChannelPointer(int16_t sNumLayers, RFile* pcfChannel);

		// Fuction to read a specific layer.  It works like
		// LoadPSD, but only keeps the layer it wants.
		int16_t ReadLayer(int16_t sRequestedLayer);

		// Reads one layer from the Photoshop file and covnerts
		// it to a new 32-bit ARGB RImage in the RLaymage.
		int16_t ReadLayerInfo(int16_t sLayerNum, RFile* pcfLayer, RFile* pcfChannel);
		
		// Reads the name of a layer and saves it in the names array
		int16_t ReadLayerName(int16_t sLayerNum, RFile* pcfLayer);

		// RLE decompression routine to decompress the Photoshop channel
		// data.
		int16_t RLE_Decompress(char* pcBuffer, uint32_t ulCompSize, RFile* pcfRLE);
				
		// Convert the m_pcChannels of Photoshop Alpha, R, G and B data into
		// a RImage 32-bit ARGB format.  The data in the channels is bounded by
		// a rectangle which needs to be mapped on to a full size CLamage layer.
		int16_t ConvertToImage(int16_t sLayer, uint32_t ulTop, uint32_t ulBottom, uint32_t ulLeft, uint32_t ulRight);

		// Deallocate channel buffers and reset the pointers to NULL
		void ClearChannelBuffers(void);

		// Allocate 4 standard channel buffers
		int16_t AllocateChannelBuffers(uint32_t ulSize);

		// Allocate an array of RImage pointers and
		// char* pointers, one for each layer.
		// Calls FreeLayerArrays if the pointers are not
		// initially NULL
		int16_t AllocateLayerArrays(int16_t sNumLayers);

		// Deallocate RImage pointers and char* pointers
		// for all of the layers.  Also deallocates what
		// each pointer is pointing to
		void FreeLayerArrays(void);

					
};

#endif // LAYMAGE_H

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
