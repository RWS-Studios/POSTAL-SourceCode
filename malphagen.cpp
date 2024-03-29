#include <iostream>
#include <cstdlib>
#include "RSPiX/Src/ORANGE/color/colormatch.h"

const double transparencyLevels[] = {0.0588235294118, 0.117647058824, 0.172549019608, 0.23137254902, 0.290196078431, 0.352941176471, 0.411764705882, 0.470588235294, 0.525490196078, 0.58431372549, 0.643137254902, 0.705882352941, 0.764705882353, 0.823529411765, 0.878431372549, 0.937254901961};

void malphagen(const char* in, const char* out)
{
	RImage myIm;
	myIm.Load((char*) in);
	RMultiAlpha myMAl;
	myMAl.Alloc(16);
	for (int_fast8_t i = 0; i < 16; ++i)
	{
		myMAl.CreateLayer(i, transparencyLevels[i], 10, 236);
		myMAl.m_pAlphaList[i]->ms_SetPalette(&myIm);
		myMAl.m_pAlphaList[i]->CreateAlphaRGB(transparencyLevels[i], 10, 236);
	}
	myMAl.Finish(TRUE);
	FILE* myF = fopen(out, "wb");
	RFile myRF;
	myRF.Open(myF, RFile::LittleEndian, RFile::Binary);
	myMAl.Save(&myRF);
	myRF.Close(); // closes myF also
}

