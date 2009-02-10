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
#define USE_STRING
#define USE_ERRNO
#define USE_LEXICO
#include "use.h"



/***********************************************************

		Constructor

***********************************************************/

Lexico::Lexico( void (* pfFreeElt)(const char *psz, void *pData) ) :
	m_pRacine(NULL),
	m_pfFreeElt(pfFreeElt)
{
}


/***********************************************************

		Destructor

***********************************************************/

Lexico::~Lexico(void)
{
	Free();
}



/***********************************************************

		Add (public)

***********************************************************/

bool Lexico::Add(const char *psz, void *pvData)
{
	// appel la fonction Add recursive
	TS_BrowseInTree stBrowse = {	&m_pRacine, NULL, NULL, psz, 
									0, pvData, NULL, 0 };

	return Add(&stBrowse);
}


/***********************************************************

		Get (public)

***********************************************************/

bool Lexico::Get(const char *psz, void **ppvData)
{
	TS_BrowseInTree stBrowse = { NULL, m_pRacine, NULL, psz, 0, NULL, ppvData, 0};

	return Get( &stBrowse);
}


/***********************************************************

		Free (public)

***********************************************************/

void Lexico::Free(void)
{ 
	Free(m_pRacine); 
	m_pRacine = NULL;
}

/***********************************************************

		Add (recursive)

***********************************************************/

bool Lexico::Add( TS_BrowseInTree *pBrowse)
{
	/* test validite des pointeurs */
	if( !pBrowse->ppElt || !pBrowse->psz) {
		errno	= EINVAL;
		return false;
	}

	/* Nouvel elt */
	if( !(*pBrowse->ppElt) ){

		/* allocation de l'elt */
		*pBrowse->ppElt = (TS_Lexico *)calloc( 1, sizeof(TS_Lexico));
		if( !(*pBrowse->ppElt)) {
			errno = ENOMEM;
			return false;
		}

		/* rempli les donnees de l'elt */
		(*pBrowse->ppElt)->psz		= pBrowse->psz;
		(*pBrowse->ppElt)->pvData	= pBrowse->pvData;
		return true;
	}

	/* compare la string Þ stoker avec celle de l'elt courant */
	pBrowse->iFrom = strcmp(pBrowse->psz, (*pBrowse->ppElt)->psz);

	/* la string existe dïjÞ */
	if( !pBrowse->iFrom) {
		errno = EEXIST;
		return false;
	}

	/* la string est lexicographicalement > Þ celle de l'elt courant */
	if( pBrowse->iFrom>0){
		pBrowse->ppElt = &(*pBrowse->ppElt)->psRight;
		return Add(pBrowse);
	}
	
	/* la string est lexicographicalement < Þ celle de l'elt courant */
	pBrowse->ppElt = &(*pBrowse->ppElt)->psLeft;
	return Add(pBrowse);
}


/***********************************************************

		Get (recursive)

***********************************************************/

bool Lexico::Get(TS_BrowseInTree *pBrowse)
{
	/* test validite des pointeurs */
	if( !pBrowse->psz || !pBrowse->ppvData) {
		errno	= EINVAL;
		return false;
	}

	/* l'elt n'existe pas */
	if( !pBrowse->pElt){
		*pBrowse->ppvData = NULL;
		errno = ENOENT;
		return false;
	}

	/* compare la string Þ trouver avec celle de l'elt courant */
	pBrowse->iStrcmpReturn = strcmp(pBrowse->psz, pBrowse->pElt->psz);

	/* la string est trouve */
	if( !pBrowse->iStrcmpReturn) {
		*pBrowse->ppvData	= pBrowse->pElt->pvData;
		return true;
	}

	/* la string est lexicographicalement > Þ celle de l'elt courant */
	if( pBrowse->iStrcmpReturn > 0){
		pBrowse->pElt = pBrowse->pElt->psRight;
		return Get(pBrowse);
	}
	
	/* la string est lexicographicalement < Þ celle de l'elt courant */
	pBrowse->pElt = pBrowse->pElt->psLeft;
	return Get(pBrowse);
	
}



/***********************************************************

		Supp (recursive)

***********************************************************/

