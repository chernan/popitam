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
	Created	on		   : 01.06.2004
	Last modification  : 24.02.2005
	Comments		   : 

***********************************************************/


#ifndef	__USE_H__
#define	__USE_H__




/***************************************************

		DEFINE

****************************************************/

#ifdef USE_MANAGER
#define	USE_PATHS
#define	USE_POPITAMFORM
#define	USE_POPIRESULT
#define USE_FLOAT
#define	INCLUDE_MANAGER
#endif

#ifdef USE_POPIRESULT
#define	USE_PATHS
#define	USE_TAGFILE
#define	USE_POPIPARSER
#define	USE_UTIL
#define	INCLUDE_POPIRESULT
#endif

#ifdef USE_POPIPARSER
#define	USE_TAGFILE
#define	USE_UTIL
#define	INCLUDE_POPIPARSER
#endif

#ifdef USE_POPITAMFORM
#define	USE_PATHS
#define	USE_FIELD
#define	USE_ITEMLIST
#define	USE_RADIOGROUP
#define	USE_CHECK
#define	USE_DISPLAY
#define	USE_UTIL
#define	USE_TAXONOMY
#define	INCLUDE_POPITAMFORM
#endif

#ifdef USE_TAXONOMY
#define	USE_TAXON
#define	USE_PATHS
#define	USE_FILE
#define	USE_UTIL
#define	USE_MENU
#define	USE_MSG
#define	USE_CGI
#define	USE_FIELD
#define	INCLUDE_TAXONOMY
#endif

#ifdef USE_TAXON
#define	USE_STDLIB
#define	USE_ERROR
#define	USE_ERRNO
#define	USE_STRING
#define	USE_TAGFILE
#define	USE_ITEM
#define	INCLUDE_TAXON
#endif

#ifdef USE_PATHS
#define	USE_PSZLIST
#define	INCLUDE_PATHS
#endif

#ifdef USE_DISPLAY
#define	USE_FIELD
#define	INCLUDE_DISPLAY
#endif

#ifdef USE_FIELD
#define	USE_TAGFILE
#define	USE_CGI
#define	USE_VALUELIMIT
#define	INCLUDE_FIELD
#endif

#ifdef USE_ITEMLIST
#define	USE_CGI
#define	USE_TAGFILE
#define	USE_ITEM
#define	INCLUDE_ITEMLIST
#endif

#ifdef USE_ITEM
#define	USE_TAGFILE
#define	INCLUDE_ITEM
#endif

#ifdef USE_RADIOGROUP
#define	USE_CGI
#define	USE_TAGFILE
#define	USE_RADIO
#define	INCLUDE_RADIOGROUP
#endif

#ifdef USE_RADIO
#define	USE_TAGFILE
#define	INCLUDE_RADIO
#endif

#ifdef USE_CHECK
#define	USE_CGI
#define	USE_TAGFILE
#define	INCLUDE_CHECK
#endif


#ifdef USE_CGI
#define	USE_MAP
#define	USE_STDLIB
#define	USE_DYNAMICARRAY
#define	INCLUDE_CGI
#endif

#ifdef USE_MAP
#define	USE_LEXICO
#define	INCLUDE_MAP
#endif

#ifdef USE_LEXICO
#define	INCLUDE_LEXICO
#endif

#ifdef USE_HTML
#define	USE_FILE
#define	INCLUDE_HTML
#endif

#ifdef USE_TAGFILE
#define	USE_TXTFILE
#define	USE_DYNAMICARRAY
#define	USE_VALUELIMIT
#define	INCLUDE_TAGFILE
#endif

#ifdef USE_VALUELIMIT
#define	INCLUDE_VALUELIMIT
#endif

#ifdef USE_TXTFILE
#define	USE_FILE
#define	INCLUDE_TXTFILE
#endif

#ifdef USE_FILE
#define	USE_STDIO
#define	INCLUDE_FILE
#endif

#ifdef USE_UTIL
#define	INCLUDE_UTIL
#endif


#ifdef USE_DYNAMICARRAY
#define	USE_ERROR
#define	USE_STDLIB
#define	USE_STRING
#define	USE_ERRNO
#define	INCLUDE_DYNAMICARRAY
#endif

