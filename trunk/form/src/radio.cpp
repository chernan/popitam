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

#define	USE_RADIO
#define	USE_STDLIB
#define	USE_UTIL
#include "use.h"




/******************************************************

		Constructor

*******************************************************/
Radio::Radio(void) :
	m_iId(0),
	m_psz(NULL),
	m_bSelected(false)
{
}


/******************************************************

		Destructor

*******************************************************/
Radio::~Radio(void)
{
	if(m_psz){
		free(m_psz);
		m_psz =	NULL;
	}
}


/***********************************************************

	Set

***********************************************************/

void Radio::Set(int	iId, const char	*psz)
{
	m_iId		= iId;
	m_psz		= Strdup(psz);
	m_bSelected	= false;
}


/***********************************************************

	Hide

***********************************************************/

void Radio::Hide(File &file, const char	*pszName)
{
	if(!m_bSelected)
		return;

//	  fprintf(file,	"<input	type=hidden	name=\"%s\"	value=\"%d|%s\">\n", pszName, m_iId, m_psz);
    
	fprintf(file, "<input type=\"hidden\" name=\"%s\" value=\"%d|%s\">\n", pszName,	m_iId, m_psz);
}


/***********************************************************

	Write
    
Attention pszName est le nom du	groupe auquel se rattache
 le	radiobouton
 
***********************************************************/

void Radio::Write(File &file, const	char *pszName)
{
	if(m_bSelected)	{
		//fprintf(file,	"<option selected value=\"%d|%s\">%s\n", m_iId,	m_psz, m_psz);
		fprintf(file, "<input type=\"radio\" name=\"%s\" value=\"%d|%s\" checked>%s", pszName, m_iId, m_psz, m_psz);
	}
	else {
		//fprintf(file,	"<option value=\"%d|%s\">%s\n",	m_iId, m_psz, m_psz);
		fprintf(file, "<input type=\"radio\" name=\"%s\" value=\"%d|%s\">%s", pszName, m_iId, m_psz, m_psz);
	}
}















