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
	Created on         : 02.04.2004
	Last modification  : 07.07.2004
	Comments           : 

***********************************************************/

#ifndef __DIGEST_H__
#define __DIGEST_H__



#include "ptmmanager.h"
#include "modifmanager.h"
#include "ioparam.h"
#include "peptidedigest.h"
#include "mcmanager.h"



class Compare;



/******************************************************

Class Digest :

*******************************************************/

class Digest
{
private :

	Compare						*m_pCompare;					
	PtmManager				m_ptmManager;
	ModifManager			m_modifManager;
	MCManager					m_MCManager;

	double						m_dAddCTerm;
	double						m_dAddNTerm;
	double						m_dHPlus;

	int								m_iMCLimit;

	PeptideDigest			*m_pCurrent;				//peptide en cours de lecture
	
	
	int								m_iNbAA;
	const char				*m_pszSequence;

	double						m_dMassMin;						//mojorant apres modificationd + ptm
	double						m_dMassMax;						//minorant apres modificationd + ptm

	int								m_iNbPeptideGenerated;


	//somme d'un ou plusieur TmpPeptide si missed cleavage
	PeptideDigest			m_peptide;


	//pointeur sur fct 
	void (Digest::*m_pfctEnzyme)(void);				//type d'enzyme


	double						m_tdAAMass[26];			//masse de chaque acide amine.



public:

	Digest(void);
	~Digest(void);


	inline void SetCompare(Compare *pCompare)
	{ m_pCompare = pCompare; }


	void Load(IOParam *pParam);
	void Limit(double dMassMin, double dMassMax);

	void Run(const char *pszSequence, DynamicArray<TS_Ptm> &aPtm);
	
	inline int GetNbPeptideGenerated(void)
	{ return m_iNbPeptideGenerated; }

	inline void InitPosCombination(void)
	{ m_modifManager.InitPosCombination(m_pszSequence, &m_peptide); }

	inline bool NextPosCombination(void)
	{ return m_modifManager.NextPosCombination(&m_peptide); }

	void GetMassPosArray(DynamicArray<double> &aMass);

	void WritePeptide(void);

	void ReloadPeptide(const char *pszSQ, DynamicArray<TS_Ptm> &aPtm, PeptideBase *pPeptide);

	//void StoreIOPeptide(IOPeptide *pIOPeptide);

private :

	void FindPeptide(char cCtermAA, int iEnd);
	void Modification(void);


	void Trypsin(void);
	void TrypsinLowSpecificity(void);
	void TrypsinHighSpecificity(void);	
	void LysC(void);
	void LysN(void);
	void Cnbr(void);
	void ArgC(void);
	void AspN(void);
	void AspN_GluN(void);
	void AspN_LysC(void);
	void AspN_GluN_LysC(void);
	void GluCBicarbo(void);
	void GluCPhos(void);
	void Chymotrypsin1(void);
	void Chymotrypsin2(void);
	void Tryp_Chymo(void);
	void Pepsin1(void);
	void Pepsin2(void);
	void ProteinaseK(void);
	void TrypsinGluC(void);

};



#endif
