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



#ifndef __DBFILEWRITER_H__
#define __DBFILEWRITER_H__

#include "pimw.h"
#include "dbfile.h"
#include "taxonomy.h"
#include "ptmsearchmanager.h"


/******************************************************

Class DBFileWriter


*******************************************************/

class DBFileWriter : public DBFile
{
	friend class Taxon;

private :

	char							m_szTmpFileName[FILE_PATH_LEN];
	Taxonomy					m_taxonomy;
	
	File							m_tmpFile;
	unsigned int			m_uiTmpOffset;

	PiMw							m_piMw;

	bool							m_bLoaded;

	
	PtmSearchManager	m_ptmSearch;

public :

	DBFileWriter(void);
	virtual ~DBFileWriter(void);

	void Load(const char *pszTaxonomyFile, TE_Taxonomy eTaxonomy, const char *pszPtmFileName);	

	void Run(	const char *pszDBFileName, const char *pszDBRelease, const char *pszDBLabel, 
						bool bLicensed=true);


	void Run(	const char *pszDBFileName, const char *pszDBRelease, const char *pszDBLabel, 
						const char *pszTaxonomyFileName, TE_Taxonomy eTaxonomy, const char *pszPtmFileName, 
						bool bLicensed=true);

	virtual void ParserCallBack(void) = 0;
	

	void Write(	const char *pszAC, const char *pszID, const char *pszDE, const char *pszSQ, 
							int iTaxId, unsigned char ucType, int iChildId, int iChildStart, int iChildEnd,
							DynamicArray<unsigned short> &aKW, DynamicArray<TS_Ptm> &aPtm);
	

	inline int GetNbUnclassified(void)
	{ return m_taxonomy.GetNbUnclassified(); }

	DBPtm *GetPtm(const char *psz);

private :

	void Reset(void);

	void OpenTmpFile(TE_OpenFileMode eMode);

	inline void CloseTmpFile(void)
	{ m_tmpFile.Close(); }
	
	void SortEntriesByTaxId(void);
	void SortEntry(unsigned int uiTmpOffset);

};


#endif
