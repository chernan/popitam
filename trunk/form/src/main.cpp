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
	Last modification  : 16.02.2005
	Comments		   : 

***********************************************************/

#define	USE_MANAGER
#define	USE_HTML
#include "use.h"



/***********************************************************

	main

***********************************************************/

int	main(int argc, char	**argv)
{   
	try{

		Manager	manager;

		if(argc	== 1)
			manager.Run();				//appel	via	cgi
		else{
			if(argc	== 4)
				manager.BatchMode(argv[1], argv[2],	argv[3]); //appel en mode batch	result par mail
			else
				printf("<syntaxe for send mail>	<exe> <paramFile>\n");
		}
	    
	    
		    
	}catch(Error *pError){
		WriteHtmlContentType();
		printf("<pre>");
		pError->Print();
		printf("</pre>");
		delete pError;
	}

    

	return 0;
}



