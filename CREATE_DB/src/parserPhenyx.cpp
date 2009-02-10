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


#include "parserPhenyx.h"
#include "error.h"
#include "dbentry.h"
#include "util.h"

#include <string.h>
#include <time.h>


/******************************************************

		Constructor

*******************************************************/
ParserPhenyx::ParserPhenyx(void)
{
}


/******************************************************

		Destructor

*******************************************************/
ParserPhenyx::~ParserPhenyx(void)
{
	m_aUnknownModRes.DeletePtrs();
}



/***********************************************************

   Run

 ***********************************************************/
void ParserPhenyx::Run(const char *pszFastaFileName, const char *pszDBFileName, const char *pszDBLabel,
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
		ThrowError("ParserPhenyx::Run", "file is empty");
	if(c != '>')
		ThrowError("ParserPhenyx::Run", "Entry should start with '>'");

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
void ParserPhenyx::ParserCallBack(void)
{
	printf("- parsing...\n");
	while(Read())
		Write();
}



/***********************************************************

   Read

***********************************************************/
bool ParserPhenyx::Read(void)
{
	Reset();

	if(!fgets(m_szHeader, LINE_LEN, m_fastaFile))
			return false;

	strtok(m_szHeader, "\r\n");

			
	m_pszAC = strtok(m_szHeader, " \\");

	char *pszToken;
	m_aToken.Reset();
	while( (pszToken = strtok(NULL, "\\")) )
		m_aToken.Add(pszToken);

	
	char *pszLabel;
	char *pszValue;

	int i;
	for(i=0; i<m_aToken.GetNbElt(); i++){
		pszLabel	= strtok(m_aToken[i], "=");
		pszValue	= strtok(NULL, "\\");

		if(!pszLabel)
			ThrowError("ParserPhenyx::Read", "bad format");

		if(!pszValue)
			pszValue = "";
		else
			pszValue = RemoveEndWhiteCharacters(pszValue);

		switch(pszLabel[0]){

			case 'I':
				if(!strcmp(pszLabel, "ID"))
					m_pszID = pszValue;
				break;

			case 'A':
				if(!strcmp(pszLabel, "ACOR"))
					m_pszACOR = pszValue;
				break;

			case 'D':
				if(!strcmp(pszLabel, "DE"))
					m_pszDE = pszValue;
				break;

			case 'N':
				if(!strcmp(pszLabel, "NCBITAXID"))
					if(sscanf(pszValue, "%d", &m_iTaxId) != 1)
						ThrowError("ParserPhenyx::Read", "NCBITAXID is not an integer");
				break;

			case 'M':
				if(!strcmp(pszLabel, "MODRES"))
					ReadPtm(pszValue);
				break;

			case 'V':
				//if(!strcmp(pszLabel, "VARIANT"))	
				break;

			case 'L':
				if(!strcmp(pszLabel, "LENGTH"))
					if(sscanf(pszValue, "%d", &m_iSQLen) != 1)
						ThrowError("ParserPhenyx::Read", "LENGTH is not an integer");
				break;
		}

	}


	//SQ
	char c;
	while(fread(&c, 1, 1, m_fastaFile) ==1){

		if(c == '>')
			break;
		
		if(strchr(" \t\r\n", c))
			continue;

		m_aSQ.Add(c);
	}
	
	m_aSQ.Add('\0');

	return true;
}




/***********************************************************

   Write

***********************************************************/
void ParserPhenyx::Write(void)
{
	//No keywords
	DynamicArray<unsigned short> aKW;
	
	DBFileWriter::Write(m_pszAC, m_pszID, m_pszDE, m_aSQ, m_iTaxId, 0, 0, 0, m_aSQ.GetNbElt() -1, aKW, m_aPtm);
}


/***********************************************************

   RemoveEndWhiteCharacters

 ***********************************************************/
char *ParserPhenyx::RemoveEndWhiteCharacters(char *psz)
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
void ParserPhenyx::Reset(void)
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
void ParserPhenyx::ReadPtm(char *psz)
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
			ThrowError("ParserPhenyx::ReadPtm", "Ptm are bad formated");


		p = strtok(NULL, "|");

		if(!p)
			ThrowError("ParserPhenyx::ReadPtm", "Ptm are bad formated");


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
void ParserPhenyx::AddUnknownModRes(const char *psz)
{

	for(int i=0; i<m_aUnknownModRes.GetNbElt(); i++){	
		if(!strcmp(m_aUnknownModRes[i], psz))
			return;
	}

	m_aUnknownModRes.Add( Strdup(psz) );
}
