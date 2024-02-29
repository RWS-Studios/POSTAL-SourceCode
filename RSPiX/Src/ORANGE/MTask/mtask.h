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
#ifndef MTASK_H
#define MTAST_H
//////////////////////////////////////////////////////////////////////
//
// MTask.h
//
// Multitasking module for game run routines
//
// Created On:	10/18/96	BRH
// Implemented:10/18/96 BRH
//
//////////////////////////////////////////////////////////////////////

#include "System.h"

#ifdef PATHS_IN_INCLUDES
	#include "ORANGE/CDT/List.h"
#else
	#include "LIST.H"
#endif // PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////
//	Task Info Structure
//////////////////////////////////////////////////////////////////////

typedef struct tag_TaskInfo
{
	long* plStackAddress;		// Address of allocated memory for stack
	long* plSP;					// Current saved position of Stack Pointer
	char* pszFunctionName;	// Name of task using this stack 
									// (used for reporting errors)
} TASKINFO, *PTASKINFO;

//////////////////////////////////////////////////////////////////////
// Function prototypes
//////////////////////////////////////////////////////////////////////

// This is the function to run all of the tasks once.  It
// should be called in the main loop of the game
void MTaskManager(void);

// This is used to add tasks to be processed by 
// MTaskManager.  Note only tasks designed for
// this module should be added.  Tasks should
// never return and need to call MTaskWait
// periodically.
short MTaskAddFunc(void* pFunction, char* pszFuncName, short sStackSize = 1024);

#define MTaskAddwSize(fnTask, sStackSz)	MTaskAddFunc(fnTask, #fnTask, sStackSz)
#define MTaskAdd(fnTask)						MTaskAddFunc(fnTask, #fnTask);

// This is used to remove tasks from the task list.  
// Only call this function from within the task's process
// since it kills the currently running task and removes
// it from the list.  
// This is what should be called when you no longer wish
// to run a task.  For example if the task is character
// logic, it would normally loop until the guy got killed.
// When he is killed, call MTaskKill to remove it from
// the task processing list.  
void MTaskKill(void);

// This funciton is to be called from within the task's
// process.  It is used to allow other tasks to be run.  
// This funciton must be called periodically to allow the
// other tasks to run.  Your code will resume immediately
// after this call.
long* MTaskWait(void);

#endif // MTASK_H

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
