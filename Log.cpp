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
// log.cpp
// Project: Nostril (aka Postal)
//
// This module with a mechanism to log network traffic created by the game
//
// History:
//		12/05/97 AJC	Started.
//
//		12/23/97 SPA	Moved from Play.cpp to seperate file
////////////////////////////////////////////////////////////////////////////////
#include "RSPiX.h"
#include "game.h"
#include "net.h"
#include "netmsgr.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////////
// OpenLogFile()
//			Open a file for logging
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
short OpenLogFile()
	{
	short sResult = 0; // Assume success
	if (g_GameSettings.m_bLogNetTime)
		{
			if (!g_GameSettings.m_rfNetSyncLog.IsOpen())
			{
#ifdef SYS_ENDIAN_BIG
			if (g_GameSettings.m_rfNetSyncLog.Open(g_GameSettings.m_szNetSyncLogFile, 
				"wt+", RFile::BigEndian) != 0)
#else
			if (g_GameSettings.m_rfNetSyncLog.Open(g_GameSettings.m_szNetSyncLogFile, 
				"wt+", RFile::LittleEndian) != 0)
#endif
				{
				sResult = 1;
				TRACE("Play: Cannot open network syn log file\n");
				}			
			}
		}
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// CloseLogFile()
//			Close a file for logging
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
short CloseLogFile()
	{
	short sResult = 0; // Assume success
	if (g_GameSettings.m_bLogNetTime)
		{
		if ((g_GameSettings.m_rfNetSyncLog.Close()) != 0)
			{
			sResult = 1;
			TRACE("Play: Failed to close the network syn log file\n");
			}
		}
	return sResult;
	}


////////////////////////////////////////////////////////////////////////////////
// WriteTimeStamp()
//			Write the network time log
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
extern
short WriteTimeStamp(char *pszCaller,						// Name of calling routine
							char *pszCalleeName,					// Name of player being sent or sending 
							unsigned char ucMsgType,			// Message type
							Net::SEQ seqStart,					// Beginning sequent sent/received
							long sNum,								// Number of seq's sent/received
							char bReceived,							// a received or a sent message? TRUE if received
							U16 u16PackageID/*=0*/)				// Uniquely identifiable package id																		//		True if receiving, false if sending
	{	
	short sResult = 0;
	char *szCallerMsg;
	char szTime[256]; 
	char szSeq[256];
	char szNum[256];
	long lTime = rspGetMilliseconds();

	if ((ucMsgType == NetMsg::START_REALM)&&(bReceived))
		{
		g_GameSettings.m_lStartRealmTime = lTime;
		_ltoa(lTime, szTime, 10);
		}

	_ltoa(lTime - g_GameSettings.m_lStartRealmTime, szTime, 10);

#ifdef SYS_ENDIAN_BIG
	RFile::Endian endian = RFile::BigEndian;
#else
	RFile::Endian endian = RFile::LittleEndian;
#endif

	szCallerMsg = 0;

	// For convenience
	RFile *prfLog = &(g_GameSettings.m_rfNetSyncLog);

	// Log file should be open, if not, open it
	if (!prfLog->IsOpen())
		{
		if ((prfLog->Open(g_GameSettings.m_szNetSyncLogFile, "wt+", endian)) != 0)
			{
			TRACE("WriteTimeStamp: Failed to open network time stamp log file\n");
			sResult = -1;
			}
		}

	// Write name of player calling
	prfLog->Write(g_GameSettings.m_szPlayerName);

	// Write receiving or sending
	if (bReceived)
		prfLog->Write(" Received ");
	else
		prfLog->Write(" Sent     ");

	// Write name of person who will be receiving or has sent the message
	if (pszCalleeName != NULL)
		prfLog->Write(pszCalleeName);
	else
		prfLog->Write("Server");
	prfLog->Write(" ");

	// Write package ID
	char szPackageID[256];
	ltoa((long)u16PackageID, szPackageID, 10);
	prfLog->Write(szPackageID);
	prfLog->Write(" ");

	// Write time of logging
	prfLog->Write(szTime);
	prfLog->Write(" ");

	// Write starting sequence sent/received
	itoa(seqStart, szSeq, 10);
	prfLog->Write(szSeq);
	prfLog->Write(" ");

	// Write number of sequences sent/received
	ltoa(sNum, szNum, 10);
	prfLog->Write(szNum);
	prfLog->Write(" ");

	// Write type of message
	switch (ucMsgType)
		{
		case NetMsg::NOTHING:
			szCallerMsg = "NOTHING";
			break;
		case NetMsg::STAT:
			szCallerMsg = "STAT";
			break;
		case NetMsg::ERR:
			szCallerMsg = "ERR";
			break;
		case NetMsg::LOGIN:
			szCallerMsg = "LOGIN";
			break;
		case NetMsg::LOGIN_ACCEPT:
			szCallerMsg = "LOGIN_ACCEPT";
			break;
		case NetMsg::LOGIN_DENY:
			szCallerMsg = "LOGIN_DENY";
			break;
		case NetMsg::LOGOUT:
			szCallerMsg = "LOGOUT";
			break;
		case NetMsg::JOIN_REQ:
			szCallerMsg = "JOIN_REQ";
			break;
		case NetMsg::JOIN_ACCEPT:
			szCallerMsg = "JOIN_ACCEPT";
			break;
		case NetMsg::JOIN_DENY:
			szCallerMsg = "JOIN_DENY";
			break;
		case NetMsg::JOINED:
			szCallerMsg = "JOINED";
			break;
		case NetMsg::CHANGE_REQ:
			szCallerMsg = "CHANGE_REQ";
			break;
		case NetMsg::CHANGED:
			szCallerMsg = "CHANGED";
			break;
		case NetMsg::DROP_REQ:
			szCallerMsg = "DROP_REQ";
			break;
		case NetMsg::DROPPED:
			szCallerMsg = "DROP_REQ";
			break;
		case NetMsg::DROP_ACK:
			szCallerMsg = "DROP_ACK";
			break;
		case NetMsg::INPUT_REQ:
			szCallerMsg = "INPUT_REQ";
			break;
		case NetMsg::INPUT_DATA:
			szCallerMsg = "INPUT_DATA";
			break;
		case NetMsg::INPUT_MARK:
			szCallerMsg = "INPUT_MARK";
			break;
		case NetMsg::CHAT_REQ:
			szCallerMsg = "CHAT_REQ";
			break;
		case NetMsg::CHAT:
			szCallerMsg = "CHAT";
			break;
		case NetMsg::SETUP_GAME:
			szCallerMsg = "SETUP_GAME";
			break;
		case NetMsg::START_GAME:
			szCallerMsg = "START_GAME";
			break;
		case NetMsg::ABORT_GAME:
			szCallerMsg = "ABORT_GAME";
			break;
		case NetMsg::READY_REALM:
			szCallerMsg = "READY_REALM";
			break;
		case NetMsg::BAD_REALM:
			szCallerMsg = "BAD_REALM";
			break;
		case NetMsg::START_REALM:
			szCallerMsg = "START_REALM";
			break;
		case NetMsg::HALT_REALM:
			szCallerMsg = "HALT_REALM";
			break;
		case NetMsg::NEXT_REALM:
			szCallerMsg = "NEXT_REALM";
			break;
		case NetMsg::PROGRESS_REALM:
			szCallerMsg = "PROGRESS_REALM";
			break;
		case NetMsg::PROCEED:
			szCallerMsg = "PROCEED";
			break;
		case NetMsg::PING:
			szCallerMsg = "PING";
			break;
		case NetMsg::RAND:
			szCallerMsg = "RAND";
			break;
		default:
			szCallerMsg = "INVALID";
		}

	// Write the message type
	prfLog->Write(szCallerMsg);
	prfLog->Write(" ");

	// Write the calling routine
	prfLog->Write(pszCaller);

	prfLog->Write("\n");

	sResult = prfLog->Error();

	return sResult;
	}
/*** 12/5/97 AJC ***/
/*** 12/7/97 AJC ***/
////////////////////////////////////////////////////////////////////////////////
// WriteInputData()
//			Write the network input data to network sync log
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
extern
short WriteInputData(U32 *input)
	{
	short sResult = 0;
	char szInput[256]; 

	// For convenience
	RFile *prfLog = &(g_GameSettings.m_rfNetSyncLog);
	
	ltoa(*input, szInput, 16);

	prfLog->Write(szInput);
	prfLog->Write("\n");

	sResult = prfLog->Error();

	return sResult;
	}

/*** 12/7/97 AJC ***/

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
