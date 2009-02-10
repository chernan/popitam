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

#define USE_CGI
#define USE_STDIO
#define USE_STDLIB
#define USE_STRING
#define USE_STDARG
#define USE_DYNAMICARRAY
#define USE_ERROR
#define USE_ERRNO
#define USE_UTIL
#include "use.h"


#define APPLICATION_URLENCODED	0
#define	MULTIPART_FORM_DATA		1


/**********************************************************

		Constructor

**********************************************************/

Cgi::Cgi(void) :
	m_iNbCharToRead(0)
{
}


/**********************************************************

		Destructor

**********************************************************/

Cgi::~Cgi(void)
{
	m_map.Free();
}


/***********************************************************

	IsDefaultCall

***********************************************************/

bool Cgi::IsDefaultCall(void)
{
	if(!strcmp(GetEnv("REQUEST_METHOD"),"GET"))
		return true;

	return false;
}


/***********************************************************

	Load

***********************************************************/

int Cgi::Load(void)
{	
	m_map.Init(10, Cgi::FreeElt);

	if(strcmp(GetEnv("REQUEST_METHOD"),"POST"))
		ThrowError("Cgi::Load", "This program only uses a POST METHOD for html FORM tag");

	//recupere le type d'encodage
	GetContentType();
	
	//recupere le nbre de caracteres a lire depuis stdin
	m_iNbCharToRead = atoi(GetEnv("CONTENT_LENGTH"));
	
	int iNbByte = m_iNbCharToRead;

	switch(m_iContentType){

		case APPLICATION_URLENCODED:
			while(m_iNbCharToRead > 0)
				DecodeUrlEncodedCouple(GetUrlEncodedCouple());
			break;

		case MULTIPART_FORM_DATA:
			DecodeMultipart();
			break;
	}

	return iNbByte;
}


/*
void Replace(char *psz, char cFrom, char cTo)
{
	while( (*psz == cFrom)? (*psz++ = cTo) : (*psz++) );  
}
*/



/***********************************************************

	GetUrlEncodedCouple

Recupere depuis stdin une string contanant : "name=value\0"
ou name et value sont encodee : 
'+' pour espace
"%hh" ou hh est le nbre en hexadecimal d'un caractere ascii
***********************************************************/

char *Cgi::GetUrlEncodedCouple(void)
{
	DynamicArray<char>	string;		//tableau dynamic de char
	char				cChar;

	string.Alloc(512);

	while(m_iNbCharToRead--){

		//lecture jusqu'au separateur de couple '&'
		if( (cChar = fgetc(stdin)) == '&' ){
			string.Add('\0');
			return string.GetWithoutAjust();
		}

		string.Add(cChar);
	}

	string.Add('\0');

	return string.GetWithoutAjust();
}


/***********************************************************

	DecodeUrlEncodedCouple

Decode "name=value\0" pour ajouter dans l'objet Map
un couple (pszName, pszData)
***********************************************************/

void Cgi::DecodeUrlEncodedCouple(char *psz)
{
	char *pszEqual;
	char *pszName = NULL;;
	char *pszData = NULL;

	//trouve l'adresse du caractere de separation '=' entre name et data 
	if(!(pszEqual = strchr(psz, '=') ))
		ThrowError("Cgi::DecodeUrlEncodedCouple", "Url Bad format");

	//nbre de caract de la partie Name
	int iNameLen = (int)(pszEqual - psz);

	if(!(	(pszName = (char *)malloc(iNameLen +1))			&&
			(pszData = (char *)malloc(strlen(pszEqual)))	))
		ThrowError("Cgi::DecodeUrlEncodedCouple", "%s", strerror(errno));

	//recupere Name
	strncpy(pszName, psz, iNameLen);
	pszName[iNameLen] = '\0';

	//recupere Data
	strcpy(pszData, pszEqual+1);

	//remplace '+' par ' ' et transforme les hexa ex : "%56" en caractere ascii.
	PlusToSpace(pszName);
	RemoveEscapeToChar(pszName);

	PlusToSpace(pszData);
	RemoveEscapeToChar(pszData);

	//ajoute le couple a la liste
	if( !m_map.Add(pszName, new CgiData(pszData)) )
		ThrowError("Cgi::DecodeUrlEncodedCouple", "Variable : \"%s\" already exist", pszName );

	//libere la string allouee par ReadCouple
	free(psz);
}


/***********************************************************

	PlusToSpace

remplace tous les caracteres '+' par ' '
***********************************************************/

void Cgi::PlusToSpace(char *psz)
{
	while( (*psz == '+')? (*psz++ = ' ') : (*psz++) );  
}


/***********************************************************

	HexToChar

renvoi le caractere ascci encode par "%hh" a l'adresse *ppsz
et avance le pointeur de 2 cases
***********************************************************/

char Cgi::HexToChar(char **ppsz)
{
	char sz[3];
	strncpy(sz, (++(*ppsz))++, 2);
	sz[2] = '\0';
	return (char)strtol(sz, NULL, 16);
}


