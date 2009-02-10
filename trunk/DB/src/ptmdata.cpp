/*
 * This file is part of the Popitam software
 * Copyright (C) 2009 Swiss Institute of Bioinformatics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */

/***********************************************************

	Company            : Swiss Intitute of Bioinformatics
	Author             : Marc Tuloup
	Contact            : Marc.Tuloup@isb-sib.ch
	Created on         : 28.11.2003
	Last modification  : 02.07.2004
	Comments           : 

***********************************************************/


#include "ptmdata.h"
#include "MATerror.h"

#include <stdlib.h>



/******************************************************

		Constructor

*******************************************************/
PtmData::PtmData(void) :
	m_iId(0),
	m_pszName(NULL),
	m_pszAbv(NULL),
	m_pszFormula(NULL)
{
}


/******************************************************

		Destructor

*******************************************************/
PtmData::~PtmData(void)
{
	if(m_pszName){
		delete [] m_pszName;
		m_pszName = NULL;
	}

	if(m_pszAbv){
		delete [] m_pszAbv;
		m_pszAbv = NULL;
	}

	if(m_pszFormula){
		delete[]  m_pszFormula;
		m_pszFormula = NULL;
	}
}

/***********************************************************

   Read

***********************************************************/
void PtmData::Read(TagFile &file, int iId)
{
	m_iId = iId;

	file.ReadStartObject("Item");

	file.ReadMember("String", &m_pszName);
	file.ReadMember("Abv", &m_pszAbv);
	file.ReadMember("Formula", &m_pszFormula);

	file.ReadEndObject();	//Item
}










/******************************************************

		Constructor

*******************************************************/
PtmDataList::PtmDataList(void) 
{
}

/******************************************************

		Destructor

*******************************************************/
PtmDataList::~PtmDataList(void)
{
	m_aPtmData.DeletePtrs();
}

/***********************************************************

   Load

***********************************************************/
void PtmDataList::Load(const char *pszFileName)
{
	TagFile file;
	file.Open(pszFileName, "r");

	file.ReadStartObject("Ptm");

	int iNbItem;
	file.ReadMember("NbItem", &iNbItem);

	m_aPtmData.EnsureSize(iNbItem);
	
	PtmData *pPtmData;
	for(int i=0; i<iNbItem; i++){
		pPtmData = new PtmData;
		m_aPtmData.Add(pPtmData);
		pPtmData->Read(file, i);
	}

	file.ReadEndObject();	//Ptm

	file.Close();
}
