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
#include "error.h"
#include "parserPhenyx.h"
#include "parserCamsi.h"
#include "parserBase.h"
#include "parserSBG.h"
#include "dbfile.h"



/***********************************************************

	main

***********************************************************/

int main(int argc, char **argv)
{
	if(argc != 8){

		if(argc == 2 && !strcmp(argv[1], "-v")){
			printf("Compilation %s, database file version %d\n", __DATE__, DBFile::iVersion);
			return 0;
		}


		printf("\nSyntax : <inputName> <outputName> <dbLabel> <NCBI taxo fileName | NULL> <MODRES fileName | NULL> <releaseTag> <headerType>\n"
		"-v to print the version\n\n"		
		"   ARGUMENTS (all arguments are mandatory):\n\n"
		"   <inputName>                : filename of the fasta database you want to index\n"
		"   <outputName>               : db output filename\n"
		"   <dbLabel>                  : any label that represents your database\n"
		"   <NCBI taxo fileName | NULL>: choose NULL, except if the header type is PHENYX, and\n"
		"                                if it contains taxonomy information. In that case,\n"
		"                                specify the path to a file that contains the NCBI taxIDs\n"
		"   <MODRES filename | NULL>   : choose NULL except if the header type is PHENYX. In that case\n"
		"                                specify the path to a file that contains the list of available PTMs\n"
		"   <releaseTag>               : you can specify the current date, or any release tag\n"
		"   <headerType>               : choose between the three following header types:\n"
		"                                PHENYX: \n"
		"                                >P48668 \\ID=K2C6C_HUMAN \\MODRES= \\NCBITAXID=9606 \\DE=Keratin, type II cytoskeletal 6C - Homo sapiens (Human).\n"
        "                                CAMSI (works for CAMSI and FLYBASE)\n"
		"                                >P48668\n"
		"                                BASE: \n"
		"                                >P48668|K2C6C_HUMAN Keratin, type II cytoskeletal 6C - Homo sapiens (Human).\n\n"
		"   Here follows some examples:\n"
		"   ./createDB ../testDBs/demoDB_PHENHEADER.fasta demoDB_PHENHEADER.bin demoDB_PHENHEADER NULL NULL PHEN_1.0 PHENYX\n"
		"   ./createDB ../testDBs/demoDB_PHENHEADER_DECOY.fasta demoDB_PHENHEADER_DECOY.bin demoDB_PHENHEADER_DECOY NULL NULL PHEN_1.0_DECOY PHENYX\n"	
		"   ./createDB ../testDBs/demoDB_CAMHEADER.fasta demoDB_CAMHEADER.bin demoDB_CAMHEADER NULL NULL CAM_1.0 CAMSI\n"
		"   ./createDB ../testDBs/demoDB_BASE.fasta demoDB_BASE.bin demoDB NULL NULL BASE_1.0 BASE\n");
		return -1;
	}



	try{
		if (!strncmp(argv[7], "PHENYX", 6)) 
		{
			ParserPhenyx parser;      
			const char *pszTaxonomy = (!strcmp(argv[4], "NULL"))? NULL : argv[4];
			const char *pszModRes = (!strcmp(argv[5], "NULL"))? NULL : argv[5];
			parser.Run(argv[1], argv[2], argv[3], pszTaxonomy, NCBI_TAXO, pszModRes, argv[6]);
			//parser.Run("C:/tmp/homoSapiens.fasta", "C:/tmp/human.bin", "Human", "C:/tmp/nodes.txt", NCBI_TAXO, "C:/tmp/ptm_phenyx.txt", xx.0, PHENYX);
			//parser.Run("C:/tmp/homoSapiens.fasta", "C:/tmp/human.bin", "Human", NULL, NCBI_TAXO, NULL, xx.0, PHENYX);
		}
		if (!strncmp(argv[7], "CAMSI", 3)) 
		{
			ParserCamsi parser;
			parser.Run(argv[1], argv[2], argv[3], NULL, NCBI_TAXO, NULL, argv[6]);     
		}
		if (!strncmp(argv[7], "SBG", 3)) 
		{
			ParserSBG parser;
			parser.Run(argv[1], argv[2], argv[3], NULL, NCBI_TAXO, NULL, argv[6]);     
		}
		if (!strncmp(argv[7], "BASE", 4)) 
		{	
			ParserBase parser;
			parser.Run(argv[1], argv[2], argv[3], NULL, NCBI_TAXO, NULL, argv[6]);
		}
		
        
	}catch(Error *pError){
		pError->Print();
		delete pError;
	}
	printf("- done...\n");
	return 0;
}





