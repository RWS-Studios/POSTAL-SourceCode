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
// NetMsgr.cpp
// Project: Net Messenger
//
// History:
//		05/24/97 MJR	Final assault begins.
//
//		05/25/97	JMI	Filled out CNetMsgr::InfoMsg declaration.
//
//					MJR	Made some changes to union, structs, etc.
//
//		05/26/97 MJR	Moved some functions from header to here.
//
//		06/11/97	JMI	Added FINISH_REALM and FINISHED_REALM net messages.
//
//		06/14/97 MJR	Removed LOAD_REALM and FINISHED_REALM messages and
//							modified START_GAME message.
//
//		08/13/97 MJR	Changed from ASSERT() to TRACE() when message len is
//							incorrect.
//
//		08/14/97 MJR	Changed from TRACE() when message len is incorrect to
//							generating an actual error.  This is safer in case this
//							ever occurs in a real game situation.
//
//							Also fixed bug where the message byte would be ungotten
//							twice if the message hadn't been fully received yet.
//
//		08/15/97 MJR	Cleaned up usage of CBufQ along with some other potential
//							(actual???) bugs.
//
//		08/18/97 MJR	Added "ChangeReq" and "Changed" messages.
//
//		09/01/97 MJR	Lots of changes as part of overall network overhaul.
//
//		09/02/97 MJR	Tested and tuned alot, and fixed a bunch of bugs.  Now
//							appears to be very stable.
//
//		09/07/97 MJR	Added "Proceed" message.
//
//		06/02/98	JMI	The handling of variable length messages was assuming the
//							message size was 2 (a short) but it was really a long.  
//							Fixed.
//
////////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"
#include "netmsgr.h"
#include "NetDlg.h"

////////////////////////////////////////////////////////////////////////////////
// This macro creates the horrendous amount of crap needed to declare the
// various pieces of information about a particular message struct.
//
// For each struct, we get it's size and pointers to it's Read() and Write()
// functions.  The usage of these function pointers is very similar to C++
// virtual functions, but for various reasons, we went with structs instead
// of classes, so we have to do our own table of function pointers.
////////////////////////////////////////////////////////////////////////////////
#define INFO(x) NetMsg::x::Size,	\
			NetMsg::x::Read,			\
			NetMsg::x::Write


////////////////////////////////////////////////////////////////////////////////
// Initialize array of information about each message struct.
////////////////////////////////////////////////////////////////////////////////
CNetMsgr::InfoMsg CNetMsgr::ms_aInfoMsg[NetMsg::NumMessages] =
	{
		{	NetMsg::NOTHING				,	INFO(Nothing)			},
		{	NetMsg::STAT					,	INFO(Stat)				},
		{	NetMsg::ERR						,	INFO(Err)				},
		{	NetMsg::LOGIN					,	INFO(Login)				},
		{	NetMsg::LOGIN_ACCEPT			,	INFO(LoginAccept)		},
		{	NetMsg::LOGIN_DENY			,	INFO(LoginDeny)		},
		{	NetMsg::LOGOUT					,	INFO(Logout)			},
		{	NetMsg::JOIN_REQ				,	INFO(JoinReq)			},
		{	NetMsg::JOIN_ACCEPT			,	INFO(JoinAccept)		},
		{	NetMsg::JOIN_DENY				,	INFO(JoinDeny)			},
		{	NetMsg::JOINED					,	INFO(Joined)			},
		{	NetMsg::CHANGE_REQ			,	INFO(ChangeReq)		},
		{	NetMsg::CHANGED				,	INFO(Changed)			},
		{	NetMsg::DROP_REQ				,	INFO(DropReq)			},
		{	NetMsg::DROPPED				,	INFO(Dropped)			},
		{	NetMsg::DROP_ACK				,	INFO(DropAck)			},
		{	NetMsg::INPUT_REQ				,	INFO(InputReq)			},
		{	NetMsg::INPUT_DATA			,	(size_t) (INFO(InputData))	},
		{	NetMsg::INPUT_MARK			,	INFO(InputMark)		},
		{	NetMsg::CHAT_REQ				,	INFO(ChatReq)			},
		{	NetMsg::CHAT					,	INFO(Chat)				},
		{	NetMsg::SETUP_GAME			,	INFO(SetupGame)		},
		{	NetMsg::START_GAME			,	INFO(StartGame)		},
		{	NetMsg::ABORT_GAME			,	INFO(AbortGame)		},
		{	NetMsg::READY_REALM			,	INFO(ReadyRealm)		},
		{	NetMsg::BAD_REALM				,	INFO(BadRealm)			},
		{	NetMsg::START_REALM			,	INFO(StartRealm)		},
		{	NetMsg::HALT_REALM			,	INFO(HaltRealm)		},
		{	NetMsg::NEXT_REALM			,	INFO(NextRealm)		},
		{	NetMsg::PROGRESS_REALM		,	INFO(ProgressRealm)	},
		{	NetMsg::PROCEED				,	INFO(Proceed)			},
		{	NetMsg::PING					,	INFO(Ping)				},
		{	NetMsg::RAND					,	INFO(Rand)				}
	};


