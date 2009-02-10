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
	Created on         : 30.11.2007
	Last modification  : 30.11.2007
	Comments           : 

***********************************************************/

#include "mcmanager.h"


/******************************************************

		Constructor 

*******************************************************/

MCManager::MCManager(void) :
	m_pPeptideArray(NULL),
	m_iNbpeptide(0),
	m_iCurrent(0),
	m_iResetSize(0)
{
}

/******************************************************

		Destructor 

*******************************************************/

MCManager::~MCManager(void)
{
	if(m_pPeptideArray){
		delete [] m_pPeptideArray;
		m_pPeptideArray = NULL;
	}
}

/***********************************************************

   Init

 ***********************************************************/
void MCManager::Init(int iNbMC)
{
	assert(iNbMC >= 0);

	m_iNbpeptide		= iNbMC + 1;
	m_pPeptideArray = new PeptideDigest[m_iNbpeptide];;
	m_iCurrent			= 0;
	m_iResetSize		= sizeof(PeptideDigest) * m_iNbpeptide;
}


/***********************************************************

   Reset

***********************************************************/
PeptideDigest *MCManager::Reset(void)
{
	memset(m_pPeptideArray, 0, m_iResetSize);
	m_iCurrent = 0;
	return GetCurrent();
}

/***********************************************************

	Next

***********************************************************/
PeptideDigest *MCManager::Next(int iStart)
{
	//change le peptide current
	if(++m_iCurrent == m_iNbpeptide)
		m_iCurrent = 0;

	PeptideDigest *pCurrent = GetCurrent();
	
	//reset
	memset(pCurrent, 0, sizeof(PeptideDigest));

	//set start
	pCurrent->m_iStart = iStart;

	return pCurrent;
}

/***********************************************************

   GetPeptide

 ***********************************************************/
bool MCManager::GetPeptide(int iNbMC, PeptideDigest &peptide)
{
	PeptideDigest &current	= m_pPeptideArray[m_iCurrent];
	PeptideDigest *pPeptide = &current;

	if(iNbMC > 0){
		
		int iIdx = m_iCurrent - iNbMC;
		if(iIdx < 0)
			iIdx += m_iNbpeptide;

		pPeptide = &m_pPeptideArray[iIdx];

		//le peptide avec MC n'existe peut etre pas en debut de sequence
		bool bPeptideNotDefine = (pPeptide->m_dMass == 0)? true : false;

		pPeptide->m_dMass	+= current.m_dMass;							
		pPeptide->m_iEnd		= current.m_iEnd;							
		pPeptide->m_cNbMC++;							
		pPeptide->m_bTrash |= current.m_bTrash;

		//AA composition
		for(int j=0; j<26; j++)
			pPeptide->m_tiAACmp[j] += current.m_tiAACmp[j];

		//PTM
		if(pPeptide->m_iNbPtm){
			pPeptide->m_iNbPtm							+=	current.m_iNbPtm;
			pPeptide->m_bMultiPtmPerlocus	|=	current.m_bMultiPtmPerlocus;
		}else{
			pPeptide->m_iNbPtm							=		current.m_iNbPtm;
			pPeptide->m_ptstPtm						=		current.m_ptstPtm;
			pPeptide->m_bMultiPtmPerlocus	|=	current.m_bMultiPtmPerlocus;
		}

		if(bPeptideNotDefine)
			return false;
	}


	peptide.Copy(pPeptide);

	//ajoute une molecule d'eau +- protonee... etc...
	peptide.m_dMass += m_dPeptideAdd;

	
	return true;
}



/***********************************************************

   GetStartMin

 ***********************************************************/
int MCManager::GetStartMin(void)
{
	int iIdx = m_iCurrent +1;
	if(iIdx == m_iNbpeptide)
		iIdx = 0;

	return m_pPeptideArray[iIdx].GetStart();
}
