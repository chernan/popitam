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
	Created on         : 23.06.2004
    Last modification  : 24.02.2005
	Comments           : 

***********************************************************/

#ifndef __HTML_H__
#define __HTML_H__


void WriteHtmlContentType(void);
void WriteHtmlHeader(File &file, const char *pszTitle=NULL, const char *pszHead=NULL);
void WriteHtmlEnd(File &file);

void Cell(File &file, const char *pszType, int iCellIndex, const char *pszFormat, ...);
void HRef(File &file, const char *psz, const char *pszLink);
void HRefU(File &file, const char *psz, const char *pszLink);



#endif
