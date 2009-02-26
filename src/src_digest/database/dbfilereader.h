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
	Created on         : 12.05.2006
	Last modification  : 12.05.2006
	Comments           : 

***********************************************************/


#ifndef __DBFILEREADER_H__
#define __DBFILEREADER_H__

#include "dbfile.h"
#include "actree.h"
#include "dynamicarray.h"
#include "kwmodedefine.h"

#include <limits.h>



/******************************************************

Class OffsetRange


*******************************************************/

class OffsetRange
{
	friend class DBReader;

private :

	unsigned int m_uiStart;
	unsigned int m_uiEnd;


public :

	OffsetRange(unsigned int uiStart, unsigned int uiEnd);
	~OffsetRange(void);

};



/******************************************************

Class DBFileReader


*******************************************************/

class DBFileReader : public DBFile
{

private :


	DynamicArray<TS_OffsetRange *>		m_aOffsetRange;
	int																m_iMwMin;
	int																m_iMwMax;
	unsigned short										m_usPiMin;
	unsigned short										m_usPiMax;
	bool															m_bRemoveVarsplice;
	bool															m_bRemoveFragment;

	DynamicArray<unsigned short>			m_aUserKW;
	TE_KwMode													m_eKwMode;

	AcTree														m_ACTree;

	bool															m_bUseFilters;


	unsigned int											m_uiEndOffset;
	int																m_iCurrentOffsetIndex;

	unsigned int											m_uiStartProgressBar;
	unsigned int											m_uiEndProgressBar;
	unsigned int											m_uiNextProgress;
	unsigned int											m_uiStepProgress;

	FILE															*m_pStdout;

public :

	DBFileReader(FILE *pStdout=NULL);
	~DBFileReader(void);

	void Open(const char *pszFileName);

	DBEntry *GetNextEntry(void);
	DBEntry *GetEntry(unsigned int uiOffset);

	void RemoveAllFilters(void);
	void SetTaxId(int *pTaxIdArray=NULL, int iNbTaxId=0);
	void SetMwRange(int iMin=0, int iMax=INT_MAX);
	void SetPIRange(double dMin=0, double dMax=14);
	void RemoveVarsplice( bool bRemove);
	void RemoveFragment( bool bRemove);
	void SetKeyword(int *pKWArray=NULL, int iNbKW=0, TE_KwMode eMode=KW_MODE_OR);
	
	inline void ResetAC(void)
	{ m_ACTree.Free(); }

	void AddAC(const char *pszAC);


	void ResetSearch(void);
	
	inline bool IsTaxonomyDefine(void)
	{ return m_stFileHeader.iNbTaxId? true : false; }

private :

	inline bool ReadEntry(void)
	{ return DBEntry::Read(m_file, &m_uiCurrentOffset, this); }


	inline void UseFilters(bool bUse)
	{ m_bUseFilters = bUse; }


	bool IsEntryInRange(void);
	bool IsKeywordInRange(void);

	bool SetNextOffsetRange(void);
	bool SetNextDB(void);


	void AddOffsetRange(unsigned int uiStart, unsigned int uiEnd);
	void RemoveOffsetRange(unsigned int uiStart, unsigned int uiEnd);
	void ResetOffsetRange(void);


	//fct de classe pour qsort
	inline static int SortKeyword(const void *p1, const void *p2)
	{ return *(unsigned short *)p1 - *(unsigned short *)p2; }

	inline static int SortOffsetRange(const void *p1, const void *p2)
	{ return (*(TS_OffsetRange **)p1)->uiStart - (*(TS_OffsetRange **)p2)->uiStart; }
};


#endif
