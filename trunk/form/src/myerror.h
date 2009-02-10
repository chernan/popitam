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




void ThrowError(const char *pszFctName, char *pszFormat, ...);


#define ERROR_STACK_MAX	10


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

	int			m_iNbStack;
	TS_Error	m_tstError[ERROR_STACK_MAX];


public :

	Error(void);
	~Error(void);

	inline int GetNbStack(void)
	{ return m_iNbStack; }

	inline TS_Error *GetError(int iStackIndex)
	{ return &m_tstError[iStackIndex]; }

	void Print(void);

	void Stack(const char *pszFctName, char *pszFormat, ...);

	void Set(char *psz, int iBufferSize);

private :

	void Add(const char *pszFct, const char *psz);
};


#endif
