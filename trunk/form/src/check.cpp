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
	Last modification  : 24.02.2005
	Comments		   : 

***********************************************************/

#define	USE_CHECK
#define	USE_IOTAG
#define	USE_UTIL
#define	USE_PATH
#define	USE_HTML
#include "use.h"



/******************************************************

		Constructor

*******************************************************/
Check::Check(const char	*pszName) :
	m_bChecked(false)
{
	m_pszName	= Strdup(pszName);
}


/******************************************************

		Destructor

*******************************************************/
Check::~Check(void)
{
	if(m_pszName){
		free(m_pszName);
		m_pszName =	NULL;
	}
}


/***********************************************************

	Read

***********************************************************/

void Check::ReadCgi(Cgi	*pCgi, int iIndex)
{
	char szBuffer[128];
	sprintf(szBuffer, "%s%d", m_pszName, iIndex);

	m_bChecked = pCgi->GetCheckBox(szBuffer);
}

/***********************************************************

	ReadFile

***********************************************************/

void Check::ReadFile(TagFile *pFile, const char	*pszTag)
{
	pFile->ReadMember(pszTag, &m_bChecked);
}

/***********************************************************

	WriteFile

***********************************************************/

void Check::WriteFile(TagFile *pFile, const	char *pszTag)
{
	pFile->WriteMember(pszTag, m_bChecked);
}


/***********************************************************

	Hide

***********************************************************/

void Check::Hide(File &file, int iIndex)
{
	char szBuffer[128];
	sprintf(szBuffer, "%s%d", m_pszName, iIndex);

	fprintf(file, "<input type=\"hidden\" name=\"%s\" value=%s>\n",	szBuffer, m_bChecked? "1" :	"0");
}


/***********************************************************

	WriteLink

***********************************************************/

void Check::WriteLink(File &file, const	char *psz, const char *pszLink,	int	iIndex)
{
	fprintf(file, "<td>");
	HRef(file, psz,	pszLink);
	fprintf(file, "</td><td>");
	Write(file,	iIndex);
	fprintf(file, "</td>\n");
}

/***********************************************************

	Write

***********************************************************/

void Check::Write(File &file, int iIndex)
{
	char szBuffer[128];
	sprintf(szBuffer, "%s%d", m_pszName, iIndex);

	fprintf(file, "<input type=\"checkbox\"	name=\"%s\"	value=1%s>", szBuffer, m_bChecked? " checked" :	"");
}





