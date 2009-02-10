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

#define	USE_FIELD
#define	USE_UTIL
#define	USE_PATH
#define	USE_MSG
#define	USE_HTML
#include "use.h"





/******************************************************

		Constructor

*******************************************************/
Field::Field(const char	*pszName) :
	m_eLimit(NO_LIMIT)
{
	m_pszName	= Strdup(pszName);
	m_pszField	= Strdup("");
}



/******************************************************

		Destructor

*******************************************************/
Field::~Field(void)
{
	if(m_pszField){
		free(m_pszField);
		m_pszField = NULL;
	}
    
	if(m_pszName){
		free(m_pszName);
		m_pszName =	NULL;
	}   
}


/***********************************************************

	IsWhite

***********************************************************/

bool Field::IsWhite(void)
{
	const char *psz	= m_pszField;

	while(*psz && strchr(" \t",	*psz))
		psz++;

	if(*psz)
		return false;
    
	return true;
}


/***********************************************************

	Set

***********************************************************/

void Field::Set(const char *psz)
{
	if(m_pszField){
		free(m_pszField);
		m_pszField = NULL;
	}

	m_pszField = Strdup(psz);
}



/***********************************************************

	ReadCgi

***********************************************************/

void Field::ReadCgi(Cgi	*pCgi, int iIndex)
{
	if(m_pszField)
		free(m_pszField);

	m_pszField = pCgi->GetString(m_pszName,	iIndex);
}


/***********************************************************

	Write

***********************************************************/

void Field::Write(File &file, int iSize)
{
	fprintf(file, "<input name=\"%s\" size=%d value=\"%s\">\n",	m_pszName, iSize, m_pszField);  
}


/***********************************************************

	WriteLink

***********************************************************/

void Field::WriteLink(File &file, int iSize, const char	*psz, const	char *pszLink)
{   
	fprintf(file, "<td nowrap>");
	HRef(file, psz,	pszLink);
	fprintf(file, "</td><td>");
	Write(file,	iSize);
	fprintf(file, "</td>\n");
}

/***********************************************************

	Hide

***********************************************************/

void Field::Hide(File &file)
{
	fprintf(file, "<input type=hidden name=\"%s\" value=\'%s\'>\n",	m_pszName, m_pszField);
}





/******************************************************

		Constructor

*******************************************************/
AreaField::AreaField(const char	*pszName) :
	TextField(pszName)
{
}

/***********************************************************

	Write

***********************************************************/

void AreaField::Write(File &file, int iRow,	int	iCol)
{
	fprintf(file, "<textarea name=\"%s\" rows=%d cols=%d>%s</textarea>", m_pszName,	iRow, iCol,	m_pszField);
}

/***********************************************************

	Hide

***********************************************************/

void AreaField::HideTxt(File &file,	const char*	strText)
{
	fprintf(file, "<input type=hidden name=\"%s\" value=\'%s\'>\n",	m_pszName, strText);
}

/***********************************************************

	WriteLink

***********************************************************/

void AreaField::WriteLink(File &file, int iRow,	int	iCol, const	char *psz, const char *pszLink)
{   
	fprintf(file, "<td nowrap>");
	HRef(file, psz,	pszLink);
	fprintf(file, "</td><td>");
	Write(file,	iRow, iCol);
	fprintf(file, "</td>\n");
}



/******************************************************

		Constructor

*******************************************************/
FileField::FileField(const char	*pszName) :
	TextField(pszName)
{
	m_pszFileName	= Strdup("");
}



/******************************************************

		Destructor

*******************************************************/
FileField::~FileField(void) 
{
	if(m_pszFileName){
		free(m_pszFileName);
		m_pszFileName =	NULL;
	}
}


/***********************************************************

   ReadCgi

***********************************************************/
void FileField::ReadCgi(Cgi	*pCgi, int iIndex)
{
	Field::ReadCgi(pCgi, iIndex);

	if(!IsWhite())
		m_pszFileName =	pCgi->GetFileName(m_pszName);

}


/***********************************************************

	Write

***********************************************************/

void FileField::WriteLink(File &file, int iSize, const char	*psz, const	char *pszLink)
{
	fprintf(file, "<td nowrap>\n");
	HRef(file, psz,	pszLink);
	fprintf(file, "</td>\n<td>\n");
	Write(file,	iSize);
	fprintf(file, "</td>\n");
}


