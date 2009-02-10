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

#ifndef __PARSER_H__
#define __PARSER_H__

#include "file.h"
#include "dbfilewriter.h"
#include "pimw.h"


#define LINE_LEN 1024

/******************************************************

Class Parser


*******************************************************/

class Parser : public DBFileWriter
{

private :

	File								m_fastaFile;
	char								m_szHeader[LINE_LEN];

	const char					*m_pszAC;
	const char					*m_pszID;
	const char 					*m_pszDE;
	DynamicArray<char>	m_aSQ;

public :

	Parser(void);
	~Parser(void);

	void Run(	const char *pszFastaFileName, const char *pszDBFileName, const char *pszDBLabel,
						const char *pszDBRelease,	const char *pszTaxonomyFileName, const char *pszPtmFileName);

	void ParserCallBack(void);

private:

	bool Read(void);
	bool ReadSBGFormat(void);
	bool ReadPhenyxFormat(void);
	void Write(void);
};


#endif
