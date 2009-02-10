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
	Created on         : 01.04.2004
	Last modification  : 26.05.2004
	Comments           : 

***********************************************************/


#ifndef __MODIFMANAGER_H__
#define __MODIFMANAGER_H__

#include "ioparam.h"
#include "atomicmass.h"
#include "dynamicarray.h"
#include "peptidedigest.h"
//#include "ioresult.h"

#include <math.h>


/******************************************************

Class Modif


*******************************************************/

class Modif
{
	friend class ModifManager;

private :

	char								*m_pszLabel;				//label de la modif
	double							m_dMass;						//masse a rajoute a chaque locus
	double							m_dScoreCoeff;			//score associe a cette modif
	bool								m_bNTerm;
	bool								m_bCTerm;
	int									m_iTolerance;				//tolerance du nbre de modif non attendu sur le peptide
	int									m_iNbPeptideLocus;	//nombre de locus pour cette modif sur le peptide en cours
	int									m_iLimit;						//limit de la combinatoire en cours
	int									m_iNbLocus;					//nbre de locus "modifies" en cours : FIXE=non-modifies VARIABLE=modifies
	DynamicArray<int>		m_aLocus;						//index de chaque acide amine : ex 'R'-'A' 
	bool								m_bMode;						//mode variable ou fixed

	DynamicArray<int>		m_aPeptideLocusPos;
	DynamicArray<int>		m_aPosComb;
	DynamicArray<int>		m_aPosEnd;
	int									m_iEndPosComb;

public :

	Modif(void);
	~Modif(void);

	void Set(IOParamModif *pIOParamModif, AtomicMass &atomicMass);

	int SetNbPeptideLocus(int *m_piAACmp);

	inline double GetMass(void)
	{ return GetNbLocusModified() * m_dMass; }

	inline double GetScore(void) 
	{ return pow(m_dScoreCoeff, m_iNbLocus); }

	bool InitPosCombination(const char *pszSQ, PeptideDigest *pPeptide);
	void ResetPosCombination(void);
	bool NextPosCombination(void);

	void AddMassToPosArray(DynamicArray<double> &aMass);

	void Write(void);


private :

	inline int GetNbLocusModified(void)
	{ return (m_bMode == MODIF_FIXED)? (m_iNbPeptideLocus - m_iNbLocus) : m_iNbLocus; }

	//void AddIOModif(DynamicArray<IOPeptideModif *> &aIOModif);

};



/******************************************************

Class ModifManager


*******************************************************/

class ModifManager
{

private :

	DynamicArray<Modif *>	m_aModif;					//tableau des modif
	int										m_iNbCombination;

	DynamicArray<Modif *>	m_aModifPos;		

public :

	ModifManager(void);
	~ModifManager(void);


	void Load(IOParam *pParam);

	void InitCombination(PeptideDigest *pPeptide);

	inline void ResetCombination(PeptideDigest *pPeptide)
	{	pPeptide->m_iModifComb = -1; }


	bool Next(PeptideDigest *pPeptide);

	double GetMass(void);
	double GetScore(void);

	void InitPosCombination(const char *pszSQ, PeptideDigest *pPeptide);
	bool NextPosCombination(PeptideDigest *pPeptide);

	void AddMassToPosArray(DynamicArray<double> &aMass);

	void Reload(const char *pszSQ, PeptideDigest *pPeptide);

	void Write(void);

	//void StoreIOPeptide(IOPeptide *pIOPeptide);

private :

	void SetModif(PeptideDigest *pPeptide);
};


#endif
