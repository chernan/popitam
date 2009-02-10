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
	Created on         : 15.05.2006
	Last modification  : 15.05.2006
	Comments           : 

***********************************************************/

#include "actree.h"
#include "error.h"

#include <stdlib.h>


/***********************************************************

	Constructor

***********************************************************/

AcTree::AcTree(void) :
	m_char('\0'),
	m_bEnd(false),
	m_pNext(NULL),
	m_pBrother(NULL)
{
}


/***********************************************************

	Destructor

***********************************************************/

AcTree::~AcTree(void)
{
	if(m_pNext){
		delete m_pNext;
		m_pNext = NULL;
	}

	if(m_pBrother){
		delete m_pBrother;
		m_pBrother = NULL;
	}
}



/***********************************************************

   Free

***********************************************************/
void AcTree::Free(void)
{
	m_char = '\0';

	if(m_pNext){
		delete m_pNext;
		m_pNext = NULL;
	}

	if(m_pBrother){
		delete m_pBrother;
		m_pBrother = NULL;
	}
}


/***********************************************************

   Add

***********************************************************/
void AcTree::Add(const char *psz)
{
	if(!m_char)
		m_char = *psz;

	if(m_char == *psz){

		psz++;
	
		//fin de la recursion
		if(*psz == '\0' ){
			m_bEnd = true;
			return;
		}
		
		if(!m_pNext)
			m_pNext = new AcTree;

		m_pNext->Add(psz);	
		return;
	}

	if(!m_pBrother)
		m_pBrother = new AcTree;
	
	m_pBrother->Add(psz);
}

/***********************************************************

   Get

***********************************************************/
bool AcTree::Get(const char *psz)
{
	if(m_char == *psz){

		psz++;
	
		//fin de la recursion
		if(*psz == '\0' )
			return m_bEnd;
		
		if(!m_pNext)
			return false;

		return m_pNext->Get(psz);
		
	}

	if(!m_pBrother)
		return false;
	
	return m_pBrother->Get(psz);	
}