/***********************************************************

	ReadFile

***********************************************************/

void FileField::ReadFile(TagFile *pFile)
{
	TextField::ReadFile(pFile);

	if(m_pszFileName)
		free(m_pszFileName);

	pFile->ReadMember(m_pszName, &m_pszFileName);
}

/***********************************************************

	WriteFile

***********************************************************/

void FileField::WriteFile(TagFile *pFile)
{
	TextField::WriteFile(pFile);

	pFile->WriteMember(m_pszName, m_pszFileName);
}

/***********************************************************

	Write

***********************************************************/

void FileField::Write(File &file, int iSize)
{
	fprintf(file, "<input type=file	name=\"%s\"	size=%d>", m_pszName, iSize);
}


/***********************************************************

	Hide

***********************************************************/

void FileField::Hide(File &file)
{
	fprintf(file, "<input type=hidden name=\"%s\" value=\"\">\n", m_pszName);
}


/******************************************************

		Constructor

*******************************************************/
TextField::TextField(const char	*pszName) :
	Field(pszName),
	m_iMinLen(0),
	m_iMaxLen(0)
{
}


/***********************************************************

	Load

***********************************************************/

void TextField::Load(int iLimit, int iMin, int iMax)
{
	m_eLimit	= (TE_ValueLimit)iLimit;
	m_iMinLen	= iMin;
	m_iMaxLen	= iMax;
}

/***********************************************************

	ReadFile

***********************************************************/

void TextField::ReadFile(TagFile *pFile)
{
	if(m_pszField)
		free(m_pszField);

	pFile->ReadMember(m_pszName, &m_pszField);
}

/***********************************************************

	WriteFile

***********************************************************/

void TextField::WriteFile(TagFile *pFile)
{
	pFile->WriteMember(m_pszName, m_pszField);
}


/***********************************************************

	WriteCheckLimit

***********************************************************/

void TextField::WriteCheckLimit(File &file,	const char *pszError, int iIndex)
{
	if(iIndex == -1)
		fprintf(file, "\tif(!CheckString(document.myform.%s, %d, %d, %d, \"%s must be a	text field", 
				m_pszName, m_eLimit, m_iMinLen,	m_iMaxLen, pszError);
	else
		fprintf(file, "\tif(!CheckString(document.myform.%s[%d], %d, %d, %d, \"%s must be a	text field", 
				m_pszName, iIndex, m_eLimit, m_iMinLen,	m_iMaxLen, pszError);


	if(m_eLimit	& FLOOR)
		fprintf(file, ", >=	%d characters",	m_iMinLen);
    
	if(m_eLimit	& CEILING)
		fprintf(file, ", <=	%d characters",	m_iMaxLen);

	if(m_eLimit	& EMPTY)
		fprintf(file, "	or empty");
	    
	fprintf(file, "\")){ return	}\n");	    
}





/******************************************************

		Constructor

*******************************************************/
IntField::IntField(const char *pszName)	:
	Field(pszName),
	m_iMin(0),
	m_iMax(0)
{
}



/***********************************************************

	Get

***********************************************************/

int	IntField::Get(void)
{
	int	i=0;
	sscanf(m_pszField, "%d", &i);
	return i;
}

/***********************************************************

	SetInt

***********************************************************/

void IntField::SetInt(int i)
{
	char szBuffer[64];

	sprintf(szBuffer, "%d",	i);
	Field::Set(szBuffer);
}


/***********************************************************

	Load

***********************************************************/

void IntField::Load(int	iLimit,	int	iMin, int iMax)
{
	m_eLimit	= (TE_ValueLimit)iLimit;
	m_iMin		= iMin;
	m_iMax		= iMax;
}

/***********************************************************

	ReadFile

***********************************************************/

void IntField::ReadFile(TagFile	*pFile)
{
	int	i;
	pFile->ReadMember(m_pszName, &i);
	SetInt(i);
}

/***********************************************************

	WriteFile

***********************************************************/

void IntField::WriteFile(TagFile *pFile)
{
	pFile->WriteMember(m_pszName, (int)Get());
}


/***********************************************************

	WriteCheckLimit

***********************************************************/