/***********************************************************

	RemoveEscapeToChar

remplace toutes les sites "%hh" par le caractere ascii correspondant
***********************************************************/

void Cgi::RemoveEscapeToChar(char *psz)
{
	for(char *psz2 = psz; (*psz = *psz2); psz++, psz2++)
		if(*psz == '%')
			*psz = HexToChar(&psz2);	
}


/***********************************************************

	GetBoundary

***********************************************************/

char *Cgi::GetBoundary(void)
{
	char *psz;
	char *pszBoundary = "boundary=";
	
	if(!(psz = strstr(GetEnv("CONTENT_TYPE"), pszBoundary) ))
		ThrowError("Cgi::GetBoundary", 
					"Environment variable CONTENT_TYPE doesn't contain boundary information \"%s\"...", pszBoundary);

	psz += strlen(pszBoundary);

	return Strdup(psz);
}



/***********************************************************

	DecodeMultipart

***********************************************************/

void Cgi::DecodeMultipart(void)
{
	char				*pszBuffer;
	char				*pszEndData;
	char				*pszTagName		= "Content-Disposition: form-data; name=";
	char				*pszTagFileName	= "; filename=";
	DynamicArray<char>	name;
	DynamicArray<char>	fileName;
	bool				bFile = false;

	//recupere la string separatrice des champs
	char *pszBoundary = GetBoundary();

	//charge tout le contenu de stdin dans pszBuffer au format string
	if(!(pszBuffer = (char *)malloc(m_iNbCharToRead+1) ))
		ThrowError("Cgi::DecodeMultipart", "%s", strerror(errno));

	for(int i=0; i<m_iNbCharToRead; i++)
		pszBuffer[i] = fgetc(stdin);
		
	pszBuffer[m_iNbCharToRead] = '\0';

	//pointeur sur pszBuffer a un instant t du parsing
	char *psz = pszBuffer;
	
	//cherche le tag Name
	while((psz = strstr(psz, pszTagName)) ){

		name.Alloc(128);

		//debut de nom de la variable
		psz += strlen(pszTagName) +1;

		//charge le nom de la variable
		while(*psz != '"')
			name.Add(*psz++);

		name.Add('\0');

		//test la presence d'un nom d'un fichier.
		if( !strncmp(++psz, pszTagFileName, strlen(pszTagFileName)) ){
	
			fileName.Alloc(128);
	
			psz += strlen(pszTagFileName) +1;

			while(*psz != '"')
				fileName.Add(*psz++);

			fileName.Add('\0');
			bFile = true;
		}

		//consomme le retour a la ligne "\r\n" ou "\n"
		while(*psz++ != '\n');		


		//test la presence d'une info Content-Type alors consomme la ligne
		if(!strncmp(psz, "Content-Type:", 12))
			while(*psz++ != '\n');

				
		//consomme le retour a la ligne "\r\n" ou "\n"
		while(*psz++ != '\n');

	
		//recupere les data associe au nom
		if(!(pszEndData = strstr(psz, pszBoundary) ))
			ThrowError("Cgi::DecodeMultipart", 
						"Error receiving form data or trying to upload binary files not supported");
		
		/*pszBoundary a deux caracteres de moins que dans le corps du message !!! pourquoi ?????
		recule donc de 2 cases, et encore deux cases pour se positionner avant '\n' qui precede*/
		pszEndData -= 4;

		if(*pszEndData == '\r')				
			pszEndData--;
		
		CgiData *pCgiData;

		if(bFile)
			pCgiData = new CgiFileData(fileName.GetWithAjust(), psz, (int)(pszEndData - psz +1));
		else
			pCgiData = new CgiData(psz, (int)(pszEndData - psz +1));

		//ajoute a l'objet map
		char *pszName = name.GetWithAjust();


		//cette variable contient deja des donnees exemple des multiselection dans une liste
		if( !m_map.Add(pszName, pCgiData) ){
			Get(pszName)->Add(Strdup(pCgiData->GetData()));
			delete pCgiData;
		}
	}

	free(pszBoundary);
	free(pszBuffer);
}






/***********************************************************

	FreeElt

fct appelle par m_map pour liberer les objets stockes a la destruction
***********************************************************/

void Cgi::FreeElt(const char *psz, void *pData)
{
	free((void *)psz);
	delete (CgiData *)pData;
}




/***********************************************************

	GetContentType

***********************************************************/

const char *Cgi::GetContentType(void)
{
	char *pszUrlEncoded = "application/x-www-form-urlencoded";
	char *pszMultipart	= "multipart/form-data";

	const char *pszContentType = GetEnv("CONTENT_TYPE");

	if(!strcmp(pszContentType, pszUrlEncoded))
		m_iContentType = APPLICATION_URLENCODED;

	else if(strstr(pszContentType, pszMultipart))
		m_iContentType = MULTIPART_FORM_DATA;

	else
		ThrowError("Cgi::GetContentType", "ENCTYPE param for FORM must be : %s or %s", pszContentType, pszMultipart);

	return pszContentType;
}


