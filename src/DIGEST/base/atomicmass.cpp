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
  Last modification  : 17.11.2006
	Comments           : 

***********************************************************/

#include "atomicmass.h"
#include "MATerror.h"

#include <string.h>
#include <stdlib.h>

#define NB_LETTERS	26

/******************************************************

		Constructor

*******************************************************/
AtomicMass::AtomicMass(TE_MassMode eMode) 
{
	Set(eMode);
}



/******************************************************

		Destructor

*******************************************************/
AtomicMass::~AtomicMass(void)
{
}



/***********************************************************

	Set

***********************************************************/

void AtomicMass::Set(TE_MassMode eMode)
{
	switch(eMode){
		
		case MASS_AVERAGE:
			m_dElectron	= 0.00051;	
			m_dD				= 2.0141;			
			m_dC				= 12.0107;		
			m_dO				= 15.9994;		
			m_dN				= 14.0067;		
			m_dH				= 1.0079;			
			m_dS				= 32.0650;		
			m_dP				= 30.9738;		
			m_dF				= 19.9984;		
			m_dCl				= 35.4532;		
			m_dBr				= 79.9041;		
			m_dI				= 126.9045;		
			break;

		case MASS_MONOISOTOPIC:
			m_dElectron	= 0.000545;
			m_dD				= 2.0141;			
			m_dC				= 12;					
			m_dO				= 15.994914;	
			m_dN				= 14.003073;	
			m_dH				= 1.007825;		
			m_dS				= 31.972070;	
			m_dP				= 30.973761;	
			m_dF				= 19.998403;	
			m_dCl				= 34.968852;	
			m_dBr				= 78.918337;	
			m_dI				= 126.904460;	
			break;

		default :
			ThrowError("AtomicMass::Load", "Unknown mode %d", eMode);
	}
}



/***********************************************************

	GetMass

***********************************************************/

double AtomicMass::GetMass(const char *psz)
{
	double		dMass	= 0;
	int				iNb;
	double		d;
	const char	*pszCpy = psz;

	while( ReadAtome(&psz, &d) ){
		ReadNumber(&psz, &iNb);
		dMass += d * iNb;
	}

	if(!ReadCharge(&psz, &dMass) || *psz)
		ThrowError("AtomicMass::GetMass", "Formula \"%s\" is not valid for an atomic mass", pszCpy);

	return dMass;
}



/***********************************************************

   ReadAtome

***********************************************************/
bool AtomicMass::ReadAtome(const char **ppsz, double *pdMass)
{
	switch(**ppsz){

		case '\0':
		case '(':
			return false;

		case 'D':
			*pdMass = m_dD;
			(*ppsz)++;
			return true;

		case 'C':
			if( (*ppsz)[1] == 'l'){
				*pdMass = m_dCl;
				(*ppsz) += 2;
			}else{
				*pdMass = m_dC;
				(*ppsz)++;
			}
			return true;

		case 'O':
			*pdMass = m_dO;
			(*ppsz)++;
			return true;

		case 'N':
			*pdMass = m_dN;
			(*ppsz)++;
			return true;

		case 'H':
			*pdMass = m_dH;
			(*ppsz)++;
			return true;

		case 'S':
			*pdMass = m_dS;
			(*ppsz)++;
			return true;

		case 'P':
			*pdMass = m_dP;
			(*ppsz)++;
			return true;

		case 'F':
			*pdMass = m_dF;
			(*ppsz)++;
			return true;

		case 'I':
			*pdMass = m_dI;
			(*ppsz)++;
			return true;

		case 'B':
			if( (*ppsz)[1] == 'r'){
				*pdMass = m_dD;
				(*ppsz) += 2;
				return true;
			}
	}
	
	ThrowError("AtomicMass::ReadAtome", "Unknown atome %c", **ppsz);
	return false;	//inutile mais compilo content
}



/***********************************************************

	ReadNumber

***********************************************************/

void AtomicMass::ReadNumber(const char **ppsz, int *pi)
{
	int i;
	
	if(sscanf(*ppsz, "%d", &i) != 1){
		*pi = 1;						//par default pas de nombre = 1 fois
		return;
	}

	*pi = i;

	if(**ppsz == '-')
		(*ppsz)++;

	//avance le pointeur de ce qui a ete lu
	while( (**ppsz >= '0') && (**ppsz <= '9') )
		(*ppsz)++;
}


/***********************************************************

	ReadCharge

***********************************************************/

bool AtomicMass::ReadCharge(const char **ppsz, double *pdMass)
{
	//pas de charge
	if(**ppsz == '\0')
		return true;

	if(**ppsz != '(')
		return false;

	(*ppsz)++;

	int iNbCharge = 0;
	ReadNumber(ppsz, &iNbCharge);

	switch(**ppsz){
		case '+':
			*pdMass += - (iNbCharge * m_dElectron);
			break;

		case '-':
			*pdMass += iNbCharge * m_dElectron;
			break;

		default :
			return false;
	}

	(*ppsz)++;
	
	if(**ppsz != ')')
		return false;

	(*ppsz)++;

	return true;
}

