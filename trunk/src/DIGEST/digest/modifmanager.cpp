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
  Last modification  : 09.02.2007
	Comments           : 

***********************************************************/


#include "modifmanager.h"
#include "MATerror.h"
#include "minmax.h"
#include "util.h"


/******************************************************

		Constructor

*******************************************************/
Modif::Modif(void) :
	m_pszLabel(NULL),
	m_dMass(0),
	m_dScoreCoeff(0),
	m_bNTerm(false),
	m_bCTerm(false),
	m_iTolerance(0),
	m_iNbPeptideLocus(0),
	m_iLimit(0),
	m_iNbLocus(0),
	m_bMode(MODIF_FIXED)
{
}


/******************************************************

		Destructor

*******************************************************/
Modif::~Modif(void)
{
	if(m_pszLabel){
		delete [] m_pszLabel;
		m_pszLabel = NULL;
	}

}



/***********************************************************

	Set

***********************************************************/
void Modif::Set(IOParamModif *pIOParamModif, AtomicMass &atomicMass)
{
	assert(pIOParamModif->GetLabel());

	m_pszLabel		= Strdup(pIOParamModif->GetLabel());
	m_dMass				= atomicMass.GetMass(pIOParamModif->GetFormula());
	m_iTolerance	= pIOParamModif->GetTolerance();
	m_bMode				= pIOParamModif->GetMode();
	m_dScoreCoeff	= pIOParamModif->GetScoring();
	
	
	const char *psz = pIOParamModif->GetLocus();

	while(*psz){
		if( *psz == '^')
			m_bNTerm = true;					//symbole de debut de peptide
		else if( *psz == '$')
			m_bCTerm = true;					//symbole de fin de peptide
		else{
			int iLocus = *psz - 'A';
			if(iLocus < 0 || iLocus >= 26)
				ThrowError("Modif::Set", "Locus '%c' is not valid for modif %s", *psz, m_pszLabel);
			m_aLocus.Add(*psz - 'A');	//acide amine code
		}

		psz++;
	}

	m_aLocus.Ajust();
}

/***********************************************************

	SetNbPeptideLocus

***********************************************************/
int Modif::SetNbPeptideLocus(int *m_piAACmp)
{
	m_iNbPeptideLocus = 0;

	//additionne le nombre de sites possibles sur le peptide
	for(int i=0; i<m_aLocus.GetNbElt(); i++)
		m_iNbPeptideLocus += m_piAACmp[m_aLocus[i]];

	//ajoute les extremites si elles ont ete demandees par l'utilisateur
	if(m_bNTerm)
		m_iNbPeptideLocus++;

	if(m_bCTerm)
		m_iNbPeptideLocus++;

	//s'il y a plus de site que la tolerance, on arrete la combinatoire a la tolerance.
	m_iLimit = MIN(m_iNbPeptideLocus, m_iTolerance);

	m_iLimit++;	//si 1 locus => 2 cas : sans modif et avec une modif

	return m_iLimit;	
}


/***********************************************************

   InitPosCombination

 ***********************************************************/
bool Modif::InitPosCombination(const char *pszSQ, PeptideDigest *pPeptide)
{
	ResetPosCombination();

	int p	= GetNbLocusModified();

	if(!m_iNbPeptideLocus || !p)
		return false;
	
	//Set locus positions on peptide
	int i,j;

	m_aPeptideLocusPos.Reset();


	if(m_bNTerm)
		m_aPeptideLocusPos.Add(-1);		//index -1 pour position NTerm



	//nbre de locus sur la sequence en dehors de CTerm et NTerm
	int iNbLocus = m_iNbPeptideLocus;

	if(m_bNTerm)
		iNbLocus--;
	if(m_bCTerm)
		iNbLocus--;

	for(i=pPeptide->GetStart(); iNbLocus; i++){
		for(j=0; j<m_aLocus.GetNbElt(); j++){
			if(pszSQ[i] == (char)(m_aLocus[j] + 'A')){
				m_aPeptideLocusPos.Add(i - pPeptide->GetStart());
				iNbLocus--;
			}
		}
	}
	

	if(m_bCTerm)
		m_aPeptideLocusPos.Add( pPeptide->GetEnd() +1);		//index peptide end +1 pour position CTerm

	
	//set end configuration
	m_aPosEnd.Reset();

	m_iEndPosComb	= m_iNbPeptideLocus - p;
	for(i=m_iEndPosComb; i<m_iNbPeptideLocus; i++)
		m_aPosEnd.Add(i);

	return true;
}

