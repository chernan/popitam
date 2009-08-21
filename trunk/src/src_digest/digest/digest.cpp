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
  Last modification  : 09.02.2007
	Comments           : 

***********************************************************/


#include "digest.h"
#include "MATerror.h"
#include "compare.h"
#include <float.h>

/******************************************************

		Constructor 

*******************************************************/
Digest::Digest(void) :
	m_pCompare(NULL),
	m_iMCLimit(1),
	m_dMassMin(0),
	m_dMassMax(DBL_MAX),
	m_iNbPeptideGenerated(0)
{
}

/******************************************************

		Destructor 

*******************************************************/
Digest::~Digest(void)
{
}

/***********************************************************

	Load

***********************************************************/
void Digest::Load(IOParam *pParam)
{
	m_modifManager.Load(pParam); 
	m_ptmManager.Load(pParam);

	//Missed Cleavage
	m_MCManager.Init(pParam->m_iMissedClevage);
	m_iMCLimit = pParam->m_iMissedClevage + 1;

	//Masse des AA
	const char *ppszAAFormula[26] = {
		"C3H5ON",		//A
		"",			//B
		"C3H5ONS", 		//C
		"C4H5O3N", 		//D
		"C5H7O3N", 		//E
		"C9H9ON",		//F
		"C2H3ON", 		//G
		"C6H7ON3", 		//H
		"C6H11ON", 		//I
		"C6H11ON", 		//J
		"C6H12ON2", 		//K
		"C6H11ON", 		//L
		"C5H9ONS",		//M
		"C4H6O2N2", 		//N
		"", 			//O
		"C5H7ON", 		//P
		"C5H8O2N2", 		//Q
		"C6H12ON4", 		//R
		"C3H5O2N", 		//S
		"C4H7O2N", 		//T
		"", 			//U
		"C5H9ON", 		//V
		"C11H10ON2", 		//W
		"", 			//X
		"C9H9O2N",		//Y
		""			//Z	
	};

	AtomicMass atomicMass(pParam->m_eResolution);

	//tous les amino acids
	for(int i=0; i<26; i++)
		m_tdAAMass[i] = atomicMass.GetMass(ppszAAFormula[i]);

	//H2O
	m_dHPlus		= atomicMass.GetMass("H(+)"); 
	m_dAddNTerm	= atomicMass.GetMass("H");
	m_dAddCTerm	= atomicMass.GetMass("OH");

	m_MCManager.m_dPeptideAdd = m_dAddCTerm + m_dAddNTerm; 
	
	//proton

	switch(pParam->m_eIonMode){
		case ION_MODE_MPLUS:	
			m_MCManager.m_dPeptideAdd += m_dHPlus;	
			break;

		case ION_MODE_MMINUS:	
			m_MCManager.m_dPeptideAdd -= m_dHPlus;	
			break;

		case ION_MODE_M :	
			break;

		default :
			ThrowError("Digest::Init", "Unknown Ion mode %d", pParam->m_eIonMode );	
	}

	//Enzyme
	const char *pszEnzyme = pParam->m_pszEnzyme;

	if(!strcmp(pszEnzyme, "Trypsin") )
		m_pfctEnzyme = &Digest::Trypsin;
	else if(!strcmp(pszEnzyme, "Trypsin Low") )
		m_pfctEnzyme = &Digest::TrypsinLowSpecificity;
	else if(!strcmp(pszEnzyme, "Trypsin High") )
		m_pfctEnzyme = &Digest::TrypsinHighSpecificity;
	else if(!strcmp(pszEnzyme, "LysC") )
		m_pfctEnzyme = &Digest::LysC;
	else if(!strcmp(pszEnzyme, "LysN") )
		m_pfctEnzyme = &Digest::LysN;
	else if(!strcmp(pszEnzyme, "CNBr") ){
		m_pfctEnzyme = &Digest::Cnbr;
		m_MCManager.m_dPeptideAdd -= atomicMass.GetMass("SCH4");
	}else if(!strcmp(pszEnzyme, "ArgC") )
		m_pfctEnzyme = &Digest::ArgC;
	else if(!strcmp(pszEnzyme, "AspN") )
		m_pfctEnzyme = &Digest::AspN;
	else if(!strcmp(pszEnzyme, "AspN + GluN") )
		m_pfctEnzyme = &Digest::AspN_GluN;
	else if(!strcmp(pszEnzyme, "AspN + LysC") )
		m_pfctEnzyme = &Digest::AspN_LysC;
	else if(!strcmp(pszEnzyme, "AspN + GluN + LysC") )
		m_pfctEnzyme = &Digest::AspN_GluN_LysC;
	else if(!strcmp(pszEnzyme, "GluC bicarbonate") )
		m_pfctEnzyme = &Digest::GluCBicarbo;
	else if(!strcmp(pszEnzyme, "GluC phosphate") )
		m_pfctEnzyme = &Digest::GluCPhos;
	else if(!strcmp(pszEnzyme, "Chymotrypsin Low") )
		m_pfctEnzyme = &Digest::Chymotrypsin1;
	else if(!strcmp(pszEnzyme, "Chymotrypsin High") )
		m_pfctEnzyme = &Digest::Chymotrypsin2;
	else if(!strcmp(pszEnzyme, "Tryp + Chymo") )
		m_pfctEnzyme = &Digest::Tryp_Chymo;
	else if(!strcmp(pszEnzyme, "Pepsin pH 1.3") )
		m_pfctEnzyme = &Digest::Pepsin1;
	else if(!strcmp(pszEnzyme, "Pepsin pH > 2") )
		m_pfctEnzyme = &Digest::Pepsin2;
	else if(!strcmp(pszEnzyme, "Proteinase K") )
		m_pfctEnzyme = &Digest::ProteinaseK;
	else if(!strcmp(pszEnzyme, "Tryp + GluC") )
		m_pfctEnzyme = &Digest::TrypsinGluC;
	else
		ThrowError("Digest::Init", "unknown enzyme %s", pszEnzyme);
}

