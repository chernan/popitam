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
  Last modification  : 23.02.2007
	Comments           : 

***********************************************************/


#include "dbfile.h"
#include "MATerror.h"
#include "util.h"

#include <time.h>


char DBFile::szCheck[8] = "DBFILE";
int DBFile::iVersion		= 5;


/******************************************************

		Constructor

*******************************************************/
DBFile::DBFile(void) :
	m_bLicensed(false)
{	
}


/******************************************************

		Destructor

*******************************************************/
DBFile::~DBFile(void)
{
	m_aPtmList.DeletePtrs();
}


/***********************************************************

   Open

***********************************************************/
void DBFile::Open(const char *pszFileName, TE_OpenFileMode eMode)
{	
	switch(eMode){
		case OPEN_FOR_READ:
			m_file.Open(pszFileName, "rb");
			ReadFileHeader();
			ReadPtm();
			m_uiCurrentOffset = m_uiTaxIdOffset;
			break;

		case OPEN_FOR_WRITE:
			{
			m_file.Open(pszFileName, "wb");
			ResetFileHeader();
			m_uiCurrentOffset = 0;
			break;
			}

		default:
			ThrowError("DBFile::Open", "Unknown open mode %d", eMode);
	}

	
}


/***********************************************************

   GetFileSize

***********************************************************/
void DBFile::GetFileSize(void)
{
	long lOffset = ftell(m_file);

	fseek(m_file, 0, SEEK_END);
	m_uiEndOfFile = (unsigned int)ftell(m_file);

	fseek(m_file, lOffset, SEEK_SET);
}

/***********************************************************

   Close

***********************************************************/
void DBFile::Close(void)
{
	m_file.Close();
}


/***********************************************************

   SeekToEntriesSection

***********************************************************/
void DBFile::SeekToEntriesSection(void)
{
	fseek(m_file, m_uiEntriesOffset, SEEK_SET);

	m_uiCurrentOffset = m_uiEntriesOffset;
}

/***********************************************************

   SeekTo

***********************************************************/
void DBFile::SeekTo(unsigned int uiOffset)
{
	fseek(m_file, uiOffset, SEEK_SET);
	m_uiCurrentOffset = uiOffset;
}


/***********************************************************

   ReadPtm

***********************************************************/
void DBFile::ReadPtm(void)
{
	TS_PtmDef stPtmDef;
	m_aPtmList.EnsureSize(m_stFileHeader.iNbPtm);

	for(int i=0; i<m_stFileHeader.iNbPtm; i++){
		if(fread(&stPtmDef, sizeof(TS_PtmDef), 1, m_file) != 1)
			ThrowError("DBFile::ReadPtm", "Unable to read ptm in file \"%s\"", m_file.GetFileName());

		m_aPtmList.Add(new DBPtm(i, stPtmDef.szLabel, stPtmDef.dMass));
	}
}


/***********************************************************

   WritePtm

***********************************************************/
void DBFile::WritePtm(void)
{
	TS_PtmDef stPtmDef;
	DBPtm *pPtm;

	for(int i=0; i<m_aPtmList.GetNbElt(); i++){
		pPtm = m_aPtmList[i];

		if(strlen(pPtm->GetLabel()) >= PTM_LABEL_LEN)
			ThrowError("DBFile::WritePtm", "Ptm label \"%s\" should be %d character max", pPtm->GetLabel(), PTM_LABEL_LEN);

		strcpy(stPtmDef.szLabel, pPtm->GetLabel());
		stPtmDef.dMass = pPtm->GetMass();

		fwrite(&stPtmDef, sizeof(TS_PtmDef), 1, m_file);
	}
}


/***********************************************************

   ResetFileHeader

***********************************************************/
void DBFile::ResetFileHeader(void)
{
	memset(&m_stFileHeader, 0, sizeof(TS_FileHeader));

	//check et version
	strcpy(m_stFileHeader.szCheck,	DBFile::szCheck);
	m_stFileHeader.iVersion	= DBFile::iVersion;
}


/***********************************************************

   ReadFileHeader

***********************************************************/
void DBFile::ReadFileHeader(void)
{
	GetFileSize();

	//read file header
	if(fread(&m_stFileHeader, sizeof(TS_FileHeader), 1, m_file) != 1)
		ThrowError("DBFile::ReadFileHeader", "Unable to read header in file \"%s\"", m_file.GetFileName());

	if(strcmp(m_stFileHeader.szCheck, DBFile::szCheck))
		ThrowError("DBFile::ReadFileHeader", "File \"%s\" is not a DB file", m_file.GetFileName());

	if(m_stFileHeader.iVersion != DBFile::iVersion)
		ThrowError("DBFile::ReadFileHeader", "File \"%s\" is version %d, should be %d", m_file.GetFileName(), m_stFileHeader.iVersion, DBFile::iVersion); 


	//Section file offsets
	m_uiPtmOffset			= sizeof(TS_FileHeader);
	m_uiTaxIdOffset		= m_uiPtmOffset		+ (m_stFileHeader.iNbPtm * sizeof(TS_PtmDef));
	m_uiEntriesOffset = m_uiTaxIdOffset + (m_stFileHeader.iNbTaxId * sizeof(TS_OffsetRange));


	//check license
	if(m_stFileHeader.uiLicense == m_uiEndOfFile + DB_LICENSED)
		m_bLicensed = true;
	else if(m_stFileHeader.uiLicense == m_uiEndOfFile + DB_NOT_LICENSED)
		m_bLicensed = false;
	else
		ThrowError("DBFile::ReadFileHeader", "Error on file licensing");
}	

/***********************************************************

   WriteFileHeader

***********************************************************/
void DBFile::WriteFileHeader(void)
{
	GetFileSize();

	m_stFileHeader.iNbPtm = m_aPtmList.GetNbElt();

	//Section file offsets
	m_uiPtmOffset			= sizeof(TS_FileHeader);
	m_uiTaxIdOffset		= m_uiPtmOffset		+ (m_stFileHeader.iNbPtm * sizeof(TS_PtmDef));
	m_uiEntriesOffset = m_uiTaxIdOffset + (m_stFileHeader.iNbTaxId * sizeof(TS_OffsetRange));

	//lisence
	if(m_bLicensed)
		m_stFileHeader.uiLicense = m_uiEndOfFile + DB_LICENSED;
	else
		m_stFileHeader.uiLicense = m_uiEndOfFile + DB_NOT_LICENSED;

	
	fwrite(&m_stFileHeader, sizeof(TS_FileHeader), 1, m_file);
}


/***********************************************************

   SetLabel

***********************************************************/
void DBFile::SetLabel(const char *pszLabel)
{
	if(strlen(pszLabel) >= DB_LABEL_LEN)
		ThrowError("DBFile::SetLabel", "Label should have less than %d characters", DB_LABEL_LEN);

	strcpy(m_stFileHeader.szLabel, pszLabel);
}

/***********************************************************

   SetRelease

***********************************************************/
void DBFile::SetRelease(const char *pszRelease)
{
	if(strlen(pszRelease) >= DB_RELEASE_LEN)
		ThrowError("DBFile::SetRelease", "Release should have less than %d characters", DB_RELEASE_LEN);

	strcpy(m_stFileHeader.szRelease, pszRelease);
}

/***********************************************************

   WriteEntry

***********************************************************/
void DBFile::WriteEntry(void)
{
	m_stFileHeader.iNbEntries++;
	DBEntry::Write(m_file, &m_uiCurrentOffset);
}

