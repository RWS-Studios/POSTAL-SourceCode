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
// average.h
// 
//
// This module conveniently implemetns running averages
//
// History:
//		09/03/97 JRD	Started.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef AVERAGE_H
#define AVERAGE_H

////////////////////////////////////////////////////////////////////////////////
// This is a fully configurable template.  This first version must have a 
// static width, which is set upon instantiation.  This version also relies
// on an initial seed value, which has a FULL weighting of the average width
// and will take a full interval to remove.  Obviously, your template type must
// have overloads for addition, subtraction, assignment, casting and division.
////////////////////////////////////////////////////////////////////////////////
// WARNING: a subtle restriction of this class template is that the maximum
// value of ValueType_data * clFixedWidth should NOT overflow.
////////////////////////////////////////////////////////////////////////////////

// This is a basic box filter!
// The filter width is static so there is no dynamic allocation of data
// Let me know if this kills your heap
//
template <class ValueType, long clFixedWidth>
class	CRunningAverage	
	{
public:
	//---------------------------------------
	// Member Functions
	//---------------------------------------

	//=======================================
	// set average value
	void	Seed(ValueType vtSeedValue)
		{
		short i;
		m_lSignificantNumber = clFixedWidth;
		m_lNextValue = 0;

		for (i = 0; i < m_lSignificantNumber;i++)	m_avDataList[i] = vtSeedValue;

		m_vCurrentTotal = ValueType(vtSeedValue * m_lSignificantNumber);
		}

	//=======================================
	// returns the current running average
	ValueType Average()
		{
		ASSERT(m_lSignificantNumber > 0);

		return ValueType(m_vCurrentTotal / m_lSignificantNumber);
		}

	//=======================================
	// Give it a new value
	// The oldest value will drop off the list if the "window" is filled.
	// Soon it will suport an expanding width.
	//
	void AddValue(ValueType vtNewValue)	
		{
		m_vCurrentTotal -= m_avDataList[m_lNextValue]; // remove point
		m_avDataList[m_lNextValue++] = vtNewValue;
		m_vCurrentTotal += vtNewValue;

		if (m_lNextValue >= clFixedWidth) m_lNextValue = 0;
		}

	//---------------------------------------
	// Instantiation
	//---------------------------------------

	//=======================================
	// Give it an initial average value
	CRunningAverage(ValueType vtSeedValue)
		{
		short i;
		m_lSignificantNumber = clFixedWidth;
		m_lNextValue = 0;
	
		for (i = 0; i < m_lSignificantNumber;i++)	m_avDataList[i] = vtSeedValue;

		m_vCurrentTotal = vtSeedValue * m_lSignificantNumber;
		}

	//=======================================
	// Currently, this just seeds with xero.
	// Soon, it will allow an expanding average option:
	CRunningAverage()
		{
		Seed(ValueType(0));
		}

	//=======================================
	~CRunningAverage()
		{
		m_lSignificantNumber = 0;	// catch bugs fast!
		}

	//---------------------------------------
	// Member Variables
	//---------------------------------------
	ValueType	m_avDataList[clFixedWidth];
	ValueType	m_vCurrentTotal;
	long			m_lSignificantNumber;
	long			m_lNextValue;
	};



#endif
