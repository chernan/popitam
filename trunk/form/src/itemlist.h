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
	Last modification  : 24.02.2005
	Comments		   : 

***********************************************************/

#ifndef	__ITEMLIST_H__
#define	__ITEMLIST_H__


/******************************************************

Class ItemList

*******************************************************/

class ItemList
{

protected :

	char	*m_pszName;	    

	Item	*m_ptItem;	    
	int		m_iNbItem;		    


	int		m_iMin;
	int		m_iMax;

	int		m_iCurrent;

public :

	ItemList(const char	*pszName);
	virtual	~ItemList(void);

	void Load(int iNbItem, int iMin, int iMax);
    
	void Add(const char	*psz);
	void Add(const int iId,	const char *psz);

	bool ReadCgi(Cgi *pCgi);

	void ReadFile(TagFile *pFilee);
	void WriteFile(TagFile *pFile);

	void WriteLink(File	&file, int iSize, const	char *psz, const char *pszLink);
	void Write(File	&file, int iSize);
	void Hide(File &file);

	void Reset(void);

	int	GetNbSelectedItem(void);

	Item *GetFirstSelectedItem(void);
	Item *GetNextSelectedItem(void);

	inline void	Select(int iId,	bool bSelected)
	{ m_ptItem[iId].Select(bSelected); }


	inline Item	*GetItem(int i)
	{ return &m_ptItem[i]; }


protected :

	bool Select(int	iId, const char	*psz);
};


#endif
