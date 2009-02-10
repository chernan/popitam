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

	Company			   : Swiss Intitute	of Bioinformatics
	Author			   : Marc Tuloup
	Contact			   : Marc.Tuloup@isb-sib.ch
	Created	on		   : 28.11.2003
	Last modification  : 15.09.2004
	Comments		   : 

***********************************************************/

#define	USE_TAXON
#define	USE_STDLIB
#define	USE_ERROR
#define	USE_ERRNO
#define	USE_STRING
#define	USE_TAGFILE
#include "use.h"



int	Taxon::iDeepInTree = 0;

/******************************************************

		Constructor

*******************************************************/
Taxon::Taxon(void) :
	m_iNbSPEntries(0),
	m_iNbTREntries(0),
	m_iNbChild(0),
	m_ptpTaxon(NULL)
{
}


/******************************************************

		Destructor

*******************************************************/
Taxon::~Taxon(void)
{
	if(m_ptpTaxon){

		for(int	i=0; i<m_iNbChild; i++)
			delete m_ptpTaxon[i];

		free(m_ptpTaxon);
		m_ptpTaxon = NULL;
	}
}



/**********************************************************

		Read

fct	recursive.  
**********************************************************/

void Taxon::Read(TagFile *pFile)
{
	pFile->ReadStartObject("taxon");

	pFile->ReadMember("id",		   &m_iId);
	pFile->ReadMember("string",	   &m_psz);
	pFile->ReadMember("nbSPEntries",   &m_iNbSPEntries);
	pFile->ReadMember("nbTREntries",   &m_iNbTREntries);
    
	//recupere le nombre de	sous taxons
	pFile->ReadMember("nbSubTaxon",	  &m_iNbChild);

	//fin de la	recursion
	if(!m_iNbChild){
		pFile->ReadEndObject();
		return;
	}
    
	//lit tous les sous	taxons  
	if(!(m_ptpTaxon	= (Taxon **)malloc(m_iNbChild *	sizeof(Taxon *)) ))
		ThrowError("Taxon::Read", "%s",	strerror(errno));
    
	for(int	i=0; i<m_iNbChild; i++){
		m_ptpTaxon[i] =	new	Taxon;
		m_ptpTaxon[i]->Read(pFile);
	}

	pFile->ReadEndObject();
}



/***********************************************************

	Find

***********************************************************/

Taxon *Taxon::Find(int iId)
{
	if(m_iId ==	iId)
		return this;

	Taxon *pTaxon;

	for(int	i=0; i<m_iNbChild; i++){
		if((pTaxon = m_ptpTaxon[i]->Find(iId)))
			return pTaxon;
	}

	return NULL;
}

/***********************************************************

	Find

***********************************************************/

Taxon *Taxon::Find(const char *psz)
{
	if(!strcmp(psz,	m_psz))
		return this;

	Taxon *pTaxon;

	for(int	i=0; i<m_iNbChild; i++){
		if((pTaxon = m_ptpTaxon[i]->Find(psz)))
			return pTaxon;
	}

	return NULL;
}


/***********************************************************

	Reset

***********************************************************/

void Taxon::Reset(void)
{
	m_bSelected	= false;

	for(int	i=0; i<m_iNbChild; i++)
		m_ptpTaxon[i]->Reset();
}


/***********************************************************

	GetNbSelectedTaxon

***********************************************************/

int	Taxon::GetNbSelectedTaxon(void)
{
	int	iNbSelected	= 0;

	for(int	i=0; i<m_iNbChild; i++)
		iNbSelected	+= m_ptpTaxon[i]->GetNbSelectedTaxon();

	if(m_bSelected)
		iNbSelected++;

	return iNbSelected;
}

/***********************************************************

	GetNbSPEntries

***********************************************************/

int	Taxon::GetNbSPEntries(void)
{
	int	iNbEntries = 0;

	for(int	i=0; i<m_iNbChild; i++)
		iNbEntries += m_ptpTaxon[i]->GetNbSPEntries();

	iNbEntries+=m_iNbSPEntries;

	return iNbEntries;
}

/***********************************************************

	GetNbTREntries

***********************************************************/

int	Taxon::GetNbTREntries(void)
{
	int	iNbEntries = 0;

	for(int	i=0; i<m_iNbChild; i++)
		iNbEntries += m_ptpTaxon[i]->GetNbTREntries();

	iNbEntries+=m_iNbTREntries;

	return iNbEntries;
}

/***********************************************************

	GetFirstSelectedItem

***********************************************************/

Taxon* Taxon::GetFirstSelectedItem(void)
{
	Taxon* ptTaxon;
	for(int	i=0; i<m_iNbChild; i++)	{
		if(m_ptpTaxon[i]->IsSelected())
			return m_ptpTaxon[i];
		if((ptTaxon	= m_ptpTaxon[i]->GetFirstSelectedItem())!= NULL)
			return ptTaxon;
	}
	return NULL;
}

/***********************************************************

	WriteFile

***********************************************************/

void Taxon::WriteFile(TagFile *pFile)
{
	if(m_bSelected){
		pFile->WriteStartObject("Item");
		pFile->WriteMember("id",	 m_iId);
		pFile->WriteMember("string", m_psz);
		pFile->WriteMember("only",	 (int)0);
		pFile->WriteEndObject();
	}

	for(int	i=0; i<m_iNbChild; i++)
		m_ptpTaxon[i]->WriteFile(pFile);
}

/***********************************************************

	WriteFileAlone

***********************************************************/

void Taxon::WriteFileAlone(TagFile *pFile)
{
	pFile->WriteStartObject("Item");
	pFile->WriteMember("id",	 m_iId);
	pFile->WriteMember("string", m_psz);
	pFile->WriteMember("only",	 (int)0);
	pFile->WriteEndObject();
}


/***********************************************************

	Write

***********************************************************/

void Taxon::Write(File &file)
{
	int	i;

	if(m_bSelected)
		fprintf(file, "<option selected	value=\"%d|%s\">", m_iId, m_psz);
	else
		fprintf(file, "<option value=\"%d|%s\">", m_iId, m_psz);


	for(i=0; i<iDeepInTree;	i++)
		fprintf(file, "..");

	fprintf(file, "%s (SP:%d;TR:%d)\n",	m_psz, GetNbSPEntries(), GetNbTREntries());

	iDeepInTree++;

	for(i=0; i<m_iNbChild; i++)
		m_ptpTaxon[i]->Write(file);

	iDeepInTree--;
}


/***********************************************************

	WriteChildren

***********************************************************/

void Taxon::WriteChildren(File &file) {
	for(int i=0; i<m_iNbChild; i++)
		m_ptpTaxon[i]->Write(file);
}


/**********************************************************
	Hide

***********************************************************/

void Taxon::Hide(File &file, const char	*pszName)
{
	Item::Hide(file, pszName);

	for(int	i=0; i<m_iNbChild; i++)
		m_ptpTaxon[i]->Hide(file, pszName);
}


/***********************************************************

	WriteHtmlInputSummary

***********************************************************/

void Taxon::WriteHtmlInputSummary(File &file)
{
	if(m_bSelected)
		fprintf(file, "%s ", m_psz);

	for(int	i=0; i<m_iNbChild; i++)
		m_ptpTaxon[i]->WriteHtmlInputSummary(file);
}



