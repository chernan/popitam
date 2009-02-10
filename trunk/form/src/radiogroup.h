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
	Author			   : Celine	Hernandez
	Contact			   : Celine.Hernandez@isb-sib.ch
	Created	on		   : 06.04.2005
	Last modification  : 06.04.2005
	Comments		   : 

***********************************************************/

#ifndef	__RADIOGROUP_H__
#define	__RADIOGROUP_H__


/******************************************************

Class RadioGroup

*******************************************************/

class RadioGroup
{

protected :

	char	*m_pszName;	    

	Radio	 *m_ptItem;
	int		m_iNbItem;		    


	int		m_iMin;
	int		m_iMax;

	int		m_iCurrent;

public :

	RadioGroup(const char *pszName);
	virtual	~RadioGroup(void);

	void Load(int iNbItem, int iMin, int iMax);
    
	void Add(const char	*psz);

	bool ReadCgi(Cgi *pCgi);

	void ReadFile(TagFile *pFilee);
	void WriteFile(TagFile *pFile);

	void WriteLink(File	&file, int iSize, const	char *psz, const char *pszLink);
	void Write(File	&file, int iSize);
	void Hide(File &file);

	void Reset(void);

	int	GetNbSelectedItem(void);

	Radio *GetSelectedItem(void);

	void Select(int	iId);

	inline Radio *GetItem(int i)
	{ return &m_ptItem[i]; }

	inline int	GetNbItem()
	{ return m_iNbItem;	}
    
	inline char	*GetName()
	{ return m_pszName;	}
    
protected :

	bool Select(int	iId, const char	*psz);
};


#endif
