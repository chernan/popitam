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
	Author			   : Celine	Hernandez
	Contact			   : Celine.Hernandez@isb-sib.ch
	Created	on		   : 06.04.2005
	Last modification  : 06.04.2005
	Comments		   : 

***********************************************************/

#define	USE_RADIOGROUP
#define	USE_ERROR
#define	USE_ERRNO
#define	USE_STDLIB
#define	USE_STRING
#define	USE_IOTAG
#define	USE_PATH
#define	USE_UTIL
#define	USE_HTML
#include "use.h"




/******************************************************

		Constructor

*******************************************************/
RadioGroup::RadioGroup(const char *pszName)	:
	m_pszName(NULL),
	m_ptItem(NULL),
	m_iNbItem(0),
	m_iCurrent(0)
{
	m_pszName =	Strdup(pszName);
}


/******************************************************

		Destructor

*******************************************************/
RadioGroup::~RadioGroup(void)
{
	if(m_ptItem){
		delete [] m_ptItem;
		m_ptItem = NULL;
	}

	if(m_pszName){
		free(m_pszName);
		m_pszName =	NULL;
	}
}

/***********************************************************

	Add

***********************************************************/

void RadioGroup::Add(const char	*psz)
{
	if(m_iCurrent >= m_iNbItem)
		ThrowError("RadioGroup::Add", "Not enough element in array size=%d to set the %d th", m_iNbItem, m_iCurrent);

	m_ptItem[m_iCurrent].Set(m_iCurrent, psz);
	m_iCurrent++;
}



/***********************************************************

	Load

***********************************************************/

void RadioGroup::Load(int iNbItem, int iMin, int iMax)
{
	m_iNbItem	= iNbItem;
	m_iMin		= iMin;
	m_iMax		= iMax;

	m_iCurrent	= 0;

	if(!( m_ptItem = new Radio[m_iNbItem] ))
		ThrowError("RadioGroup::Load", "%s", strerror(ENOMEM));
}



/***********************************************************

	ReadCgi

<input>	value de la	forme "int|string"
***********************************************************/

bool RadioGroup::ReadCgi(Cgi *pCgi)
{
	int			iId;
	const char	*psz;

	    
	int	iNbItem	= pCgi->GetNbElt(m_pszName);


	if(	(iNbItem < m_iMin) || (iNbItem > m_iMax) )
		return false;


	for(int	i=0; i<iNbItem;	i++){
		psz	= pCgi->GetStringPtr(m_pszName,	i); 

		if(!( (sscanf(psz, "%d", &iId) == 1) &&	(psz = strchr(psz, '|')) ))
			ThrowError("RadioGroup::Read", "Cgi	: <input value=...>	not	formated like :	\"int |	string\"");

		psz++;

		if(!Select(iId,	psz)){
			ThrowError("RadioGroup::Read", "Cgi	: No item id %d	string \"%s\" for object %s	/ %d",
						iId, psz, m_pszName, iNbItem);
		}
	}

	return true;
}


/***********************************************************

	ReadFile

***********************************************************/

void RadioGroup::ReadFile(TagFile *pFile)
{
	Reset();
    
	int		iNbItem, iId;
	char	*pszString;

	    
	pFile->ReadStartObject(m_pszName);


	pFile->ReadMember("iNbItem", &iNbItem);

	for(int	i=0; i<iNbItem;	i++){

		pFile->ReadStartObject("Item");
		pFile->ReadMember("iId",		&iId);
		pFile->ReadMember("sString",	&pszString);
		pFile->ReadEndObject();

		if(	(iId < 0) || (iId >= m_iNbItem)	)
			ThrowError("RadioGroup::ReadFile", "%s", strerror(EINVAL));

		if(strcmp(m_ptItem[iId].GetString(), pszString))
			ThrowError("RadioGroup::ReadFile", "Item id=\"%d\" shoud be	string=\"%s\" and not \"%s\"", 
						iId, m_ptItem[iId].GetString(),	pszString);

		free(pszString);

		m_ptItem[iId].Select(true);
	}

	pFile->ReadEndObject();
}



/***********************************************************

	WriteFile

***********************************************************/

void RadioGroup::WriteFile(TagFile *pFile)
{
	pFile->WriteStartObject(m_pszName);
    
	pFile->WriteMember("iNbItem", GetNbSelectedItem());

	Radio *pItem = GetSelectedItem();
	pFile->WriteStartObject("Item");
	pFile->WriteMember("iId",		pItem->GetId());
	pFile->WriteMember("sString",	pItem->GetString());
	pFile->WriteEndObject();

	pFile->WriteEndObject();
}





/***********************************************************

	Reset

***********************************************************/

void RadioGroup::Reset(void)
{
	for(int	i=0; i<m_iNbItem; i++)
		m_ptItem[i].Select(false);

	m_iCurrent = 0;
}


/***********************************************************

	Hide

***********************************************************/

void RadioGroup::Hide(File &file)
{
	for(int	i=0; i<m_iNbItem; i++)
		m_ptItem[i].Hide(file, m_pszName);
}




/***********************************************************

	WriteLink

N'affiche qu'un	seul lien
Pour afficher les Radio	(acces par GetItem(index))
 utiliser Radio::Write(File	file, char*	groupName)
***********************************************************/

void RadioGroup::WriteLink(File	&file, int iSize, const	char *psz, const char *pszLink)
{
//	  fprintf(file,	"<tr><td nowrap>");
//	  HRef(file, psz, pszLink);
//	  fprintf(file,	"</td></tr>\n");
//	  Write(file, iSize);
//	  fprintf(file,	"\n");

	  fprintf(file,	"<td nowrap>");
	  HRef(file, psz, pszLink);
	  fprintf(file,	"</td>");
}



/***********************************************************

	Write
Ne fait	plus rien :	passer par les Radio en	utilisant GetItem(index)
***********************************************************/

void RadioGroup::Write(File	&file, int iSize)
{
/*	  for(int i=0; i<m_iNbItem;	i++) {
		fprintf(file, "<tr><td nowrap>");//+
		m_ptItem[i].Write(file,	m_pszName);
		fprintf(file, "</td></tr>\n");//+
	}*/
}



/***********************************************************

	Select

***********************************************************/

void RadioGroup::Select(int	iId)
{
	Reset();
	m_ptItem[iId].Select(true);
}

bool RadioGroup::Select(int	iId, const char	*psz)
{
	if(	(iId < 0) || (iId >= m_iNbItem)	)
		return false;

	//test si la string	est	la meme
	if(strcmp(m_ptItem[iId].GetString(), psz) )
		return false;

	m_ptItem[iId].Select(true);
    
	return true;
}

/***********************************************************

	GetNbSelectedItem

***********************************************************/

int	RadioGroup::GetNbSelectedItem(void)
{
	return 1;
}


/***********************************************************

	GetSelectedItem

***********************************************************/

Radio *RadioGroup::GetSelectedItem(void)
{
	m_iCurrent = 0;

	for(int	i=0; i<m_iNbItem; i++){
		if(m_ptItem[i].IsSelected()){
			m_iCurrent = i;
			return &m_ptItem[i];
		}
	}

	return NULL;
}



