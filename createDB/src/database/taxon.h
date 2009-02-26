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
  Last modification  : 08.02.2006
	Comments           : 

***********************************************************/

#ifndef __TAXON_H__
#define __TAXON_H__

#include "dynamicarray.h"


class DBFileWriter;

/******************************************************

Class Taxon


*******************************************************/

class Taxon
{
friend class Taxonomy;

private :

	int														m_iId;						
	int														m_iParentId;	
	DynamicArray<Taxon *>					m_aChild;
	DynamicArray<unsigned int>		m_aOffset;

	unsigned int									m_uiOffsetStart;
	unsigned int									m_uiOffsetEnd;
	
public :

	Taxon(int iId, int iParentId);
	~Taxon(void);

	inline int GetId(void) const
	{ return m_iId; }

	inline int GetParentId(void) const
	{ return m_iParentId; }

	inline void AddChild(Taxon *pTaxon)
	{ m_aChild.Add(pTaxon); }


	inline void AddOffset(unsigned int uiOffset)
	{ m_aOffset.Add(uiOffset); }


	void ResetOffset(void);

	void WriteEntries(DBFileWriter *pFileWriter);

	inline int GetNbOffset(void)
	{ return m_aOffset.GetNbElt(); }

	inline unsigned int GetOffset(int i)
	{ return m_aOffset[i]; }

	inline unsigned int GetOffsetStart(void)
	{ return m_uiOffsetStart; }

	inline unsigned int GetOffsetEnd(void)
	{ return m_uiOffsetEnd; }

};


#endif