#ifdef USE_REQUEST
#define	INCLUDE_REQUEST
#endif

#ifdef USE_ERROR
#define	USE_STDIO
#define	INCLUDE_ERROR
#endif

#ifdef USE_MSG
#define	INCLUDE_MSG
#endif

#ifdef USE_PSZLIST
#define	INCLUDE_PSZLIST
#endif

//include standard
#ifdef USE_STDIO
#define	INCLUDE_STDIO
#endif

#ifdef USE_STDLIB
#define	INCLUDE_STDLIB
#endif

#ifdef USE_STRING
#define	INCLUDE_STRING
#endif

#ifdef USE_ERRNO
#define	INCLUDE_ERRNO
#endif

#ifdef USE_STDARG
#define	INCLUDE_STDARG
#endif

#ifdef USE_FLOAT
#define INCLUDE_FLOAT
#endif

#ifdef USE_MATH
#define	INCLUDE_MATH
#endif

#ifdef USE_TIME
#define	INCLUDE_TIME
#endif

/***************************************************

		INCLUDE

****************************************************/

//include standard
#ifdef INCLUDE_ERRNO
#include "errno.h" 
#endif

#ifdef INCLUDE_STRING
#include "string.h" 
#endif

#ifdef INCLUDE_STDLIB
#include "stdlib.h" 
#endif

#ifdef INCLUDE_STDIO
#include "stdio.h" 
#endif

#ifdef INCLUDE_FLOAT
#include "float.h"
#endif

#ifdef INCLUDE_STDARG
#include "stdarg.h" 
#endif

#ifdef INCLUDE_MATH 
#include "math.h" 
#endif

#ifdef INCLUDE_TIME
#include "time.h"
#endif

//include perso
#ifdef INCLUDE_PSZLIST
#include "pszlist.h"
#endif

#ifdef INCLUDE_MSG
#include "msg.h"
#endif

#ifdef INCLUDE_ERROR
#include "myerror.h"
#endif

#ifdef INCLUDE_REQUEST
#include "request.h"
#endif

#ifdef INCLUDE_UTIL
#include "util.h"
#endif

#ifdef INCLUDE_DYNAMICARRAY
#include "dynamicarray.h"
#endif

#ifdef INCLUDE_FILE
#include "file.h" 
#endif

#ifdef INCLUDE_TXTFILE
#include "txtfile.h"
#endif

#ifdef INCLUDE_VALUELIMIT
#include "valuelimit.h"
#endif

#ifdef INCLUDE_TAGFILE
#include "tagfile.h"
#endif

#ifdef INCLUDE_HTML
#include "html.h"
#endif

#ifdef INCLUDE_LEXICO
#include "lexico.h" 
#endif

#ifdef INCLUDE_MAP
#include "map.h" 
#endif

#ifdef INCLUDE_CGI
#include "cgi.h" 
#endif


#ifdef INCLUDE_CHECK
#include "check.h"
#endif

#ifdef INCLUDE_RADIO
#include "radio.h"
#endif

#ifdef INCLUDE_RADIOGROUP
#include "radiogroup.h"
#endif

#ifdef INCLUDE_ITEM
#include "item.h"
#endif

#ifdef INCLUDE_ITEMLIST
#include "itemlist.h"
#endif

#ifdef INCLUDE_FIELD
#include "field.h"
#endif

#ifdef INCLUDE_DISPLAY
#include "display.h"
#endif

#ifdef INCLUDE_TAXON
#include "taxon.h"
#endif

#ifdef INCLUDE_TAXONOMY
#include "taxonomy.h"
#endif

#ifdef INCLUDE_PATHS
#include "paths.h"
#endif

#ifdef INCLUDE_POPITAMFORM
#include "popitamform.h"
#endif

#ifdef INCLUDE_POPIPARSER
#include "popiparser.h"
#endif

#ifdef INCLUDE_POPIRESULT
#include "popiresult.h"
#endif

#ifdef INCLUDE_MANAGER
#include "manager.h"
#endif

#endif
