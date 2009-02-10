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

#define	USE_ITEMLIST
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
ItemList::ItemList(const char *pszName)	:
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
ItemList::~ItemList(void)
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

void ItemList::Add(const char *psz)
{
	if(m_iCurrent >= m_iNbItem)
		ThrowError("ItemList::Add",	"Not enough	element	in array size=%d to	set	the	%d th",	m_iNbItem, m_iCurrent);

	m_ptItem[m_iCurrent].Set(m_iCurrent, psz);
	m_iCurrent++;
}

void ItemList::Add(const int iId, const	char *psz)
{
	if(m_iCurrent >= m_iNbItem)
		ThrowError("ItemList::Add",	"Not enough	element	in array size=%d to	set	the	%d th",	m_iNbItem, m_iCurrent);

	m_ptItem[m_iCurrent].Set(iId, psz);
	m_iCurrent++;
}


/***********************************************************

	Load

***********************************************************/

void ItemList::Load(int	iNbItem, int iMin, int iMax)
{
	m_iNbItem	= iNbItem;
	m_iMin		= iMin;
	m_iMax		= iMax;

	m_iCurrent	= 0;

	if(!( m_ptItem = new Item[m_iNbItem] ))
		ThrowError("ItemList::Load", "%s", strerror(ENOMEM));
}



/***********************************************************

	ReadCgi

<input>	value de la	forme "int|string"
***********************************************************/

bool ItemList::ReadCgi(Cgi *pCgi)
{
	int			iId;
	const char	*psz;

	    
	int	iNbItem	= pCgi->GetNbElt(m_pszName);


	if(	(iNbItem < m_iMin) || (iNbItem > m_iMax) )
		return false;


	for(int	i=0; i<iNbItem;	i++){
		psz	= pCgi->GetStringPtr(m_pszName,	i); 


		if(!( (sscanf(psz, "%d", &iId) == 1) &&	(psz = strchr(psz, '|')) ))
			ThrowError("ItemList::Read", "cgi :	<input value=...> not formated like	: \"int	| string\"");

		psz++;

		if(!Select(iId,	psz)){
			ThrowError("ItemList::Read", "Cgi :	No item	id %d string \"%s\"	for	object %s", 
						iId, psz, m_pszName);
		}
	}

	return true;
}


/***********************************************************

	ReadFile

***********************************************************/

void ItemList::ReadFile(TagFile	*pFile)
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

		Select(iId,	pszString);
//		  if( (iId < 0)	|| (iId	>= m_iNbItem) )
//			  ThrowError("ItemList::ReadFile", "%s", strerror(EINVAL));
//
//		  if(strcmp(m_ptItem[iId].GetString(), pszString))
//			  ThrowError("ItemList::ReadFile", "Item id=\"%d\" shoud be	string=\"%s\" and not \"%s\"",
//						  iId, m_ptItem[iId].GetString(), pszString);

		free(pszString);

//		  m_ptItem[iId].Select(true);
	}

	pFile->ReadEndObject();
}



/***********************************************************

	WriteFile

***********************************************************/

void ItemList::WriteFile(TagFile *pFile)
{
	pFile->WriteStartObject(m_pszName);
    
	pFile->WriteMember("iNbItem", GetNbSelectedItem());

	for(Item *pItem	= GetFirstSelectedItem(); pItem; pItem = GetNextSelectedItem()){
		pFile->WriteStartObject("Item");
		pFile->WriteMember("iId",		pItem->GetId());
		pFile->WriteMember("sString",	pItem->GetString());
		pFile->WriteEndObject();
	}

	pFile->WriteEndObject();
}





/***********************************************************

	Reset

***********************************************************/

void ItemList::Reset(void)
{
	for(int	i=0; i<m_iNbItem; i++)
		m_ptItem[i].Select(false);

	m_iCurrent = 0;
}


/***********************************************************

	Hide

***********************************************************/

void ItemList::Hide(File &file)
{
	for(int	i=0; i<m_iNbItem; i++)
		m_ptItem[i].Hide(file, m_pszName);
}




/***********************************************************

	WriteLink

***********************************************************/

void ItemList::WriteLink(File &file, int iSize,	const char *psz, const char	*pszLink)
{
	fprintf(file, "<td nowrap>");
	HRef(file, psz,	pszLink);
	fprintf(file, "</td>\n<td>");
	Write(file,	iSize);
	fprintf(file, "</td>\n");  
}



/***********************************************************

	Write

***********************************************************/

void ItemList::Write(File &file, int iSize)
{
	fprintf(file, "<select name=\"%s\" ", m_pszName);

	if(m_iMax >	1)
		fprintf(file, "MULTIPLE	");

	if(iSize > 1)
		fprintf(file, "size=%d", iSize);

	fprintf(file, ">\n");

	for(int	i=0; i<m_iNbItem; i++)
		m_ptItem[i].Write(file);

	fprintf(file, "</select>\n");   
}



/***********************************************************

	Select

***********************************************************/

bool ItemList::Select(int iId, const char *psz)
{
//	  old version of method	code: iId and index	in tab of Item were	the	same
//	  if( (iId < 0)	|| (iId	>= m_iNbItem) )
//		  return false;

//	  //test si	la string est la meme
//	  if(strcmp(m_ptItem[iId].GetString(), psz)	)
//		  return false;

//	  m_ptItem[iId].Select(true);
    
//	  return true;

	  for(int i=0; i<m_iNbItem;	i++) {
		  if(m_ptItem[i].GetId()==iId && !strcmp(m_ptItem[i].GetString(), psz))	{
			  m_ptItem[i].Select(true);
			  return true;
		  }
	  }
	  return false;
}

/***********************************************************

	GetNbSelectedItem

***********************************************************/

int	ItemList::GetNbSelectedItem(void)
{
	int	iNbItem	= 0;

	for(Item *pItem	= GetFirstSelectedItem(); pItem; pItem = GetNextSelectedItem())
		iNbItem++;

	return iNbItem;
}


/***********************************************************

	GetFirstSelectedItem

***********************************************************/

Item *ItemList::GetFirstSelectedItem(void)
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


/***********************************************************

	GetNextSelectedItem

***********************************************************/

Item *ItemList::GetNextSelectedItem(void)
{
	for(int	i=m_iCurrent+1;	i<m_iNbItem; i++){
		if(m_ptItem[i].IsSelected()){
			m_iCurrent = i;
			return &m_ptItem[i];
		}
	}

	return NULL;

}










