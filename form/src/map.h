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
	Created on         : 28.11.2003
	Last modification  : 28.11.2003
	Comments           : 

***********************************************************/

#ifndef __MAP_H__
#define __MAP_H__



class Map
{

private :

	/* nb groupe de hachage */
	int		m_iNbMapGroup;

	/* tableau de pointeur sur arbres binaires lexicographiques. */
	Lexico	**m_ptpLexico;


public :

	Map(void);
	~Map(void);

	bool Init(int iNbMapGroup, void (*pfFreeElt)(const char *psz, void *pvData));
	bool Add(const char *psz, void *pvData);
	bool Supp(const char *psz);
	bool Get(const char *psz, void **ppvData);
	void Free(void);

private :

	int	 Hachage(const char *psz);


};

#endif

