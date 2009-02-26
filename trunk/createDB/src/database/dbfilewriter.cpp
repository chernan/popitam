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


#include "dbfilewriter.h"
#include "error.h"
#include "util.h"
#include "atomicmass.h"



/******************************************************

		Constructor

*******************************************************/
DBFileWriter::DBFileWriter(void) :
	m_piMw(MASS_AVERAGE),
	m_bLoaded(false)
{	
}

/******************************************************

		Destructor

*******************************************************/
DBFileWriter::~DBFileWriter(void)
{
}

/***********************************************************

   Reset

***********************************************************/
void DBFileWriter::Reset(void)
{
	m_taxonomy.ResetOffset();
}

/***********************************************************

   Load

 ***********************************************************/
void DBFileWriter::Load(const char *pszTaxonomyFile, TE_Taxonomy eTaxonomy, const char *pszPtmFileName)
{
	//taxonomy
	m_taxonomy.Load(pszTaxonomyFile, eTaxonomy);
	
	//PTM
	m_ptmSearch.Load(pszPtmFileName);

	DynamicArray<PtmData *> &aPtmData = m_ptmSearch.GetPtmdataArray();

	AtomicMass atomicMass(MASS_MONOISOTOPIC);

	m_aPtmList.EnsureSize(aPtmData.GetNbElt());

	PtmData *pPtmData;
	for(int i=0; i<aPtmData.GetNbElt(); i++){
		pPtmData = aPtmData[i];
		m_aPtmList.Add( new DBPtm(i, pPtmData->GetAbv(), atomicMass.GetMass( pPtmData->GetFormula() ) ) );
	}

	m_bLoaded = true;
}

/***********************************************************

   Run

it is possible to call Load only once and then call this implementation of Run several times.
 ***********************************************************/
void DBFileWriter::Run(const char *pszDBFileName, const char *pszDBRelease, const char *pszDBLabel, 
											 bool bLicensed)
{
	if(!m_bLoaded)
		ThrowError("DBFileWriter::Run", "You must call Load() before Run()");

	Reset();

	//write unsorted entries in tmp file
	sprintf(m_szTmpFileName, "%s_tmp", pszDBFileName);
	OpenTmpFile(OPEN_FOR_WRITE);
	ParserCallBack();							//donne la main a la classe qui herite de DBFileWriter
	CloseTmpFile();


	//sort entries in final output file
	DBFile::Open(pszDBFileName, OPEN_FOR_WRITE);
	DBFile::SetLabel(pszDBLabel);

	if(pszDBRelease == NULL){
	
		//time
		time_t lTime;
		time(&lTime); 
		m_stFileHeader.uiTime = (unsigned int)lTime;

		//default Release
		char szDate[32];
		DBFile::SetRelease( FormatDate(lTime, szDate) );

	}else{
		DBFile::SetRelease(pszDBRelease);
	}

	DBFile::SetLicensed(bLicensed);

	m_stFileHeader.iNbTaxId	= m_taxonomy.GetNbTaxId();
	SortEntriesByTaxId();
	DBFile::Close();

	//remove tmp file
	remove(m_szTmpFileName);
}


/***********************************************************

   Run

Default call to Run
 ***********************************************************/
void DBFileWriter::Run(const char *pszDBFileName, const char *pszDBRelease, const char *pszDBLabel, 
											 const char *pszTaxonomyFileName,TE_Taxonomy eTaxonomy, const char *pszPtmFileName, bool bLicensed)
{
	Load(pszTaxonomyFileName, eTaxonomy, pszPtmFileName);
	Run(pszDBFileName, pszDBRelease, pszDBLabel, bLicensed);
}



/***********************************************************

   OpenTmpFile

***********************************************************/
void DBFileWriter::OpenTmpFile(TE_OpenFileMode eMode)
{
	switch(eMode){
		case OPEN_FOR_READ:
			m_tmpFile.Open(m_szTmpFileName, "rb");
			break;

		case OPEN_FOR_WRITE:
			m_tmpFile.Open(m_szTmpFileName, "wb");
			break;

		default:
			ThrowError("DBFileWriter::OpenTmpFile", "Unknown open mode %d", eMode);
	}

	m_uiTmpOffset	= 0;
}




/***********************************************************

   SortEntry

***********************************************************/
void DBFileWriter::SortEntry(unsigned int uiTmpOffset)
{
	fseek(m_tmpFile, uiTmpOffset, SEEK_SET);

	DBEntry::Read(m_tmpFile,	&m_uiTmpOffset, this);
	DBFile::WriteEntry();
}


/***********************************************************

   SortEntriesByTaxId

***********************************************************/
void DBFileWriter::SortEntriesByTaxId(void)
{	
	//printf("- Sorting Entries by taxId...\n");

	//Header
	DBFile::WriteFileHeader();

	//write Ptm
	DBFile::WritePtm();

	//taxo : l'info n'est pas encore connue reserve la place dans le fichier
	m_taxonomy.WriteTaxIdOffset(m_file);

	//Entries
	m_uiCurrentOffset = m_uiEntriesOffset;
	OpenTmpFile(OPEN_FOR_READ);
	m_taxonomy.WriteEntries(this);
	CloseTmpFile();


	//taxo : l'info est connue => update
	fseek(m_file, DBFile::GetTaxIdOffset(), SEEK_SET);
	m_taxonomy.WriteTaxIdOffset(m_file);

	//update nb entries
	fseek(m_file, 0, SEEK_SET);
	DBFile::WriteFileHeader();
}

/***********************************************************

   Write

 ***********************************************************/
void DBFileWriter::Write(const char *pszAC, const char *pszID, const char *pszDE, const char *pszSQ, 
						int iTaxId, unsigned char ucType, int iChildId, int iChildStart, int iChildEnd,
						DynamicArray<unsigned short> &aKW, DynamicArray<TS_Ptm> &aPtm)
{
	int			iMw = (int)(m_piMw.GetMw(pszSQ) + 0.5);
	double	dPi = m_piMw.GetPi(pszSQ);

	//store data
	DBEntry::Set(pszAC, pszID, pszDE, pszSQ, iTaxId, iMw, dPi, ucType, iChildId, iChildStart, iChildEnd, aKW, aPtm);

	//write in tmp file
	DBEntry::Write(m_tmpFile, &m_uiTmpOffset);

	//stoke l'offset de cette entree pour ce taxon
	m_taxonomy.GetTaxon(iTaxId)->AddOffset(DBEntry::GetOffset());
}
/***********************************************************

   GetPtm

 ***********************************************************/
DBPtm *DBFileWriter::GetPtm(const char *psz)
{
	int iId = m_ptmSearch.GetPtmId(psz);

	if(iId == -1 )
		return NULL;

	return m_aPtmList[iId];
}
