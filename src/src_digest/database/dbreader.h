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
	Created on         : 15.05.2006
	Last modification  : 15.05.2006
	Comments           : 

***********************************************************/


#ifndef __DBREADER_H__
#define __DBREADER_H__


#include "dbfilereader.h"
#include "dynamicarray.h"



/******************************************************

Class ReloadDBEntry


*******************************************************/

class ReloadDBEntry
{

public :

	int				m_iDBFileIndex;
	unsigned int	m_uiEntryOffset;

public :

	ReloadDBEntry(void) :
		m_iDBFileIndex(-1),
		m_uiEntryOffset(0)
	{};

	~ReloadDBEntry(void){};
	
	
	void Copy(ReloadDBEntry &r)
	{
		m_iDBFileIndex = r.m_iDBFileIndex;
		m_uiEntryOffset = r.m_uiEntryOffset;
		
	}

};



/******************************************************

Class DBReader


*******************************************************/

class DBReader
{

protected :

	DynamicArray<DBFileReader *>	m_aFile;
	int														m_iCurrentFileIndex;
	FILE													*m_pStdout;

public :

	DBReader(FILE *pStdout=NULL);
	~DBReader(void);

	DBFileReader *Open(const char *pszFileName);
	void CloseAll(void);

	DBEntry *GetNextEntry(void);

	DBEntry *GetEntry(int iFileIndex, unsigned int uiOffset);
	DBEntry *GetEntry(ReloadDBEntry *pReloadDBEntry);

	inline int GetNbFile(void)
	{ return m_aFile.GetNbElt(); }

	inline DBFileReader *GetFile(int i)
	{ return m_aFile[i]; }

	inline int GetCurrentFileIndex(void)
	{ return m_iCurrentFileIndex; }

	inline const char *GetCurrentFileLabel(void)
	{ return m_aFile[m_iCurrentFileIndex]->GetLabel(); }

	void ResetSearch(void);

	void StoreReloadEntry(ReloadDBEntry *pReloadDBEntry);


};


#endif
