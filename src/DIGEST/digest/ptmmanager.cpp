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
	Created on         : 05.04.2004
	Last modification  : 26.08.2004
	Comments           : 

***********************************************************/


#include "ptmmanager.h"
#include "error.h"
#include "util.h"
#include <assert.h>

/******************************************************

		Constructor

*******************************************************/
PtmManager::PtmManager(void) :
	m_pPosInList(NULL),
	m_pEndList(NULL),
	m_iNbPtmLocus(0),
	m_iNbCombination(0),
	m_ptiLimit(NULL),
	m_ppSameLocus(NULL),
	m_iNbMaxPtmLocus(0),
	m_bUse(false)
{
}

/******************************************************

		Destructor

*******************************************************/
PtmManager::~PtmManager(void)
{
	Free();
}

/***********************************************************

	Free

***********************************************************/
void PtmManager::Free(void)
{	
	if(m_ptiLimit){
		delete [] m_ptiLimit;
		m_ptiLimit = NULL;
	}
	
	if(m_ppSameLocus){
		delete [] m_ppSameLocus;
		m_ppSameLocus = NULL;
	}
}

/***********************************************************

	Load

***********************************************************/
void PtmManager::Load(IOParam *pParam)
{
	Free();

	//utilise ou pas les info contenu dans le fichier DB
	m_bUse = pParam->m_bPTM;

	//nbre max de ptm par peptide au dela duquel on laisse tomber la combinatoire
	m_iNbMaxPtmLocus = 3;

	//alloc
	m_ptiLimit		= new int[m_iNbMaxPtmLocus];
	m_ppSameLocus	= new TS_Ptm *[m_iNbMaxPtmLocus];

	m_pPosInList	= NULL;
	m_pEndList		= NULL;
}

/***********************************************************

   Set

***********************************************************/
void PtmManager::Set(DynamicArray<TS_Ptm> &aPtm)
{
	if(!m_bUse)
		return;

	if(aPtm.GetNbElt()){
		m_pPosInList	= &aPtm[0];
		m_pEndList	= &aPtm[aPtm.GetNbElt()-1];
		m_pEndList++;	//une case apres la fin du tableau
	}else{
		m_pPosInList	= NULL;
		m_pEndList		= NULL;
	}
}

/***********************************************************

	Link

***********************************************************/
void PtmManager::Link(PeptideDigest *pPeptide, int iStartMin)
{
	assert(pPeptide);

	pPeptide->m_iNbPtm	= 0;
	m_iNbPtmLocus				= 0;

	//pas ou plus de ptm
	if(m_pPosInList == m_pEndList )
		return;

	//avance dans la liste des ptm 
	while(m_pPosInList != m_pEndList && m_pPosInList->iPos < iStartMin)
		m_pPosInList++;

	TS_Ptm *p = m_pPosInList;

	//avance jusqu'a etre dans le peptide (utile en cas de reload)
	while(p != m_pEndList && p->iPos < pPeptide->m_iStart)
		p++;

	if(p == m_pEndList || p->iPos > pPeptide->m_iEnd)
		return;

	//note la premiere position 
	pPeptide->m_ptstPtm = p;

	int iPreviousPos = -1;

	pPeptide->m_bMultiPtmPerlocus = false;

	do{
		//on a trouve une ptm
		if(p->iPos == iPreviousPos)
			pPeptide->m_bMultiPtmPerlocus = true;

		iPreviousPos = p->iPos;

		pPeptide->m_iNbPtm++;
		p++;

		if(p == m_pEndList)
			break;
	
	}while( p->iPos <= pPeptide->m_iEnd );
}

/***********************************************************

	InitCombination


***********************************************************/
void PtmManager::InitCombination(PeptideDigest *pPeptide)
{
	assert(pPeptide);

	m_iNbCombination			= 1;
	pPeptide->m_iPtmComb	= -1;

	if(!pPeptide->m_iNbPtm){
		m_iNbPtmLocus = 0;
		return;
	}

	TS_Ptm	*ptstPtm		= pPeptide->m_ptstPtm;
	int			iNbPtm			= pPeptide->m_iNbPtm;

	if(!pPeptide->m_bMultiPtmPerlocus){
		
		//trop de combinatoire laisse tomber
		if(iNbPtm > m_iNbMaxPtmLocus){
			pPeptide->m_iNbPtm	= 0;
			m_iNbPtmLocus				= 0;
			return;
		}

		//save
		m_iNbPtmLocus	= iNbPtm;

		//set limit
		m_iNbCombination = 1 << iNbPtm;

	}else{

		//reset
		memset(m_ptiLimit, 0, sizeof(int) * m_iNbMaxPtmLocus);

		//set limit
		m_iNbPtmLocus			= 0;
		int iPreviousPos	= -1;
		int i;

		int iStop	= m_iNbMaxPtmLocus - 1;	//-1 car m_iNbPtmLocus est incremente apres la boucle for

		for(i=0; i<iNbPtm; i++, ptstPtm++, m_iNbPtmLocus++){

			if(ptstPtm->iPos != iPreviousPos)			
				m_ppSameLocus[m_iNbPtmLocus] = ptstPtm;
			else
				m_iNbPtmLocus--;

			//trop de combinatoire laisse tomber
			if(m_iNbPtmLocus >= iStop){	
				pPeptide->m_iNbPtm	= 0;
				m_iNbPtmLocus				= 0;
				return;
			}

			m_ptiLimit[m_iNbPtmLocus]++;

			iPreviousPos = ptstPtm->iPos;
		}

		for(i=0; i<m_iNbPtmLocus; i++)
			m_iNbCombination *= ++m_ptiLimit[i];	//++ car si une ptm => 2 choix : aucune ou une ptm
	}
}



