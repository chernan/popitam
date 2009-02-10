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
	Created	on		   : 12.02.2004
	Last modification  : 21.10.2004
	Comments		   : 

***********************************************************/


#define	USE_TAGFILE
#define	USE_ERROR
#define	USE_STRING
#define	USE_ERRNO
#define	USE_DYNAMICARRAY
#define	USE_UTIL
#define	USE_STDLIB
#include "use.h"



#define	TAG_LEN		64	    


const char	*TagFile::pszDefaultDoubleFormat = "%f";

/******************************************************

		Constructor

*******************************************************/
TagFile::TagFile(int iFormat) :
	m_iFormat(iFormat),
	m_iNbIndent(0),
	m_bFirstObject(true)
{
	if(m_iFormat ==	XML_FORMAT)
		m_tagPile.Alloc(10);
}



/******************************************************

		Destructor

*******************************************************/
TagFile::~TagFile(void)
{
}


/***********************************************************

	Open

***********************************************************/

void TagFile::Open(const char *pszFileName,	const char *pszMode)
{
	m_bFirstObject = true;

	File::Open(pszFileName,	pszMode);
}



/**********************************************************

		ReadWord

lit	un mot,	les	separateurs	etant espace \t	\n
**********************************************************/

char *TagFile::ReadWord(char *pszBuffer, int iBufferLen)
{
	char szFormat[8];

	//determine	le nombre de caractere max que scanf peut assigner
	sprintf(szFormat, "%%%ds", iBufferLen);
	pszBuffer[iBufferLen - 1] =	'\0';

	if(	fscanf(m_pFile,	szFormat, pszBuffer) !=	1)
		ThrowError("TagFile::ReadWord",	"File \"%s\" bad format", m_pszFileName);

	//test que le buffer etait assez grand pour	lire un	mot
	if(pszBuffer[iBufferLen	- 1])
		ThrowError("TagFile::ReadWord",	"File \"%s\" Buffer	too	small",	m_pszFileName);

	return pszBuffer;
}





/**********************************************************

		ReadString

une	string commence	et fini	par	un guillemet.
si elle	contient des guillemets	ils	sont precedes 
du caractere d'echapement \
pour ecrire	un caractere d'echapement \, il	doit lui meme
etre precede par \

La string est allouee par malloc() et doit donc	etre liberee par free()
**********************************************************/

char *TagFile::ReadString(void)
{
	DynamicArray<char>	string;
	string.Alloc(1024);

	char	c;

	try{

		//consomme les espaces (char \040 en octal)	tab	et retour a	la ligne
		do{
			ReadChar(&c);
		}while(strchr("\040\t\n\r",	c));

		//la string	doit commencer par un guillemet
		if(c !=	'"')
			ThrowError("TagFile::ReadString", "File	\"%s\" bad format",	m_pszFileName);
	    
		//lit tous les caracteres jusqu'au guillemet final
		ReadChar(&c);

		while(c	!= '"'){
		    
			//rencontre	un caract d'echappement
			if(c ==	'\\'){
	    
				ReadChar(&c);

				switch(c){

					case '\\' :
						string.Add('\\');   
						break;

					case '"' :
						string.Add('"');
						break;

					default	:
						ThrowError("TagFile::ReadString", "File	\"%s\" bad format :	caract '\\'	not	followed by	'\"' or	'\\' ",	m_pszFileName);
				}

			    
			}else
			string.Add(c);

			ReadChar(&c);	    
		}

		//null format string 
		string.Add('\0');

	}catch(Error *pError){
		pError->Stack("TagFile::ReadString", "File \"%s\" bad format", m_pszFileName);  
	}

	return string.GetWithAjust();
}



/**********************************************************

		Indent

**********************************************************/

void TagFile::Indent(void)
{
	for(int	i=0; i<m_iNbIndent;	i++)
		fprintf(m_pFile, "\t");
}




/**********************************************************

		WriteStartObject

**********************************************************/

