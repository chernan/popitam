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
	Last modification  : 12.05.2004
	Comments           :	
	
Creation d'un tableau d'un type donne.
Alloue au fur et a mesure du remplissage par block successif
GetWithAjust() permet de recuperer un pointeur sur le tableau
ajuste exactement au nombre d'element.
l'idee est de detruire l'objet DynamicArray qui a permit la 
construction du tableau une fois les donnees stockees

***********************************************************/

#ifndef __DYNAMICARRAY_H__
#define __DYNAMICARRAY_H__



/******************************************************

Class DynamicArray :



*******************************************************/

template <class Type> class DynamicArray
{

protected :

	Type	*m_pArray;			//tableau du type defini
	
	int		m_iReallocSize;		//taille d'initialisation et de reallocation
	int		m_iSize;			//nbre de cases du tableau
	int		m_iNbElt;			//nbre d'elt du tableau


public :



/******************************************************

		Constructor

*******************************************************/

DynamicArray(void) :
	m_pArray(NULL),
	m_iReallocSize(0),
	m_iSize(0),
	m_iNbElt(0)
{
}



/******************************************************

		Destructor

*******************************************************/

~DynamicArray(void)
{
	if(m_pArray){
		free(m_pArray);
		m_pArray = NULL;
	}
}


/******************************************************

		Alloc

Alloue le tableau pour commencer a la taille indiquee par iReallocSize
*******************************************************/

void Alloc(int iReallocSize)
{
	m_iReallocSize = iReallocSize;

	//alloue un tableau du type defini
	if(!(m_pArray = (Type *)malloc(m_iReallocSize * sizeof(Type)) ))
		ThrowError("DynamicArray::Alloc", "%s", strerror(errno));
	
	m_iSize		= m_iReallocSize;
	m_iNbElt	= 0;
}


/******************************************************

		Add

Ajoute un elt au tableau et l'agrandi si necessaire.
*******************************************************/

void Add( Type elt)
{
	//le tableau n'est pas plein
	if(m_iNbElt < m_iSize){
		m_pArray[m_iNbElt++] = elt;
		return;
	}
	
	//le tableau est plein
	Type *pNewArray;

	//agrandi le tableau et verifie que le tableau a ete alloue.
	if( !(m_iSize += m_iReallocSize) )
		ThrowError("DynamicArray::Add", "%s", strerror(EINVAL));
	
	if(!(pNewArray = (Type *)malloc(m_iSize * sizeof(Type)) ))
		ThrowError("DynamicArray::Add", "%s", strerror(errno));

	memcpy(pNewArray, m_pArray, (m_iSize - m_iReallocSize) * sizeof(Type) );
	free(m_pArray);
	m_pArray = pNewArray;

	//ajoute l'elt.
	m_pArray[m_iNbElt++] = elt;
}


/******************************************************

		Add

Ajoute n elts au tableau et l'agrandi si necessaire.
*******************************************************/

void Add(Type *pElt, int iNbElt)
{
	//le tableau n'est pas assez gd
	if(m_iNbElt + iNbElt > m_iSize){
		
		Type *pNewArray;

		//agrandi le tableau et verifie que le tableau a ete alloue.
		if( !(m_iSize += m_iReallocSize) )
			ThrowError("DynamicArray::Add", "%s", strerror(EINVAL));
		
		if(!(pNewArray = (Type *)malloc(m_iSize * sizeof(Type)) ))
			ThrowError("DynamicArray::Add", "%s", strerror(errno));

		memcpy(pNewArray, m_pArray, m_iNbElt * sizeof(Type) );
		free(m_pArray);
		m_pArray = pNewArray;

		//rappelle la fct Add recursivement
		Add(pElt, iNbElt);
		return;
	}


	//le tableau est assez gd
	memcpy(m_pArray + m_iNbElt, pElt, iNbElt * sizeof(Type) );
	m_iNbElt += iNbElt;
}


/******************************************************

		Add

Cette fct est concue pour mettre des structures
directement dans le tableau et non des pointeurs.
Il faut etre conscient que lorsque le tableau
est agrandi, ce sont les structures dans leurs intergralite
qui sont recopiees.
*******************************************************/

Type *Add(void)
{
	//le tableau n'est pas plein
	if(m_iNbElt < m_iSize)
		return &(m_pArray[m_iNbElt++]);

	//le tableau est plein
	Type *pNewArray;

	//agrandi le tableau et verifie que le tableau a ete alloue.
	if( !(m_iSize += m_iReallocSize) )
		ThrowError("DynamicArray::Add", "%s", strerror(EINVAL));

	if(!(pNewArray = (Type *)malloc(m_iSize * sizeof(Type)) ))
		ThrowError("DynamicArray::Alloc", "%s", strerror(errno));

	memcpy(pNewArray, m_pArray, (m_iSize - m_iReallocSize) * sizeof(Type) );
	free(m_pArray);
	m_pArray = pNewArray;

	//renvoi l'adresse de l'elt pour pouvoir le remplir.
	return &(m_pArray[m_iNbElt++]);
}


/**********************************************************

		Remove

Elimine un elt et decale tous les suivants
renvoie l'elt pour pouvoir eventuellement le detruire
**********************************************************/

Type Remove(int iIndex)
{
	Type pRemove = m_pArray[iIndex];

	for(int i=iIndex+1; i<m_iNbElt; i++)
		m_pArray[i-1] = m_pArray[i];

	m_iNbElt--;

	return pRemove;
}

/***********************************************************

	Pop

***********************************************************/

Type Pop(void)
{
	if(m_iNbElt)
		return m_pArray[--m_iNbElt];

	ThrowError("DynamicArray::Pop", "no more element to pop");
	return NULL;
}

/**********************************************************

		Get

**********************************************************/

Type Get(int i)
{ return m_pArray[i]; }


/**********************************************************

		GetArray

**********************************************************/

Type * GetArray(void)
{ return m_pArray; }



/**********************************************************

		GetNbElt

**********************************************************/

int GetNbElt(void)
{ return m_iNbElt; }



/**********************************************************

		GetAdress

**********************************************************/

Type *GetAdress(int i)
{ return &m_pArray[i]; }



/**********************************************************

		Reset

**********************************************************/

void Reset(void)
{ m_iNbElt = 0; }



/******************************************************

		GetWithAjust

Renvoi un tableau du type defini de la taille exacte
du nombre d'elt.
le tableau qui a permit la construction est libere
*******************************************************/

Type *GetWithAjust(void)
{
	Type *pNewArray;

	//alloue un tableau de la taille exacte du nombre d'element
	if(!(pNewArray = (Type *)malloc(m_iNbElt * sizeof(Type)) ))
		ThrowError("DynamicArray::GetWithAjust", "%s", strerror(errno));

	//recopie les donnees
	memcpy(pNewArray, m_pArray, m_iNbElt * sizeof(Type) );
	free(m_pArray);
	m_pArray = NULL;

	m_iReallocSize	= 0;		
	m_iSize			= 0;			
	m_iNbElt		= 0;

	return pNewArray;
}


/******************************************************

		GetWithoutAjust

recupere le tableau tel qu'il est (donc trop grand)
inutilisee je crois dans cette appli.
*******************************************************/

Type *GetWithoutAjust(void)
{	
	Type *pArray = m_pArray;
	m_pArray = NULL;

	m_iReallocSize	= 0;		
	m_iSize			= 0;			
	m_iNbElt		= 0;

	return pArray;
}



/******************************************************

		GetWithoutDelete

Renvoi un tableau du type defini de la taille exacte
du nombre d'elt.
le tableau qui a permit la construction n'est pas libere
*******************************************************/

Type *GetWithoutDelete(void)
{
	Type *pNewArray;

	//alloue un tableau de la taille exacte du nombre d'element
	if(!(pNewArray = (Type *)malloc(m_iNbElt * sizeof(Type)) ))
		ThrowError("DynamicArray::GetWithoutDelete", "%s", strerror(errno));

	//recopie les donnees
	memcpy(pNewArray, m_pArray, m_iNbElt * sizeof(Type) );

	return pNewArray;
}




/**********************************************************

		Sort

tri une partie du tableau
appelle qsort avec la fct de comparaison passee en argument
**********************************************************/

void Sort(int iBegin, int iEnd, 
	int (* pfCmp)(const void *pElt1, const void *pElt2) )
{
	qsort(&m_pArray[iBegin], iEnd - iBegin +1, sizeof(Type), pfCmp);
}


/**********************************************************

		Sort

tri tout le tableau
appelle qsort avec la fct de comparaison passee en argument
**********************************************************/

void Sort( int (* pfCmp)(const void *pElt1, const void *pElt2) )
{
	if(m_iNbElt > 1)
		qsort(m_pArray, m_iNbElt, sizeof(Type), pfCmp);
}



/**********************************************************

		Clear

detruit les objets qui sont dans le tableau
alors que Reset ne les detruit pas.
**********************************************************/

void Clear( void (*pfFree)(Type) )
{
	for(int i=0; i<m_iNbElt; i++)
		pfFree( m_pArray[i] );
	
	m_iNbElt = 0;
}









};

#endif