/***********************************************************

   NextPosCombination

 ***********************************************************/
bool Modif::NextPosCombination(void)
{
	if(m_aPosComb[0] == m_iEndPosComb )
		return false;

	int p = GetNbLocusModified();
	int i;

	for(i=p-1; m_aPosComb[i] == m_aPosEnd[i]; i--);
	m_aPosComb[i]++;

	while(++i < p)
		m_aPosComb[i] = m_aPosComb[i-1] +1;
	
	return true;
}

/***********************************************************

   ResetPosCombination

 ***********************************************************/
void Modif::ResetPosCombination(void)
{
	int p = GetNbLocusModified();

	m_aPosComb.Reset();

	for(int i=0; i<p; i++)
		m_aPosComb.Add(i);
}

/***********************************************************

   AddMassToPosArray

***********************************************************/
void Modif::AddMassToPosArray(DynamicArray<double> &aMass)
{
	for(int i=0; i<m_aPosComb.GetNbElt(); i++){
		int iPos = m_aPeptideLocusPos[m_aPosComb[i]] +1;	//+1 a cause de NTerm
		assert(iPos >= 0 && iPos < aMass.GetNbElt());
		aMass[iPos] += m_dMass; 
	}
}

/***********************************************************

   Write

***********************************************************/
void Modif::Write(void)
{
	if(!m_iNbPeptideLocus)
		return;

	printf("%s %d/%d ", m_pszLabel, GetNbLocusModified(), m_iNbPeptideLocus);
	
	//info sur la position des modifs
	if(m_aPosComb.GetNbElt()){
	
		printf("(");

		for(int i=0; i<m_aPosComb.GetNbElt(); i++)
			printf("%d ", m_aPeptideLocusPos[ m_aPosComb[i] ]);
		
		printf(") ");

	}	
}

/***********************************************************

	AddIOModif

***********************************************************/
/*
void Modif::AddIOModif(DynamicArray<IOPeptideModif *> &aIOModif)
{
	aIOModif.Add( new IOPeptideModif(	m_iNbPeptideLocus, GetNbLocusModified()) );
}
*/






/******************************************************

		Constructor

*******************************************************/
ModifManager::ModifManager(void) :
	m_iNbCombination(0)
{
}

/******************************************************

		Destructor

*******************************************************/
ModifManager::~ModifManager(void)
{
	m_aModif.DeletePtrs();
}


/***********************************************************

	Load

***********************************************************/
void ModifManager::Load(IOParam *pParam)
{
	AtomicMass atomicMass(pParam->m_eResolution);

	int iNbModif = pParam->m_aModif.GetNbElt();
	m_aModif.EnsureSize(iNbModif);

	Modif *pModif;
	for(int i=0; i<iNbModif; i++){
		pModif = new Modif;
		pModif->Set(pParam->m_aModif[i], atomicMass);
		m_aModif.Add(pModif);
	}
}


/***********************************************************

	InitCombination

Initialise la combinatoire sur une peptide donnee
renvoie le nbre total de combinaison a effectuer
***********************************************************/
void ModifManager::InitCombination(PeptideDigest *pPeptide)
{
	m_iNbCombination				= 1;
	pPeptide->m_iModifComb	= -1;

	for(int i=0; i<m_aModif.GetNbElt(); i++)
		m_iNbCombination *= m_aModif[i]->SetNbPeptideLocus(pPeptide->m_tiAACmp);
}


/***********************************************************

	Next

Incremente la combinatoire
***********************************************************/

bool ModifManager::Next(PeptideDigest *pPeptide)
{
	if(++(pPeptide->m_iModifComb) >= m_iNbCombination)
		return false;
	
	SetModif(pPeptide);
	return true;
}

