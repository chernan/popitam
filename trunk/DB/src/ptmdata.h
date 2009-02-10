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
	Last modification  : 12.02.2004
	Comments           : 

***********************************************************/

#ifndef __PTMDATA_H__
#define __PTMDATA_H__

#include "tagfile.h"
#include "dynamicarray.h"



/******************************************************

Class PtmData


*******************************************************/

class PtmData
{

private :

	int		m_iId;
	char	*m_pszName;
	char	*m_pszAbv;
	char	*m_pszFormula;

public :

	PtmData(void);
	~PtmData(void);

	void Read(TagFile &file, int iId);
	
	inline int GetId(void)
	{ return m_iId; }

	inline const char *GetName(void)
	{ return m_pszName; }

	inline const char *GetAbv(void)
	{ return m_pszAbv; }

	inline const char *GetFormula(void)
	{ return m_pszFormula; }

};




/******************************************************

Class PtmDataList


*******************************************************/

class PtmDataList
{

protected :

	DynamicArray<PtmData *> m_aPtmData;

public :

	PtmDataList(void);
	~PtmDataList(void);

	void Load(const char *pszFileName);
	
	inline int GetNbPtmData(void)
	{ return m_aPtmData.GetNbElt(); }

	inline PtmData *GetPtmData(int i)
	{ return m_aPtmData[i]; }


	inline DynamicArray<PtmData *> &GetPtmdataArray(void)
	{ return m_aPtmData; }
};



#endif
