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
	Last modification  : 28.11.2003
	Comments           : 

***********************************************************/

#ifndef __PTMSEARCHMANAGER_H__
#define __PTMSEARCHMANAGER_H__


#include "ptmdata.h"
#include "dynamicarray.h"


/******************************************************

Class PtmSearchManager


*******************************************************/

class PtmSearchManager : public PtmDataList
{

private :

	DynamicArray<PtmData *>	m_aPtmArray[128];		//acces par la premiere lettre aux ptms

	bool	m_bDefined;

public :

	PtmSearchManager(void);
	~PtmSearchManager(void);

	void Load(const char *pszFileName);

	int GetPtmId(const char *psz);
};


#endif
