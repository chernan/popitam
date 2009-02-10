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

#define USE_TXTFILE
#define USE_ERROR
#include "use.h"




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

	if( fscanf(m_pFile, "%d", &iInt) != 1)
		ThrowError("File::ReadInt", "File \"%s\" bad format", m_pszFileName);

	return iInt;
}


/**********************************************************

		ReadDouble

lit un double dans un fichier texte il doit exister sinon ThrowError
**********************************************************/

double TxtFile::ReadDouble(void)
{
	double	dDouble;

	if( fscanf(m_pFile, "%lf", &dDouble) != 1) 
		ThrowError("File::ReadDouble", "File \"%s\" bad format", m_pszFileName);

	return dDouble;
}



/**********************************************************

		ReadChar

**********************************************************/

void TxtFile::ReadChar(char *pc)
{	
	if( fscanf(m_pFile, "%c", pc) != 1)
		ThrowError("File::ReadChar", "File \"%s\" bad format", m_pszFileName);	
}







