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
	Last modification  : 27.11.2003
	Comments           : 

***********************************************************/

#define USE_PSZLIST
#define USE_FILE
#define USE_STDLIB
#define USE_STRING
#define USE_UTIL
#define USE_ERROR
#include "use.h"




/***********************************************************

	Load

***********************************************************/

void PszList::Load(const char *pszFileName, char **ppsz, int iNbPsz)
{
	File file;
	file.Open(pszFileName, "r");

	char szBuffer[512];

	for(int i=0; i<iNbPsz; i++, ppsz++){

		if(!fgets(szBuffer, 512, file))
			ThrowError("PszList::Load", "%s not enough data", file.GetFileName());

		char *psz = szBuffer + strlen(szBuffer) -1;
		while(strchr("\n\r\t ", *psz))
			*psz-- = '\0';
		
		*ppsz = Strdup(szBuffer);
	}

	//test qu'il n'y a pas d'autres infos
	if(fscanf(file, "%s", szBuffer) != EOF)
		ThrowError("PszList::Load", "%s File contains to many data", file.GetFileName());

	file.Close();
}


/***********************************************************

	Free

***********************************************************/

void PszList::Free(char **ppsz, int iNbPsz)
{
	for(int i=0; i<iNbPsz; i++, ppsz++)
		free(*ppsz);
}


