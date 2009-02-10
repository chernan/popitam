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
	Created on         : 25.06.2004
	Last modification  : 18.08.2004
	Comments           : 

***********************************************************/

#define USE_HTML
#define USE_UTIL
#define USE_STDARG
#define USE_STDIO
#define USE_STRING
#define USE_PATH
#include "use.h"




/***********************************************************

   WriteHtmlContentType

***********************************************************/
void WriteHtmlContentType(void)
{
	printf("Content-type: text/html\n\n");
}

/***********************************************************

	WriteHtmlHeader

***********************************************************/

void WriteHtmlHeader(File &file, const char *pszTitle, const char *pszHead)
{
	fprintf(file, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
	fprintf(file, "<HTML LANG=\"en-US\">\n<HEAD>\n");
	
	if(pszTitle)
		fprintf(file, "<TITLE>%s</TITLE>\n", pszTitle);
	
	if(pszHead)
		fprintf(file, "%s\n", pszHead);

	fprintf(file, "</HEAD>\n<BODY>\n");
}


/***********************************************************

	WriteHtmlEnd

***********************************************************/

void WriteHtmlEnd(File &file)
{
	fprintf(file, "\n</BODY></HTML>");
}




/***********************************************************

	Cell

***********************************************************/

void Cell(File &file, const char *pszType, int iCellIndex, const char *pszFormat, ...)
{
	va_list vl;
	
	va_start( vl, pszFormat );	
	
	fprintf(file, "<td nowrap");

	//une cellule d'index pair est grisee
	if(!(iCellIndex % 2)){
		fprintf(file, " class=%sG>", pszType);	
	}else{
	
		if(!strcmp(pszType, ""))
			fprintf(file, ">");
		else
			fprintf(file, " class=%s>", pszType);
	}

	vfprintf(file, pszFormat, vl);
	fprintf(file, "</td>");
	va_end( vl );
}



/***********************************************************

	HRef

***********************************************************/

void HRef(File &file, const char *psz, const char *pszLink)
{
	fprintf(file, "<a href=\"%s\" target=\"_blank\">%s</a>", pszLink, psz);
}


/***********************************************************

	HRefU

***********************************************************/

void HRefU(File &file, const char *psz, const char *pszLink)
{
	fprintf(file, "<a class=blacklink target=\"_blank\" href=\"%s\">%s</a>", pszLink, psz);	
}
