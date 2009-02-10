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
	Created on         : 09.02.2006
  Last modification  : 09.02.2006
	Comments           : 
	
***********************************************************/

#include "dbfilereader.h"
#include "MATerror.h"
#include "util.h"
#include "minmax.h"


#define PROGRESS_STEP	10

/******************************************************

		Constructor

*******************************************************/
DBFileReader::DBFileReader(FILE *pStdout) :
	m_bUseFilters(false),
	m_uiEndOffset(0),
	m_pStdout(pStdout)
{	
}


/******************************************************

		Destructor

*******************************************************/
DBFileReader::~DBFileReader(void)
{
	m_aOffsetRange.DeletePtrs();
}



/***********************************************************

   Open

***********************************************************/
void DBFileReader::Open(const char *pszFileName)
{
	DBFile::Open(pszFileName, OPEN_FOR_READ);

	//default search
	RemoveAllFilters();
}

/***********************************************************

   RemoveAllFilters

***********************************************************/
void DBFileReader::RemoveAllFilters(void)
{
	SetTaxId();
	SetMwRange();
	SetPIRange();
	RemoveVarsplice(false);
	RemoveFragment(false);
	SetKeyword();
	ResetAC();

	m_bUseFilters = false;
}





/***********************************************************

   SetTaxId

***********************************************************/
void DBFileReader::SetTaxId(int *pTaxIdArray, int iNbTaxId)
{
	m_aOffsetRange.DeletePtrs();

	//taxid
	if(iNbTaxId != 0){

		if(!IsTaxonomyDefine())
			ThrowError("DBFileReader::SetTaxId", "No taxonomy associate to this file \"%s\"", m_file.GetFileName());
		
		TS_OffsetRange	stOffset;
		int							iTaxId;
		unsigned int		uiOffset;

		for(int i=0; i<iNbTaxId; i++){
		
			iTaxId		= (pTaxIdArray[i] < 0)? -pTaxIdArray[i] : pTaxIdArray[i];

			//unknown taxid
			if(iTaxId >= m_stFileHeader.iNbTaxId)
				continue;

			uiOffset	= iTaxId * sizeof(TS_OffsetRange) + DBFile::GetTaxIdOffset();

			fseek(m_file, uiOffset, SEEK_SET);
			fread(&stOffset, sizeof(TS_OffsetRange), 1, m_file);

			if(pTaxIdArray[i] > 0)
				AddOffsetRange(stOffset.uiStart, stOffset.uiEnd);
			else
				RemoveOffsetRange(stOffset.uiStart, stOffset.uiEnd);
		}

		if(m_aOffsetRange.GetNbElt() == 0)
			ThrowError("DBFileReader::SetTaxId", "No taxonomy associate to this selection");
			

		m_aOffsetRange.Sort(DBFileReader::SortOffsetRange);

		m_uiEndOffset	= 0;
		
	}

	ResetSearch();
}




/***********************************************************

   AddOffsetRange

***********************************************************/
void DBFileReader::AddOffsetRange(unsigned int uiStart, unsigned int uiEnd)
{
	TS_OffsetRange *p;
	
	for(int i=0; i<m_aOffsetRange.GetNbElt(); i++){

		p = m_aOffsetRange[i];
		
		if(p->uiEnd < uiStart)
			continue;
				
		if(p->uiStart > uiEnd)
			continue;

		//overlap
		if(p->uiStart < uiStart)
			uiStart = p->uiStart;

		if(p->uiEnd > uiEnd)
			uiEnd = p->uiEnd;

		delete m_aOffsetRange.Remove(i);
		i--;
	}

	p						= new TS_OffsetRange;
	p->uiStart	= uiStart;
	p->uiEnd		= uiEnd;

	m_aOffsetRange.Add(p);
}

