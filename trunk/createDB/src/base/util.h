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
	Last modification  : 09.07.2004
	Comments           : 

***********************************************************/

#ifndef __UTIL_H__
#define __UTIL_H__

#include <time.h>


char *Strdup(const char *psz);
char *StrdupFormat(char *pszFormat, ...);

int SystemCmd(char *pszFormat, ...);

const char *GetEnv(const char *pszEnvVar);

char *StrDouble(char *pszBuffer, double d);

char *FormatDouble(char *pszBuffer, double d);

const char *FormatDate(time_t ltime, char *pszBuffer);

#endif
