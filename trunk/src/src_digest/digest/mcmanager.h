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
	Created on         : 29.11.2007
	Last modification  : 29.11.2007
	Comments           : 

***********************************************************/


#ifndef __MCMANAGER_H__
#define __MCMANAGER_H__

#include "peptidedigest.h"

#include <stdlib.h>
#include <string.h>



/******************************************************

Class MCManager :

Gere des PeptideDigest et genere les missedCleavage
*******************************************************/

class MCManager
{
friend class Digest;

private :

	PeptideDigest	*m_pPeptideArray;
	int						m_iNbpeptide;
	int						m_iCurrent;
	int						m_iResetSize;

	double				m_dPeptideAdd;		//masse d'une molecule d'eau ET +- un proton
																	//ET la digestion peut entrainer une modification de 
																	//structure (exemple digestion avec CNBr)

public :

	MCManager(void);
	~MCManager(void);

	void Init(int iNbMC);

	PeptideDigest *Reset(void);

	PeptideDigest *Next(int iStart);

	void Run(void);

	bool GetPeptide(int iNbMC, PeptideDigest &peptide);

	inline PeptideDigest *GetCurrent(void)
	{ return &m_pPeptideArray[m_iCurrent]; }

	int GetStartMin(void);
};



#endif

