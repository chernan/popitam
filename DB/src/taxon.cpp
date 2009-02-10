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

#include "taxon.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include "dbfilewriter.h"

/******************************************************

		Constructor

*******************************************************/
Taxon::Taxon(int iId, int iParentId) :
	m_iId(iId),
	m_iParentId(iParentId)
{
}


/******************************************************

		Destructor

*******************************************************/
Taxon::~Taxon(void)
{
	m_aChild.DeletePtrs();
}





/***********************************************************

   ResetOffset

fct recursive
***********************************************************/
void Taxon::ResetOffset(void)
{
	m_aOffset.Reset();

	for(int i=0; i<m_aChild.GetNbElt(); i++)
		m_aChild[i]->ResetOffset();
}

/***********************************************************

   WriteEntries

***********************************************************/
void Taxon::WriteEntries(DBFileWriter *pFileWriter)
{
	int i;

	m_uiOffsetStart = pFileWriter->GetCurrentOffset();

	//deplace toutes les entrees de ce taxon dans le fichier trie
	for(i=0; i<m_aOffset.GetNbElt(); i++)
		pFileWriter->SortEntry(m_aOffset[i]);

	//recursion
	for(i=0; i<m_aChild.GetNbElt(); i++)
		m_aChild[i]->WriteEntries(pFileWriter);

	m_uiOffsetEnd = pFileWriter->GetCurrentOffset();
}
