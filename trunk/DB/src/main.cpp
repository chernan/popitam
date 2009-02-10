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
	Created on         : 19.07.2006
	Last modification  : 19.07.2006
	Comments           : 

***********************************************************/


#include "file.h"
#include "MATerror.h"
#include "parser.h"
#include "dbfile.h"


/***********************************************************

	main

***********************************************************/
char head[256];

int main(int argc, char **argv)
{
	if(argc != 6){

		if(argc == 2 && !strcmp(argv[1], "-v")){
			printf("Compilation %s, database file version %d\n", __DATE__, DBFile::iVersion);
			return 0;
		}
   
		

		printf("\nSyntax : <fasta fileName> <database filename> <database label> <release tag> <header tag\n"
						"-v to print the version\n\n"
				        "Headers:\n"
				        "PHENYX:\n"
				        ">AC \\ID=... \\MODRES=... \\NCBITAXID=... \\DE=...\n\n"
				        "SBG:\n"
				 		">sp|AC|ID DE\n\n"
				        "LIGHT:\n"
				        ">AC|ID DE\n\n");
		return -1;
	}

strcpy(head, argv[5]);

try{

		Parser parser;
		parser.Run(argv[1], argv[2], argv[3],	argv[4], NULL, NULL);
		//parser.Run(pszFastaFileName, pszDBFileName, pszDBLabel,	pszDBRelease, pszTaxonomyFileName, pszPtmFileName);

	}catch(Error *pError){
		pError->Print();
		delete pError;
	}

	return 0;
}





