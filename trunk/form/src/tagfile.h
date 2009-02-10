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
	Created	on		   : 27.11.2003
	Last modification  : 08.07.2004
	Comments		   : 

***********************************************************/

#ifndef	__TAGFILE_H__
#define	__TAGFILE_H__



enum{
	SIMPLE_FORMAT =	0,
	XML_FORMAT
};

/******************************************************

Class TagFile


*******************************************************/

class TagFile :	public TxtFile
{

private	:

	int							m_iFormat;
	int							m_iNbIndent;
	bool						m_bFirstObject;

	DynamicArray<char *>		m_tagPile;  


	static const char			*pszDefaultDoubleFormat;

public :

	TagFile(int	iFormat=SIMPLE_FORMAT);
	~TagFile(void);


	void Open(const	char *pszFileName, const char *pszMode);
	inline void	Open(FILE *pFile, char *pszFileName	= "No File Name")
	{ File::Open(pFile,	pszFileName); }

	    
	void WriteStartObject(const	char *psz);
	void WriteEndObject(void);
	void WriteTxt(const	char *psz);
	void WriteSplInt(int i);
	void WriteSplDouble(double d, const	char *pszFormat=TagFile::pszDefaultDoubleFormat);
	void WriteMember(const char	*pszTag, const char	*psz);
	void WriteMember(const char	*pszTag, int i);
	void WriteMember(const char	*pszTag, double	d, const char *pszFormat=TagFile::pszDefaultDoubleFormat);
	void WriteMember(const char	*pszTag, bool b);


	void ReadStartObject(const char	*pszTag);
	void ReadEndObject(void);
	char *ReadMember(const char	*pszTag, char **ppsz, int iLimit=NO_LIMIT, int iMin=0, int iMax=0);
	int	ReadMember(const char *pszTag, int *pi,	int	iLimit=NO_LIMIT, int iMin=0, int iMax=0);
	double ReadMember(const	char *pszTag, double *pd, int iLimit=NO_LIMIT, double dMin=0, double dMax=0);
	bool ReadMember(const char *pszTag,	bool *b);

	void Convert(TagFile *pFile);

	static inline void SetDefaultDoubleFormat(const	char *pszFormat)
	{ TagFile::pszDefaultDoubleFormat =	pszFormat; }


private	:

	char *ReadString(void);

	char *ReadWord(char	*pszBuffer,	int	iBufferLen);

	void Indent(void);

	bool CheckValueLimit(int iLimit, double	d, double dMin,	double dMax);   

};



#endif