void TagFile::WriteStartObject(const char *psz)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			if(!m_bFirstObject)
				fprintf(m_pFile, "\n");

			m_bFirstObject = false;

			Indent();
			fprintf(m_pFile, "%s ",	psz);
			m_iNbIndent++;
			break;

		case XML_FORMAT:
			//memorise les tags	et leur	empilement
			m_tagPile.Add(Strdup(psz));

			Indent();
			fprintf(m_pFile, "<%s>\n", psz);
			m_iNbIndent++;
			break;

		default	:
			ThrowError("TagFile::WriteStartObject",	"Unknown format	%d", m_iFormat);
	}

}


/**********************************************************

		WriteEndObject

**********************************************************/

void TagFile::WriteEndObject(void)
{ 
	char *psz;


	switch(m_iFormat){
		case SIMPLE_FORMAT:
			if(m_iNbIndent) 
				m_iNbIndent--; 
			break;

		case XML_FORMAT:		    
			if(m_iNbIndent) 
				m_iNbIndent--; 

			Indent();
			psz	= m_tagPile.Pop();
			fprintf(m_pFile, "</%s>\n",	psz);
			free(psz);
			break;

		default	:
			ThrowError("TagFile::WriteStartObject",	"Unknown format	%d", m_iFormat);
	}
}

/**********************************************************

		WriteTxt
	    
a simple method	to write text

**********************************************************/

void TagFile::WriteTxt(const char *psz)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			//ecrit	simplement le texte
			fprintf(m_pFile, "%s", psz);
			break;

		default	:
			ThrowError("TagFile::WriteTxt",	"Unknown format	%d (XML_FORMAT not implemented)", m_iFormat);
	}
}


/**********************************************************

		WriteSplInt
	    
a simple method	to write an	int

**********************************************************/

void TagFile::WriteSplInt(int i)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			//ecrit	simplement l'entier
			fprintf(m_pFile, "%d", i);
			break;

		default	:
			ThrowError("TagFile::WriteInt",	"Unknown format	%d (XML_FORMAT not implemented)", m_iFormat);
	}
}


/**********************************************************

		WriteDouble
	    
a simple method	to write a double

**********************************************************/

void TagFile::WriteSplDouble(double	d, const char *pszFormat)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			//ecrit	le double en suivant le	format voulu
			fprintf(m_pFile, pszFormat,	d);
			break;

		default	:
			ThrowError("TagFile::WriteInt",	"Unknown format	%d (XML_FORMAT not implemented)", m_iFormat);
	}
}


/**********************************************************

		WriteMember

une	string commence	et fini	par	un guillemet.
si elle	contient des guillemets	ils	sont precedes 
du caractere d'echapement \
pour ecrire	un caractere d'echapement \, il	doit lui meme
etre precede par \

**********************************************************/

void TagFile::WriteMember(const	char *pszTag, const	char *psz)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			//ouvre	le tag et un guillemet de debut	de string
			fprintf(m_pFile, "%s \"", pszTag);
		    
			while(*psz){

				if(	(*psz == '"') || (*psz == '\\')	)
					fprintf(m_pFile, "%c", '\\');
			    
				fprintf(m_pFile, "%c", *psz++);
			}

			//ferme	le guillemet de	fin	de string
			fprintf(m_pFile, "\" ");
			break;

		case XML_FORMAT:
			Indent();
			fprintf(m_pFile, "<%s>%s</%s>\n", pszTag, psz, pszTag);
			break;

		default	:
			ThrowError("TagFile::WriteStartObject",	"Unknown format	%d", m_iFormat);
	}
}

/**********************************************************

		WriteMember

**********************************************************/

void TagFile::WriteMember(const	char *pszTag, int i)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			fprintf(m_pFile, "%s %d	", pszTag, i);
			break;

		case XML_FORMAT:
			Indent();
			fprintf(m_pFile, "<%s>%d</%s>\n", pszTag, i, pszTag);
			break;

		default	:
			ThrowError("TagFile::WriteStartObject",	"Unknown format	%d", m_iFormat);
	}
}

/**********************************************************

		WriteMember

**********************************************************/

