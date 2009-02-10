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
	Created on         : 06.07.2006
	Last modification  : 06.07.2006
	Comments           : 

***********************************************************/


#ifndef __PIMW_H__
#define __PIMW_H__

#include "atomicmass.h"



/******************************************************

Class PiMw


*******************************************************/

class PiMw
{

private :

	double	m_dPka[26][3];		//tableau des valeur de PKa de chaque AA
	int			m_iComp[26];			//composition en AA : nbre de chaque AA dans la sequence
	double	m_tdAAMass[26];		//masse de chaque AA (masse average)
	double  m_dH2O;						//masse d'une molecule d'eau 

public :

	PiMw(TE_MassMode eMode=MASS_MONOISOTOPIC);
	~PiMw(void);

	double GetMw(const char *psz);
	double GetPi(const char *psz);

	void SetMassMode(TE_MassMode eMode);
};







#endif
