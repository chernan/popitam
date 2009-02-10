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
	Created on         : 10.02.2006
	Last modification  : 10.02.2006
	Comments           : 

***********************************************************/


#ifndef __ACTREE_H__
#define __ACTREE_H__




/***********************************************************

Class AcTree

***********************************************************/


class AcTree
{

private :

	char				m_char;
	bool				m_bEnd;

	AcTree			*m_pNext;
	AcTree			*m_pBrother;

	
public :

	AcTree(void);
	~AcTree(void);

	void Add(const char *psz);
	bool Get(const char *psz);

	void Free(void);

	inline bool IsEmpty(void)
	{ return (m_char == '\0'); }
};


#endif