void TagFile::WriteMember(const	char *pszTag, double d,	const char *pszFormat)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			fprintf(m_pFile, "%s ",	pszTag);
			fprintf(m_pFile, pszFormat,	d);
			fprintf(m_pFile, " ");
			break;

		case XML_FORMAT:
			Indent();
			fprintf(m_pFile, "<%s>", pszTag);
			fprintf(m_pFile, pszFormat,	d);
			fprintf(m_pFile, "</%s>\n",	pszTag);
			break;

		default	:
			ThrowError("TagFile::WriteStartObject",	"Unknown format	%d", m_iFormat);
	}
}

/**********************************************************

		WriteMember

**********************************************************/

void TagFile::WriteMember(const	char *pszTag, bool b)
{
	switch(m_iFormat){
		case SIMPLE_FORMAT:
			fprintf(m_pFile, "%s %d	", pszTag, b? 1	: 0);
			break;

		case XML_FORMAT:
			Indent();
			fprintf(m_pFile, "<%s>%d</%s>\n", pszTag, b? 1 : 0,	pszTag);
			break;

		default	:
			ThrowError("TagFile::WriteStartObject",	"Unknown format	%d", m_iFormat);
	}
}



/**********************************************************

		ReadStartObject

**********************************************************/

void TagFile::ReadStartObject(const	char *psz)
{
	char szBuffer[TAG_LEN]; 

	ReadWord(szBuffer, TAG_LEN);
    
	if(strcmp(psz, szBuffer) )
		ThrowError("TagFile::ReadStartObject", "In file	\"%s\" : Error reading tag \"%s\", the file	contains \"%s\"", 
					m_pszFileName, psz,	szBuffer);
}


/**********************************************************

		ReadEndObject

**********************************************************/

void TagFile::ReadEndObject(void)
{
}


/**********************************************************

		ReadMember

**********************************************************/

char *TagFile::ReadMember(const	char *pszTag, char **ppsz, int iLimit, int iMin, int iMax)
{
	ReadStartObject(pszTag);
	*ppsz =	ReadString();
//	  printf("In TagFile::ReadMember %s\n",	*ppsz);

	//test le nbre de caracter de la string	si demande par iLimit
	if(iLimit && !CheckValueLimit(iLimit, strlen(*ppsz), iMin, iMax) ){

		switch(iLimit){

			case FLOOR:
			case FLOOR|EMPTY:
				ThrowError("TagFile::ReadMember", 
					"In	file \"%s\"	: %s should	be a string	with at	least %d characters", m_pszFileName, pszTag, iMin);
				break;

			case CEILING:
			case CEILING|EMPTY:
				ThrowError("TagFile::ReadMember", 
					"In	file \"%s\"	: %s should	be a string	with a maximum of %d characters", m_pszFileName, pszTag, iMax);
				break;

			case FLOOR|CEILING:
			case FLOOR|CEILING|EMPTY:
				ThrowError("TagFile::ReadMember", 
					"In	file \"%s\"	: s	should be a	string with	%d to %d characters", m_pszFileName, pszTag, iMin, iMax);
				break;

			default:
				ThrowError("TagFile::ReadMember", "No Value	for	iLimit");
		}
	}

	return *ppsz;
}


/**********************************************************

		ReadMember

**********************************************************/

int	TagFile::ReadMember(const char *pszTag,	int	*pi, int iLimit, int iMin, int iMax)
{
	ReadStartObject(pszTag);
	*pi	= ReadInt();

	//test le nbre de caracter de la string	si demande par iLimit
	if(iLimit && !CheckValueLimit(iLimit, *pi, iMin, iMax) ){

		switch(iLimit){

			case FLOOR:
				ThrowError(	"File::ReadMember", 
					"In	file \"%s\"	: %s should	be an integer >= %d", m_pszFileName, pszTag, iMin);
				break;

			case CEILING:
				ThrowError(	"File::ReadMember", 
					"In	file \"%s\"	: %s should	be an integer <= %d", m_pszFileName, pszTag, iMax);
				break;

			case FLOOR|CEILING:
				ThrowError(	"File::ReadMember", 
					"In	file \"%s\"	: %s should	be an integer >= %d	and	<= %d",	m_pszFileName, pszTag, iMin, iMax);
				break;

			default:
				ThrowError(	"File::ReadMember",	"No	Value for iLimit");
		}
	}

	return *pi;
}

/**********************************************************

		ReadMember

**********************************************************/

