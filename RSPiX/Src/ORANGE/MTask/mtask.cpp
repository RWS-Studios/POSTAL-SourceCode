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
// MTask.cpp
//
// Multitasking module for game run routines.  
//
// This multitasking module allows you to write character
//	logic in a more linear fashion.  Traditionally run routines
// were written as a series of switch statements based on
// some saved state for that character.  They would run 
// through the logic, going through the switch statement
// based on their state, and would exit at the end via
// return statement.  Then the next time they were called
// to run, they started at the beginning of the function and
// examined the state again etc.  
//
// Using this multitasking module, you can write logic
// in such a way that it seems as if the logic code is the
// only code running.  Rather than having an exit point
// at the end of the logic, you can write the code that 
// doesn't seem to exit.  At various points in the logic, 
// you must call MTaskWait which allows your task to be 
// switched out and allows other tasks to run.  The MTaskWait
// call can be put anywhere in your logic code, and when it
// is time for your task to run again, it will resume with
// its state restored, right after the MTaskWait call.  
//
// Created On:	10/18/96 BRH
// Implemented:10/17/96 BRH
//
// History:
//	
//	10/17/96 BRH	Started a test version in another file to
//						try the stack swapping and task switching.
//
// 10/18/96 BRH	Started this file and imported the test
//						functions dealing with the Multitasking.
//						Renamed all of the functions for the
//						Multitasking module MTask____.  
//
//	11/08/96	JMI	Changed CList to RList in one location.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

 #ifdef PATHS_IN_INCLUDES
	#include "ORANGE/MTask/mtask.h"

#else
	#include "MTASK.H"

#endif	//PATHS_IN_INCLUDES

//////////////////////////////////////////////////////////////////////
// "Member" variables (Globals)
//////////////////////////////////////////////////////////////////////

static long m_lMainProgramStack;		// save Main program stack here
static long m_lTaskStack;				// save Current task stack here
static PTASKINFO ptiCurrentTask;		// Pointer to current task for
												// use in error reporting
static RList<TASKINFO> MTaskList;

//////////////////////////////////////////////////////////////////////
// Static Functions
//////////////////////////////////////////////////////////////////////

// This function is to be called from within the task's
// process.  It is used to transfer control to other tasks.
// Your task will resume immediately following this call,
// when its turn comes up again.
static long* MTaskRun(void);

// This function is an error trap in case a task
// returns.  Tasks are not supposed to return, they
// call MTaskKill when they are done.
static void MTaskReturnCatcher(void);

//////////////////////////////////////////////////////////////////////
//
//	MTaskManager
//
// Description:
//		This is the routine that is called in the main loop of
//		the application.  It will run through all allocated
//		tasks and switch the stack for the task and let it
//		continue running where it left off.  Once all of the
//		tasks have been processed once, the manager will return
//		back to the main loop of the application.  
//
// Parameters:
//		none
//
// Returns:
//		none
//
//////////////////////////////////////////////////////////////////////

void MTaskManager(void)
{
	PTASKINFO ptiTask = MTaskList.GetHead();

	while (ptiTask)
	{
		ASSERT(ptiTask->plSP != NULL);
		// Set current task for error reporting & killing
		ptiCurrentTask = ptiTask;
		m_lTaskStack = (long) ptiTask->plSP;
		ptiTask->plSP = MTaskRun();
		if (ptiTask->plSP == 0)
		{
			ASSERT(ptiTask->plStackAddress != NULL);
			free(ptiTask->plStackAddress);
			ASSERT(ptiTask->pszFunctionName != NULL);
			free(ptiTask->pszFunctionName);
			MTaskList.Remove(ptiTask);
			delete ptiTask;
		}
		ptiTask = MTaskList.GetNext();
	}
}

//////////////////////////////////////////////////////////////////////
//
// MTaskAdd
//
//	Description:
//		Adds a new task to the list of tasks to be processed.  Note
//		that functions passed to this routine need to be specifically
//		designed to work with that task manager.  That means that they
//		must periodically call MTaskWait to give up time to other
//		tasks, otherwise they will end up being the only task ever to
//		run.  Also, they must never return.  Once they are added
//		with MTaskAdd, they should run until they are no longer
//		needed at which time they should call MTaskKill to take them
//		off of the task list.  If a task ever issues a return,
//		this module will catch it and ASSERT the failure.
//
// Parameters:
//		void* pFunction = pointer to the task you wish to add.
//
// Returns:
//		SUCCESS if a task was allocated and added
//		FAILURE if memory could not be allocated for the task
//
//////////////////////////////////////////////////////////////////////

