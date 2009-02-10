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

#include "ptmsearchmanager.h"
#include "util.h"
#include "tagfile.h"
#include <stdlib.h>



/******************************************************

		Constructor

*******************************************************/
PtmSearchManager::PtmSearchManager(void) :
	m_bDefined(false)
{
}


/******************************************************

		Destructor

*******************************************************/
PtmSearchManager::~PtmSearchManager(void)
{
}


/***********************************************************

	Load

***********************************************************/

void PtmSearchManager::Load(const char *pszFileName)
{
	if(!pszFileName){
		m_bDefined = false;
		return;
	}

	m_bDefined = true;

	//load ptmdata
	PtmDataList::Load(pszFileName);

	PtmData *pPtmData;
	for(int i=0; i<m_aPtmData.GetNbElt(); i++){
		pPtmData = m_aPtmData[i];	
		m_aPtmArray[pPtmData->GetName()[0]].Add(pPtmData);
	}
}


/***********************************************************

	GetPtmId

***********************************************************/

int PtmSearchManager::GetPtmId(const char *psz)
{
	if(!m_bDefined)
		return -1;

	//index du tableau correspondant a la premiere lettre de psz
	DynamicArray<PtmData *> &aPtmData = m_aPtmArray[(int)psz[0]];
	
	//parse toutes les ptms commencant par cette lettre
	for(int i=0; i<aPtmData.GetNbElt(); i++){
		if(!strcmp(aPtmData[i]->GetName(), psz))
			return aPtmData[i]->GetId();
	}

	//cette ptm n'existe pas
	return -1;
}