bool Lexico::Supp( TS_Lexico **ppstLexico, const char *psz)
{
	TS_Lexico	*pEltToSuppParent;
	TS_Lexico	*pEltToSupp;
	TS_Lexico	*pEltToSuppRight;
	TS_Lexico	*pEltLastLeft;
	TS_Lexico	*pEltLastLeftParent;
	int			ifrom;

	TS_BrowseInTree stBrowse = { NULL, *ppstLexico, NULL, psz, ORIGIN, NULL, NULL, 0};

	/* trouve l'element Þ supprimer et son parent */
	if( !GetParent(&stBrowse) )
		return false;

	pEltToSupp			= stBrowse.pElt;
	pEltToSuppParent	= stBrowse.pParent;
	ifrom				= stBrowse.iFrom;

	/* concerve le pointeur droit de l'elt Þ supp */
	pEltToSuppRight		= stBrowse.pElt->psRight;

	/* trouve l'elt le plus Þ gauche par rapport Þ l'elt Þ supp et son parent */
	if(pEltToSupp->psRight){

		stBrowse.pElt		= pEltToSupp->psRight;
		stBrowse.pParent	= pEltToSupp;
		if(!GetLastLeft(&stBrowse))
			return false;

		pEltLastLeft		= stBrowse.pElt;
		pEltLastLeftParent	= stBrowse.pParent;

		/* accroche arborescence Þ gauche se l'elt Þ supp Þ la gauche de l'elt
		le plus Þ gauche par rapport Þ l'elt Þ supp */
		pEltLastLeft->psLeft = pEltToSupp->psLeft;

		/* accroche Þ la place de l'elt Þ supp l'elt le plus Þ gauche... */
		if( ifrom == LEFT )
			pEltToSuppParent->psLeft = pEltLastLeft;
		else if( ifrom == RIGHT )
			pEltToSuppParent->psRight = pEltLastLeft;
		else if( ifrom == ORIGIN )
			*ppstLexico = pEltLastLeft;

		/* accroche arborescence Þ doite de l'elt le plus Þ gauche Þ la gauche
		du parent de l'elt le plus Þ gauche si ce parent existe*/
		if( pEltLastLeftParent != pEltToSupp){
			pEltLastLeftParent->psLeft = pEltLastLeft->psRight;

			/* accroche arborescence Þ doite de l'elt Þ supp Þ la droite
			de l'elt le plus Þ gauche */
			pEltLastLeft->psRight = pEltToSuppRight;
		}

		/* detruit l'elt */
		m_pfFreeElt(pEltToSupp->psz, pEltToSupp->pvData);
		return true;
	}
	
	/* l'elt Þ supp n'a pas d'arborescence Þ droite elimine juste le maillon */
	if( ifrom == LEFT )
		pEltToSuppParent->psLeft = pEltToSupp->psLeft;
	else if( ifrom == RIGHT )
		pEltToSuppParent->psRight = pEltToSupp->psLeft;
	else if( ifrom == ORIGIN )
		*ppstLexico = pEltToSupp->psLeft;
	
	/* detruit l'elt */
	m_pfFreeElt(pEltToSupp->psz, pEltToSupp->pvData);

	return true;
}





/***********************************************************

		Free  (recursive)

***********************************************************/

void Lexico::Free(TS_Lexico *pstLexico)
{
	/* test validite des pointeurs */
	if( !pstLexico) {
		errno	= EINVAL;
		return;
	}

	/* rappel Þ droite de l'elt courant */
	if( pstLexico->psRight)
		Free( pstLexico->psRight);
	
	/* rappel Þ gauche de l'elt courant */
	if( pstLexico->psLeft)
		Free( pstLexico->psLeft);

	m_pfFreeElt( pstLexico->psz, pstLexico->pvData);

	free(pstLexico);
}




/***********************************************************

		GetParent

***********************************************************/

bool Lexico::GetParent( TS_BrowseInTree *pBrowse)
{
	/* test validite des pointeurs */
	if( !pBrowse->psz ){
		errno	= EINVAL;
		return false;
	}

	/* l'elt n'existe pas */
	if( !pBrowse->pElt){
		errno = ENOENT;
		return false;
	}

	/* compare la string cherchïe avec celle de l'elt en cours */
	pBrowse->iStrcmpReturn = strcmp(pBrowse->psz, pBrowse->pElt->psz);

	/* elt trouve */
	if(!pBrowse->iStrcmpReturn)
		return true;

	/* la string est lexicographicalement > Þ celle de l'elt courant */
	if( pBrowse->iStrcmpReturn > 0){
		pBrowse->iFrom = RIGHT;
		pBrowse->pParent = pBrowse->pElt;
		pBrowse->pElt = pBrowse->pElt->psRight;
		return GetParent(pBrowse);
	}
	
	/* la string est lexicographicalement < Þ celle de l'elt courant */
	pBrowse->iFrom = LEFT;
	pBrowse->pParent = pBrowse->pElt;
	pBrowse->pElt = pBrowse->pElt->psLeft;
	return GetParent(pBrowse);
}




/***********************************************************

		GetLastLeft

***********************************************************/

bool Lexico::GetLastLeft( TS_BrowseInTree *pBrowse)
{
	/* test validite des pointeurs */
	if( !pBrowse->pElt){
		errno	= EINVAL;
		return false;
	}

	/* dernier elt vers la gauche trouve */
	if(!pBrowse->pElt->psLeft)
		return true;
	

	/* rappel avec l'elt Þ gauche de l'elt courant */
	pBrowse->pParent	= pBrowse->pElt;
	pBrowse->pElt		= pBrowse->pElt->psLeft;
	return GetLastLeft(pBrowse);
}



