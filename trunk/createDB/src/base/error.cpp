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
	Created on         : 17.09.2004
    Last modification  : 13.12.2005
	Comments           : 

***********************************************************/


#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <util.h>


/***********************************************************

	ThrowError

***********************************************************/

void ThrowError(const char *pszFctName, char *pszFormat, ...)
{	
	Error *pError = new Error;

	//le message d'erreur n'est pas sense depasser 1024 caracteres...
	char szBuffer[1024];
	va_list vl;
	va_start( vl, pszFormat );
	vsprintf(szBuffer, pszFormat, vl);
	va_end( vl );

	pError->Add(pszFctName, szBuffer);

	throw pError;
}


/***********************************************************

	Warning

***********************************************************/

void Warning(const char *pszFctName, char *pszFormat, ...)
{	
	/*
	printf("In fct \"%s\" : ", pszFctName);

	//le message d'erreur n'est pas sense depasser 1024 caracteres...
	va_list vl;
	va_start( vl, pszFormat );
	vprintf(pszFormat, vl);
	va_end( vl );
	*/
}


/***********************************************************

	Constructor

***********************************************************/

Error::Error(void)
{
}



/***********************************************************

	Constructor

***********************************************************/

Error::Error(const char *pszFctName, char *pszFormat, ...)
{
	//le message d'erreur n'est pas sense depasser 1024 caracteres...
	char szBuffer[1024];
	va_list vl;
	va_start( vl, pszFormat );
	vsprintf(szBuffer, pszFormat, vl);
	va_end( vl );

	Add(pszFctName, szBuffer);
}


/***********************************************************

	Destructor

***********************************************************/

Error::~Error(void)
{
	for(int i=0; i<m_aError.GetNbElt(); i++){
		if(m_aError[i].pszFct){
			delete [] m_aError[i].pszFct;
			m_aError[i].pszFct = NULL;
		}

		if(m_aError[i].psz){
			delete [] m_aError[i].psz;
			m_aError[i].psz = NULL;
		}
	}
}

/***********************************************************

	Stack

***********************************************************/

void Error::Stack(const char *pszFctName, char *pszFormat, ...)
{
	char szBuffer[1024];

	va_list vl;
	va_start( vl, pszFormat );
	vsprintf(szBuffer, pszFormat, vl);
	va_end( vl );

	Add(pszFctName, szBuffer);

	throw this;
}


/***********************************************************

	Add

***********************************************************/

void Error::Add(const char *pszFct, const char *psz)
{
	TS_Error *pError	= m_aError.Add();
	pError->pszFct		= Strdup(pszFct);
	pError->psz				= Strdup(psz);
}


/***********************************************************

	Print

***********************************************************/

void Error::Print(FILE *pFile)
{
	for(int i=0; i<m_aError.GetNbElt(); i++)
		fprintf(pFile, "In fct \"%s\" : %s\n", m_aError[i].pszFct, m_aError[i].psz);
}

/***********************************************************

	Set

***********************************************************/

void Error::Set(char *psz, int iBufferSize)
{
	int iNbByte;
	char szBuffer[1024];

	for(int i=m_aError.GetNbElt()-1; i>=0; i--){

		iNbByte = sprintf(szBuffer, "In fct \"%s\" : %s\n", m_aError[i].pszFct, m_aError[i].psz);

		if(iNbByte < iBufferSize){
			strcpy(psz, szBuffer);	
			psz			+= iNbByte;
			iBufferSize -= iNbByte;
		}
	}
}