double TagFile::ReadMember(const char *pszTag, double *pd, int iLimit, double dMin,	double dMax)
{
	ReadStartObject(pszTag);
	*pd	= ReadDouble(); 

	//test le nbre de caracter de la string	si demande par iLimit
	if(iLimit && !CheckValueLimit(iLimit, *pd, dMin, dMax) ){

		switch(iLimit){

			case FLOOR:
				ThrowError(	"TagFile::ReadMember", 
					"In	file \"%s\"	: %s should	be a floating value	>= %d",	m_pszFileName, pszTag, dMin);
				break;

			case CEILING:
				ThrowError(	"TagFile::ReadMember", 
					"In	file \"%s\"	: %s should	be a floating value	<= %d",	m_pszFileName, pszTag, dMax);
				break;

			case FLOOR|CEILING:
				ThrowError(	"TagFile::ReadMember", 
					"In	file \"%s\"	: %s should	be a floating value	>= %d and <= %d", m_pszFileName, pszTag, dMin, dMax);
				break;

			default:
				ThrowError(	"TagFile::ReadMember", "No Value for iLimit");
		}
	}

	return *pd;
}


/**********************************************************

		ReadMember

**********************************************************/

bool TagFile::ReadMember(const char	*pszTag, bool *b)
{
	int	i;

	ReadMember(pszTag, &i, FLOOR|CEILING, 0, 1);

	*b = (i)? true : false;

	return *b;
}


/***********************************************************

	CheckValueLimit

***********************************************************/

bool TagFile::CheckValueLimit(int iLimit, double d,	double dMin, double	dMax)
{
	if(	(iLimit	& FLOOR) &&	(d < dMin) )
			return false;

	if(	(iLimit	& CEILING) && (d > dMax) )
		return false;

	return true;
}


/***********************************************************

	Convert

***********************************************************/

void TagFile::Convert(TagFile *pFile)
{
	char	szBuffer[128];
	int		iLastIdent		= 0;
	int		iCurrentIdent	= m_iNbIndent;
	int		i;
	double	d;
	char	*psz;
	int		iStartPile,	iEndPile;

	iStartPile = m_tagPile.GetNbElt();

	bool bFirstLoop	= true;

	while( fread(szBuffer, sizeof(char), 1,	*pFile)	== 1){

		switch(szBuffer[0]){
			case ' ':
			case '\r':
				break;

			case '\t':
				iCurrentIdent++;
				break;

			case '\n':
				iCurrentIdent =	iStartPile;
				break;

			case 'i':
				// int member
				fscanf(*pFile, "%s", szBuffer+1);
				fscanf(*pFile, "%d", &i);
				WriteMember(szBuffer, i);
				break;

			case 'f':
				//float	member
				fscanf(*pFile, "%s", szBuffer+1);
				fscanf(*pFile, "%lf", &d);

				if(	(d != 0) &&	(d > 1e-6) )
					WriteMember(szBuffer, d);
				else
					WriteMember(szBuffer, d, "%.2e");
				break;

			case 's':
				// int member
				fscanf(*pFile, "%s", szBuffer+1);
				psz	= pFile->ReadString();
				WriteMember(szBuffer, psz);
				break;

			case 'b':
				// int member
				fscanf(*pFile, "%s", szBuffer+1);
				fscanf(*pFile, "%d", &i);
				WriteMember(szBuffer, (bool)(i?	true : false));
				break;

			default	:

				//end object
				if(!bFirstLoop){
					for(i=iCurrentIdent; i<=iLastIdent;	i++)
						WriteEndObject();
				}

				iLastIdent = iCurrentIdent;

				//start	object
				if(szBuffer[0]<'A' || szBuffer[0]>'Z')
					goto LABEL_ERROR;

				fscanf(*pFile, "%s", szBuffer+1);
				WriteStartObject(szBuffer);
			    
		}

		bFirstLoop = false;
	}


	iEndPile = m_tagPile.GetNbElt();

	for(i=iStartPile; i<iEndPile; i++)
		WriteEndObject();

	return;

	LABEL_ERROR:
	ThrowError("TagFile::ConvertToXml",	"File %s is	bad	formated", pFile->GetFileName());
}




