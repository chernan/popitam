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

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "aa.h"
#include "ion.h"

#include "params.h"
#include "aa.h"
#include "error.h"
#include "defines.h"

using namespace std;

/******************************************************************************/

// GLOBAL VARIABLES DECLARED ELSEWHERE 



/******************************************************************************/

/* COMPARISON ROUTINE FOR QSORT */

int compar_PROBSALL(const void *a, const void *b) {
  if (((TRIALL*)a)->prob < ((TRIALL*)b)->prob)
    return 1;
  if (((TRIALL*)a)->prob > ((TRIALL*)b)->prob)
    return -1;
  return 0;
}


int compar_PROBSBIN(const void *a, const void *b) {
  if (((TRIBIN*)a)->prob < ((TRIBIN*)b)->prob)
    return 1;
  if (((TRIBIN*)a)->prob > ((TRIBIN*)b)->prob)
    return -1;
  return 0;
}



/******************************************************************************/


/* CONSTRUCTEUR */

ion::ion() 
{
  totIonNb    = 0;
  NionNb      = 0;
  CionNb      = 0;
  binNb       = 0;
}


void ion::init(char* fileErrorName, char* filename, aa* aP) 
{  
  strcpy(ionFile,filename);
  readInputIons(fileErrorName);
  reComputeDeltas(aP);
  sortProbs();
}


/******************************************************************************/

/* DESTRUCTEUR */

ion::~ion() 
{
}

/******************************************************************************/

void  ion::readInputIons(char* fileErrorName) {

  ifstream fin;
  fin.open(ionFile); if (!fin) fatal_error(fileErrorName, FILES, ionFile);
  
  char        line[1000], name[256], temp[16];
  int     i = 0;

  
  while (fin.getline(line, 999)) {
  	    // REMOVE SPECIAL CHARACTERS
    int lenght = strlen(line);
	if ((line[strlen(line)-1] == '\n') | (line[strlen(line)-1] == '\r')) {
		while ((line[lenght-1] == '\n') | (line[lenght-1] == '\r')) {
			lenght--;
		}
	}
	line[lenght] = '\0';
  	
    if (!strncmp(line, "BIN_NB", 6)) {
      i = 0;
      while (line[i]!= '=') i++;
      strcpy(temp, line+6+1);
      binNb = atoi(temp);
    }
    
    if (!strcmp(line, "BEGIN_N_TERM_CHARGED1")) {       // début des ions N-term
      while (true) {
		fin>>name;
		if (!strcmp(name, "END")) break;
	
		strcpy(ions[NionNb].name, name);
		fin>>ions[NionNb].delta;
		fin>>ions[NionNb].probs[binNb];  // lit la prob. totale
		for (int n = 0; n < binNb; n++) 
			fin>>ions[NionNb].probs[n];
	    ions[NionNb].charge   = 1;
		ions[NionNb].term    = 'N';
		fin.getline(line, 999);
		NionNb ++; 
		if (NionNb > MAX_IONS) fatal_error(fileErrorName, MEMORY, "in ion()");
      }
    }

 if (!strcmp(line, "BEGIN_N_TERM_CHARGED2")) {       // début des ions N-term
      while (true) {
	fin>>name;
	if (!strcmp(name, "END")) break;
	
	strcpy(ions[NionNb].name, name);
	fin>>ions[NionNb].delta;
	fin>>ions[NionNb].probs[binNb];
	for (int n = 0; n < binNb; n++) fin>>ions[NionNb].probs[n];
	ions[NionNb].charge       =  2;
	ions[NionNb].term         = 'N';
	fin.getline(line, 999);
	NionNb ++; 
  	if (NionNb > MAX_IONS) fatal_error(fileErrorName, MEMORY, "in ion()");
      }
    }

    if (!strcmp(line, "BEGIN_C_TERM_CHARGED1")) {  
      while (true) {
	fin>>name;
	if (!strcmp(name, "END")) break;

	strcpy(ions[NionNb+CionNb].name, name);
	fin>>ions[NionNb+CionNb].delta;
	fin>>ions[NionNb+CionNb].probs[binNb];
	for (int n = 0; n < binNb; n++) fin>>ions[NionNb+CionNb].probs[n];
	ions[NionNb+CionNb].charge = 1;
	ions[NionNb+CionNb].term   = 'C';
	fin.getline(line, 999);
	CionNb++;   
	if (NionNb+CionNb > MAX_IONS) fatal_error(fileErrorName, MEMORY, "in ion()");
      }
    }

    if (!strcmp(line, "BEGIN_C_TERM_CHARGED2")) {  
      while (true) {
	fin>>name;
	if (!strcmp(name, "END")) break;

	strcpy(ions[NionNb+CionNb].name, name);
	fin>>ions[NionNb+CionNb].delta;
	fin>>ions[NionNb+CionNb].probs[binNb];
	for (int n = 0; n < binNb; n++) fin>>ions[NionNb+CionNb].probs[n];
	ions[NionNb+CionNb].charge = 2;
	ions[NionNb+CionNb].term   = 'C';
	fin.getline(line, 999);
	CionNb++;
	if (NionNb+CionNb > MAX_IONS) fatal_error(fileErrorName, MEMORY, "in ion()");
      }
    }

  }
  totIonNb = NionNb + CionNb;
  
  fin.close();
}
  
