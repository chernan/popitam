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
	Created on         : 08.02.2007
	Last modification  : 08.02.2007
	Comments           : 

***********************************************************/



#ifndef __PEPTIDEBASE_H__
#define __PEPTIDEBASE_H__



/******************************************************

Class PeptideBase


*******************************************************/


class PeptideBase
{
friend class Digest;
friend class PtmManager;
friend class ModifManager;
friend class MCManager;

protected :

	double		m_dMass;							//mass
	double		m_dModifScore;				//score de toutes les modifs du peptide
	int				m_iStart;							//start sur la sequence
	int				m_iEnd;								//end sur la sequence
	int 			m_iPtmComb;						//combinatoire des ptm 
	int 			m_iModifComb;					//combinatoire des modifs
	int				m_iModifPosComb;			//combinatoire des positions des modifs
	char			m_cCtermAA;						//AA terminal
	char			m_cNbMC;							//nbre de missed cleavage


public :

	PeptideBase(void);
	~PeptideBase(void);
			
	inline double GetMass(void) const
	{ return m_dMass; };

	inline char GetNbMC(void) const
	{ return m_cNbMC; };

	inline int GetNbAA(void) const
	{ return m_iEnd - m_iStart +1; };

	inline int GetStart(void) const
	{ return m_iStart; };

	inline int GetEnd(void) const
	{ return m_iEnd; };

	inline char GetCtermAA(void) const
	{ return m_cCtermAA; };
	
	inline int GetPtmCombination(void) const
	{ return m_iPtmComb; }

	inline int GetModifCombination(void) const
	{ return m_iModifComb; }

	inline int GetModifPosCombination(void) const
	{ return m_iModifPosComb; }


	inline double GetModifScore(void) const
	{ return m_dModifScore; }

	void Copy(PeptideBase *pPeptide);

};



#endif

