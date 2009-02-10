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

#include "memorycheck.h"
#include "fun.h"
#include <cstdlib>
#include <fstream>
#include <cstring>
#include "error.h"
#include "defines.h"

using namespace std;

// ********************************************************************************************** //

#define MAX_DOB 100000

// ********************************************************************************************** //

/* GLOBAL VARIABLES DECLARED ELSEWHERE */

extern memorycheck memCheck;


// ********************************************************************************************** //


/* CONSTRUCTEUR */

fun::fun() {
  runManParam  = NULL;
}

// ********************************************************************************************** //

fun::~fun() {}

// ********************************************************************************************** //

void fun::init(runManagerParameters* rMP, int ID) 
{
  runManParam = rMP;
  NEGBefore   = 0;
  NEGAfter    = 0;
  POSBefore   = 0;
  POSAfter    = 0;
  SCENBefore  = 0;
  SCENAfter   = 0;
  char filename[256];
  sprintf(filename, "%s_%i.fun", runManParam->GEN_OR_FILENAME_SUFF, ID);
  fpFUN.Open(filename, "w");
}

// ********************************************************************************************** //

void fun::endRun(int ID)
{
  fpFUN.Close();
  processLearningSet(ID);
  display(runManParam->FILEOUT);
}

// ********************************************************************************************** //

void fun::processLearningSet(int specID)
{
  // FILENAMES
  char nameOR[256];
  char nameNOD[256];


  sprintf(nameOR,  "%s_%i.fun", runManParam->GEN_OR_FILENAME_SUFF,  specID);
  sprintf(nameNOD, "%s_%i.fun", runManParam->GEN_NOD_FILENAME_SUFF, specID);
  
  int  POSNb  = 0;
  int  NEGNb  = 0;
  int  SCENNb = 0;
  countPeptides(nameOR, &POSNb, &NEGNb, &SCENNb);
  NEGBefore  = NEGNb;
  POSBefore  = POSNb;
  SCENBefore = SCENNb;
  putAwayOrphans(nameOR, nameNOD);

  POSNb  = 0;
  NEGNb  = 0;
  SCENNb = 0;
  countPeptides(nameNOD, &POSNb, &NEGNb, &SCENNb);

  NEGAfter   = NEGNb;
  POSAfter   = POSNb;
  SCENAfter  = SCENNb;

  char com[256];
  sprintf(com, "rm %s", nameOR);
  system(com);
}

// ********************************************************************************************** //

void fun::putAwayOrphans(char* nameIn, char* nameOut) {
  
  ifstream IN2;  IN2.open(nameIn);   if (!IN2) fatal_error(runManParam->FILE_ERROR_NAME, FILES, nameIn);
  ofstream OUT2; OUT2.open(nameOut); if (!OUT2) fatal_error(runManParam->FILE_ERROR_NAME, FILES, nameOut);
  
  char   line[256];
  char   title[256];
  int    b = 0;
  
  char** buf;
  buf = new char*[MAX_ARRANGEMENT_NB];                                                            memCheck.fun++;
  for (int i = 0; i < MAX_ARRANGEMENT_NB; i++) {
    buf[i] = new char[256];                                                                       memCheck.fun++;
  }
  
  while (IN2.getline(line, 255)) 
    {
      if (line[strlen(line)-1] != ';') fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function putAwayOrphans, a.");
        
      if ((!strncmp(line, "NEWPOS", 6)) | (!strncmp(line, "NEWNEG", 6)))
	{
	  
	 // RECUPERE LES ARRANGEMENTS (JUSQU'A LA PROCHAINE LIGNE VIDE)
	  b = 0;
	  
	  // dans line, il y a le titre
	  strcpy(title, line);
	  
	  while (true) 
	    {
	      IN2.getline(line, 255); 
	      if (line[strlen(line)-1] != ';') fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function putAwayOrphans, b.");
	      if (strlen(line) == 1) break; // juste le ;
	      strcpy(buf[b],line);
	      b++;
	      if (b >=MAX_ARRANGEMENT_NB) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function processLearningSet(); please increase MAX_ARRANGEMENT_NB");
	    }
	  
	  // RETRANSCRIT LES ARRANGEMENTS S'IL Y A AU MOINS UN SCENARIO POUR LE PEPTIDE COURANT
	  if (b > 0)
	    {
	      // D'ABORD LE TITRE, en ajoutant le = et le nombre d'arrangements
	      strcpy(line, title);
	      line[6] = '\0';
	      OUT2<<line<<"="<<b<<" "<<title+6<<endl;
	      for (int k = 0; k < b; k++) {OUT2<<"> "<<k<<buf[k]+1<<endl;}
	      OUT2<<";"<<endl;
	    }	   
	}
    }
  
  IN2.close(); IN2.clear();
  OUT2.close();OUT2.clear();
  for (int i = 0; i < MAX_ARRANGEMENT_NB; i++) {   delete[] buf[i];                               memCheck.fun--;}
  delete[] buf; buf = 0;                                                                          memCheck.fun--;
}

// ********************************************************************************************** //

void fun::countPeptides(char* name, int* NbPOS, int* NbNEG, int* NbSCEN) {
  char line[1024];
  (*NbSCEN) = 0;
  (*NbPOS)  = 0;
  (*NbNEG)  = 0;
  ifstream IN;
  IN.open(name); if (!IN) fatal_error(runManParam->FILE_ERROR_NAME, FILES, name); 
  while (IN.getline(line, 1023)) {
    if (line[strlen(line)-1] != ';') {fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function tagopop::countPeptides()");}
   
    if (!strncmp(line, "NEWPOS", 6)) {(*NbPOS)++;}
    if (!strncmp(line, "NEWNEG", 6)) {(*NbNEG)++;}
    if (!strncmp(line, ">", 1))      {(*NbSCEN)++;}
  }
  IN.close(); IN.clear();  
}

// ********************************************************************************************** //

void fun::display(File &fp)
{
  fprintf(fp, "\n");
  fprintf(fp, "Number of POS peptides : %i (%i with orphans)\n", POSAfter, POSBefore);
  fprintf(fp, "Number of NEG peptides : %i (%i with orphans)\n", NEGAfter, NEGBefore);
  fprintf(fp, "Number of scenarios :    %i \n", SCENAfter);
}

// ********************************************************************************************** //