/******************************************************************************/

void ion::reComputeDeltas(aa* aP) {
  for (int i =0; i < totIonNb; i++) {
    if ((!strcmp(ions[i].name, "a"))  | (!strcmp(ions[i].name, "a++")))  {
      ions[i].delta   =    -aP->C_mass - aP->O_mass; continue;
    }
    if ((!strcmp(ions[i].name, "a°")) | (!strcmp(ions[i].name, "a°++"))) {
      ions[i].delta   =    -aP->C_mass - aP->O_mass - 2*aP->H_mass - aP->O_mass ; continue;
    }    
    if ((!strcmp(ions[i].name, "a*")) | (!strcmp(ions[i].name, "a*++"))) {
      ions[i].delta   =    -aP->C_mass - aP->O_mass - aP->N_mass - 3*aP->H_mass; continue;
    }
    if ((!strcmp(ions[i].name, "b°")) | (!strcmp(ions[i].name, "b°++"))) {
      ions[i].delta   =  -2*aP->H_mass - aP->O_mass; continue;
    }    
    if ((!strcmp(ions[i].name, "b*")) | (!strcmp(ions[i].name, "b*++")))   {
      ions[i].delta   =    -aP->N_mass - 3*aP->H_mass; continue;
    }  
    if ((!strcmp(ions[i].name, "y"))  | (!strcmp(ions[i].name, "y++")))  {
      ions[i].delta   =   2*aP->H_mass; continue;
    }
    if ((!strcmp(ions[i].name, "y°")) | (!strcmp(ions[i].name, "y°++"))) {
      ions[i].delta   =   2*aP->H_mass - 2*aP->H_mass - aP->O_mass ; continue;
    }    
    if ((!strcmp(ions[i].name, "y*")) | (!strcmp(ions[i].name, "y*++"))) {
      ions[i].delta   =   2*aP->H_mass - aP->N_mass - 3*aP->H_mass; continue;
    }
  }
}


/******************************************************************************/

void ion::sortProbs() {
  // remplissage du tableau linéaire triAll qui va contenir toutes les probs pour les 10 bins
  for (int i = 0; i < totIonNb; i++) {
    triAll[i].prob = ions[i].probs[binNb];
    triAll[i].iIon = i;
  }

  // remplissage du tableau linéaire triBin qui va contenir toutes les probs pour chaque bin
  int k = 0;
  for (int i = 0; i < totIonNb; i++) {
    for (int j = 0; j < binNb; j++) {
      triBin[k].prob = ions[i].probs[j];
      triBin[k].iIon = i;
      triBin[k].jBin = j;
      k++;
    }
  }

  // tri des tableaux selon les probs
  qsort(triAll, totIonNb, sizeof(TRIALL), compar_PROBSALL);
  qsort(triBin, totIonNb * binNb, sizeof(TRIBIN), compar_PROBSBIN);
}

/******************************************************************************/

void ion::write(File &fp) {
  

  fprintf(fp, "ion_param : \n\n");
  fprintf(fp, "N-term : \n");
  for (int i = 0; i < NionNb; i++) {
    fprintf(fp, "%s\t%f\t%i\t", ions[i].name, ions[i].delta, ions[i].charge);
    for (int j = 0; j < binNb+1; j++) {
      fprintf(fp, "%f\"%i\t", ions[i].probs[j], j);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n C-term : \n");
  for (int i = NionNb; i < NionNb + CionNb; i++) {
    fprintf(fp, "%s\t%f\t%i\t", ions[i].name, ions[i].delta, ions[i].charge);
    for (int j = 0; j < binNb+1; j++) {
      fprintf(fp, "%f\"%i\t", ions[i].probs[j], j);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
}

/******************************************************************************/

void ion::writeSorted(File &fp) {
 
  
  fprintf(fp, "LIST OF ION TYPES SORTED BY PROBABILITY VALUE\n\n");
  fprintf(fp, "ALL BINS\n");
  for (int i = 0; i < totIonNb; i++) {
    fprintf(fp, "%s\t%i\t%c\t", ions[triAll[i].iIon].name,  ions[triAll[i].iIon].charge, ions[triAll[i].iIon].term);
    fprintf(fp, "%f\t%f\n",     ions[triAll[i].iIon].delta, ions[triAll[i].iIon].probs[binNb]);
  }
  fprintf(fp, "BY BIN\n");
  for (int i = 0; i < totIonNb * binNb; i++) {
    fprintf(fp, "%s\t%i\t%c\t",     ions[triBin[i].iIon].name,  ions[triBin[i].iIon].charge,  ions[triBin[i].iIon].term);
    fprintf(fp, "%f\t%i\t%f\n",   ions[triBin[i].iIon].delta, triBin[i].jBin, ions[triBin[i].iIon].probs[triBin[i].jBin]);
  }
}



/******************************************************************************/