/***********************************************************

   RemoveOffsetRange

***********************************************************/
void DBFileReader::RemoveOffsetRange(unsigned int uiStart, unsigned int uiEnd)
{
	TS_OffsetRange *p;
	TS_OffsetRange *pNew;
	
	for(int i=0; i<m_aOffsetRange.GetNbElt(); i++){

		p = m_aOffsetRange[i];
		
		if(p->uiEnd <= uiStart)
			continue;
				
		if(p->uiStart >= uiEnd)
			continue;

		//overlap
		if( (p->uiStart >= uiStart) && (p->uiEnd <= uiEnd)){
			delete m_aOffsetRange.Remove(i);
			i--;
			continue;
		}
			
		if( (p->uiStart < uiStart) && (p->uiEnd > uiEnd)){
			pNew					= new TS_OffsetRange;
			pNew->uiStart = uiEnd;
			pNew->uiEnd		= p->uiEnd;

			m_aOffsetRange.Add(pNew);

			p->uiEnd = uiStart;
			continue;
		}

		if(p->uiStart < uiStart){
			p->uiEnd = uiStart;
			continue;
		}

		if(p->uiEnd > uiEnd)
			p->uiStart = uiEnd;
	}

}

/***********************************************************

   GetNextEntry

***********************************************************/
DBEntry *DBFileReader::GetNextEntry(void)
{
	//parcourt les entrees
	while(DBFile::m_uiCurrentOffset < m_uiEndOffset){
		if(ReadEntry()){

			//progress bar
			if(m_pStdout){
				if(m_uiCurrentOffset >= m_uiNextProgress){

					//nom de la base de donnee
					if(m_uiNextProgress == m_uiStartProgressBar)
						fprintf(m_pStdout, "\n%s :", GetLabel() );
					
					fprintf(m_pStdout, "%d%% ", ((m_uiCurrentOffset - m_uiStartProgressBar) / m_uiStepProgress) * PROGRESS_STEP);

					//if(m_uiCurrentOffset == m_uiEndProgressBar)
					//	fprintf(m_pStdout, "\n");

					m_uiNextProgress += m_uiStepProgress;
				}
			}

			if(IsEntryInRange())
				return this;

		}else
			break;
	}
	
	//passe a l'Offset suivant
	if(SetNextOffsetRange())
		return GetNextEntry();
	
	return NULL;
}

/***********************************************************

   SetNextOffsetRange

***********************************************************/
bool DBFileReader::SetNextOffsetRange(void)
{
	if(++m_iCurrentOffsetIndex < m_aOffsetRange.GetNbElt()){
		TS_OffsetRange *p = m_aOffsetRange[m_iCurrentOffsetIndex];
		DBFile::SeekTo(p->uiStart);
		m_uiEndOffset	= p->uiEnd;
		return true;
	}

	return false;
}

/***********************************************************

   ResetOffsetRange

***********************************************************/
void DBFileReader::ResetOffsetRange(void)
{
	m_iCurrentOffsetIndex = -1;

	if(m_aOffsetRange.GetNbElt() == 0)
		m_uiEndOffset = m_uiEndOfFile;
	else
		m_uiEndOffset = 0;
}

/***********************************************************

   IsEntryInRange

***********************************************************/
bool DBFileReader::IsEntryInRange(void)
{
	if(!m_bUseFilters)
		return true;

	//Mw range
	if( (GetMw() > m_iMwMax) || (GetMw() < m_iMwMin) )
		return false;

	//pi range
	if( (GetU16Pi() > m_usPiMax) || (GetU16Pi() < m_usPiMin) )
		return false;

	//varsplice
	if(m_bRemoveVarsplice && IsVarsplice() )
		return false;

	//fragment
	if(m_bRemoveFragment && IsFragment() )
		return false;

	//KW
	if(!IsKeywordInRange())
		return false;

	//AC list
	if(!m_ACTree.IsEmpty() && !m_ACTree.Get(GetAC()) )
		return false;

	return true;
}

