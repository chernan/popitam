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
  Last modification  : 28.11.2006
	Comments           :	
	
Creation d'un tableau d'un type donne.
Alloue au fur et a mesure du remplissage par block successif

***********************************************************/

#ifndef __DYNAMICARRAY_H__
#define __DYNAMICARRAY_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/******************************************************

Class DynamicArray :


*******************************************************/

template <class Type> class DynamicArray
{

protected :

	Type	*m_pArray;		//tableau du type defini
	
	int		m_iSize;			//nbre de cases du tableau
	int		m_iNbElt;			//nbre d'elt du tableau


public :


/******************************************************

		Constructor

*******************************************************/
DynamicArray(void) :
	m_pArray(NULL),
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
		delete [] m_pArray;
		m_pArray = NULL;
	}
}

/******************************************************

		operator[]

*******************************************************/
inline operator Type *() 
{ return m_pArray; }

/***********************************************************

	Pop

elimine le dernier element
***********************************************************/
inline Type Pop(void)
{
	assert(m_iNbElt > 0);
	return m_pArray[--m_iNbElt];
}

/***********************************************************

	GetLastElt

retournr le dernier element
***********************************************************/
inline Type GetLastElt(void)
{
	assert(m_iNbElt > 0);
	return m_pArray[m_iNbElt-1];
}

/**********************************************************

		GetNbElt

**********************************************************/
inline int GetNbElt(void)
{ return m_iNbElt; }


/**********************************************************

		SetNbElt

Attention a l'utilisation de cette fct, on peut faire du degat...
permet de deplacer le curseur du nbre d'element valide,
la fct assure que le nbre de cases est allouees, 
mais il n'y a pas d'initialisation des cases.
**********************************************************/
void SetNbElt(int iNbElt)
{ 
	if(iNbElt > m_iSize)
		EnsureSize(iNbElt);

	m_iNbElt = iNbElt; 
}

/**********************************************************

		Reset

**********************************************************/
inline void Reset(void)
{ m_iNbElt = 0; }

/******************************************************

		EnsureSize

Assure que le tableau possede au moins le nbre de cases demandees.
*******************************************************/
void EnsureSize(int iSize)
{
	if(m_iSize < iSize)
		Increase(iSize);
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
	DoubleSize();

	//ajoute l'elt.
	m_pArray[m_iNbElt++] = elt;
}

/******************************************************

		Add

Ajoute n elts au tableau et l'agrandi si necessaire.
*******************************************************/
void Add(Type *pElt, int iNbElt)
{
	assert(iNbElt >= 0);
	assert(iNbElt? pElt!=NULL : true);

	//alloue tant que le tableau n'est pas assez gd
	if(m_iSize == 0){
		EnsureSize(iNbElt);
	}else{
		while(m_iNbElt + iNbElt > m_iSize)
			DoubleSize();
	}

	//le tableau est assez gd
	memcpy(m_pArray + m_iNbElt, pElt, iNbElt * sizeof(Type) );
	m_iNbElt += iNbElt;
}


/******************************************************

		Add

Cette fct est concue pour mettre des structures / objets
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
	DoubleSize();

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
	assert(m_iNbElt > 0);
	assert( (iIndex >=0) && (iIndex < m_iNbElt) );

	Type pRemove = m_pArray[iIndex];

	for(int i=iIndex+1; i<m_iNbElt; i++)
		m_pArray[i-1] = m_pArray[i];

	m_iNbElt--;

	return pRemove;
}

/**********************************************************

		RemoveWithoutDelete

Elimine les elt indexes sans les detruire, garde les autres
**********************************************************/
void RemoveWithoutDelete(DynamicArray<int> &aIndex)
{
	DynamicArray<bool> aRemove;
	aRemove.SetNbElt(m_iNbElt);
	aRemove.ZeroMem();

	for(int i=0; i<aIndex.GetNbElt(); i++){
		assert(aIndex[i] >= 0 && aIndex[i] < m_iNbElt);
		aRemove[aIndex[i]] = true;
	}

	DynamicArray<Type> aNew;
	aNew.EnsureSize(m_iNbElt);

	for(int i=0; i<m_iNbElt; i++){
		if(aRemove[i] == false)
			aNew.Add(m_pArray[i]);
	}

	this->Copy(aNew);
}

/**********************************************************

		RemoveWithDelete

Elimine les elt indexes en les detruisant, garde les autres
**********************************************************/
void RemoveWithDelete(DynamicArray<int> &aIndex)
{
	DynamicArray<bool> aRemove;
	aRemove.SetNbElt(m_iNbElt);
	aRemove.ZeroMem();

	for(int i=0; i<aIndex.GetNbElt(); i++){
		assert(aIndex[i] >= 0 && aIndex[i] < m_iNbElt);
		aRemove[aIndex[i]] = true;
	}

	DynamicArray<Type> aNew;
	aNew.EnsureSize(m_iNbElt);

	for(int i=0; i<m_iNbElt; i++){
		if(aRemove[i] == false)
			aNew.Add(m_pArray[i]);
		else
			delete m_pArray[i];
	}

	this->Copy(aNew);
}