short MTaskAddFunc(void* pFunction, char* pszFuncName, short sStackSize)
{
	PTASKINFO ptiNewTask = NULL;
	long* plNewStack = NULL;
	short sLongElements = sStackSize/4;
	short sReturn = SUCCESS;
	
	ptiNewTask = new TASKINFO;
	if (ptiNewTask != NULL)
	{
		plNewStack = (long*) calloc(sLongElements, 4);
		if (plNewStack)
		{
			plNewStack[sLongElements-1] = (long) MTaskReturnCatcher;
			plNewStack[sLongElements-2] = (long) pFunction;
			plNewStack[sLongElements-3] = 0; //bp

			ptiNewTask->plStackAddress = plNewStack;
			ptiNewTask->plSP = (plNewStack + (sLongElements-3));
			ptiNewTask->pszFunctionName = (char*) calloc(sizeof(pszFuncName), 1);
			strcpy(ptiNewTask->pszFunctionName, pszFuncName);
		}
		else
		{
			TRACE("MTaskAdd - Error allocating stack for new task\n");
			sReturn = FAILURE;
		}
		MTaskList.Add(ptiNewTask);
	}
	else
	{
		TRACE("MTaskAdd - Error allocating a new task info structure\n");
		sReturn = FAILURE;
	}
	return sReturn;				
}

//////////////////////////////////////////////////////////////////////
//
// MTaskKill
//
// Description:
//		Removes the task from the list.  This should only be called
//		from within the currently running task.  It calls this when
//		it is done running, for example, if the task is a run
//		routine for character logic and the character dies, then
//		it would call this funtion to take it off of the task list.
//		This funcition kills the ptiCurrentTask.  
//
//		This function is called in place of a normal MTaskWait,
//		and therefore returns the current tasks's stack pointer.
//		This routine always returns 0 to indicate that the task
//		should be killed which is actually cleaned up in the
//		MTaskManager routine when it gets a return SP valuie of 0.
//
// Parameters:
//		none
//
// Returns:
//		0 to indicate that the task should be deleted
//
//////////////////////////////////////////////////////////////////////

__declspec (naked) long* TaskKill(void)
{
	__asm
	{
		mov	esp,m_lMainProgramStack	;restore the program's stack
		pop	ebp							;restore programs's stack frame
		mov	eax,0h						;return NULL to flag as 
		mov	dx,0h							;ready to delete
		ret
	}
}

//////////////////////////////////////////////////////////////////////
//
// MTaskWait
//
// Description:
//		Switches back to the program's main stack and returns
//		to MTaskManager where it left off in the loop after it
//		called MTaskRun.  The MTaskManager calls the MTaskRun
//		which switches stacks and resumes the task.  Then the
//		task calls this function which switches back to the main
//		program stack and returns the task's stack pointer to
//		MTaskManager which then can switch to the next task.
//
// Parameters:
//		none
//
// Returns:
//		SP pointer location for the task just run
//
//////////////////////////////////////////////////////////////////////

__declspec (naked) long* MTaskWait(void)
{
	__asm
	{
		push	ebp							;save task's frame pointer
		mov	m_lTaskStack,esp			;Save task's stack pointer
		mov	esp,m_lMainProgramStack	;restore the program's main stack
		pop	ebp							;restore Real Stack's frame pointer
		mov	eax,m_lTaskStack			;put current stack in ax
		mov	edx,m_lTaskStack			;put current stack in dx
		shr	edx,16						;move high word to low word of register
		ret
	}
}

//////////////////////////////////////////////////////////////////////
//
// MTaskRun
//
// Description:
//		Switches from the main stack to the task's stack and
//		then returns to where the task was last running.
//
// Parameters:
//		none
//
// Returns:
//		Note: The function seems to return a long of the
//			   task's stack pointer, but in fact, the return
//				value does not come from this function, but from
//				MTaskWait or MTaskKill.
//
//////////////////////////////////////////////////////////////////////

__declspec (naked) long* MTaskRun(void)
{
	__asm
	{
		push	ebp								;Save Real Stack's frame pointer
		mov	m_lMainProgramStack,esp		;Save current stack
		mov	esp,m_lTaskStack				;Set task's stack
		pop	ebp								;restore task's frame pointer
		ret
	}
}

//////////////////////////////////////////////////////////////////////
//
// MTaskReturnCatcher
//
// Description:
//		Catches tasks that have returned when they were not
//		supposed to.  Because we are switching tasks and stacks, 
//		if a task returns, it would not clean up its stack and
//		free its task.  Therefore tasks should never return.  They
//		should call MTaskKill when they are done to clean up
//		their task and stack.  If they do accidently return, they
//		will come here, where the program will hang and report
//		an error.
//
// Parameters:
//		none
//
// Returns:
//		never returns;
//
//////////////////////////////////////////////////////////////////////

void MTaskReturnCatcher(void)
{
	TRACE("MTask - The task %s has returned rather than calling MTaskKill\n", ptiCurrentTask->pszFunctionName);
	ASSERT(FALSE);
}