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
// log.h
// Project: Postal
//
// History:12/5/97 AJC	Started.
////////////////////////////////////////////////////////////////////////////////
#ifndef LOG_H
#define LOG_H


short OpenLogFile();
short CloseLogFile();

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
							U16 u16PackageID = 0);				// Uniquely identifiable package id
////////////////////////////////////////////////////////////////////////////////
// WriteInputData()
//			Write the network input data to network sync log
//		global variables used:		g_GameSettings
////////////////////////////////////////////////////////////////////////////////
extern short WriteInputData(U32 *input);


#endif //LOG_H
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
