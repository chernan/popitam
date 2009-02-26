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


#include "peptidebase.h"
#include <string.h>


/******************************************************

		Constructor 

*******************************************************/
PeptideBase::PeptideBase(void) :
	m_dMass(0),
	m_dModifScore(0),
	m_iStart(0),
	m_iEnd(0),
	m_iPtmComb(0),
	m_iModifComb(0),
	m_iModifPosComb(0),
	m_cCtermAA('\0'),
	m_cNbMC(0)
{
}

/******************************************************

		Destructor 

*******************************************************/
PeptideBase::~PeptideBase(void)
{
}

/***********************************************************

   Copy

 ***********************************************************/
void PeptideBase::Copy(PeptideBase *pPeptide)
{
	memcpy(this, pPeptide, sizeof(PeptideBase));
}