/**********************************************************

		KeepWithoutDelete

Garde les elt indexes , elimine les autres sans les detruire
**********************************************************/
void KeepWithoutDelete(DynamicArray<int> &aIndex)
{
	DynamicArray<bool> aRemove;
	aRemove.SetNbElt(m_iNbElt);
	memset(aRemove, 1, m_iNbElt);

	for(int i=0; i<aIndex.GetNbElt(); i++){
		assert(aIndex[i] >= 0 && aIndex[i] < m_iNbElt);
		aRemove[aIndex[i]] = false;
	}

	DynamicArray<Type> aNew;
	aNew.EnsureSize(m_iNbElt);

	for(int i=0; i<m_iNbElt; i++){
		if(aRemove[i] == false)
			aNew.Add(m_pArray[i]);
	}

	this->Copy(aNew);
}


/**********************************************************

		KeepWithDelete

Garde les elt indexes , elimine les autres en les detruisant
**********************************************************/
void KeepWithDelete(DynamicArray<int> &aIndex)
{
	DynamicArray<bool> aRemove;
	aRemove.SetNbElt(m_iNbElt);
	memset(aRemove, 1, m_iNbElt);

	for(int i=0; i<aIndex.GetNbElt(); i++){
		assert(aIndex[i] >= 0 && aIndex[i] < m_iNbElt);
		aRemove[aIndex[i]] = false;
	}

	DynamicArray<Type> aNew;
	aNew.EnsureSize(m_iNbElt);

	for(int i=0; i<m_iNbElt; i++){
		if(aRemove[i] == false)
			aNew.Add(m_pArray[i]);
		else
			delete m_pArray[i];
	}

	this->Copy(aNew);
}

/******************************************************

		Ajust

Ajuste la taille du tableau au nbre d'element
*******************************************************/
void Ajust(void)
{
	if(m_iSize == m_iNbElt)
		return;

	Type *pNewArray = GetAjustCpy();

	if(m_pArray)
		delete [] m_pArray;
	
	m_iSize		= m_iNbElt;			
	m_pArray	= pNewArray;
}

/******************************************************

		GetAjustCpy

renvoie une copie ajustee au nbre d'element
*******************************************************/
Type *GetAjustCpy(void)
{
	Type *pNewArray = new Type[m_iNbElt];
	memcpy(pNewArray, m_pArray, m_iNbElt * sizeof(Type) );	
	return pNewArray;
}

/**********************************************************

		Sort
 
appelle qsort avec la fct de comparaison passee en argument
**********************************************************/
void Sort(int (* pfCmp)(const void *pElt1, const void *pElt2),  int iStart=0, int iNbElt=-1)
{
	if(iNbElt == -1)
		iNbElt = m_iNbElt;

	qsort(&m_pArray[iStart], iNbElt, sizeof(Type), pfCmp);
}

/**********************************************************

		ZeroMem
 
**********************************************************/
void ZeroMem(void)
{
	memset(m_pArray, 0, m_iNbElt * sizeof(Type));
}

/**********************************************************

		Read
 
**********************************************************/
int Read(FILE *pFile, int iNbElt)
{
	EnsureSize(m_iNbElt + iNbElt);	
	int iCount = (int)fread(&m_pArray[m_iNbElt], sizeof(Type), iNbElt, pFile);
	m_iNbElt += iCount;
	return iCount;
}

/**********************************************************

		Write
 
**********************************************************/
int Write(FILE *pFile, int iStart=0, int iNbElt=-1)
{
	if(iNbElt == -1)
		iNbElt = m_iNbElt;

	return (int)fwrite(&m_pArray[iStart], sizeof(Type), iNbElt, pFile);
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

/**********************************************************

		DeletePtrs

Possibilite de deleter tous les objets si ce sont des pointeurs
**********************************************************/
void DeletePtrs(void)
{
	for(int i=0; i<m_iNbElt; i++)
		delete m_pArray[i];
	
	m_iNbElt = 0;
}

/**********************************************************

		Copy

Copy un tableau, Reset avant la copie
**********************************************************/
void Copy(DynamicArray<Type> &array)
{
	Reset();
	Add(array, array.GetNbElt());
}


protected :


/**********************************************************

		DoubleSize

augmente la taille par deux
**********************************************************/
void DoubleSize(void)
{
	if(m_iSize == 0)
		Increase(1);
	else
		Increase(m_iSize * 2);
}


private :


/**********************************************************

		Increase

**********************************************************/
void Increase(int iSize)
{
	m_iSize	= iSize;
	
	if(m_iNbElt){
		Type *pNewArray = new Type [m_iSize];
		memcpy(pNewArray, m_pArray, m_iNbElt * sizeof(Type) );
		delete [] m_pArray;
		m_pArray = pNewArray;
	}else{

		if(m_pArray)
			delete [] m_pArray;

		m_pArray = new Type [m_iSize]; 
	}
	
}


};







/**********************************************************

		CopyArray

Fct globale.
Recopie un tableau en reallouant et recopiant chaque element
sense etre un obj ptr ayant une fct Copy()

**********************************************************/
template <class T> void CopyArray(DynamicArray<T *> &src, DynamicArray<T *> &dest)
{
	dest.DeletePtrs();
	dest.SetNbElt(src.GetNbElt());

	T *p;
	for(int i=0; i<src.GetNbElt(); i++){
		p = new T;
		p->Copy(src[i]);
		dest[i] = p;
	}
}





#endif
