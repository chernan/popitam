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
	Last modification  : 09.06.2004
	Comments           : 

***********************************************************/

#ifndef __CGI_H__
#define __CGI_H__


/******************************************************

Class CgiData


*******************************************************/

class CgiData
{
private :

	DynamicArray<char *>	m_array;

public :

	CgiData(char *pszData);
	CgiData(const char *pszData, int iLen);
	virtual ~CgiData(void);

	inline void Add(char *psz)
	{ m_array.Add(psz); }

	inline int GetNbData(void)
	{ return m_array.GetNbElt(); }

	inline char *GetData(int i=0)
	{ return m_array.Get(i); }

	virtual const char *GetFileName(void);

private :

	static inline void Free(char *psz)
	{ free(psz); };
};



/******************************************************

Class CgiFileData


*******************************************************/

class CgiFileData : public CgiData
{

private :

	char *m_pszFileName;

public :

	CgiFileData(char *pszFileName, const char *pszData, int iLen);
	~CgiFileData(void);

	inline const char *GetFileName(void)
	{ return m_pszFileName; }

};


/******************************************************

Class Cgi


*******************************************************/

class Cgi
{
	int		m_iNbCharToRead;
	int		m_iContentType;

	Map		m_map;

public :

	Cgi(void);
	~Cgi(void);


	int Load(void);

	bool isFieldEmpty(const char *pszName);

	const char *GetFileNamePtr(const char *pszName);
	char *GetFileName(const char *pszName);

	char *GetString(const char *pszName, int iIndex=0);
	const char *GetStringPtr(const char *pszName, int iIndex=0);

	int GetInt(const char *pszName, int iIndex=0);
	double GetDouble(const char *pszName, int iIndex=0);
	
	bool GetCheckBox(const char *pszName);

	int GetNbElt(const char *pszName);


	static bool IsDefaultCall(void);


private : 

	//multipart fct
	char *GetBoundary(void);
	void DecodeMultipart(void);

	//ulrencoded fct
	char *GetUrlEncodedCouple(void);
	void DecodeUrlEncodedCouple(char *psz);
	void PlusToSpace(char *psz);
	char HexToChar(char **ppsz);
	void RemoveEscapeToChar(char *psz);

	
	CgiData *Get(const char *pszName);	

	const char *GetContentType(void);
	
	static void FreeElt(const char *psz, void *pData);
};







#endif
