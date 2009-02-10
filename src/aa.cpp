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

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <cstdio>

#include "defines.h"
#include "aa.h"
#include "memorycheck.h"
#include "error.h"


using namespace std;


// ********************************************************************************************** //

/* GLOBAL VARIABLES DECLARED ELSEWHERE */

extern memorycheck memCheck;


// ********************************************************************************************** //


/* COMPARISON ROUTINES FOR QSORT() */

int compar_TAB2(const void *a, const void *b) {
  if (((TAB2*)a)->mass < ((TAB2*)b)->mass)
    return -1;
  if (((TAB2*)a)->mass > ((TAB2*)b)->mass)
    return 1;
  return 0;
}

// ********************************************************************************************** //

int compar_TAB(const void *a, const void *b) {
  if (((TAB*)a)->mass < ((TAB*)b)->mass)
    return -1;
  if (((TAB*)a)->mass > ((TAB*)b)->mass)
    return 1;
  return 0;
}

// ********************************************************************************************** //

/* CONSTRUCTEUR */

aa::aa() {
  strcpy(aaFile, "");
  runManParam    = NULL;
  aaTable        = NULL;
  aa2Table       = NULL;
};

// ********************************************************************************************** //

void aa::init(runManagerParameters* rMP) {
  
  runManParam    = rMP;
  
  strcpy(aaFile, runManParam->AMINO_ACID_INFILE);
  
  build_aaTable();
  qsort(aaTable, aa1Nb, sizeof(TAB), compar_TAB);
  for (int i = 0; i < aa1Nb; i++)  aaTable[i].indice = i;
  
  build_aa2Table();
  qsort(aa2Table, aa2Nb, sizeof(TAB2), compar_TAB2);
  for (int i = 0; i < aa2Nb; i++)  aa2Table[i].indice = i;  

  if (runManParam->r_CHECK) {
    char filename[256] = "";
    sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "AMINO_ACIDS_sorted.txt");
    File fp;
    fp.Open(filename, "w");
    write(fp);
    fp.Close();
  }
}

// ********************************************************************************************** //

/* DESTRUCTEUR */

aa::~aa() 
{
  if (aaTable  != NULL) {delete[] aaTable;  aaTable  = NULL;                                     memCheck.aa--;}
  if (aa2Table != NULL) {delete[] aa2Table; aa2Table = NULL;                                     memCheck.aa--;}
}

// ********************************************************************************************** //


/* FONCTIONS MEMBRE */


void aa::build_aaTable() {
  ifstream fin;
  fin.open(aaFile);
  if (!fin) fatal_error(runManParam->FILE_ERROR_NAME, FILES, aaFile);
  
  char line[256];
  fin>>aa1Nb; fin.getline(line, 255);
  aaTable = new TAB[aa1Nb+1];                                                                    memCheck.aa++;

  aaTable[aa1Nb].mass = 1000;         // ceci est une astuce utilisée lors de la connection du graphe (ENCORE D'ACTUALITE?)
  strcpy(aaTable[aa1Nb].code, "~");   // l'acide aminé supplémentaire est aussi important pour le suffixtree!!!!!! j'y mets le caractère ~
  strcpy(aaTable[aa1Nb].name, "~");

  for (int i = 0; i < aa1Nb; i++){   
    fin>>aaTable[i].code[0];
    aaTable[i].code[1] = '\0';
    for (int n = 0; n < 3; n++) {
      fin>>aaTable[i].name[n];
      aaTable[i].name[n] = toupper(aaTable[i].name[n]);
    }
    aaTable[i].name[3] = '\0';
    fin>>aaTable[i].mass; fin.getline(line, 255);
  }
  
  // il reste les masses individuelles à prendre
  int t = 0;
  fin.getline(line,255);
  while (line[t] != '=') t++;
  H_mass = (float)atof(line+t+1);
  t = 0;
  fin.getline(line, 255);
  while (line[t] != '=') t++;
  C_mass = (float)atof(line+t+1);
  t = 0;
  fin.getline(line, 255);
  while (line[t] != '=') t++;
  N_mass = (float)atof(line+t+1);
  t = 0;
  fin.getline(line, 255);
  while (line[t] != '=') t++;
  O_mass = (float)atof(line+t+1);

  fin.close();


}


// ********************************************************************************************** //


void aa::build_aa2Table() {
 
  aa2Nb = aa1Nb * aa1Nb;
  aa2Table = new TAB2[aa2Nb+1];                                                                  memCheck.aa++;
  aa2Table[aa2Nb].mass = 1000;  // ceci est une astuce utilisée lors de la connection du graphe
  int k = 0;
  
  for (int i = 0; i < aa1Nb; i++) {
    for (int j = 0; j < aa1Nb; j++) {
      aa2Table[k].indice1 = i;
      aa2Table[k].indice2 = j;
      aa2Table[k].mass = aaTable[i].mass + aaTable[j].mass;

      aa2Table[k].code[0] = aaTable[i].code[0];
      aa2Table[k].code[1] = aaTable[j].code[0];
      aa2Table[k].code[2] = '\0';

      strcpy(aa2Table[k].name, "");
      strcat(aa2Table[k].name, aaTable[i].name);
      strcat(aa2Table[k].name, aaTable[j].name);
      aa2Table[k].name[6] = '\0';

      // LL: what does this loop accomplish?
      for (int l = 0; l < 6; l++)
	aa2Table[k].name[l] = aa2Table[k].name[l];

      k++;
    }
  }


}


// ********************************************************************************************** //

void aa::write(File &fp) {
  
  fprintf(fp, "AMINO ACID MASSES (aa1Nb = %i)\n\n", aa1Nb);
  fprintf(fp, "%s%s%s%s", 
	  "a) indice \n",
	  "b) mass \n", 
	  "c) 1 letter code \n",
	  "d) 3 letters code \n\n");

  
  for (int i = 0; i < aa1Nb; i++) {
    fprintf(fp, "%4i %10.6f %s %s\n", 
	    aaTable[i].indice, 
	    aaTable[i].mass, 
	    aaTable[i].code, 
	    aaTable[i].name);
  }

   
  fprintf(fp, "\n\n");
  fprintf(fp, "AMINO ACID MASSES (aa2Nb = %i)\n\n", aa2Nb);
  fprintf(fp, "%s%s%s%s%s%s", 
	                         "a) indice\n", 
	                         "b) indiceAA1\n", 
	                         "c) indiceAA2\n",
                                 "d) mass\n",
                                 "e) 1 letter code\n",
	                         "f) 3 letters code\n\n");

  for (int i = 0; i < aa2Nb; i++) 
    {
      fprintf(fp, "%4i (%3i/%3i) %10.6f %s %s\n", 
	      aa2Table[i].indice,  
	      aa2Table[i].indice1, 
	      aa2Table[i].indice2, 
	      aa2Table[i].mass, 
	      aa2Table[i].code, 
	      aa2Table[i].name);
    }

  fprintf(fp, "H_mass: %.6f\nC_mass: %.6f\nN_mass: %.6f\nO_mass: %.6f", H_mass, C_mass, N_mass, O_mass);
  
}  

/******************************************************************************/

int aa::findIndice(char car) {
  int i;
  car = toupper(car);
  for (i = 0; i < aa1Nb; i++) {
  	if (aaTable[i].code[0] == car) return i;
  }
  return -1;
}

/******************************************************************************/