/***********************************************************

	Get

***********************************************************/

CgiData *Cgi::Get(const char *pszName)
{
	CgiData *pCgiData;

	if(!m_map.Get(pszName, (void **)(&pCgiData)) )
		ThrowError("Cgi::Get", "No data associate to string %s in cgi form", pszName);
	
	return pCgiData;
}


/***********************************************************

	isFieldEmpty

***********************************************************/

bool Cgi::isFieldEmpty(const char *pszName)
{
	const char *psz = Get(pszName)->GetData();

	while(*psz && strchr(" \t", *psz))
		psz++;

	if(*psz)
		return false;
	
	return true;
}

/***********************************************************

	GetNbElt

***********************************************************/

int Cgi::GetNbElt(const char *pszName)
{
	int iNbElt;

	try{

		iNbElt = Get(pszName)->GetNbData();

	}catch(Error *pError){
		delete pError;
		iNbElt = 0;
	}

	return iNbElt;
}



/***********************************************************

	GetFileNamePtr

***********************************************************/

const char *Cgi::GetFileNamePtr(const char *pszName)
{	
	return ((CgiFileData *)Get(pszName))->GetFileName();
}


/***********************************************************

	GetFileName

***********************************************************/

char *Cgi::GetFileName(const char *pszName)
{
	return Strdup(GetFileNamePtr(pszName));
}




/***********************************************************

	GetString

recupere une string associee a pszName, 
la string est allouee.
***********************************************************/

char *Cgi::GetString(const char *pszName, int iIndex)
{
	return Strdup(GetStringPtr(pszName, iIndex));
}



/***********************************************************

	GetStringPtr

recupere une string associee a pszName
***********************************************************/

const char *Cgi::GetStringPtr(const char *pszName, int iIndex)
{	
	return Get(pszName)->GetData(iIndex);
}



/***********************************************************

	GetInt

recupere un int associee a pszName
***********************************************************/

int Cgi::GetInt(const char *pszName, int iIndex)
{
	int iInt;
	
	if(sscanf(Get(pszName)->GetData(iIndex), "%d", &iInt) != 1)
		ThrowError("Cgi::GetInt", "%s should be an integer", pszName);

	return iInt;
}



/***********************************************************

	GetDouble

recupere un double associee a pszName, determine eventuellement
un intervalle pour ce double
***********************************************************/

double Cgi::GetDouble(const char *pszName, int iIndex)
{
	double d;
	
	if(sscanf(Get(pszName)->GetData(iIndex), "%lf", &d) != 1)
		ThrowError("Cgi::GetDouble", "%s should be a floating value", pszName);

	return d;
}


/***********************************************************

	GetCheckBox

lorsqu'une checkbox n'est pas cochee, elle n'apparait pas dans
l'url, c'est con, mais c'est comme ca !!!
on ne peux pas tester si c'est pszName qui est different ou
si le check n'est pas coche !!!!!!
***********************************************************/

bool Cgi::GetCheckBox(const char *pszName)
{
	CgiData *pCgiData;

	if(!m_map.Get(pszName, (void **)(&pCgiData)) )
		return false;

	const char *psz = pCgiData->GetData();

	if( !strcmp(psz, "1") )
		return true;

	if( !strcmp(psz, "0") )
		return false;
	
	//tous les autres cas
	return true;	
}



/***********************************************************

	Constructor

***********************************************************/

CgiData::CgiData(char *pszData) 
{
	m_array.Alloc(1);
	m_array.Add(pszData);
}

/***********************************************************

	Constructor

***********************************************************/

CgiData::CgiData(const char *pszData, int iLen)
{
	m_array.Alloc(1);

	char *psz;
	if(!(psz = (char *)malloc(iLen + 1) ))
		ThrowError("CgiData::CgiData", "%s", strerror(errno));

	strncpy(psz, pszData, iLen);
	psz[iLen] = '\0';

	m_array.Add(psz);
}


/***********************************************************

	Destructor

***********************************************************/

CgiData::~CgiData(void)
{
	m_array.Clear(CgiData::Free);
}

/***********************************************************

	GetFileName

***********************************************************/

const char *CgiData::GetFileName(void)
{
	ThrowError("CgiData::GetFileName", "trying to upload a not uploadable form object");
	return NULL;
}



/***********************************************************

	Constructor

***********************************************************/

CgiFileData::CgiFileData(char *pszFileName, const char *pszData, int iLen) :
	CgiData(pszData, iLen),
	m_pszFileName(pszFileName)
{
}


/***********************************************************

	Destructor

***********************************************************/

CgiFileData::~CgiFileData(void)
{
	if(m_pszFileName){
		free(m_pszFileName);
		m_pszFileName = NULL;
	}
}







