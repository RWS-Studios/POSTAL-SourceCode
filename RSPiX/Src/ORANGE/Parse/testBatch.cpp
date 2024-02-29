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
#include "simplebatch.h"

main()
	{
	RBatch ftest("testBatch.in");
	ftest.configure(" \t,","~");

	while (ftest.GetLine() != EOF)
		{
		// Dump info:
		TRACE("Line %ld has %hd tokens:\n",ftest.m_lCurrentLine,ftest.m_sNumTokens);
		for (short i=0;i<ftest.m_sNumTokens;i++)
			{
			TRACE("\tToken #%hd at char %hd = {%s}\n",i,
				ftest.m_sLinePos[i],ftest.m_pszTokenList[i]);
			}
		}
	
	return 0;
	}
