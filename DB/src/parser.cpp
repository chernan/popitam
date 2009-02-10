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

  Company	     : Swiss Intitute of Bioinformatics
	Author		   : Marc Tuloup
	Contact		   : Marc.Tuloup@isb-sib.ch
	Created	on	   : 19.07.2006
	Last modification  : 19.07.2006
	Comments	   : 

***********************************************************/


#include "parser.h"
#include "MATerror.h"
#include "dbentry.h"

#include <string.h>
#include <time.h>

extern char head[256];
/******************************************************

		Constructor

*******************************************************/
Parser::Parser(void)
{
}


/******************************************************

		Destructor

*******************************************************/
Parser::~Parser(void)
{
}


/***********************************************************

   Run

 ***********************************************************/
void Parser::Run(const char *pszFastaFileName, const char *pszDBFileName, const char *pszDBLabel,
									const char *pszDBRelease, const char *pszTaxonomyFileName, const char *pszPtmFileName)
{	
	m_fastaFile.Open(pszFastaFileName, "r");
	char c;
	if(fscanf(m_fastaFile, "%c", &c) != 1)
		ThrowError("Parser::Run", "file is empty");
	if(c != '>')
		ThrowError("Parser::Run", "Entry should start with '>'");

	DBFileWriter::Run(pszDBFileName, pszDBRelease, pszDBLabel, pszTaxonomyFileName, pszPtmFileName);
	m_fastaFile.Close();
}


/***********************************************************

   ParserCallBack

 ***********************************************************/
void Parser::ParserCallBack(void)
{
	if (!strncmp(head, "PHENYX", 6))
	{
		while(ReadPhenyxFormat())
			Write();
	}
	if (!strncmp(head, "SBG", 3))
	{
		while(ReadSBGFormat())
			Write();
	}
	if (!strncmp(head, "LIGHT", 5))
	{
		while(Read())
			Write();
	}
}


/***********************************************************

   Read

***********************************************************/

// reader pour format:
// Q4U9M9|104K_THEAN 104 kDa microneme/rhoptry antigen precursor - Theileria annulata

bool Parser::Read(void)
{
	if(!fgets(m_szHeader, LINE_LEN, m_fastaFile))
			return false;
	
	char *psz = strtok(m_szHeader, "\r\n");
	if(!psz)
		ThrowError("Parser::Read", "No white line allowed");

	m_pszAC = strtok(m_szHeader, "|");
	if(!m_pszAC)
		ThrowError("Parser::Read", "No AC defined");
    
	m_pszID = strtok(NULL, " ");	
	if(!m_pszID)
		ThrowError("Parser::Read", "In AC='%s' No ID defined", m_pszAC);

	m_pszDE = strtok(NULL, "|");
	if(!m_pszDE)
		ThrowError("Parser::Read", "In AC='%s' No DE defined", m_pszAC);


	
	//SQ
	m_aSQ.Reset();
	char c;
	while(fread(&c, 1, 1, m_fastaFile) ==1){

		if(c == '>')
			break;
		
		if(strchr("* \t\r\n", c))
			continue;

		m_aSQ.Add(c);
	}
	
	m_aSQ.Add('\0');

	return true;
}

// reader pour format
// >sp_SHUFFLE|P48668|K2C6C_HUMAN Keratin, type II cytoskeletal 6C - Homo sapiens (Human).
bool Parser::ReadSBGFormat(void)
{
	if(!fgets(m_szHeader, LINE_LEN, m_fastaFile))
			return false;

	char *psz = strtok(m_szHeader, "\r\n");
	if(!psz)
		ThrowError("Parser::Read", "No white line allowed");

	m_pszAC = strtok(m_szHeader, "|"); // saute le sp| ou sp_SHUFFLE|
	m_pszAC = strtok(NULL, "|");
	if(!m_pszAC)
		ThrowError("Parser::Read", "No AC defined");

	m_pszID = strtok(NULL, " ");	
	if(!m_pszID)
		ThrowError("Parser::Read", "In AC='%s' No ID defined", m_pszAC);

	m_pszDE = strtok(NULL, "|");
	if(!m_pszDE)
		ThrowError("Parser::Read", "In AC='%s' No DE defined", m_pszAC);


	
	//SQ
	m_aSQ.Reset();
	char c;
	while(fread(&c, 1, 1, m_fastaFile) ==1){

		if(c == '>')
			break;
		
		if(strchr("* \t\r\n", c))
			continue;

		m_aSQ.Add(c);
	}
	
	m_aSQ.Add('\0');

	return true;
}

// reader pour format
// >DECOY_P17958 \ID=YALV_TRYBB \MODRES= \NCBITAXID=5702 \DE=Uncharacterized 25.6 kDa protein in aldolase locus (ORFV).
bool Parser::ReadPhenyxFormat(void)
{
	if(!fgets(m_szHeader, LINE_LEN, m_fastaFile))
			return false;

	char *psz = strtok(m_szHeader, "\r\n");
	if(!psz)
		ThrowError("Parser::Read", "No white line allowed");

	m_pszAC = strtok(m_szHeader, " ");
	if(!m_pszAC)
		ThrowError("Parser::Read", "No AC defined");
    //printf("AC==%s\n", m_pszAC);

	m_pszID = strtok(NULL, " ");	
	m_pszID = m_pszID+4;
	if(!m_pszID)
		ThrowError("Parser::Read", "In AC='%s' No ID defined", m_pszAC);
	//printf("ID==%s\n", m_pszID);
	
	m_pszDE = strtok(NULL, "\r\n");
	//printf("DE==%s\n", m_pszDE);
	
	if(!m_pszDE)
		ThrowError("Parser::Read", "In AC='%s' No DE defined", m_pszAC);
	
	//SQ
	m_aSQ.Reset();
	char c;
	while(fread(&c, 1, 1, m_fastaFile) ==1){

		if(c == '>')
			break;
		
		if(strchr("* \t\r\n", c))
			continue;

		m_aSQ.Add(c);
	}
	
	m_aSQ.Add('\0');

	return true;
}
/***********************************************************

   Write

***********************************************************/
void Parser::Write(void)
{
	DynamicArray<unsigned short> aKW;
	DynamicArray<TS_Ptm> aPtm;

	// void Write(	const char *pszAC, const char *pszID, const char *pszDE, const char *pszSQ, 
	//							int iTaxId, unsigned char ucType, int iChildId, int iChildStart, int iChildEnd,
	//							DynamicArray<unsigned short> &aKW, DynamicArray<TS_Ptm> &aPtm);
	DBFileWriter::Write(m_pszAC, m_pszID, m_pszDE, m_aSQ, 1/*taxid*/, 0, 0, 0, m_aSQ.GetNbElt() -1, aKW, aPtm);
}