/***********************************************************

	GetMass

***********************************************************/

double PtmManager::GetMass(PeptideDigest *pPeptide)
{
	double	dMass = 0;

	if(!pPeptide->m_bMultiPtmPerlocus){

		for(int i=0; i<m_iNbPtmLocus; i++){	
			if(pPeptide->m_iPtmComb & (1<<i))
				dMass += pPeptide->GetPtm(i).pPtm->GetMass();
		}

	}else{

		int	iCombIdx = pPeptide->m_iPtmComb;

		for(int i=0; i<m_iNbPtmLocus; i++){
			int iIdx = iCombIdx % m_ptiLimit[i];
			iCombIdx /= m_ptiLimit[i];

			if(iIdx)
				dMass += m_ppSameLocus[i][ iIdx -1 ].pPtm->GetMass();
			//else inutile index == 0 signifie pas de ptm
			// dMass += 0;
		}
	}

	return dMass;
}



/***********************************************************

   AddMassToPosArray

 ***********************************************************/

void PtmManager::AddMassToPosArray(PeptideDigest *pPeptide, DynamicArray<double> &aMass)
{
	if(!pPeptide->m_bMultiPtmPerlocus){

		for(int i=0; i<m_iNbPtmLocus; i++){	
			if(pPeptide->m_iPtmComb & (1<<i)){
				TS_Ptm &stPtm = pPeptide->GetPtm(i);

				int iPos = stPtm.iPos - pPeptide->GetStart() +1; //+1 car NTerm en position 0 dans aMass
				assert(iPos > 0 && iPos < aMass.GetNbElt());

				aMass[iPos] += stPtm.pPtm->GetMass();
			}
		}

	}else{

		int	iCombIdx = pPeptide->m_iPtmComb;

		for(int i=0; i<m_iNbPtmLocus; i++){
			int iIdx = iCombIdx % m_ptiLimit[i];
			iCombIdx /= m_ptiLimit[i];

			if(iIdx){
				TS_Ptm &stPtm = m_ppSameLocus[i][ iIdx -1 ];

				int iPos = stPtm.iPos - pPeptide->GetStart() +1; //+1 car NTerm en position 0 dans aMass
				assert(iPos > 0 && iPos < aMass.GetNbElt());

				aMass[iPos] += stPtm.pPtm->GetMass();
			}
			
		}
	}
}



/***********************************************************

   Reload

 ***********************************************************/
void PtmManager::Reload(DynamicArray<TS_Ptm> &aPtm, PeptideDigest *pPeptide)
{
	Set(aPtm);
	Link(pPeptide, 0);

	int iPtmComb = pPeptide->m_iPtmComb;

	InitCombination(pPeptide);

	
	while(Next(pPeptide)){
		if(pPeptide->m_iPtmComb == iPtmComb)
			break;
	}

	assert(pPeptide->m_iPtmComb == iPtmComb);
}


/***********************************************************

   Write

 ***********************************************************/
void PtmManager::Write(PeptideDigest *pPeptide)
{
	if(!pPeptide->m_bMultiPtmPerlocus){

		for(int i=0; i<m_iNbPtmLocus; i++){	
			if(pPeptide->m_iPtmComb & (1<<i)){
				TS_Ptm &stPtm = pPeptide->GetPtm(i);
				printf("%s(%d) ", stPtm.pPtm->GetLabel(), stPtm.iPos - pPeptide->GetStart());
			}
		}

	}else{

		int	iCombIdx = pPeptide->m_iPtmComb;

		for(int i=0; i<m_iNbPtmLocus; i++){
			int iIdx = iCombIdx % m_ptiLimit[i];
			iCombIdx /= m_ptiLimit[i];

			if(iIdx){
				TS_Ptm &stPtm = m_ppSameLocus[i][ iIdx -1 ];
				printf("%s(%d) ", stPtm.pPtm->GetLabel(), stPtm.iPos - pPeptide->GetStart());
			}
			
		}
	}

}

/***********************************************************

	StoreIOPeptide

***********************************************************/
/*
void PtmManager::StoreIOPeptide(IOPeptide *pIOPeptide, PeptideDigest *pPeptide)
{	
	//recopie les ptms de ce peptide
	DynamicArray<IOPeptidePtm *> &aIOPeptidePtm = pIOPeptide->m_aPtm;

	if(!pPeptide->m_bMultiPtmPerlocus){

		for(int i=0; i<m_iNbPtmLocus; i++){	
			if(pPeptide->m_iPtmComb & (1<<i)){
				TS_Ptm &stPtm = pPeptide->GetPtm(i);
				aIOPeptidePtm.Add(new IOPeptidePtm( stPtm.pPtm->GetLabel(), stPtm.iPos ));
			}
		}

	}else{

		int	iCombIdx = pPeptide->m_iPtmComb;

		for(int i=0; i<m_iNbPtmLocus; i++){
			int iIdx = iCombIdx % m_ptiLimit[i];
			iCombIdx /= m_ptiLimit[i];

			if(iIdx){
				TS_Ptm &stPtm = m_ppSameLocus[i][ iIdx -1 ];
				aIOPeptidePtm.Add(new IOPeptidePtm( stPtm.pPtm->GetLabel(), stPtm.iPos ));
			}
			
		}
	}

}
*/