/***********************************************************

	Limit

minorant et majorant des masses de recherche
***********************************************************/
void Digest::Limit(double dMassMin, double dMassMax)	
{
	m_dMassMin = dMassMin;
	m_dMassMax = dMassMax;
}

/******************************************************

		Run

*******************************************************/
void Digest::Run(DBEntry* prot)
{	
	//reset
	m_pCurrent	= m_MCManager.Reset();

	//set sequence
	m_pszSequence	= prot->GetSQ();
	m_iNbAA		= (int)strlen(m_pszSequence);

	//set ptm list
	m_ptmManager.Set(prot->GetPtm());
	
	//digestion proprement dite par l'enzyme choisie
	(this->*m_pfctEnzyme)();
}

/***********************************************************

	FindPeptide

***********************************************************/
void Digest::FindPeptide(char cCtermAA, int iEnd)
{
	m_pCurrent->m_cCtermAA	= cCtermAA;
	m_pCurrent->m_iEnd			= iEnd;

	//note les ptm sur le peptide
	m_ptmManager.Link(m_pCurrent, m_MCManager.GetStartMin());

	//Peptide avec missed cleavage
	for(int i=0; i<m_iMCLimit; i++){
		if(m_MCManager.GetPeptide(i, m_peptide))
			Modification();
	}

	//prepare le peptide suivant
	m_pCurrent = m_MCManager.Next(iEnd + 1);
}

/******************************************************

		Modification

*******************************************************/
void Digest::Modification(void)
{
	if(m_peptide.m_bTrash)
		return;

	double		dPeptideWithPtmMass;

	//initialise la combinatoire des modif et des ptm
	m_modifManager.InitCombination(&m_peptide);
	m_ptmManager.InitCombination(&m_peptide);

	while(m_ptmManager.Next(&m_peptide)){

		dPeptideWithPtmMass = m_peptide.m_dMass + m_ptmManager.GetMass(&m_peptide);

		m_modifManager.ResetCombination(&m_peptide);

		while(m_modifManager.Next(&m_peptide)){

			m_peptide.m_dMass				= dPeptideWithPtmMass + m_modifManager.GetMass();
			m_peptide.m_dModifScore	= m_modifManager.GetScore();

			m_iNbPeptideGenerated++;

			//passe la main a process
			if( (m_peptide.m_dMass >= m_dMassMin) && (m_peptide.m_dMass <= m_dMassMax) ){
				m_peptide.m_iModifPosComb = -1;
				m_pCompare->FindPeptide(&m_peptide);
			}
		}		
	}

}

/***********************************************************

   GetMassPosArray

 ***********************************************************/
void Digest::GetMassPosArray(DynamicArray<double> &aMass)
{
	int iNbAA = m_peptide.GetNbAA();
	
	aMass.Reset();
	aMass.SetNbElt(iNbAA +2);	//+2 pour les modifs NTerm et CTerm
	aMass.ZeroMem();

	aMass[0]				= m_dAddNTerm + m_dHPlus;
	aMass[iNbAA+1]	= m_dAddCTerm;

	//ptms
	m_ptmManager.AddMassToPosArray(&m_peptide, aMass);	

	//modifs
	m_modifManager.AddMassToPosArray(aMass);

	//aa
	int iPos = 1;
	for(int i=m_peptide.GetStart(); i<=m_peptide.GetEnd(); i++, iPos++)
		aMass[iPos] += m_tdAAMass[ m_pszSequence[i] - 'A' ];
}

/***********************************************************

   WritePeptide

 ***********************************************************/
