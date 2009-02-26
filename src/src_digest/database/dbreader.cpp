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


#include "dbreader.h"
#include "error.h"




/******************************************************

		Constructor

*******************************************************/
DBReader::DBReader(FILE *pStdout) :
	m_iCurrentFileIndex(0),
	m_pStdout(pStdout)
{	
}


/******************************************************

		Destructor

*******************************************************/
DBReader::~DBReader(void)
{
	CloseAll();
}


/***********************************************************

   Open

***********************************************************/
DBFileReader *DBReader::Open(const char *pszFileName)
{
	DBFileReader *pDBFileReader = new DBFileReader(m_pStdout);
	pDBFileReader->Open(pszFileName);
	m_aFile.Add(pDBFileReader);

	return pDBFileReader;
}

/***********************************************************

   CloseAll

***********************************************************/
void DBReader::CloseAll(void)
{
	for(int i=0; i<m_aFile.GetNbElt(); i++){
		m_aFile[i]->Close();
		delete m_aFile[i];
	}

	m_aFile.Reset();
	m_iCurrentFileIndex = 0;
}


/***********************************************************

   GetNextEntry

***********************************************************/
DBEntry *DBReader::GetNextEntry(void)
{
	DBEntry *pEntry = m_aFile[m_iCurrentFileIndex]->GetNextEntry();

	if(pEntry)
		return pEntry;

	//passe au fichier suivant: swissprot, trembl...
	if(++m_iCurrentFileIndex == m_aFile.GetNbElt())
		return NULL;

	return GetNextEntry();
}


/***********************************************************

   GetEntry

***********************************************************/
DBEntry *DBReader::GetEntry(int iFileIndex, unsigned int uiOffset)
{
	assert(iFileIndex >= 0 && iFileIndex < m_aFile.GetNbElt());

	m_iCurrentFileIndex = iFileIndex;

	return m_aFile[m_iCurrentFileIndex]->GetEntry(uiOffset);
}

/***********************************************************

   ResetSearch

***********************************************************/
void DBReader::ResetSearch(void)
{
	for(int i=0; i<m_aFile.GetNbElt(); i++)
		m_aFile[i]->ResetSearch();

	m_iCurrentFileIndex = 0;
}


/***********************************************************

   StoreReloadEntry

 ***********************************************************/
void DBReader::StoreReloadEntry(ReloadDBEntry *pReloadEntry)
{
	assert(pReloadEntry);

	pReloadEntry->m_iDBFileIndex	= m_iCurrentFileIndex;
	pReloadEntry->m_uiEntryOffset	= m_aFile[m_iCurrentFileIndex]->GetOffset();
}

/***********************************************************

   GetEntry

***********************************************************/
DBEntry *DBReader::GetEntry(ReloadDBEntry *pReloadDBEntry)
{
	assert(pReloadDBEntry);
	if(pReloadDBEntry->m_iDBFileIndex == -1)
		return NULL;

	return GetEntry(pReloadDBEntry->m_iDBFileIndex, pReloadDBEntry->m_uiEntryOffset);
}
