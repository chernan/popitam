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
  Last modification  : 20.02.2008
	Comments           : 

***********************************************************/

#ifndef __DBFILE_H__
#define __DBFILE_H__

#include "dbentry.h"

#define DB_LICENSED				0x10101010
#define DB_NOT_LICENSED		0x01010101


#define DB_LABEL_LEN		64
#define DB_RELEASE_LEN	64

#pragma pack(1)

//header
typedef struct{
	char					szCheck[8];
	int						iVersion;
	unsigned int  uiTime;
	char					szLabel[DB_LABEL_LEN];
	char					szRelease[DB_RELEASE_LEN];
	int 					iNbPtm;
	int 					iNbTaxId;
	int 					iNbEntries;
	unsigned int	uiLicense;
}TS_FileHeader;

//taxonomy
typedef struct{
	unsigned int uiStart;
	unsigned int uiEnd;
}TS_OffsetRange;

//Ptm
#define PTM_LABEL_LEN	12
typedef struct{
	char		szLabel[PTM_LABEL_LEN];
	double	dMass;
}TS_PtmDef;

#pragma pack()



typedef enum{
	OPEN_FOR_READ = 0,
	OPEN_FOR_WRITE
}TE_OpenFileMode;



/******************************************************

Class DBFile


*******************************************************/

class DBFile : public DBEntry
{
public :
	static int			iVersion;

private :
	static char			szCheck[8];

protected :
		
	File									m_file;
	unsigned int					m_uiCurrentOffset;
	TS_FileHeader					m_stFileHeader;
	DynamicArray<DBPtm *> m_aPtmList;

	unsigned int					m_uiPtmOffset;
	unsigned int					m_uiTaxIdOffset;
	unsigned int					m_uiEntriesOffset;

	unsigned int					m_uiEndOfFile;

	bool									m_bLicensed;

protected :

	DBFile(void);
	~DBFile(void);

public :

	void Close(void);

	inline const char *GetLabel(void)
	{ return m_stFileHeader.szLabel; }

	inline const char *GetRelease(void)
	{ return m_stFileHeader.szRelease; }

	inline unsigned int GetTime(void)
	{ return m_stFileHeader.uiTime; }

	inline int GetNbEntries(void)
	{ return m_stFileHeader.iNbEntries; }

	inline DBPtm *GetPtm(int i)
	{ return m_aPtmList[i]; }

	inline bool IsLicensed(void)
	{ return m_bLicensed; }

	inline void SetLicensed(bool bLicensed)
	{ m_bLicensed = bLicensed; }

protected:

	void Open(const char *pszFileName, TE_OpenFileMode eMode);

	void ReadPtm(void);
	void WritePtm(void);

	inline unsigned int GetTaxIdOffset(void)
	{ return m_uiTaxIdOffset; }

	inline unsigned int GetCurrentOffset(void)
	{ return m_uiCurrentOffset; }

	void SeekToEntriesSection(void);
	void SeekTo(unsigned int uiOffset);

	void ReadFileHeader(void);
	void WriteFileHeader(void);

	void ResetFileHeader(void);

	void WriteEntry(void);


	//SET
	void SetLabel(const char *pszLabel);
	void SetRelease(const char *pszRelease);

private :

	void GetFileSize(void);

};


#endif
