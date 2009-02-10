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
	Last modification  : 28.11.2003
	Comments           : 

***********************************************************/

#define USE_STDLIB
#define USE_ERRNO
#define USE_MAP
#include "use.h"


/***********************************************************

		Constructor

***********************************************************/

Map::Map(void) :
	m_iNbMapGroup(0),
	m_ptpLexico(NULL)
{
}


/***********************************************************

		Destructor

***********************************************************/

Map::~Map(void)
{
	if(!m_ptpLexico)
		return;

	//efface toutes les donnees
	Free();

	/* libere le tableau des groupe de hach */
	for(int i=0; i<m_iNbMapGroup; i++)
		delete m_ptpLexico[i];

	delete [] m_ptpLexico;
}



/***********************************************************

		Hachage

***********************************************************/

int Map::Hachage(const char *psz)
{
	/* associe une case de m_ptpLexico en fct des lettres du mot a ranger */
	unsigned long ulValHach;

	for( ulValHach=0; *psz; psz++)
		ulValHach = *psz + 31*ulValHach;

	return ulValHach % m_iNbMapGroup;
}

/***********************************************************

		Init

***********************************************************/

bool Map::Init(int iNbMapGroup, void (*pfFreeElt)(const char *psz, void *pvData))
{
	/* test si l'initialisation a deja ete faite */
	if(m_ptpLexico){
		errno = EEXIST;
		return false;
	}
	
	/* test la valeur iNbHachGroup fournit par l'utilisateur */
	if( !(iNbMapGroup > 0) || !pfFreeElt ){
		errno = EINVAL;
		return false;
	}

	m_iNbMapGroup = iNbMapGroup;

	/* allocation dynamite du tableau des groupes de hachage */
	if(!( m_ptpLexico = new Lexico * [m_iNbMapGroup] )){
		errno = ENOMEM;
		return false;
	}

	/* chaque cases de m_ptpLexico pointe sur un objet Lexico */
	for(int i=0; i<m_iNbMapGroup; i++){
		if(!( m_ptpLexico[i] = new Lexico(pfFreeElt) )){
			errno = ENOMEM;
			return false;
		}
	}
	
	return true;
}



/***********************************************************

		Add

***********************************************************/

bool Map::Add(const char *psz, void *pvData)
{
	if(!psz || !pvData){
		errno = EINVAL;
		return false;
	}

	return m_ptpLexico[Hachage(psz)]->Add(psz, pvData); 
}


/***********************************************************

		Supp

***********************************************************/

bool Map::Supp(const char *psz)
{ 
	if(!psz){
		errno = EINVAL;
		return false;
	}

	return m_ptpLexico[Hachage(psz)]->Supp(psz); 
}


/***********************************************************

		Get

***********************************************************/

bool Map::Get(const char *psz, void **ppvData)
{ 
	if(!psz || !ppvData){
		errno = EINVAL;
		return false;
	}

	return m_ptpLexico[Hachage(psz)]->Get(psz, ppvData); 
}


/***********************************************************

		Free

***********************************************************/
void Map::Free(void)
{
	/*	efface toutes les entree de chaque objet Lexico  */
	for(int i=0; i<m_iNbMapGroup; i++)
		if(m_ptpLexico[i])
			m_ptpLexico[i]->Free();
}









