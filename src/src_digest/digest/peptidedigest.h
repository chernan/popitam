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
	Created on         : 07.02.2007
	Last modification  : 07.02.2007
	Comments           : 

***********************************************************/


#ifndef __PEPTIDEDIGEST_H__
#define __PEPTIDEDIGEST_H__


#include "peptidebase.h"
#include "dbentry.h"

/******************************************************

Class PeptideDigest


*******************************************************/


class PeptideDigest : public PeptideBase
{
friend class Digest;

public :

	int				m_tiAACmp[26];				//composition en AA

	TS_Ptm		*m_ptstPtm;						//pointeur vers une liste de PTM
	int				m_iNbPtm;							//nbre de PTM a prendre en compte a partir du pointeur
	bool			m_bMultiPtmPerlocus;	//plusieurs ptm partage un meme locus sur la sequence

	bool			m_bTrash;							//ce peptide contient un AA inconnu(J,O,U) ou indetermine(B,X,Z) il est poubellise.


public :

	PeptideDigest(void);
	~PeptideDigest(void);

	void Copy(PeptideDigest *pPeptide);

	inline void Copy(PeptideBase *pPeptide)
	{ PeptideBase::Copy(pPeptide); }

	void SetAACmp(const char *pszSQ);
		
	inline TS_Ptm &GetPtm(int i)
	{ return m_ptstPtm[i]; }
};



#endif

