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

	Company			   : Swiss Intitute	of Bioinformatics
	Author			   : Marc Tuloup
	Contact			   : Marc.Tuloup@isb-sib.ch
	Created	on		   : 28.11.2003
	Last modification  : 15.09.2004
	Comments		   : 

***********************************************************/

#ifndef	__TAXON_H__
#define	__TAXON_H__



/******************************************************

Class Taxon

Fait le	lien entre un taxon	phylogenetique et les fichiers
de la base de donnes proteiques.
*******************************************************/

class Taxon	: public Item
{


private	:

	int		m_iNbSPEntries;		//nombre d'entrees dans	SwissProt
	int		m_iNbTREntries;		//nombre d'entrees dans	Trembl
	int		m_iNbChild;		//nombre de	sous taxons
	Taxon	**m_ptpTaxon;		//tableau de pointeurs sur les sous	taxons.


public :
    
	static int iDeepInTree;

public :

	Taxon(void);
	~Taxon(void);

	void Read(TagFile *pFile);


	void WriteFile(TagFile *pFile);

	void WriteFileAlone(TagFile	*pFile);


	Taxon *Find(int	iId);
	Taxon *Find(const char *psz);


	void Write(File	&file);
	void WriteChildren(File &file);

	void Hide(File &file, const	char *pszName);

	void Reset(void);

	int	GetNbSelectedTaxon(void);
	int	GetNbSPEntries(void);
	int	GetNbTREntries(void);
	Taxon *GetFirstSelectedItem(void);
    

	void WriteHtmlInputSummary(File	&file);

};


#endif
