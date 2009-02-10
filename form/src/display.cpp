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
	Created	on		   : 11.06.2004
	Last modification  : 14.10.2004
	Comments		   : 

***********************************************************/

#define	USE_DISPLAY
#define	USE_MSG
#define	USE_HTML
#include "use.h"





/******************************************************

		Constructor

*******************************************************/
Display::Display(void) :
	m_mail("mail")
{
}


/******************************************************

		Destructor

*******************************************************/
Display::~Display(void)
{   
}

/***********************************************************

	Load

***********************************************************/

void Display::Load(void)
{
	m_mail.Load(FLOOR|CEILING|EMPTY, 0,	64);    
}

/***********************************************************

	ReadFile

***********************************************************/

void Display::ReadFile(TagFile *pFile)
{
	pFile->ReadStartObject("Display");
	m_mail.ReadFile(pFile);
	pFile->ReadEndObject();
}


/***********************************************************

	WriteFile

***********************************************************/

void Display::WriteFile(TagFile	*pFile)
{
	pFile->WriteStartObject("Display");
	m_mail.WriteFile(pFile);
	pFile->WriteEndObject();
}




/***********************************************************

	ReadCgi

***********************************************************/

void Display::ReadCgi(Cgi *pCgi)
{
	m_mail.ReadCgi(pCgi);
}

/***********************************************************

	CheckCgi

***********************************************************/

void Display::CheckCgi(void)
{
	if(!IsMailUsed())
		return;

	const char *pszMail	= GetMail();
	const char *psz;
	const char *pszDomain;

	//cherche la premiere position de @, qu'il soit	unique et pas en premier ou	derniere position
	if(!( (psz = strchr(pszMail, '@')) && !strchr(psz+1, '@') && (psz != pszMail) && *(psz+1) ))
		goto LABEL_ERROR;

	pszDomain =	psz+1;

	//il doit y	avoir au moins un '.' et le	dernier	caractere n'est	pas	un '.'
	if(!( (psz = strchr(pszDomain, '.')) &&	(pszDomain[strlen(pszDomain)-1]	!= '.')	))
		goto LABEL_ERROR;

	return;

	LABEL_ERROR:
	ThrowMsg("Your email is	not	a valid	adress");
}




/***********************************************************

	Write

***********************************************************/

void Display::WriteForm(File &file)
{   
	m_mail.WriteLink(file, 50, "Your E-mail	address", "help.pl?help.html#mail");
}

void Display::WriteLink(File &file,	const char*	link)
{   
	m_mail.WriteLink(file, 50, "Your E-mail	address", link);
}

/***********************************************************

	Hide

***********************************************************/

void Display::Hide(File	&file)
{
	m_mail.Hide(file);
}



/***********************************************************

	WriteCheckLimit

***********************************************************/

void Display::WriteCheckLimit(File &file)
{
	m_mail.WriteCheckLimit(file, "mail");
}




/***********************************************************

   WriteOutput

***********************************************************/
void Display::WriteOutput(File &file)
{
	fprintf(file, "<td>");
	HRefU(file,	"mail",	"help.pl?help.html#mail");
	fprintf(file, "</td><td>%s</td>",  m_mail.GetString());

}
