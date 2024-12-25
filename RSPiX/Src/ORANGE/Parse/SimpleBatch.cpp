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
#include "SimpleBatch.h"

char RBatch::ms_Error[256];

// returns the number of tokens found, or -1 for EOF
int16_t RBatch::GetLine()
{
    m_sNumTokens = 0;
    int16_t i;
    for (i = 0; i < SB_MAX_TOKENS; i++)
    {
        m_pszTokenList[i][0] = '\0';
        m_sLinePos[i] = 0;
    }

    if (m_fp == NULL)
    {
        TRACE("RBatch:  File not open.\n");
        return -1;
    }

    int iChar;
    char c;
    int16_t sLinePos = 0;
    int16_t sTokenChar = 0;
    int16_t sLoop = TRUE;
    int16_t sRet = 0;
    int16_t sMidToken = FALSE;
    int16_t sInString = FALSE;

    while (sLoop)
    {
    BEGIN_LOOP:

        if ((iChar = fgetc(m_fp)) == EOF)
        {
            if (sMidToken)
            {
                m_pszTokenList[m_sNumTokens][sTokenChar] = '\0';
                m_sNumTokens++;
                sMidToken = FALSE;
                sTokenChar = 0;
            }

            m_fp = NULL;

            sRet = -1;
            sLoop = FALSE;
            break;
        }

        c = (char)iChar;
        sLinePos++;

        // 1) check for end of line:
        if ((c == '\n') || (c == '\r'))
        {
            m_lCurrentLine++;

            if (m_sCharEOL == 2) // ASSUME another follows
                fgetc(m_fp);

            if (sMidToken)
            {
                m_pszTokenList[m_sNumTokens][sTokenChar] = '\0';
                m_sNumTokens++;
                sMidToken = FALSE;
                sTokenChar = 0;
            }

            sLoop = FALSE;
            break;
        }

        if (!sInString)
        {
            // 2) Check for special characters:
            if (m_pszSpecialCharacters)
                for (i = 0; i < (int16_t)strlen(m_pszSpecialCharacters); i++) // Fix type mismatch
                {
                    if (c == m_pszSpecialCharacters[i]) // found it!
                    {
                        // insert a special character
                        if (sMidToken)
                        {
                            m_pszTokenList[m_sNumTokens][sTokenChar] = '\0';
                            m_sNumTokens++;
                            sTokenChar = 0;
                            sMidToken = FALSE;
                        }
                        m_sLinePos[m_sNumTokens] = sLinePos;
                        m_pszTokenList[m_sNumTokens][0] = c;
                        m_pszTokenList[m_sNumTokens][1] = '\0';
                        m_sNumTokens++;
                        goto BEGIN_LOOP;
                    }
                }

            // 3) Check for a filtered character
            if (((int16_t)c < m_sLowFilter) || ((int16_t)c > m_sHighFilter))
            {
                if (sMidToken)
                {
                    m_pszTokenList[m_sNumTokens][sTokenChar] = '\0';
                    m_sNumTokens++;
                    sTokenChar = 0;
                    sMidToken = FALSE;
                }
                continue; // KEEP SCANNING!
            }

            // 4) Check for separator
            for (i = 0; i < (int16_t)strlen(m_pszSeparators); i++) // Fix type mismatch
            {
                if (c == m_pszSeparators[i]) // found it!
                {
                    if (sMidToken)
                    {
                        m_pszTokenList[m_sNumTokens][sTokenChar] = '\0';
                        m_sNumTokens++;
                        sTokenChar = 0;
                        sMidToken = FALSE;
                    }

                    goto BEGIN_LOOP; // KEEP SCANNING!
                }
            }
        }

        // Check for string changes....
        if (c == m_cStringContext)
            if (sInString) // end the string
            {
                m_pszTokenList[m_sNumTokens][sTokenChar] = '\0';
                m_sNumTokens++;
                sTokenChar = 0;
                sMidToken = FALSE;
                sInString = FALSE;
                continue; // KEEP SCANNING!
            }
            else // begin a string context!
            {
                sInString = TRUE;
                sMidToken = TRUE;
                m_sLinePos[m_sNumTokens] = sLinePos;
                continue; // KEEP SCANNING!
            }

        // 5) Add to token
        if (sMidToken) // continue to add onto existing token
        {
            m_pszTokenList[m_sNumTokens][sTokenChar++] = c;
            continue; // KEEP SCANNING!
        }
        //************************************* START A TOKEN
        else // start a new token
        {
            sMidToken = TRUE;
            m_sLinePos[m_sNumTokens] = sLinePos;
            m_pszTokenList[m_sNumTokens][sTokenChar++] = c;
        }
        // KEEP SCANNING!
    }

    if (sRet != -1)
    {
        return m_sNumTokens;
    }
    else
        return -1;
}

char* RBatch::CreateError(int16_t sToken)
{
    if (m_fp == NULL)
    {
        sprintf_s(ms_Error, sizeof(ms_Error), "RBatch(%s):\n*   Premature end of file!\n", m_pszFileName);
        return ms_Error;
    }

    char temp[256] = "\0";

    if ((sToken != -1) && (sToken < m_sNumTokens))
    {
        sprintf_s(temp, sizeof(temp),
            "Unexpected token \"%s\" found at column %hd\n",
            m_pszTokenList[sToken], m_sLinePos[sToken]);
    }

    sprintf_s(ms_Error, sizeof(ms_Error), "RBatch(%s):\n*   Parse error at line %ld\n*   %s", m_pszFileName,
        m_lCurrentLine, temp);

    return ms_Error;
}

// Use a command syntax that is completely not line based:
// Returns NULL if EOF
// MUST ADVANCE WHEN CALLED AND SIT THERE!!!!!
char* RBatch::NextToken()
{
    if (m_sCurToken == -2) // first time:
    {
        if (GetLine() == int16_t(-1))
        {
            return NULL;
        }
        m_sCurToken = -1;
    }

search:

    m_sCurToken++; // move to next token!

    while (m_sCurToken >= m_sNumTokens)
    {
        if ((GetLine() == -1) && (m_sNumTokens == 0))
        {
            return NULL;
        }
        m_sCurToken = 0; // search for non null line
    }
    // comment lines
    if (*m_pszTokenList[m_sCurToken] == m_cComment)
    {
        m_sCurToken = m_sNumTokens;
        goto search;
    }

    return &m_pszTokenList[m_sCurToken][0];
}
