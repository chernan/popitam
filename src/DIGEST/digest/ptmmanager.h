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
	Last modification  : 08.07.2004
	Comments           : 

***********************************************************/


#ifndef __PTMMANAGER_H__
#define __PTMMANAGER_H__


#include "ioparam.h"
#include "dynamicarray.h"
#include "peptidedigest.h"
//#include "ioresult.h"


/******************************************************

Class PtmManager


*******************************************************/

class PtmManager
{
	friend class Digest;

private :
		
	TS_Ptm				*m_pPosInList;		//position dans la liste globale des Ptm pour une DBEntry
	TS_Ptm				*m_pEndList;			//fin de la liste (adresse de la case juste apres)
	

	int						m_iNbPtmLocus;

	//combinatoire
	int						m_iNbCombination;
		

	//version plusieurs ptms par locus		
	int						*m_ptiLimit;
	TS_Ptm				**m_ppSameLocus;	

	//filters
	int						m_iNbMaxPtmLocus;				//laisse tomber la combinatoire au dessus de ce seuil
	bool					m_bUse;

public :

	PtmManager(void);
	~PtmManager(void);

	void Load(IOParam *pParam);

	void Set(DynamicArray<TS_Ptm> &aPtm);

	void Link(PeptideDigest *pPeptide, int iStartMin);

	void InitCombination(PeptideDigest *pPeptide);

	inline bool Next(PeptideDigest *pPeptide)
	{ return (++(pPeptide->m_iPtmComb) < m_iNbCombination ); }

	inline void ResetCombination(PeptideDigest *pPeptide)
	{	pPeptide->m_iPtmComb = -1; }

	double GetMass(PeptideDigest *pPeptide);

	void AddMassToPosArray(PeptideDigest *pPeptide, DynamicArray<double> &aMass);

	void Reload(DynamicArray<TS_Ptm> &aPtm, PeptideDigest *pPeptide);

	void Write(PeptideDigest *pPeptide);

	//void StoreIOPeptide(IOPeptide *pIOPeptide, PeptideDigest *pPeptide);

private :

	void Free(void);
};


#endif
