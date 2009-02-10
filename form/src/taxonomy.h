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

#ifndef	__TAXONOMY_H__
#define	__TAXONOMY_H__


typedef	struct{
	int		iTaxId;
	Taxon	*pTaxon;
}TS_TaxId;




/******************************************************

Class Taxonomy

gere un	arbre de Taxon
*******************************************************/

class Taxonomy
{

private	:

	Taxon		m_root;						    
    
//	  TextField	  *m_pTaxId;
	int			m_iNbTaxId;
	TS_TaxId	*m_ptstTaxId;
	bool		m_bTaxIdOk;



private	:

	static const char *pszTaxonTag;
	static const char *pszTaxonomyTxt;
	static const char *pszFindTaxId;

public :

	Taxonomy(void);
	~Taxonomy(void);

	void Reset(void);


	void Load(void);


	void ReadCgi(Cgi *pCgi);
	void CheckCgi(void);

	void ReadFile(TagFile *pFile);
	void WriteFile(TagFile *pFile);

	void WriteContaminantParam(TagFile *pFile);


	void WriteTaxon(File &file);
//	  void WriteTaxId(File &file);

	void Hide(File &file);

	inline Item	*GetItem(int iId)
	{ return m_root.Find(iId); }


	void WriteCheckLimit(File &file);
	void WriteHtmlInputSummary(File	&file, bool	bLight);
    

	Taxon *GetOwnerTaxon(int iTaxId);

	inline Taxon *GetFirstSelectedItem()
	{ if(m_root.IsSelected()) return &m_root;
	return m_root.GetFirstSelectedItem(); }

	inline int GetNbSelectedItem()
	{ return m_root.GetNbSelectedTaxon(); }

};


#endif
