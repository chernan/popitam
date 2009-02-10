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
	Created on         : 27.11.2003
	Last modification  : 27.09.2004
	Comments           : 

***********************************************************/

#ifndef __FILE_H__
#define __FILE_H__



/******************************************************

Class File


*******************************************************/

class File
{

protected :

	FILE	*m_pFile;
	char	*m_pszFileName;


public :

	File(void);
	~File(void);

	File(const char *pszFileName, const char *pszMode);
	File(FILE *pFile);


	void Open(const char *pszFileName, const char *pszMode);
	void Open(FILE *pFile, char *pszFileName = "No File Name");
	void Close(void);

	void Cpy(FILE *pFile);
	void Cpy(char *pszBuffer, int iBufferLen);
	
	bool OpenIfExist(const char *pszFileName, const char *pszMode);

	inline char *GetFileName(void) const
	{ return m_pszFileName; };

		

	inline operator FILE *()
	{ return m_pFile; }
};



#endif
