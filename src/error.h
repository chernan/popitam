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

#ifndef __ERROR_H__
#define __ERROR_H__

/******************************************************************************/

/* ERROR CODES */

#define FILES     100        // error while opening-reading-writing a file
#define TEMP      200        // error while creating a temporary file
#define MEMORY    300        // out of memory
#define LOOK      400        // error, look why
#define DEBUG     500        // debug error
#define MODE      600
#define PARAMETER 700
#define NANI      800
#define INFI      900
#define BORN     1000
#define DATAS    1100

/******************************************************************************/


/******************************************************************************/

void fatal_error(char* fileErrorName, int, char*);

/******************************************************************************/


#endif
