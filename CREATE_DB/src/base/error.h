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
	Created on         : 28.11.2003
    Last modification  : 02.11.2004
	Comments           : 

***********************************************************/

#ifndef __MYERROR_H__
#define __MYERROR_H__

#include "dynamicarray.h"
#include <stdio.h>
#include <string.h>


/***********************************************************

	Fonctions globales

***********************************************************/

void ThrowError(const char *pszFctName, char *pszFormat, ...);
void Warning(const char *pszFctName, char *pszFormat, ...);





typedef struct{
	char *pszFct;
	char *psz;
}TS_Error;



/***********************************************************

	class Error

***********************************************************/

class Error
{

	friend void ThrowError(const char *pszFctName, char *pszFormat, ...);

private :

	DynamicArray<TS_Error>	m_aError;


public :

	Error(void);
	Error(const char *pszFctName, char *pszFormat, ...);
	~Error(void);

	inline int GetNbStack(void)
	{ return m_aError.GetNbElt(); }

	inline TS_Error *GetError(int iStackIndex)
	{ return &m_aError[iStackIndex]; }

	void Print(FILE *pFile=stdout);

	void Stack(const char *pszFctName, char *pszFormat, ...);

	void Set(char *psz, int iBufferSize);

private :

	void Add(const char *pszFct, const char *psz);
};



#endif