////////////////////////////////////////////////////////////////////////////////
// Update (must be called regularly)
////////////////////////////////////////////////////////////////////////////////
void CNetMsgr::Update(void)
	{
	switch (m_state)
		{
		case Disconnected:
			// Nothing to do
			break;

		case Connecting:
			{
			short serr = m_socket.Connect(&m_address);
			if (serr == 0)
				{
				m_state = Connected;
				m_lMsgRecvTime = rspGetMilliseconds();
				m_lMsgSentTime = rspGetMilliseconds();
				}
			else if (serr != RSocket::errWouldBlock)
				{
				TRACE("CNetMsgr::Connect(): Attempt to connect failed!\n");
				// It may seem harsh, but resetting is just fine in this
				// situation because it cleans everything up, and there's
				// nothing to be "nice" about -- we're not connected to anyone!
				Reset();
				}
			}
			break;

		case Connected:
			ReceiveData();
			SendData();
			break;

		case Disconnecting:
			// When disconnecting, we only send data (we don't bother receiving it).
			// If there's no more data to send, or there's a send error, then disconnect.
			SendData();
			if (!IsMoreToSend())
				Disconnect();
			break;

		default:
			TRACE("CNetMsgr::Update(): Unknown state!\n");
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Get message
////////////////////////////////////////////////////////////////////////////////
bool CNetMsgr::GetMsg(									// True if message was available, false otherwise
	NetMsg* pmsg)											// Out: Message is returned here
	{
	bool bGotOne = false;

	// If there is no error condition
	if (m_error == NetMsg::NoError)
		{
		// See how much data (if any) is available
		long lGetable = m_bufIn.CheckGetable();
		if (lGetable >= 1)
			{
			// Peek at first byte of data, which ought to be the message type
			unsigned char ucMsg;
			if (m_bufIn.Get(&ucMsg) == 1)
				{
				// Make sure it's a valid message type
				if ((ucMsg >= 0) && (ucMsg < NetMsg::NumMessages))
					{
					// Get the expected message length.  A value of -1 indicates a
					// variable-sized message, in which case the next 2 bytes (if
					// they are available) would indicate the message size.
					long lMsgSize = ms_aInfoMsg[ucMsg].size;
					if (lMsgSize == -1)
						{
						// Check if at least enough is available (beyond the ucMsg byte we got)
						if (lGetable >= sizeof(ucMsg) + sizeof(lMsgSize) )
							{
							// Get the message size.  We assume this will always succeed because
							// we were just told that enough was available.
							m_bufIn.Get(&lMsgSize);

							// Undo the get of lMsgSize.
							short	sInc;
							for (sInc = 0; sInc < sizeof(lMsgSize); sInc++)
								{
								m_bufIn.UnGet();
								}
							}
						else
							{
							// Set fake message size so we'll realize message is not available
							lMsgSize = 0x7fffffff;
							}
						}

					// Undo the get of ucMsg byte.
					m_bufIn.UnGet();

					// If entire message is available, then we can get it
					if (lGetable >= lMsgSize)
						{
						// Make sure the read func is the right one . . .
						ASSERT(ms_aInfoMsg[ucMsg].ucType == ucMsg);

						// Read the message
						(ms_aInfoMsg[ucMsg].funcRead)(pmsg, &m_bufIn);

						// Verify that the correct number of bytes were read
						long lNewGetable = m_bufIn.CheckGetable();
						if ((lGetable - lNewGetable) == lMsgSize)
							{
							// Update most-recent receive time
							m_lMsgRecvTime = rspGetMilliseconds();

							// Indicate that we got a message
							bGotOne = true;

/*							// 12/7/97 AJC 
#ifdef WIN32
							if (g_GameSettings.m_bLogNetTime)
								{
								if (ucMsg == NetMsg::INPUT_DATA)
									WriteTimeStamp("CNetMsgr::GetMsg()", 
														(char*)pmsg->msg.inputData.id , 
														ucMsg, 
														0,
														0,
														true);
								else if (ucMsg != NetMsg::NOTHING)
									WriteTimeStamp("CNetMsgr::GetMsg()", 
														NULL, 
														ucMsg, 
														0,
														0,
														true);
								}
#endif
							// 12/7/97 AJC
*/
							}
						else
							{
							m_error = NetMsg::ReceiveError;
							TRACE("CNetMsgr::GetMsg(): Msg len should be %ld but was %ld !\n", lMsgSize, lGetable - lNewGetable);
							}
						}
					}
				else
					{
					// An invalid message type means we're in deep shit.  There's no real way to 
					// recover because we can't tell where messages start and end.
					m_error = NetMsg::ReceiveError;
					TRACE("CNetMsgr::GetMsg(): Invalid message type: %hd !\n", (short)ucMsg);
					}
				}
			}
		}

	// If there's an error condition then we need to generate an error message.  Note that
	// the error condition might have existed before we got into this function, in which
	// case we would not have tried to get a message above.  If there was no error on the
	// way in, and there is one now, then it occured while trying to get a message above.
	// Either way, we catch it here.
	if (m_error != NetMsg::NoError)
		{
		pmsg->msg.err.ucType = NetMsg::ERR;
		pmsg->msg.err.error = m_error;

		// Got one.
		bGotOne = true;

		// Clear error now that user has been notified.
		m_error = NetMsg::NoError;

/*		// 12/7/97 AJC
#ifdef WIN32
		if (g_GameSettings.m_bLogNetTime)
			{
			WriteTimeStamp("CNetMsgr::GetMsg()", 
								NULL, 
								NetMsg::ERR,
								0,
								0,
								true);
			}
#endif
		// 12/7/97 AJC 
*/
		}

	// If we didn't get a message, then generate a "nothing" message
	if (!bGotOne)
		pmsg->msg.nothing.ucType = NetMsg::NOTHING;

	//==============================================================================
	// MUST BE AT END OF FUNCTION!!!
	//
	// If it's a send error and we're disconnecting, then disconnect now.
	//
	// We do this at the end of this function because if we disconnect, we don't
	// want anyone to accidentally change any member variables afterwards!
	//==============================================================================
	if (m_state == Disconnecting)
		{
		if (pmsg->msg.nothing.ucType == NetMsg::ERR)
			{
			if (IsSendError(pmsg->msg.err.error))
				Disconnect();
			}
		}

	return bGotOne;
	}


////////////////////////////////////////////////////////////////////////////////
// Send message
////////////////////////////////////////////////////////////////////////////////
void CNetMsgr::SendMsg(
	NetMsg* pmsg,											// In:  Message to send
	bool bSendNow /*= true*/)							// In:  Whether to send now or wait until Update()
	{
	ASSERT(m_state == Connected);

	// Only send messages if we're connected
	if (m_state == Connected)
		{
		// Get msg type.
		U8	ucMsg = pmsg->msg.nothing.ucType;

		// Make sure it's a valid message type
		if ((ucMsg >= 0) && (ucMsg < NetMsg::NumMessages))
			{
			// Determine message size.  A size of -1 indicates a variable-sized message,
			// in which case the actual size is stored within the message itself.
			long lMsgSize = ms_aInfoMsg[ucMsg].size;
			if (lMsgSize == -1)
				lMsgSize = pmsg->msg.nothing.lSize;

			// Check available space in the queue, and if it's enough for the message, go ahead
			long lPutable = m_bufOut.CheckPutable();
			if (lPutable >= lMsgSize)
				{
				// Make sure the write func is the right one . . .
				ASSERT(ms_aInfoMsg[ucMsg].ucType == ucMsg);

				// Write the message . . .
				(ms_aInfoMsg[ucMsg].funcWrite)(pmsg, &m_bufOut);

				// Verify that the correct number of bytes were written
				long lNewPutable = m_bufOut.CheckPutable();
				if ((lPutable - lNewPutable) == lMsgSize)
					{
					// Update time last message was sent (hmmmm....not really!  This merely indicates
					// that the message was queued up, but not that it was actually sent!  Perhaps
					// this needs to be changed at some point to be more accurate.  It would have to
					// be moved to wherever we actually transmit the data to the socket.  Of course,
					// at that level, it's hard to tell if an entire message was sent, but still, it
					// would be more accurate than this.
					m_lMsgSentTime = rspGetMilliseconds();
					}
				else
					{
					m_error = NetMsg::SendError;
					TRACE("CNetMsgr::SendMsg(): Msg len should be %ld but was %ld !\n", lMsgSize, lPutable - lNewPutable);
					}
				}
			else
				{
				m_error = NetMsg::OutQFullError;
				TRACE("CNetMsgr::SendMsg(): Output queue is full!\n");
				}
			}
		else
			{
			// This should obviously never occur as it would indicate a humongous internal problem,
			// like the program has blown itself up or something.
			m_error = NetMsg::SendError;
			TRACE("CNetMsgr::SendMsg(): Attempting to write invalid message type: %hd !\n", (short)ucMsg);
			ASSERT(0);
			}
		
		// If "send now" flag is set, do it
		if (bSendNow)
			{
/*			// 12/7/97 AJC 
#ifdef WIN32
			if (g_GameSettings.m_bLogNetTime)
				{
				if (ucMsg == NetMsg::INPUT_DATA)
					WriteTimeStamp("CNetMsgr::SendMsg()", 
										(char*)pmsg->msg.inputData.id , 
										ucMsg,
										0,
										0,
										false);
				else if (ucMsg != NetMsg::NOTHING)
					WriteTimeStamp("CNetMsgr::SendMsg()", 
										NULL, 
										ucMsg, 
										0,
										0,
										false);
				}
#endif
			// 12/7/97 AJC 
*/
			SendData();
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Receive data (copy data from socket into input buffer)
////////////////////////////////////////////////////////////////////////////////
void CNetMsgr::ReceiveData(void)
	{

	// If there's bytes waiting to be received and no errors, then get them.
	// This was originally a while() loop but that was too dangerous because a
	// constant stream of data would have kept us in the loop forever!  Instead
	// we now loop twice.  The reason is that our buffer is implimented in such
	// a way that if the largest block we can write to it is from the current
	// position to the end of the buffer.  However, there might be additional
	// space at the beginning of the buffer -- all we have to do is "wrap around"
	// to the beginning.  Doing two gets does exactly that.
	for (short sGet = 0; sGet < 2; sGet++)
		{
		if (m_socket.CheckReceivableBytes() && (m_error == NetMsg::NoError))
			{
	 		// Call watchdog to let it know we're still going (we're in a loop!)
			NetBlockingWatchdog();

			// No bytes received yet
			long lReceivedBytes = 0;

			// Lock the buffer so we can write directly into it
			U8* pu8Put;
			long lMaxPuttableBytes;
			m_bufIn.LockPutPtr(&pu8Put, &lMaxPuttableBytes);

			// Make sure there's room in the buffer
			if (lMaxPuttableBytes > 0)
				{
				// Receive up to the specified number of bytes
				short serr = m_socket.Receive(pu8Put, lMaxPuttableBytes, &lReceivedBytes);
				if ((serr != 0) && (serr != RSocket::errWouldBlock))
					{
					m_error = NetMsg::ReceiveError;
					TRACE("CNetMsgr::Update(): Receive error!\n");
					}
				}
			else
				{
				TRACE("CNetMsgr::Update(): Warning! Input queue is full!\n");
				}

			// Release pointer, telling it how many bytes we actually added to it
			m_bufIn.ReleasePutPtr(lReceivedBytes);
			}
		else
			break;
		}
	}


////////////////////////////////////////////////////////////////////////////////
// Send data (copy data output buffer to socket)
////////////////////////////////////////////////////////////////////////////////
void CNetMsgr::SendData(void)
	{

	// If we have data to send and there's no errors, then try to send it.  It's
	// safe to loop because we can't get stuck -- there's only so much data to send!
	while (!m_bufOut.IsEmpty() && (m_error == NetMsg::NoError))
		{
		// We don't check this in the while() because it may be somewhat slow,
		// depending on the underlying implimentation.  Instead, we only check
		// it after we've already determined that we actually have data to send.
 		if (m_socket.CanSendWithoutBlocking())
 			{
	 		// Call watchdog to let it know we're still going (we're in a loop!)
			NetBlockingWatchdog();

			// No bytes sent yet
			long lSentBytes = 0;

			// Lock the buffer so we can read directly from it
			U8* pu8Get;
			long lMaxGettableBytes;
			m_bufOut.LockGetPtr(&pu8Get, &lMaxGettableBytes);

			// Make sure we can get something from buffer (this is not really
			// necessary since we already check IsEmpty() above, but what the hell...
			if (lMaxGettableBytes > 0)
				{
				// Receive up to the specified number of bytes
				short serr = m_socket.Send(pu8Get, lMaxGettableBytes, &lSentBytes);
				if ((serr != 0) && (serr != RSocket::errWouldBlock))
					{
					m_error = NetMsg::SendError;
					TRACE("CNetMsgr::Update(): Send error!\n");
					}
				}
			else
				{
				// If for some unbelievable reason this test fails even though the
				// previous test said there was data, then we're in deep shit, but
				// let's avoid an infinite loop in any case...
				TRACE("CNetMsgr::Update(): Internal inconsistancy detected!!!\n");
				ASSERT(0);
				break;
				}

			// Release pointer, telling it how many bytes we actually got from it
			m_bufOut.ReleaseGetPtr(lSentBytes);
			}
		else
			{
			// If we can't send without blocking, then break out of the loop
			break;
			}
		}
	}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////