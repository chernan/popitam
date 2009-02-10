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
    Last modification  : 15.04.2005
	Comments           : 

***********************************************************/

#include "file.h"
#include "error.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/******************************************************

		Constructor

*******************************************************/
File::File(void) : 
	m_pFile(NULL),
	m_pszFileName(NULL)
{
}


/******************************************************

		Constructor

*******************************************************/
File::File(const char *pszFileName, const char *pszMode) : 
	m_pFile(NULL),
	m_pszFileName(NULL)
{
	Open(pszFileName, pszMode);
}


/******************************************************

		Constructor

*******************************************************/
File::File(FILE *pFile) : 
	m_pFile(NULL),
	m_pszFileName(NULL)
{
	Open(pFile);
}


/******************************************************

		Destructor

*******************************************************/
File::~File(void)
{
	if(m_pFile)
		Close();

	if(m_pszFileName){
		delete [] m_pszFileName;
		m_pszFileName = NULL;
	}
}


/***********************************************************

	Exist

***********************************************************/
bool File::OpenIfExist(const char *pszFileName, const char *pszMode)
{
	if(!(m_pFile = fopen(pszFileName, pszMode) ))
		return false;

	m_pszFileName = Strdup(pszFileName);
	return true;
}


/**********************************************************

		Open

**********************************************************/
void File::Open(const char *pszFileName, const char *pszMode)
{
	if(!(m_pFile = fopen(pszFileName, pszMode) ))
		ThrowError("File::Open", "%s \"%s\" mode \"%s\"", strerror(errno), pszFileName, pszMode);

	m_pszFileName = Strdup(pszFileName);
}


/***********************************************************

	Open

***********************************************************/

void File::Open(FILE *pFile)
{
	m_pFile = pFile;

	if(pFile == stdout)
		m_pszFileName = Strdup("stdout");
	else if(pFile == stdin)
		m_pszFileName = Strdup("stdin");
	else if(pFile == stderr)
		m_pszFileName = Strdup("stderr");
	else
		m_pszFileName = Strdup("No file name");
}


/**********************************************************

		Close

**********************************************************/

void File::Close(void)
{
	if(!m_pFile || !m_pszFileName)
		ThrowError("File::Close", "File is already closed");
	
	if( (m_pFile != stdout) && (m_pFile != stdin) && (m_pFile != stderr) && fclose(m_pFile))
		ThrowError("File::Close", "%s %s", strerror(errno), m_pszFileName);
	
	delete [] m_pszFileName;

	m_pszFileName	= NULL;
	m_pFile			= NULL;
}


/***********************************************************

	Cpy

***********************************************************/

void File::Cpy(FILE *pFile)
{
	char	c;

	while( fread(&c, sizeof(char), 1, pFile) == 1)
		fwrite(&c, sizeof(char), 1, m_pFile);
}


/***********************************************************

	Cpy

***********************************************************/

void File::Cpy(char *pszBuffer, int iBufferLen)
{
	//garde une place pour le null de fin de string
	iBufferLen--;

	int i=0;

	do{
		if(i++ > iBufferLen)
			ThrowError("File::Cpy", "Buffer too small");
		
	}while( fread(pszBuffer++, sizeof(char), 1, m_pFile) == 1);


	*(--pszBuffer) = '\0';
}


