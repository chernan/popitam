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
	Created on         : 27.11.2003
	Last modification  : 27.11.2003
	Comments           : 

***********************************************************/

#ifndef __ATOMICMASS_H__
#define __ATOMICMASS_H__

#include "massdefine.h"

/******************************************************

Class AtomicMass


*******************************************************/

class AtomicMass
{

private :

	double	m_dElectron;

	double	m_dD;
	double	m_dC;
	double	m_dO;
	double	m_dN;
	double	m_dH;
	double	m_dS;
	double	m_dP;
	double	m_dF;
	double	m_dCl;
	double	m_dBr;
	double	m_dI;


	
public :

	AtomicMass(TE_MassMode eMode);
	~AtomicMass(void);

	void Set(TE_MassMode eMode);
	double GetMass(const char *psz);

private :

	bool ReadAtome(const char **ppsz, double *pdMass);
	void ReadNumber(const char **ppsz, int *pi);
	bool ReadCharge(const char **ppsz, double *pdMass);

};


#endif
