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
	Created on         : 19.07.2006
	Last modification  : 19.07.2006
	Comments           : 
	
***********************************************************/

#ifndef __PARSER_PHENYX_H__
#define __PARSER_PHENYX_H__


#include "dbfilewriter.h"


#define LINE_LEN 32768

/******************************************************

Class ParserPhenyx


*******************************************************/

class ParserPhenyx : public DBFileWriter
{

private :

	File		m_fastaFile;
	char		m_szHeader[LINE_LEN];

	DynamicArray<char *>	m_aToken;

	const char						*m_pszAC;
	const char 						*m_pszID;
	const char 						*m_pszACOR;
	const char 						*m_pszDE;
	int										m_iTaxId;
	int										m_iSQLen;
	DynamicArray<char>		m_aSQ;

	DynamicArray<TS_Ptm>	m_aPtm;

	DynamicArray<char *>	m_aUnknownModRes;

public :

	ParserPhenyx(void);
	~ParserPhenyx(void);

	void Run(	const char *pszFastaFileName, const char *pszDBFileName, const char *pszDBLabel,
						const char *pszTaxonomyFileName, TE_Taxonomy eTaxonomy, const char *pszPtmFileName,
						const char *pszDBRelease);

	void ParserCallBack(void);

private :

	
	bool Read(void);
	void Write(void);

	void ReadPtm(char *psz);

	char *RemoveEndWhiteCharacters(char *psz);

	void Reset(void);


	void AddUnknownModRes(const char *psz);

};


#endif
