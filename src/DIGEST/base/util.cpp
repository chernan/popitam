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
	Created on         : 27.11.2003
	Last modification  : 29.09.2004
	Comments           : 

***********************************************************/

#include "util.h"
#include "MATerror.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


/**********************************************************

		Strdup

strdup n'est pas une fct ANSI, je l'ai reecrite :
**********************************************************/

char *Strdup(const char *psz)
{
	char *pszCpy = new char[strlen(psz)+1];

	return strcpy(pszCpy, psz);
}


/***********************************************************

	StrdupFormat

***********************************************************/

char *StrdupFormat(char *pszFormat, ...)
{
	va_list vl;
	char	szBuffer[1024];
	
	va_start( vl, pszFormat );	
	vsprintf(szBuffer, pszFormat, vl);
	va_end( vl );

	return Strdup(szBuffer);
}


/***********************************************************

	SystemCmd

***********************************************************/

int SystemCmd(char *pszFormat, ...)
{
	va_list vl;
	char	szBuffer[1024];
	
	va_start( vl, pszFormat );	
	vsprintf(szBuffer, pszFormat, vl);
	va_end( vl );

	return system(szBuffer);
}


/***********************************************************

	GetEnv

Recupere la variable d'environnement pszEnvVar
***********************************************************/

const char *GetEnv(const char *pszEnvVar)
{
	char *psz;

	if(!(psz = getenv(pszEnvVar)) )
		ThrowError("Cgi::GetEnv", "No environment variable named %s", pszEnvVar);
	
	return psz;
}


/***********************************************************

	StrDouble

transforme un double en string en affichant
ce qui est significatif ex :
10.300000 -> "10.3"
10.000000 -> "10"
***********************************************************/

char *StrDouble(char *pszBuffer, double d)
{
	sprintf(pszBuffer, "%f", d);

	if( !strchr(pszBuffer, '.') )
		return pszBuffer;
	
	//elimine les zeros inutiles
	char *psz = pszBuffer + strlen(pszBuffer) -1;

	while(*psz == '0')
		*psz-- = '\0';

	if(*psz == '.')
		*psz = '\0';

	return pszBuffer;
}


/***********************************************************

	FormatDouble


***********************************************************/

char *FormatDouble(char *pszBuffer, double d)
{
	char szBuffer1[64];
	char szBuffer2[64];

	//choix 1 : affichage classique sans les zeros 0.00021
	StrDouble(szBuffer1, d);

	//choix 2 : affichage exposant : 
	sprintf(szBuffer2, "%.2e", d);
	
	char *psz = strtok(szBuffer2, "e");
	double dValue;
	sscanf(psz, "%lf", &dValue);

	psz = strtok(NULL, "e");
	int iExp;
	sscanf(psz, "%d", &iExp);

	StrDouble(szBuffer2, dValue);

	psz = &szBuffer2[strlen(szBuffer2)];
	*psz++ = 'e';

	sprintf(psz, "%d", iExp);


	if( (d != 0) && !strcmp(szBuffer1, "0") ){
		strcpy(pszBuffer, szBuffer2);
	}else{

		if(strlen(szBuffer1) < strlen(szBuffer2))
			strcpy(pszBuffer, szBuffer1);
		else
			strcpy(pszBuffer, szBuffer2);
	}

	return pszBuffer;
}



/***********************************************************

   FormatDate

***********************************************************/
const char *FormatDate(time_t ltime, char *pszBuffer)
{
   const char *pszMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

   struct tm *stTime    = gmtime(&ltime);
   sprintf(pszBuffer, "%s %02d %d", pszMonths[stTime->tm_mon], stTime->tm_mday, stTime->tm_year +1900);

   return pszBuffer;
}




