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

#ifndef	__RADIO_H__
#define	__RADIO_H__



/******************************************************

Class Radio


*******************************************************/

class Radio
{

protected :

	int		m_iId;			    
	char	*m_psz; 
	bool	m_bSelected;
    



public :

	Radio(void);
	virtual	~Radio(void);

	void Set(int iId, const	char *psz);

	inline int GetId(void) const
	{ return m_iId;	}

	inline const char *GetString(void) const
	{ return m_psz;	}

	inline void	Select(bool	bSelected)
	{ m_bSelected =	bSelected; }

	void Write(File	&file, const char *pszName);

	void Hide(File &file, const	char *pszName);

	inline bool	IsSelected(void)
	{ return m_bSelected; }
};


#endif
