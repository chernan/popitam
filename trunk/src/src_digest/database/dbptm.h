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
	Created on         : 06.02.2007
	Last modification  : 06.02.2007
	Comments           : 

***********************************************************/


#ifndef __DBPTM_H__
#define __DBPTM_H__




/******************************************************

Class DBPtm


*******************************************************/

class DBPtm
{

private :

	int			m_iId;
	double	m_dMass;				//monoisotopic mass
	char		*m_pszLabel;

public :

	DBPtm(int iId, const char *pszLabel, double dMass);
	~DBPtm(void);

	inline double GetMass(void)
	{ return m_dMass; }

	inline const char *GetLabel(void)
	{ return m_pszLabel; }

	inline int GetId(void)
	{ return m_iId; }
};




#endif
