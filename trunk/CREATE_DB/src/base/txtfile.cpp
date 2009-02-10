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
	Last modification  : 12.02.2004
	Comments           : 

***********************************************************/

#include "txtfile.h"
#include "error.h"



/******************************************************

		Constructor

*******************************************************/
TxtFile::TxtFile(void)
{
}



/******************************************************

		Destructor

*******************************************************/
TxtFile::~TxtFile(void)
{
}


/**********************************************************

		ReadInt

lit un int dans un fichier texte.il doit exister sinon ThrowError
**********************************************************/

int TxtFile::ReadInt(void)
{
	int iInt;

	if( fscanf(m_pFile, "%d", &iInt) != 1){
		if(feof(m_pFile))
			ThrowError("File::ReadInt", "Reach prematured end of file \"%s\"", m_pszFileName);
		else
			ThrowError("File::ReadInt", "Unable to read an integer in file \"%s\"", m_pszFileName);
	}

	return iInt;
}


/**********************************************************

		ReadUnsignedInt

**********************************************************/

unsigned int TxtFile::ReadUnsignedInt(void)
{
	unsigned int ui;

	if( fscanf(m_pFile, "%u", &ui) != 1){
		if(feof(m_pFile))
			ThrowError("File::ReadUnsignedInt", "Reach prematured end of file \"%s\"", m_pszFileName);
		else
			ThrowError("File::ReadUnsignedInt", "Unable to read an integer in file \"%s\"", m_pszFileName);
	}

	return ui;
}

/**********************************************************

		ReadDouble

lit un double dans un fichier texte il doit exister sinon ThrowError
**********************************************************/

double TxtFile::ReadDouble(void)
{
	double	dDouble;

	if( fscanf(m_pFile, "%lf", &dDouble) != 1){
		if(feof(m_pFile))
			ThrowError("File::ReadDouble", "Reach prematured end of file \"%s\"", m_pszFileName);
		else
			ThrowError("File::ReadDouble", "Unable to read a floating value in file \"%s\"", m_pszFileName);
	}

	return dDouble;
}



/**********************************************************

		ReadChar

**********************************************************/

void TxtFile::ReadChar(char *pc)
{	
	if( fscanf(m_pFile, "%c", pc) != 1){
		if(feof(m_pFile))
			ThrowError("File::ReadChar", "Reach prematured end of file \"%s\"", m_pszFileName);
		else
			ThrowError("File::ReadChar", "Unable to read a character in file \"%s\"", m_pszFileName);
	}
}