/***********************************************************

   SetModif

 ***********************************************************/
void ModifManager::SetModif(PeptideDigest *pPeptide)
{
	Modif *pModif;
	int		iCombIdx = pPeptide->m_iModifComb;

	for(int i=0; i<m_aModif.GetNbElt(); i++){
		pModif = m_aModif[i];

		//determine le nbre de modif pour chaque modif grace a m_iCombIdx
		pModif->m_iNbLocus = iCombIdx % pModif->m_iLimit;
		iCombIdx /= pModif->m_iLimit;
	}
}

/***********************************************************

   Reload

 ***********************************************************/
void ModifManager::Reload(const char *pszSQ, PeptideDigest *pPeptide)
{
	//recalcul la composition en acide amine pour les modifs
	pPeptide->SetAACmp(pszSQ);

	//reload modif state
	int iModifComb = pPeptide->m_iModifComb;
	InitCombination(pPeptide);

	pPeptide->m_iModifComb = iModifComb;
	SetModif(pPeptide);
	

	//modif position
	if(pPeptide->m_iModifPosComb != -1){

		int iModifPosComb = pPeptide->m_iModifPosComb;

		InitPosCombination(pszSQ, pPeptide);
		
		bool bRet;
		while(pPeptide->m_iModifPosComb < iModifPosComb){
			bRet = NextPosCombination(pPeptide);
			assert(bRet);
		}
	}
}

/***********************************************************

   GetMass

 ***********************************************************/
double ModifManager::GetMass(void)
{
	double dMass = 0;
	for(int i=0; i<m_aModif.GetNbElt(); i++)
		dMass		+= m_aModif[i]->GetMass();
	
	return dMass;
}

/***********************************************************

   GetScore

 ***********************************************************/
double ModifManager::GetScore(void)
{
	double dScore = 1;
	for(int i=0; i<m_aModif.GetNbElt(); i++)
		dScore		*= m_aModif[i]->GetScore();
	
	return dScore;
}

/***********************************************************

   InitPosCombination

 ***********************************************************/
void ModifManager::InitPosCombination(const char *pszSQ, PeptideDigest *pPeptide)
{
	pPeptide->m_iModifPosComb = 0;
	m_aModifPos.Reset();

	for(int i=0; i<m_aModif.GetNbElt(); i++){
		if(m_aModif[i]->InitPosCombination(pszSQ, pPeptide)){
			m_aModifPos.Add(m_aModif[i]);
		}
	}
}
/***********************************************************

   NextPosCombination

 ***********************************************************/
bool ModifManager::NextPosCombination(PeptideDigest *pPeptide)
{
	int i;

	for(i=0; i<m_aModifPos.GetNbElt() && !m_aModifPos[i]->NextPosCombination(); i++)
		m_aModifPos[i]->ResetPosCombination();

	pPeptide->m_iModifPosComb++;

	return (i != m_aModifPos.GetNbElt());
}

/***********************************************************

   Write

***********************************************************/
void ModifManager::Write(void)
{
	//for(int i=0; i<m_aModifPos.GetNbElt(); i++)
	//	m_aModifPos[i]->Write();

	for(int i=0; i<m_aModif.GetNbElt(); i++)
		m_aModif[i]->Write();
	
}

/***********************************************************

   AddMassToPosArray

 ***********************************************************/
void ModifManager::AddMassToPosArray(DynamicArray<double> &aMass)
{
	for(int i=0; i<m_aModifPos.GetNbElt(); i++)
		m_aModifPos[i]->AddMassToPosArray(aMass);
}



/***********************************************************

   StoreIOPeptide

***********************************************************/
/*
void ModifManager::StoreIOPeptide(IOPeptide *pIOPeptide)
{
	DynamicArray<IOPeptideModif *> &aIOModif =  pIOPeptide->m_aModif;
	aIOModif.EnsureSize(m_aModif.GetNbElt());

	for(int i=0; i<m_aModif.GetNbElt(); i++)
		m_aModif[i]->AddIOModif(aIOModif);
}
*/

