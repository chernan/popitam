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
	Created	on		   : 04.06.2004
	Last modification  : 24.02.2005
	Comments		   : 

***********************************************************/

#ifndef	__CHECK_H__
#define	__CHECK_H__




/******************************************************

Class Check


*******************************************************/

class Check
{

private	:

	char	*m_pszName;	    

	bool	m_bChecked;

    

public :

	Check(const	char *pszName);
	~Check(void);

	void ReadCgi(Cgi *pCgi,	int	iIndex=0);

	void Write(File	&file, int iIndex=0);
	void WriteLink(File	&file, const char *psz,	const char *pszLink, int iIndex=0);
	void Hide(File &file, int iIndex=0);

	void ReadFile(TagFile *pFile, const	char *pszTag);
	void WriteFile(TagFile *pFile, const char *pszTag);


	inline operator	bool ()
	{ return m_bChecked; }

	inline void	Set(bool bChecked)
	{ m_bChecked = bChecked; }

	inline const char *GetName(void) const
	{ return m_pszName;	}

};


#endif
