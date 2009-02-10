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
	Last modification  : 14.10.2004
	Comments		   : 

***********************************************************/

#ifndef	__DISPLAY_H__
#define	__DISPLAY_H__



/******************************************************

Class Display


*******************************************************/

class Display
{

private	:

	TextField	m_mail;

public :

	Display(void);
	~Display(void);

	void Load(void);

	void ReadCgi(Cgi *pCgi);
	void CheckCgi(void);

	void ReadFile(TagFile *pFile);
	void WriteFile(TagFile *pFile);

	void WriteForm(File	&file);
	void WriteLink(File	&file, const char* link);
	void WriteOutput(File &file);
	void Hide(File &file);

	void WriteCheckLimit(File &file);


	inline bool	IsMailUsed(void)
	{ return !m_mail.IsWhite();	}

	inline const char *GetMail(void)
	{ return m_mail.GetString(); }


};


#endif