void Digest::WritePeptide(void)
{
	//SQ
	printf("%.*s ", m_peptide.GetNbAA(), m_pszSequence + m_peptide.GetStart());

	//Mass
	printf("%.2f ", m_peptide.GetMass());

	//PTM
	m_ptmManager.Write(&m_peptide);

	//Modif
	m_modifManager.Write();

	printf("\n");

}

/***********************************************************

   ReloadPeptide

 ***********************************************************/
void Digest::ReloadPeptide(const char *pszSQ, DynamicArray<TS_Ptm> &aPtm, PeptideBase *pPeptide)
{
	m_pszSequence = pszSQ;
	m_peptide.Copy(pPeptide);

	m_modifManager.Reload(pszSQ, &m_peptide);
	m_ptmManager.Reload(aPtm, &m_peptide);
}










/***********************************************************

	Trypsin

Sequence se termine par un NULL d'ou pas de test en position n+1

***********************************************************/

void Digest::Trypsin(void)
{
	const char	*pcSeq	= m_pszSequence;
	int			iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;							//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'R':
				if(pcSeq[1] == 'P') break;								
				FindPeptide('R', i);
				break;

			case 'K':
				if(pcSeq[1] == 'P') break;			
				FindPeptide('K', i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;

		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}




/***********************************************************

	TrypsinLowSpecificity

***********************************************************/

void Digest::TrypsinLowSpecificity(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'R':
				FindPeptide('R', i);
				break;

			case 'K':
				FindPeptide('K', i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}



/***********************************************************

	TrypsinHighSpecificity

***********************************************************/

void Digest::TrypsinHighSpecificity(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'R':
				if(pcSeq[1] == 'P')break;			

				//premier caractere de la serie
				if(i < m_iNbAA-2){	
					if( (pcSeq[1] == 'R') && (pcSeq[2] == 'H') ) break;	//RRH		
					if( (pcSeq[1] == 'R') && (pcSeq[2] == 'R') ) break;	//RRR
					if( (pcSeq[1] == 'R') && (pcSeq[2] == 'F') ) break;	//RRF
				}
				
				//deuxieme caractere de la serie
				if(i > 0){	
					if( (pcSeq[-1] == 'R') && (pcSeq[1] == 'H') ) break;	//RRH 	
					if( (pcSeq[-1] == 'R') && (pcSeq[1] == 'R') ) break;	//RRR
					if( (pcSeq[-1] == 'C') && (pcSeq[1] == 'K') ) break;	//CRK
					if( (pcSeq[-1] == 'D') && (pcSeq[1] == 'D') ) break;	//DRD	
					if( (pcSeq[-1] == 'R') && (pcSeq[1] == 'F') ) break;	//RRF
					if( (pcSeq[-1] == 'K') && (pcSeq[1] == 'R') ) break;	//KRR
				}
				
				//troisieme caractere de la serie
				if(i > 1){	
					if( (pcSeq[-2] == 'R') && (pcSeq[-1] == 'R') ) break;	//RRR
					if( (pcSeq[-2] == 'K') && (pcSeq[-1] == 'R') ) break;	//KRR
				}

				FindPeptide('R', i);
				break;

			case 'K':
				if(*(pcSeq+1) == 'P') break;				

				//premier caractere de la serie
				if(i < m_iNbAA-2){	
					if( (pcSeq[1] == 'K') && (pcSeq[2] == 'R') ) break;	//KKR		
				}

				//deuxieme caractere de la serie
				if(i > 0){	
					if( (pcSeq[-1] == 'C') && (pcSeq[1] == 'Y') ) break;	//CKY 	
					if( (pcSeq[-1] == 'D') && (pcSeq[1] == 'D') ) break;	//DKD
					if( (pcSeq[-1] == 'C') && (pcSeq[1] == 'H') ) break;	//CKH
					if( (pcSeq[-1] == 'C') && (pcSeq[1] == 'D') ) break;	//CKD	
					if( (pcSeq[-1] == 'K') && (pcSeq[1] == 'R') ) break;	//KKR
				}

				FindPeptide('K', i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	LysC

***********************************************************/

void Digest::LysC(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'K':
				FindPeptide('K', i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}


/***********************************************************

	LysN

***********************************************************/

void Digest::LysN(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;

			default:
				if(pcSeq[1] == 'K')	
					FindPeptide(*pcSeq, i);

		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	Cnbr

***********************************************************/

void Digest::Cnbr(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'M':
				FindPeptide('M', i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	ArgC

***********************************************************/

void Digest::ArgC(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'R':
				if(pcSeq[1] == 'P')	break;			
				FindPeptide('R', i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	AspN

***********************************************************/

void Digest::AspN(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;

			default:
				if(pcSeq[1] == 'D')	
					FindPeptide(*pcSeq, i);
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	AspN_GluN

***********************************************************/

void Digest::AspN_GluN(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;


			default:
				if( (pcSeq[1] == 'D') || (pcSeq[1] == 'E') )	
					FindPeptide(*pcSeq, i);
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}


/***********************************************************

	AspN_LysC

***********************************************************/

void Digest::AspN_LysC(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;

			case 'K':
				FindPeptide('K', i);
				break;

			default:
				if(pcSeq[1] == 'D')	
					FindPeptide(*pcSeq, i);
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}


/***********************************************************

	AspN_GluN

***********************************************************/

void Digest::AspN_GluN_LysC(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;

			case 'K':
				FindPeptide('K', i);
				break;

			default:
				if( (pcSeq[1] == 'D') || (pcSeq[1] == 'E') )	
					FindPeptide(*pcSeq, i);
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}


/***********************************************************

	GluCBicarbo

***********************************************************/

void Digest::GluCBicarbo(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'E':
				if(pcSeq[1] == 'P') break;				
				if(pcSeq[1] == 'E')	break;	
				FindPeptide('E', i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	GluCPhos

***********************************************************/

void Digest::GluCPhos(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	


		switch(*pcSeq){
			
			case 'E':
			case 'D':
				if(pcSeq[1] == 'P') break;				
				if(pcSeq[1] == 'E')	break;	
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	Chymotrypsin1

***********************************************************/

void Digest::Chymotrypsin1(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'F':
			case 'L':
			case 'M':
			case 'W':
				if(pcSeq[1] == 'P') break;				
				FindPeptide(*pcSeq, i);
				break;

			case 'Y':
				if(pcSeq[1] == 'P') break;	
				if( (i > 0) && (pcSeq[-1] == 'P') ) break;
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}




/***********************************************************

	Chymotrypsin2

***********************************************************/

void Digest::Chymotrypsin2(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'F':
			case 'W':
				if(pcSeq[1] == 'P') break;				
				FindPeptide(*pcSeq, i);
				break;

			case 'Y':
				if(pcSeq[1] == 'P') break;	
				if( (i > 0) && (pcSeq[-1] == 'P') ) break;
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}



/***********************************************************

	Tryp_Chymo

***********************************************************/

void Digest::Tryp_Chymo(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'K':
				if(pcSeq[1] == 'P') break;				
				FindPeptide('K', i);
				break;

			case 'R':
				if(pcSeq[1] == 'P') break;				
				FindPeptide('R', i);
				break;

			case 'F':
			case 'W':
				if(pcSeq[1] == 'P') break;				
				FindPeptide(*pcSeq, i);
				break;

			case 'Y':
				if(pcSeq[1] == 'P') break;	
				if( (i > 0) && (pcSeq[-1] == 'P') ) break;
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	Pepsin1

***********************************************************/

void Digest::Pepsin1(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'F':
			case 'L':
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	Pepsin2

***********************************************************/

void Digest::Pepsin2(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'F':
			case 'L':
			case 'W':
			case 'Y':
			case 'A':
			case 'E':
			case 'Q':
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/***********************************************************

	ProteinaseK

***********************************************************/

void Digest::ProteinaseK(void)
{
	const char *pcSeq = m_pszSequence;
	int		iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;			//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'A':
			case 'C':
			case 'G':
			case 'M':
			case 'F':
			case 'S':
			case 'Y':
			case 'W':
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;
		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}

/*******************************************/
/* Trypsin + GluC */
/*******************************************/






void Digest::TrypsinGluC(void)
{
	const char	*pcSeq	= m_pszSequence;
	int			iAA;

	for(int i=0; i<m_iNbAA; i++){
	
		iAA = *pcSeq -'A';
		m_pCurrent->m_dMass += m_tdAAMass[iAA];	//mass	
		m_pCurrent->m_tiAACmp[iAA]++;							//composition en AA du peptide courant	

		switch(*pcSeq){
			
			case 'R':
				if(pcSeq[1] == 'P') break;								
				FindPeptide('R', i);
				break;

			case 'K':
				if(pcSeq[1] == 'P') break;			
				FindPeptide('K', i);
				break;

			case 'E':
			case 'D':
				FindPeptide(*pcSeq, i);
				break;

			case 'B' :
			case 'O' :
			case 'U' :
			case 'X':
			case 'Z':
				m_pCurrent->m_bTrash = true;
				break;

		}

		pcSeq++;
	}

	//traite la fin de la sequence.
	if( m_pCurrent->m_iStart < m_iNbAA )
		FindPeptide(*(--pcSeq), m_iNbAA - 1);
}


/***********************************************************

   StoreIOPeptide

 ***********************************************************/
/*
void Digest::StoreIOPeptide(IOPeptide *pIOPeptide)
{
	m_modifManager.StoreIOPeptide(pIOPeptide);

	m_ptmManager.StoreIOPeptide(pIOPeptide, &m_peptide);
}
*/