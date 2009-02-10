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

#ifndef __TAXONOMY_H__
#define __TAXONOMY_H__

#include "taxon.h"
#include "file.h"
#include "dbfile.h"

/******************************************************

Class Taxonomy

gere un arbre de Taxon
*******************************************************/


class Taxonomy
{
	
private :


	DynamicArray<Taxon *> m_aTaxId;		//tableau faisant le lien entre chaque taxId et son taxon	
	Taxon									*m_pRoot;		//arbre de taxon

	int										m_iHighestId;

	bool									m_bDefined;

public :

	Taxonomy(void);
	~Taxonomy(void);

	void Load(const char *pszFileName);

	Taxon *GetTaxon(int iTaxId);

	inline void ResetOffset(void)
	{ m_pRoot->ResetOffset(); }


	inline void WriteEntries(DBFileWriter *pFileWriter)
	{ m_pRoot->WriteEntries(pFileWriter); }

	void WriteTaxIdOffset(File &file);

	inline int GetNbUnclassified(void)
	{ return m_pRoot->GetNbOffset(); }

	inline int GetNbTaxId(void)
	{ return m_bDefined? m_aTaxId.GetNbElt() : 0; }

	inline bool IsDefined(void)
	{ return m_bDefined; }
	
private :

	void LoadNCBI(const char *pszFileName, DynamicArray<Taxon *> &aTaxon);

	void SetTaxId(DynamicArray<Taxon *> &array);

	void Link(DynamicArray<Taxon *> &array);

};


#endif
