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

#ifndef	__FIELD_H__
#define	__FIELD_H__




/******************************************************

Class Field


*******************************************************/

class Field
{

protected :

	char			*m_pszName;	    
	char			*m_pszField;    

	TE_ValueLimit	m_eLimit;
    

public :

	Field(const	char *pszName);
	virtual	~Field(void);

	virtual	void ReadCgi(Cgi *pCgi,	int	iIndex=0);

	void Set(const char	*psz);

	virtual	void Write(File	&file, int iSize);  
	virtual	void WriteLink(File	&file, int iSize, const	char *psz, const char *pszLink);
	virtual	void Hide(File &file);

	bool IsWhite(void);

	virtual	void WriteCheckLimit(File &file, const char	*pszError, int index=-1) = 0;

	inline const char *GetString(void) const
	{ return m_pszField; }

	inline const char *GetName(void) const
	{ return m_pszName;	}


};



/******************************************************

Class TextField


*******************************************************/

class TextField	: public Field
{

private	:

	int		m_iMinLen;
	int		m_iMaxLen;
    

public :

	TextField(const	char *pszName);

	void Load(int iLimit, int iMin,	int	iMax);

	virtual	void ReadFile(TagFile *pFile);
	virtual	void WriteFile(TagFile *pFile);


	void WriteCheckLimit(File &file, const char	*pszError, int index=-1);
};


/******************************************************

Class AreaField


*******************************************************/

class AreaField	: public TextField
{

public :

	AreaField(const	char *pszName);

	void Write(File	&file, int iRow, int iCol);
	void WriteLink(File	&file, int iRow, int iCol, const char *psz,	const char *pszLink);
	void HideTxt(File &file, const char* strText);
};


/******************************************************

Class FileField


*******************************************************/

class FileField	: public TextField
{
private	:

	char *m_pszFileName;

public :

	FileField(const	char *pszName);
	~FileField(void);

	void ReadCgi(Cgi *pCgi,	int	iIndex=0);

	void WriteLink(File	&file, int iSize, const	char *psz, const char *pszLink);

	void Write(File	&file, int iSize);

	void Hide(File &file);

	void ReadFile(TagFile *pFile);
	void WriteFile(TagFile *pFile);

	const char *GetFileName(void)
	{ return m_pszFileName;	}
};


/******************************************************

Class IntField


*******************************************************/

class IntField : public	Field
{

private	:

	int		m_iMin;
	int		m_iMax;
    

public :

	IntField(const char	*pszName);
    
	void Load(int iLimit, int iMin,	int	iMax);

	void WriteCheckLimit(File &file, const char	*pszError, int index=-1);

	void ReadFile(TagFile *pFile);
	void WriteFile(TagFile *pFile);

	int	Get(void);

	void SetInt(int	i);

};




/******************************************************

Class DoubleField


*******************************************************/

class DoubleField :	public Field
{

private	:

	double		m_dMin;
	double		m_dMax;
    

public :

	DoubleField(const char *pszName);

	void Load(int iLimit, double dMin, double dMax);

	void WriteCheckLimit(File &file, const char	*pszError, int index=-1);

	void ReadFile(TagFile *pFile);
	void WriteFile(TagFile *pFile);


	double Get(void);

	void SetDouble(double d, const char	*pszFormat=NULL);

};




#endif
