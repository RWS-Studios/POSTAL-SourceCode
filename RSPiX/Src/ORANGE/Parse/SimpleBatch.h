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
#ifndef SIMPLE_BATCH_H
#define SIMPLE_BATCH_H
//=====================
#include <stdio.h>
#include <string.h>
#include "Blue.h"

// TO DO!
// REPLACE THIS WITH LINKED LISTS or some dynamic format!
// Add in a high level control for #define and #include
// Add macros for combining tokens into one.

#define SB_MAX_TOKENS 20
#define SB_MAX_TOKEN_SIZE 256
#define SB_MAX_CHAR_LIST 10


// Designed ony for ASCII style input files
// It will normally ignore control characters...
//
class RBatch
	{
//---------------- USER MEMBERS -------------
public:
	FILE* m_fp;
	int32_t	m_lCurrentLine;
	int16_t m_sNumTokens;
	char	m_pszTokenList[SB_MAX_TOKENS][SB_MAX_TOKEN_SIZE];
	int16_t m_sLinePos[SB_MAX_TOKENS];
	int16_t	m_sCurToken; // THIS IS ONLY FOR USER TO USE => Has no meaning!
	char  m_pszFileName[128]; // For user convenience
//----------- CONFIGURATION MEMBERS -------------
	char 	m_pszSeparators[SB_MAX_CHAR_LIST];
	char  m_pszSpecialCharacters[SB_MAX_CHAR_LIST];
	int16_t	m_sCharEOL; // 2 for \n\r, else 1
	int16_t m_sLowFilter;
	int16_t m_sHighFilter;
	char	m_cStringContext;
	char	m_cComment;
    //--------------- CONSTRUCTION -------------
    void clear() // NOT A RESET! Will hose memory!
    {
        m_fp = NULL;
        m_sNumTokens = m_sCharEOL = m_sLowFilter = m_sHighFilter = 0;
        m_pszSeparators[0] = '\0';
        m_pszFileName[0] = '\0';
        m_pszSpecialCharacters[0] = '\0';
        m_lCurrentLine = 0;

        for (int16_t i = 0; i < SB_MAX_TOKENS; i++)
        {
            m_pszTokenList[i][0] = '\0';
            m_sLinePos[i] = 0;
        }

        m_sCurToken = -2; // for user convenience!
    }

    void init(char* pszFileName, char* pszSeparators = " \t,", char* pszSpecialCharacters = NULL,
        char cString = '"', char cComment = '*', int16_t sCharEOL = 1, int16_t sLowFilter = 32, int16_t sHighFilter = 128)
    {
        clear();
        if (fopen_s(&m_fp, pszFileName, "r") != 0)
        {
            TRACE("RBatch: file %s not found.\n", pszFileName);
            return;
        }

        strcpy_s(m_pszFileName, sizeof(m_pszFileName), pszFileName);
        m_sCharEOL = sCharEOL;
        m_sLowFilter = sLowFilter;
        m_sHighFilter = sHighFilter;
        m_cStringContext = cString;
        m_cComment = cComment;
        if (pszSpecialCharacters) strcpy_s(m_pszSpecialCharacters, sizeof(m_pszSpecialCharacters), pszSpecialCharacters);
        if (pszSeparators) strcpy_s(m_pszSeparators, sizeof(m_pszSeparators), pszSeparators);
    }

    void configure(char* pszSeparators = " \t,", char* pszSpecialCharacters = NULL,
        char cString = '"', char cComment = '*', int16_t sCharEOL = 1, int16_t sLowFilter = 32, int16_t sHighFilter = 128)
    {
        m_sCharEOL = sCharEOL;
        m_sLowFilter = sLowFilter;
        m_sHighFilter = sHighFilter;
        m_cStringContext = cString;
        m_cComment = cComment;

        if (pszSpecialCharacters) strcpy_s(m_pszSpecialCharacters, sizeof(m_pszSpecialCharacters), pszSpecialCharacters);
        if (pszSeparators) strcpy_s(m_pszSeparators, sizeof(m_pszSeparators), pszSeparators);
    }

    RBatch() // default, no file
    {
        clear();
        configure();
    }

    RBatch(char* pszFileName, char* pszSeparators = " \t,", char* pszSpecialCharacters = NULL,
        char cString = '"', char cComment = '*', int16_t sCharEOL = 1, int16_t sLowFilter = 32, int16_t sHighFilter = 128)
    {
        init(pszFileName, pszSeparators, pszSpecialCharacters, cString, cComment, sCharEOL, sLowFilter, sHighFilter);
    }

    int16_t open(char* pszFileName)
    {
        if (m_fp == NULL)
        {
            if (fopen_s(&m_fp, pszFileName, "r") != 0)
            {
                TRACE("RBatch: file %s not found.\n", pszFileName);
                return -1;
            }

            strcpy_s(m_pszFileName, sizeof(m_pszFileName), pszFileName);
            m_lCurrentLine = 0;
            m_sNumTokens = 0;

            return 0;
        }
        else
        {
            TRACE("RBatch: file already open!\n");
            return -1;
        }
    }

    int16_t close()
    {
        if (m_fp != NULL)
        {
            fclose(m_fp);
            m_fp = NULL;
            return 0;
        }
        else
        {
            TRACE("RBatch: File already closed.\n");
            return -1;
        }
    }

    ~RBatch()
    {
        if (m_fp) fclose(m_fp);
        clear();
    }

//--------------- STATIC SPACE -------------
	static	char	ms_Error[256];
//--------------- MEMBER FUNCTIONS -------------
	// returns the number of tokens found, or -1 for EOF
	int16_t GetLine();

	// If you care nothing of the concept of file lines except for
	// reporting errors, you may take advantage of this function:
	char* NextToken();

	// creates an error message
	char* CreateError(int16_t sToken = -1);
	};

//=====================
#endif