/***********************************************************

	IsKeywordInRange

***********************************************************/
bool DBFileReader::IsKeywordInRange(void)
{
	//l'utilisateur ne veut pas prendre en compte les keywords
	if(m_aUserKW.GetNbElt() == 0)
		return true;

	//NB : les keywords de m_aUserKW ont ete tries par ordre croissant
	int i, j;


	switch(m_eKwMode){

		case KW_MODE_OR:			
			for(i=0; i<m_aUserKW.GetNbElt(); i++){
				for(j=0; j<DBEntry::GetNbKeyword(); j++){
					if(m_aUserKW[i] < DBEntry::GetKeyword(j))
						break;

					if(m_aUserKW[i] == DBEntry::GetKeyword(j))
						return true;
				}
			}

			return false;
			break;


		case KW_MODE_AND:

			//pas assez de keyword
			if(DBEntry::GetNbKeyword() < m_aUserKW.GetNbElt())
				return false;
			
			i=0;
			j=0;

			while(j < DBEntry::GetNbKeyword()){

				if(m_aUserKW[i] == DBEntry::GetKeyword(j)){
					i++;
					j++;
				}else{
					j++;
				}

				if(i == m_aUserKW.GetNbElt())
					return true;
			}

			return false;
			break;


		default:
			ThrowError("DBFileReader::IsKeywordInRange", "Unknown keyword mode %d", m_eKwMode);
			return false;
	}
}

/***********************************************************

   GetEntry

***********************************************************/
DBEntry *DBFileReader::GetEntry(unsigned int uiOffset)
{
	DBFile::SeekTo(uiOffset);

	if(!ReadEntry())
		ThrowError("DBFileReader::GetEntry", "Enable to read entry at offset %d", uiOffset);

	return this;
}

/***********************************************************

   SetMwRange

***********************************************************/
void DBFileReader::SetMwRange(int iMin, int iMax)
{
	m_iMwMin = iMin;
	m_iMwMax = iMax;	
	m_bUseFilters = true;
}

/***********************************************************

   SetPIRange

***********************************************************/
void DBFileReader::SetPIRange(double dMin, double dMax)
{
	m_usPiMin = (unsigned short)(dMin * 100 + 0.5);
	m_usPiMax = (unsigned short)(dMax * 100 + 0.5);
	m_bUseFilters = true;
}

/***********************************************************

   RemoveVarsplice

***********************************************************/
void DBFileReader::RemoveVarsplice( bool bRemove)
{
	m_bRemoveVarsplice = bRemove; 
	m_bUseFilters = true;
}

/***********************************************************

   RemoveFragment

***********************************************************/
void DBFileReader::RemoveFragment( bool bRemove)
{
	m_bRemoveFragment = bRemove;
	m_bUseFilters = true;
}

/***********************************************************

   SetKeyword

***********************************************************/
void DBFileReader::SetKeyword(int *pKWArray, int iNbKW, TE_KwMode eMode)
{
	m_eKwMode = eMode;

	if(iNbKW){
		for(int i=0; i<iNbKW; i++)
			m_aUserKW.Add((unsigned short)pKWArray[i]);

		m_aUserKW.Sort(SortKeyword);
		m_bUseFilters = true;

	}else{
		m_aUserKW.Reset();
	}

}


/***********************************************************

   SetAC

***********************************************************/
void DBFileReader::AddAC(const char *pszAC)
{
	m_ACTree.Add(pszAC);
	m_bUseFilters	= true;
}


/***********************************************************

   ResetSearch

***********************************************************/
void DBFileReader::ResetSearch(void)
{
	ResetOffsetRange();
	SeekToEntriesSection(); 

	if(m_aOffsetRange.GetNbElt()){
		m_uiStartProgressBar	= m_aOffsetRange[0]->uiStart;
		m_uiEndProgressBar		= m_aOffsetRange.GetLastElt()->uiEnd;	
	}else{
		m_uiStartProgressBar	= m_uiEntriesOffset;
		m_uiEndProgressBar		= m_uiEndOfFile;
	}

	m_uiStepProgress	= (m_uiEndProgressBar - m_uiStartProgressBar) / PROGRESS_STEP;
	m_uiNextProgress	= m_uiStartProgressBar;

	m_uiStepProgress = MAX(m_uiStepProgress, 1);
}
