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
  Last modification  : 19.02.2008
	Comments           : 

***********************************************************/


#include "parserCamsi.h"
#include "error.h"
#include "dbentry.h"
#include "util.h"

#include <string.h>
#include <time.h>


/******************************************************

		Constructor

*******************************************************/
ParserCamsi::ParserCamsi(void)
{
}


/******************************************************

		Destructor

*******************************************************/
ParserCamsi::~ParserCamsi(void)
{
	m_aUnknownModRes.DeletePtrs();
}



/***********************************************************

   Run

 ***********************************************************/
void ParserCamsi::Run(const char *pszFastaFileName, const char *pszDBFileName, const char *pszDBLabel,
								 const char *pszTaxonomyFileName, TE_Taxonomy eTaxonomy, const char *pszPtmFileName,
								 const char *pszDBRelease)
{
	char		szDate[32];
	//struct tm	*stTime;
	//time_t		lTime;

	//recupere la date
	//time(&lTime);
	//stTime = gmtime(&lTime);
	//sprintf(szDate, "%02d/%02d/%d", stTime->tm_mday, stTime->tm_mon +1, stTime->tm_year +1900);
	sprintf(szDate, "%s", pszDBRelease);
	
	m_fastaFile.Open(pszFastaFileName, "r");
	char c;
	if(fscanf(m_fastaFile, "%c", &c) != 1)
		ThrowError("ParserCamsi::Run", "file is empty");
	if(c != '>')
		ThrowError("ParserCamsi::Run", "Entry should start with '>'");

	DBFileWriter::Run(pszDBFileName, szDate, pszDBLabel, pszTaxonomyFileName, eTaxonomy, pszPtmFileName);
	m_fastaFile.Close();


	if(m_aUnknownModRes.GetNbElt()){
		printf("\n***** WARNING *****\nUnknown MODRES :\n");
		for(int i=0; i<m_aUnknownModRes.GetNbElt(); i++)
			printf("%s\n", m_aUnknownModRes[i]);
	}

	m_aUnknownModRes.DeletePtrs();
}



/***********************************************************

   ParserCallBack

 ***********************************************************/
void ParserCamsi::ParserCallBack(void)
{
	printf("- parsing...\n");
	while(Read())
		Write();
}



/***********************************************************

   Read

***********************************************************/

// reader pour format:
// Q4U9M9|104K_THEAN 104 kDa microneme/rhoptry antigen precursor - Theileria annulata

bool ParserCamsi::Read(void)
{
	Reset();

	if(!fgets(m_szHeader, LINE_LEN, m_fastaFile))
			return false;

	strtok(m_szHeader, "\r\n");

			
	m_pszAC = m_szHeader;
	if(!m_pszAC)
		ThrowError("ParserCamsi::Read", "No AC defined");
    
	m_pszID = "NO_ID";	
	m_pszDE = "NO_DE";

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
void ParserCamsi::Write(void)
{
	//No keywords
	DynamicArray<unsigned short> aKW;
	
	DBFileWriter::Write(m_pszAC, m_pszID, m_pszDE, m_aSQ, m_iTaxId, 0, 0, 0, m_aSQ.GetNbElt() -1, aKW, m_aPtm);
}


/***********************************************************

   RemoveEndWhiteCharacters

 ***********************************************************/
char *ParserCamsi::RemoveEndWhiteCharacters(char *psz)
{
	char *p = psz + strlen(psz) -1;

	while( (p >= psz) && (*p == ' ' || *p == '\t') ){
		*p = '\0';
		p--;
	}

	return psz;
}

/***********************************************************

   Reset

 ***********************************************************/
void ParserCamsi::Reset(void)
{
	m_pszAC		= NULL;
	m_pszID		= NULL;
	m_pszACOR = NULL;
	m_pszDE		= NULL;
	m_iTaxId	= 0;
	m_iSQLen	= 0;

	m_aSQ.Reset();

	m_aPtm.Reset();
}

/***********************************************************

   ReadPtm

 ***********************************************************/
void ParserCamsi::ReadPtm(char *psz)
{
	DynamicArray<char *> aToken;

	char *p = strtok(psz, "()");
	if(!p)
		return;

	while(p){
		aToken.Add(p);
		p = strtok(NULL, "()");
	}
	
	
	TS_Ptm	*pstPtm;
	DBPtm		*pPtm;
	int			iPos;

	for(int i=0; i<aToken.GetNbElt(); i++){

		p = strtok(aToken[i], "|");

		if(!p || sscanf(p, "%d", &iPos) != 1)
			ThrowError("ParserCamsi::ReadPtm", "Ptm are bad formated");


		p = strtok(NULL, "|");

		if(!p)
			ThrowError("ParserCamsi::ReadPtm", "Ptm are bad formated");


		pPtm = DBFileWriter::GetPtm(p);

		if(pPtm){
			pstPtm = m_aPtm.Add();
			pstPtm->iPos = 0;
			pstPtm->pPtm = pPtm;
		}else{
			AddUnknownModRes(p);
		}

	}

}

/***********************************************************

   AddUnknownModRes

 ***********************************************************/
void ParserCamsi::AddUnknownModRes(const char *psz)
{

	for(int i=0; i<m_aUnknownModRes.GetNbElt(); i++){	
		if(!strcmp(m_aUnknownModRes[i], psz))
			return;
	}

	m_aUnknownModRes.Add( Strdup(psz) );
}