void IntField::WriteCheckLimit(File	&file, const char *pszError, int iIndex)
{
	if(iIndex == -1)
		fprintf(file, "\tif(!CheckInt(document.myform.%s, %d, %d, %d, \"%s must	be an integer", 
				m_pszName, m_eLimit, m_iMin, m_iMax, pszError);
	else
		fprintf(file, "\tif(!CheckInt(document.myform.%s[%d], %d, %d, %d, \"%s must	be an integer", 
				m_pszName, iIndex, m_eLimit, m_iMin, m_iMax, pszError);

    
	if(m_eLimit	& FLOOR)
		fprintf(file, ", >=	%d", m_iMin);
    
	if(m_eLimit	& CEILING)
		fprintf(file, ", <=	%d", m_iMax);
    
	if(m_eLimit	& EMPTY)
		fprintf(file, "	or empty");
    
	fprintf(file, "\")){ return	}\n");	    
}







/******************************************************

		Constructor

*******************************************************/
DoubleField::DoubleField(const char	*pszName) :
	Field(pszName),
	m_dMin(0),
	m_dMax(0)
{
}

/***********************************************************

	Get

***********************************************************/

double DoubleField::Get(void)
{
	double d;
	sscanf(m_pszField, "%lf", &d);
	return d;
}


/***********************************************************

	SetDouble

***********************************************************/

void DoubleField::SetDouble(double d, const	char *pszFormat)
{
	char szBuffer1[64];
    
	if(pszFormat){

		sprintf(szBuffer1, pszFormat, d);
		Field::Set(szBuffer1);

	}else{

		//choix	1 :	affichage classique	sans les zeros 0.00021
		StrDouble(szBuffer1, d);

		//choix	2 :	affichage exposant : 
		char szBuffer2[64];
		sprintf(szBuffer2, "%.2e", d);
	    
		char *psz =	strtok(szBuffer2, "e");
		double dValue;
		sscanf(psz,	"%lf", &dValue);

		psz	= strtok(NULL, "e");
		int	iExp;
		sscanf(psz,	"%d", &iExp);

		StrDouble(szBuffer2, dValue);

		psz	= &szBuffer2[strlen(szBuffer2)];
		*psz++ = 'e';

		sprintf(psz, "%d", iExp);


//		if(	(d != 0) &&	!strcmp(szBuffer1, "0")	){
//			Field::Set(szBuffer2);
//		}else{

			if(strlen(szBuffer1) <= strlen(szBuffer2))
				Field::Set(szBuffer1);
			else
				Field::Set(szBuffer2);

//		}

	}
}


/***********************************************************

	Load

***********************************************************/

void DoubleField::Load(int iLimit, double dMin,	double dMax)
{
	m_eLimit	= (TE_ValueLimit)iLimit;
	m_dMin		= dMin;
	m_dMax		= dMax;
}


/***********************************************************

	ReadFile

***********************************************************/

void DoubleField::ReadFile(TagFile *pFile)
{
	double d;
	pFile->ReadMember(m_pszName, &d);
	SetDouble(d);
}

/***********************************************************

	ReadFile

***********************************************************/

void DoubleField::WriteFile(TagFile	*pFile)
{
	pFile->WriteMember(m_pszName, (double)Get());
}

/***********************************************************

	WriteCheckLimit

***********************************************************/

void DoubleField::WriteCheckLimit(File &file, const	char *pszError,	int	iIndex)
{   
	char szMin[64];
	char szMax[64];
    
	StrDouble(szMin, m_dMin);
	StrDouble(szMax, m_dMax);


	if(iIndex == -1)
		fprintf(file, "\tif(!CheckDouble(document.myform.%s, %d, %s, %s, \"%s must be a	decimal	value", 
				m_pszName, m_eLimit, szMin,	szMax, pszError);
	else
		fprintf(file, "\tif(!CheckDouble(document.myform.%s[%d], %d, %s, %s, \"%s must be a	decimal	value", 
				m_pszName, iIndex, m_eLimit, szMin,	szMax, pszError);

    
	if(m_eLimit	& FLOOR)    
		fprintf(file, ", >=	%s", szMin);
    
	if(m_eLimit	& CEILING)
		fprintf(file, ", <=	%s", szMax);

	if(m_eLimit	& EMPTY)
		fprintf(file, "	or empty");
	    
	fprintf(file, "\")){ return	}\n");
}



