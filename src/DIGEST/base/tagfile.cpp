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
	Created on         : 12.02.2004
    Last modification  : 21.10.2004
	Comments           : 

***********************************************************/


#include "tagfile.h"
#include "MATerror.h"
#include "dynamicarray.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>


const char	*TagFile::pszDefaultDoubleFormat = "%f";

/******************************************************

		Constructor

*******************************************************/
TagFile::TagFile(void) :
	m_bFirstObject(true),
	m_iNbIndent(0)
{
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

void TagFile::Open(const char *pszFileName, const char *pszMode)
{
	m_bFirstObject	= true;
	m_iNbIndent			= 0;

	File::Open(pszFileName, pszMode);
}



/***********************************************************

   Open

***********************************************************/
void TagFile::Open(FILE *pFile)
{
	m_bFirstObject	= true;
	m_iNbIndent			= 0;

	File::Open(pFile);
}


/**********************************************************

		ReadString

une string commence et fini par un guillemet.
si elle contient des guillemets ils sont precedes 
du caractere d'echapement \
pour ecrire un caractere d'echapement \, il doit lui meme
etre precede par \

La string est allouee avec new, elle doit etre delete []
par l'utilisateur de cette fct
**********************************************************/

char *TagFile::ReadString(void)
{
	DynamicArray<char>	string;
	string.EnsureSize(64);

	char	c;

	try{

		//consomme les espaces tab et retour a la ligne
		do{
			ReadChar(&c);
		}while(strchr(" \t\n\r", c));

		//la string doit commencer par un guillemet
		if(c != '"')
			ThrowError("TagFile::ReadString", "Bad format : string should start by \"");
		
		//lit tous les caracteres jusqu'au guillemet final
		ReadChar(&c);

		while(c != '"'){
			
			//rencontre un caract d'echappement
			if(c == '\\'){
		
				ReadChar(&c);

				switch(c){

					case '\\' :
						string.Add('\\');	
						break;

					case '"' :
						string.Add('"');
						break;

					default :
						ThrowError("TagFile::ReadString", "Bad format : charactere '\\' not followed by '\"' or '\\' ");
				}

				
			}else
				string.Add(c);

			ReadChar(&c);		
		}

		//null format string 
		string.Add('\0');

	}catch(Error *pError){
		pError->Stack("TagFile::ReadString", "Enable to read string", m_pszFileName);	
	}

	return string.GetAjustCpy();
}




/**********************************************************

		WriteString

une string commence et fini par un guillemet.
si elle contient des guillemets ils sont precedes 
du caractere d'echapement \
pour ecrire un caractere d'echapement \, il doit lui meme
etre precede par \
**********************************************************/
void TagFile::WriteString(const char *psz)
{
	//guillemet de debut de string
	fprintf(m_pFile, "\"");
	
	if(!psz){
		fprintf(m_pFile, "(NULL)");
	}else{

		while(*psz){

			if( (*psz == '"') || (*psz == '\\') )
				fprintf(m_pFile, "%c", '\\');
			
			fprintf(m_pFile, "%c", *psz++);
		}
	}

	//ferme le guillemet de fin de string
	fprintf(m_pFile, "\" ");	
}



/**********************************************************

		Indent

**********************************************************/

void TagFile::Indent(void)
{
	for(int i=0; i<m_iNbIndent; i++)
		fprintf(m_pFile, "\t");
}




/**********************************************************

		WriteStartObject

**********************************************************/

void TagFile::WriteStartObject(const char *psz)
{
	if(!m_bFirstObject)
		fprintf(m_pFile, "\n");

	m_bFirstObject = false;

	Indent();
	fprintf(m_pFile, "_%s ", psz);
	m_iNbIndent++;
}


/**********************************************************

		WriteEndObject

**********************************************************/

void TagFile::WriteEndObject(void)
{ 
	if(m_iNbIndent) 
		m_iNbIndent--; 
}


/**********************************************************

		WriteMember

une string commence et fini par un guillemet.
si elle contient des guillemets ils sont precedes 
du caractere d'echapement \
pour ecrire un caractere d'echapement \, il doit lui meme
etre precede par \

**********************************************************/

void TagFile::WriteMember(const char *pszTag, const char *psz)
{
	fprintf(m_pFile, "s%s ", pszTag);	
	WriteString(psz);
}

/**********************************************************

		WriteMember

**********************************************************/

void TagFile::WriteMember(const char *pszTag, int i)
{
	fprintf(m_pFile, "i%s %d ", pszTag, i);		
}


/**********************************************************

		WriteMember

**********************************************************/

void TagFile::WriteMember(const char *pszTag, unsigned int ui)
{
	fprintf(m_pFile, "u%s %u ", pszTag, ui);		
}

/**********************************************************

		WriteMember

**********************************************************/

void TagFile::WriteMember(const char *pszTag, double d, const char *pszFormat)
{
	fprintf(m_pFile, "f%s ", pszTag);
	fprintf(m_pFile, pszFormat, d);
	fprintf(m_pFile, " ");		
}

/**********************************************************

		WriteMember

**********************************************************/

void TagFile::WriteMember(const char *pszTag, bool b)
{
	fprintf(m_pFile, "b%s %d ", pszTag, b? 1 : 0);
}



/***********************************************************

   WriteArrayStart

***********************************************************/
void TagFile::WriteArrayStart(void)
{
	fprintf(m_pFile, "[ ");
}


/***********************************************************

   WriteArrayEnd

***********************************************************/
void TagFile::WriteArrayEnd(void)
{
	fprintf(m_pFile, " ]");
}







/***********************************************************

   ReadTag

***********************************************************/
char TagFile::ReadTag(const char *psz)
{
	try{
		
		m_szTag[TAG_LEN - 1] = '\0';

		if( fscanf(m_pFile, "%64s", m_szTag) != 1){		//remplacer 64 par la nouvelle valeur de TAG_LEN !!!!!
			if(feof(m_pFile))
				ThrowError("TagFile::ReadTag", "Reach prematured end of file");
			else
				ThrowError("TagFile::ReadTag", "Enable to read word");
		}

		//test que le buffer etait assez grand pour lire un mot
		if(m_szTag[TAG_LEN - 1])
			ThrowError("TagFile::ReadTag", "Buffer too small : tag should be %d characters max", TAG_LEN);

	}catch(Error *pError){
		pError->Stack("TagFile::ReadTag", "In file \"%s\" : Unable to read tag \"%s\"", m_pszFileName, psz);
	}
	
	if(strcmp(psz, m_szTag+1) )
		ThrowError("TagFile::ReadTag", "In file \"%s\" : Error reading tag \"%s\", the file contains \"%s\"", 
					m_pszFileName, psz, m_szTag);

	return m_szTag[0];
}

/**********************************************************

		ReadStartObject

**********************************************************/

void TagFile::ReadStartObject(const char *pszTag)
{
	if(ReadTag(pszTag) != '_')
		ThrowError("TagFile::ReadStartObject", "In file \"%s\" : %s is a start object tag, it should be preceded by '_'", m_pszFileName, pszTag);
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

char *TagFile::ReadMember(const char *pszTag, char **ppsz, int iLimit, int iMin, int iMax)
{
	if(ReadTag(pszTag) != 's')
		ThrowError("TagFile::ReadMember", "In file \"%s\" : %s should store a string an should be preceded by 's'", m_pszFileName, pszTag);

	try{
		*ppsz = ReadString();
	}catch(Error *pError){
		pError->Stack("TagFile::ReadMember", "In file \"%s\" : Error Reading tag %s", m_pszFileName, pszTag);
	}

	//test le nbre de caracter de la string si demande par iLimit
	if(iLimit && !CheckValueLimit(iLimit, strlen(*ppsz), iMin, iMax) ){

		switch(iLimit){

			case FLOOR:
			case FLOOR|EMPTY:
				ThrowError("TagFile::ReadMember", 
					"In file \"%s\" : %s should be a string with at least %d characters", m_pszFileName, pszTag, iMin);
				break;

			case CEILING:
			case CEILING|EMPTY:
				ThrowError("TagFile::ReadMember", 
					"In file \"%s\" : %s should be a string with a maximum of %d characters", m_pszFileName, pszTag, iMax);
				break;

			case FLOOR|CEILING:
			case FLOOR|CEILING|EMPTY:
				ThrowError("TagFile::ReadMember", 
					"In file \"%s\" : s should be a string with %d to %d characters", m_pszFileName, pszTag, iMin, iMax);
				break;

			default:
				ThrowError("TagFile::ReadMember", "No Value for iLimit");
		}
	}

	return *ppsz;
}


/**********************************************************

		ReadMember

**********************************************************/

int TagFile::ReadMember(const char *pszTag, int *pi, int iLimit, int iMin, int iMax)
{
	if(ReadTag(pszTag) != 'i')
		ThrowError("TagFile::ReadMember", "In file \"%s\" : %s should store an integer an should be preceded by 'i'", m_pszFileName, pszTag);

	*pi = ReadInt();

	//test le nbre de caracter de la string si demande par iLimit
	if(iLimit && !CheckValueLimit(iLimit, *pi, iMin, iMax) ){

		switch(iLimit){

			case FLOOR:
				ThrowError(	"File::ReadMember", 
					"In file \"%s\" : %s should be an integer >= %d", m_pszFileName, pszTag, iMin);
				break;

			case CEILING:
				ThrowError(	"File::ReadMember", 
					"In file \"%s\" : %s should be an integer <= %d", m_pszFileName, pszTag, iMax);
				break;

			case FLOOR|CEILING:
				ThrowError(	"File::ReadMember", 
					"In file \"%s\" : %s should be an integer >= %d and <= %d", m_pszFileName, pszTag, iMin, iMax);
				break;

			default:
				ThrowError(	"File::ReadMember", "No Value for iLimit");
		}
	}

	return *pi;
}



/**********************************************************

		ReadMember

**********************************************************/

unsigned int TagFile::ReadMember(const char *pszTag, unsigned int *pui)
{
	if(ReadTag(pszTag) != 'u')
		ThrowError("TagFile::ReadMember", "In file \"%s\" : %s should store an unsigned integer an should be preceded by 'u'", m_pszFileName, pszTag);

	*pui = ReadUnsignedInt();

	return *pui;
}


/**********************************************************

		ReadMember

**********************************************************/

double TagFile::ReadMember(const char *pszTag, double *pd, int iLimit, double dMin, double dMax)
{
	if(ReadTag(pszTag) != 'f')
		ThrowError("TagFile::ReadMember", "In file \"%s\" : %s should store a floating value an should be preceded by 'f'", m_pszFileName, pszTag);

	*pd = ReadDouble();	

	//test le nbre de caracter de la string si demande par iLimit
	if(iLimit && !CheckValueLimit(iLimit, *pd, dMin, dMax) ){

		switch(iLimit){

			case FLOOR:
				ThrowError(	"TagFile::ReadMember", 
					"In file \"%s\" : %s should be a floating value >= %d", m_pszFileName, pszTag, dMin);
				break;

			case CEILING:
				ThrowError(	"TagFile::ReadMember", 
					"In file \"%s\" : %s should be a floating value <= %d", m_pszFileName, pszTag, dMax);
				break;

			case FLOOR|CEILING:
				ThrowError(	"TagFile::ReadMember", 
					"In file \"%s\" : %s should be a floating value >= %d and <= %d", m_pszFileName, pszTag, dMin, dMax);
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

bool TagFile::ReadMember(const char *pszTag, bool *b)
{
	if(ReadTag(pszTag) != 'b')
		ThrowError("TagFile::ReadMember", "In file \"%s\" : %s should store a boolean an should be preceded by 'b'", m_pszFileName, pszTag);

	switch(ReadInt()){
	
		case 0:
			*b = false;
			break;

		case 1:
			*b = true;
			break;

		default :
			ThrowError("TagFile::ReadMember", "In file \"%s\" : tag %s : Boolean value should be 0 or 1", m_pszFileName, pszTag);
	}

	return *b;
}




/***********************************************************

   ReadArrayStart

***********************************************************/
void TagFile::ReadArrayStart(void)
{
	if(fscanf(m_pFile, "%s", m_szTag) != 1){
		if(feof(m_pFile))
				ThrowError("TagFile::ReadArrayStart", "Reach prematured end of file");
			else
				ThrowError("TagFile::ReadArrayStart", "Enable to read a word");
	}

	if(strcmp(m_szTag, "["))
		ThrowError("TagFile::ReadArrayStart", "FIle should contain start array character : \"[\"");
}



/***********************************************************

   ReadArrayEnd

***********************************************************/
void TagFile::ReadArrayEnd(void)
{
	if(fscanf(m_pFile, "%s", m_szTag) != 1){
		if(feof(m_pFile))
				ThrowError("TagFile::ReadArrayStart", "Reach prematured end of file");
			else
				ThrowError("TagFile::ReadArrayStart", "Enable to read a word");
	}

	if(strcmp(m_szTag, "]"))
		ThrowError("TagFile::ReadArrayStart", "FIle should contain start array character : \"[\"");
}

/***********************************************************

	CheckValueLimit

***********************************************************/

bool TagFile::CheckValueLimit(int iLimit, double d, double dMin, double dMax)
{
	if( (iLimit & FLOOR) && (d < dMin) )
			return false;

	if( (iLimit & CEILING) && (d > dMax) )
		return false;

	return true;
}

