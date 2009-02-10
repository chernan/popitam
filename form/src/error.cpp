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
    Last modification  : 04.11.2004
	Comments           : 

***********************************************************/


#define USE_ERROR
#define USE_STDIO
#define USE_STDLIB
#define USE_STRING
#define USE_STDARG
#include "use.h"


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

	Constructor

***********************************************************/

Error::Error(void) :
	m_iNbStack(0)
{
	memset(&m_tstError, 0, sizeof(TS_Error) * ERROR_STACK_MAX);
}



/***********************************************************

	Destructor

***********************************************************/

Error::~Error(void)
{
	for(int i=0; i<m_iNbStack; i++){
		if(m_tstError[i].pszFct){
			free(m_tstError[i].pszFct);
			m_tstError[i].pszFct = NULL;
		}

		if(m_tstError[i].psz){
			free(m_tstError[i].psz);
			m_tstError[i].psz = NULL;
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
	if(m_iNbStack >= ERROR_STACK_MAX)
		return;

	m_tstError[m_iNbStack].pszFct	= strdup(pszFct);
	m_tstError[m_iNbStack].psz		= strdup(psz);
	m_iNbStack++;
}


/***********************************************************

	Print

***********************************************************/

void Error::Print(void)
{
	for(int i=0; i<m_iNbStack; i++)
		printf("In fct \"%s\" : %s\n", m_tstError[i].pszFct, m_tstError[i].psz);
}

/***********************************************************

	Set

***********************************************************/

void Error::Set(char *psz, int iBufferSize)
{
	int iNbByte;
	char szBuffer[1024];

	for(int i=m_iNbStack-1; i>=0; i--){

		iNbByte = sprintf(szBuffer, "In fct \"%s\" : %s\n", m_tstError[i].pszFct, m_tstError[i].psz);

		if(iNbByte < iBufferSize){
			strcpy(psz, szBuffer);	
			psz			+= iNbByte;
			iBufferSize -= iNbByte;
		}
	}
}

