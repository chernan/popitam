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
#include <fstream>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cctype>  //isdigit()

#include "defines.h"
#include "data.h"
#include "memorycheck.h"
#include "error.h"



/******************************************************************************/

/* GLOBAL VARIABLE DECLARED IN SEQMSMS */

extern memorycheck         memCheck;

/******************************************************************************/


/* COMPARISON ROUTINE FOR QSORT() */

int compar_PEAKS(const void *a, const void *b) {
  if (((PEAKS*)a)->intensity < ((PEAKS*)b)->intensity)
    return 1;
  if (((PEAKS*)a)->intensity > ((PEAKS*)b)->intensity)
    return -1;
  return 0;}

// -----------------------------------------

int compar_PEAK_MASSES(const void *a, const void *b) {
  if (((PEAKS*)a)->mass > ((PEAKS*)b)->mass)
    return 1;
  if (((PEAKS*)a)->mass < ((PEAKS*)b)->mass)
    return -1;
  return 0;
}

/*
int compar_doubles(const void *a, const void *b) {
  // cast void* to double* and dereference the int pointer, apply difference
  if ((*(double *)a < *(double *)b)) return -1;
  if ((*(double *)a > *(double *)b)) return  1;
  return 0;
}
*/
/******************************************************************************/

/* CONSTRUCTOR */
data::data() 
{
  runManParam      = NULL;
  peakList         = NULL;
}

// ********************************************************************************************** //

// ********************************************************************************************** //

data::~data() {
  if (peakList           != NULL)   {delete[] peakList;           peakList = NULL;                memCheck.data--;}
}


// ********************************************************************************************** //


void data::init(runManagerParameters* rMP) {
  runManParam      = rMP;
  specID           = -1;
  peakList         = new PEAKS[MAX_INIT_PEAKS];                                                          memCheck.data++;
  initPeakNb       = 0;
  peakNb           = 0;
  parentMassRAW    = 0;
  parentMassM      = 0;
  charge           = 0;
  binsize          = 0;
  strcpy(title,      "");
  strcpy(seqSpec,    "");
  strcpy(seqAsInDtb, "");
}


// ********************************************************************************************** //


bool data::load()
{
  bool LOADED = false;

  initPeakNb       = 0;
  peakNb           = 0;
  parentMassRAW    = 0;
  parentMassM      = 0;
  charge           = 0;
  binsize          = 0;
  strcpy(title,      "");
  strcpy(seqSpec,    "");
  strcpy(seqAsInDtb, "");
  
  if (!strcmp(runManParam->FORMAT, "mgf")) LOADED = loadPeaksMGF(runManParam->FILEIN); 
  if (!strcmp(runManParam->FORMAT, "pop")) LOADED = loadPeaksPOP(runManParam->FILEIN);
  if (!strcmp(runManParam->FORMAT, "dta")) LOADED = loadPeaksDTA(runManParam->FILEIN);

  if (!LOADED) return false;
  
  preprocessPeaks();       
  putBins();
  
  if (runManParam->r_CHECK) {
    char filename[256] = "";
    sprintf(filename, "%s%s", runManParam->OUTPUT_DIR, "SOURCE_SPECTRUM_FINAL.txt");
    File fp;
    fp.Open(filename, "w");
    write(fp);
    fp.Close();
  }
  

  specID++;

  return true;
}

/******************************************************************************/

bool data::loadPeaksMGF(File &fp) 
{
	char line[256] = "";
	char tag[256]  = "";
	int  nb = 0;
	bool GOT_PEAKS = false;
 

	// PLACEMENT DEVANT LE PROCHAIN SPECTRE MSMS 
	while (fgets(line, 255, fp)) {
  	
		// look if it is the first time the function is called.
		// if this is the case, look for the global parameter CHARGE
		if (specID == -1) {
		
			if (!strncmp(line, "CHARGE", 6)) {
				strcpy(tag, line);
				int i = 0;
 				while((tag[i] != '=')  && (i < 255)) i++;
 				tag[i] = '\0';

				char temp[256] = "";
 				strcpy(temp, line+strlen(tag)+1);
 				runManParam->GLOBAL_CHARGE = atoi(temp);
				continue;
			}
		}
  	  	
 		if (!strncmp(line, "BEGIN IONS", 10)) break;
	}
  
  	while (fgets(line, 255, fp)) {
  		
    // REMOVE SPECIAL CHARACTERS
    int lenght = strlen(line);
	if ((line[strlen(line)-1] == '\n') | (line[strlen(line)-1] == '\r')) {
		while ((line[lenght-1] == '\n') | (line[lenght-1] == '\r')) {
			lenght--;
		}
	}
	line[lenght] = '\0';
   	

    // RECUPERE TITLE, PEPMASS ET CHARGE, puis les pics jusqu'à END ION

   if (isdigit(line[0])) {
	GOT_PEAKS = true;
	int j= 0;
    while (!((line[j] == ' ') | (line[j] == '\t'))) j++;
	
	char temp1[256], temp2[256];
	strcpy(temp2, line+j+1);
	line[j] = '\0';
	strcpy(temp1, line);
	peakList[nb].mass      = atof(temp1);
	peakList[nb].intensity = atof(temp2);
	peakList[nb].noBin     = 0;
	peakList[nb].used      = false; // important
	peakList[nb].CHARGE1   = false;
	peakList[nb].CHARGE2   = false;
	peakList[nb].CHARGE3   = false;
	nb++;
	if (nb >= MAX_INIT_PEAKS) fatal_error(runManParam->FILE_ERROR_NAME, DATAS, "too much peaks in spectrum");
   }    
   
   if (!strncmp(line, "TITLE", 5)) {
     strcpy(tag, line);
     int i = 0;
     while((tag[i] != '=')  && (i < 255)) i++;
     tag[i] = '\0';
     strcpy(title, line+strlen(tag)+1);
     continue;
   }
    if (!strncmp(line, "PEPMASS", 7)) {
      strcpy(tag, line);
      int i = 0;
      while((tag[i] != '=')  && (i < 255)) i++;
      tag[i] = '\0';

      char temp[256] = "";
      strcpy(temp, line+strlen(tag)+1);
      parentMassRAW = atof(temp);
      continue;
    }
    if (!strncmp(line, "CHARGE", 6)) {
      strcpy(tag, line);
      int i = 0;
      while((tag[i] != '=')  && (i < 255)) i++;
      tag[i] = '\0';

      char temp[256] = "";
      strcpy(temp, line+strlen(tag)+1);
      charge = atoi(temp);
      continue;
    }
    if (!strncmp(line, "END IONS",8)) {
      initPeakNb = peakNb = nb;
      break;
    }
  }
  
  if (GOT_PEAKS) 
    {
      if (charge == 0)
      {
      	charge = runManParam->GLOBAL_CHARGE;
  		if (runManParam->GLOBAL_CHARGE == 0) fatal_error(runManParam->FILE_ERROR_NAME, DATAS, "Spectrum charge is missing");
      }
      parentMassM = (parentMassRAW * charge) - charge;
      
      strcpy(seqSpec, "");
      strcpy(seqAsInDtb, "");
    }

  return GOT_PEAKS;
}


/*******************************************************************************/

bool data::loadPeaksPOP(File &fp) 
{
  char line[256] = "";
  char tag[256]  = "";
  int  nb = 0;
  bool GOT_PEAKS = false;

  while (fgets(line, 255, fp)) {
     // REMOVE SPECIAL CHARACTERS
    int lenght = strlen(line);
	if ((line[strlen(line)-1] == '\n') | (line[strlen(line)-1] == '\r')) {
		while ((line[lenght-1] == '\n') | (line[lenght-1] == '\r')) {
			lenght--;
		}
	}
	line[lenght] = '\0';

    // RECUPERE TITLE, PEPMASS ET CHARGE, puis les pics jusqu'à END ION

   if (isdigit(line[0])) 
      {
	GOT_PEAKS = true;
	int j= 0;
	while (!((line[j] == ' ') | (line[j] == '\t'))) j++;
	
	char temp1[256], temp2[256];
	strcpy(temp2, line+j+1);
	line[j] = '\0';
	strcpy(temp1, line);
	peakList[nb].mass      = atof(temp1);
	peakList[nb].intensity = atof(temp2);
	peakList[nb].noBin     = 0;
	peakList[nb].used      = false; // important
	peakList[nb].CHARGE1   = false;
	peakList[nb].CHARGE2   = false;
	peakList[nb].CHARGE3   = false;
	nb++;
	if (nb >= MAX_INIT_PEAKS) fatal_error(runManParam->FILE_ERROR_NAME, DATAS, "too much peaks in spectrum");
      }    
   
   if (!strncmp(line, "TITLE", 5)) {
     strcpy(tag, line);
     int i = 0;
     while((tag[i] != '=')  && (i < 255)) i++;
     tag[i] = '\0';
     strcpy(title, line+strlen(tag)+1);
     continue;
   }
    if (!strncmp(line, "PEPMASS", 7)) {
      strcpy(tag, line);
      int i = 0;
      while((tag[i] != '=')  && (i < 255)) i++;
      tag[i] = '\0';

      char temp[256] = "";
      strcpy(temp, line+strlen(tag)+1);
      parentMassRAW = atof(temp);
      continue;
    }
    if (!strncmp(line, "CHARGE", 6)) {
      strcpy(tag, line);
      int i = 0;
      while((tag[i] != '=')  && (i < 255)) i++;
      tag[i] = '\0';

      char temp[256] = "";
      strcpy(temp, line+strlen(tag)+1);
      charge = atoi(temp);
      continue;
    }
    if (!strncmp(line, "#SEQ_SPEC", 9)) {
      strcpy(tag, line);
      int i = 0;
      while((tag[i] != '=')  && (i < 255)) i++;
      tag[i] = '\0';
      strcpy(seqSpec, line+strlen(tag)+1);
      if (seqSpec[strlen(seqSpec)-1]== '\r') seqSpec[strlen(seqSpec)-1] = '\0';
      continue;
    }
    if (!strncmp(line, "#SEQ_AS_IN_DTB", 14)) {
      strcpy(tag, line);
      int i = 0;
      while((tag[i] != '=')  && (i < 255)) i++;
      tag[i] = '\0';
      strcpy(seqAsInDtb, line+strlen(tag)+1);
      if (seqAsInDtb[strlen(seqAsInDtb)-1]== '\r') seqAsInDtb[strlen(seqAsInDtb)-1] = '\0';
      continue;
    }
    
    if (!strncmp(line, "END IONS",8)) {
      initPeakNb = peakNb = nb;
      break;
    }
  }
  
  if (GOT_PEAKS) 
    {
      parentMassM = (parentMassRAW * charge) - charge;
      
      if (strlen(seqSpec) == 0) strcpy(seqSpec, seqAsInDtb);  // PAS DE MODIFICATION
      if ((strlen(seqAsInDtb) == 0) && runManParam->s_IDSET)  fatal_error(runManParam->FILE_ERROR_NAME, MODE, "in IDSET mode, spectra must be identified by their theoretical sequence");
    }
  
  return GOT_PEAKS;
}


/******************************************************************************/


bool data::loadPeaksDTA(File &fp) 
{
  char line[256] = "";
  char tag[256]  = "";
  bool GOT_PEAKS = false;

  int nb = 0;
 
  // on enlève tous les éventuels blancs qui précèderaient un spectre
  
  while (fgets(line, 255, fp))
    {

      if (!isdigit(line[0])) {
	while (!isdigit(line[0])) {
	  if (!fgets(line, 255, fp)) break;
	}
      }
     
    // REMOVE SPECIAL CHARACTERS
    int lenght = strlen(line);
	if ((line[strlen(line)-1] == '\n') | (line[strlen(line)-1] == '\r')) {
		while ((line[lenght-1] == '\n') | (line[lenght-1] == '\r')) {
			lenght--;
		}
	}
	line[lenght] = '\0';
      
      // ici, line doit contenir la première ligne d'un spectre.
      strcpy(tag, line);
	    
      char temp[256] = "";
      parentMassM = atof(tag)-1.0;
      
      int i = 0;
      while((tag[i] != ' ')  && (i < 255)) i++;
      tag[i] = '\0';
      
      strcpy(temp, line+strlen(tag)+1);
      charge = atoi(temp);
      
      parentMassRAW = (parentMassM + charge) / (float)charge;
      sprintf(title, "%f_%i", parentMassM, charge);
	
      // lecture des pics jusqu'au premier blanc ou jusqu'à la fin
      while (fgets(line, 255, fp))
	{
			// remove special characters
    	int lenght = strlen(line);
		if ((line[strlen(line)-1] == '\n') | (line[strlen(line)-1] == '\r')) 
		{
			while ((line[lenght-1] == '\n') | (line[lenght-1] == '\r')) 
			{
				lenght--;
			}
		}
		line[lenght] = '\0';
	  if (!isdigit(line[0])) break;
	  GOT_PEAKS = true;
	  int j= 0;
      while (!((line[j] == ' ') | (line[j] == '\t'))) j++;
	  
	  char temp1[256], temp2[256];
	  strcpy(temp2, line+j+1);
	  line[j] = '\0';
	  strcpy(temp1, line);
	  peakList[nb].mass      = atof(temp1);
	  peakList[nb].intensity = atof(temp2);
	  peakList[nb].noBin     = 0;
	  peakList[nb].used      = false; // important
	  peakList[nb].CHARGE1   = false;
	  peakList[nb].CHARGE2   = false;
	  peakList[nb].CHARGE3   = false;
	  nb++;
	  if (nb >= MAX_INIT_PEAKS) fatal_error(runManParam->FILE_ERROR_NAME, DATAS, "too much peaks in spectrum");
	} 
      if (GOT_PEAKS) break;
    }

  initPeakNb = peakNb = nb;
  
  if (GOT_PEAKS)
    {
      strcpy(seqSpec, "");
      strcpy(seqAsInDtb, "");
      if (runManParam->s_IDSET)  fatal_error(runManParam->FILE_ERROR_NAME, MODE, "can't use dta format in IDSET mode");
    }

  return GOT_PEAKS;
}

/******************************************************************************/

void data::preprocessPeaks() {
  
  // trie les pics par masse (ce qu'ils sont sensé déjà être, mais juste au cas où
  qsort(peakList, peakNb, sizeof(PEAKS), compar_PEAK_MASSES);
  
  // je commence par marquer la masse parente et ses isotopes
  markParentMass();
  
  // trie les pics par intensité pour la normalisation et l'enlèvement
  // des pics de basse intensité
  qsort(peakList, peakNb, sizeof(PEAKS), compar_PEAKS);
  normInt();
  
  // vire les pics qui ont une intensité en dessous du seuil de PEAK_INT_SEUIL
  for (int i = 0; i < peakNb; i++) {if (peakList[i].intensity < runManParam->PEAK_INT_SEUIL) {peakNb = i; break;}}
  
  // attribution des charges à considérer pour chaque pic
  for (int i = 0; i < peakNb; i++) {
    if ((peakList[i].mass - 4 > (parentMassM+2)/2) && (peakList[i].mass - 4 < parentMassM)) {
      // le +4, c'est pour que tous les isotopes d'un pic parent soient classés comme
      // le pic parent, même s'ils sont à +1, +2 ou +3...
      peakList[i].CHARGE1     = true;
    }
    if ((peakList[i].mass - 4 > (parentMassM+3)/3) && (peakList[i].mass - 4 < (parentMassM+2)/2)) {
      peakList[i].CHARGE1     = true; 
      peakList[i].CHARGE2     = true;
    }
    if (peakList[i].mass - 4 < (parentMassM+3)/3) {
      peakList[i].CHARGE1     = true; 
      peakList[i].CHARGE2     = true; 
      peakList[i].CHARGE3     = true;
    }
  }

  // trie les pics par masse pour le controle des isotopes
  qsort(peakList, peakNb, sizeof(PEAKS), compar_PEAK_MASSES);
  
  // vire tous les pics isopopiques qui ne sont pas le premier isotope
  for (int i = 0; i < peakNb; i++) {
    for (int j = i+1; j < peakNb; j++) {
      
      // recherche d'isotopes à +0.333, +0.5, +1 selon les charges à considérer pour le pic courant
      // qui dépendent de la région du spectre où se trouve le pic
    
      // +0.333
      if (peakList[i].CHARGE3 == true) {
	if ((fabs((peakList[j].mass - peakList[i].mass) - 0.333) < runManParam->FRAGMENT_ERROR1) && (peakList[j].intensity < peakList[i].intensity))  
	  // je prends l'erreur 1 car je compare des isotopes
	  // exiger que l'intensité soit plus petite fait qu'on va manquer d'oter certains isotopes (hautes masses)
	  // mais je préfère en garder trop que pas assez
	  peakList[j].used = true;
      }
      
      // +0.5
      if (peakList[i].CHARGE2 == true) {
	if ((fabs((peakList[j].mass - peakList[i].mass) - 0.5) < runManParam->FRAGMENT_ERROR1) && (peakList[j].intensity < peakList[i].intensity))
	  peakList[j].used = true;
      }
      
      // +1
      if (peakList[i].CHARGE1 == true) {
	if ((fabs((peakList[j].mass - peakList[i].mass) - 1) < runManParam->FRAGMENT_ERROR1) && (peakList[j].intensity < peakList[i].intensity))
	  peakList[j].used = true;
      }
      
      if (peakList[j].mass - peakList[i].mass > 4) break;
    }
  }
  
  // vire les pics doubles (je garde à chaque fois le pic de plus grande intensité)
  for (int i = 0; i < peakNb; i++) {
    for (int j = i+1; j < peakNb; j++) {
      if (fabs(peakList[j].mass - peakList[i].mass) < runManParam->FRAGMENT_ERROR1) {
	// repère le pic de + grande intensité
	if (peakList[i].intensity > peakList[j].intensity) peakList[j].used = true;
	else                                           peakList[i].used = true;
      }
    }
  }
}


// ********************************************************************************************** //

void  data::markParentMass() {
  for (int i = 0; i < peakNb; i++) {
    if (fabs(peakList[i].mass - parentMassRAW) < runManParam->FRAGMENT_ERROR2) {
      peakList[i].used = true;
      
      // vire tous les pics isopopiques de la masse trouvée (afin que leur intensité
      // ne soit pas prise en compte dans la normalisation)
      
      for (int j = i+1; j < peakNb; j++) {
	
	// recherche d'isotopes à +0.333, +0.5, +1 selon les charges à considérer pour le pic courant
	// qui dépendent de la région du spectre où se trouve le pic
	
	// +0.333
	if (charge == 3) {
	  if (fabs((peakList[j].mass - peakList[i].mass) - 0.333) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 0.666) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 1)    < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 1.333) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 1.666)  < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 2)     < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	}
	
	// +0.5
	if (charge == 2) {
	  if (fabs((peakList[j].mass - peakList[i].mass) - 0.5) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 1)   < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 1.5) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 2)   < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 2.5) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 3)   < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	}
	
	// +1
	if (charge == 1) {
	  if (fabs((peakList[j].mass - peakList[i].mass) - 1) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 2) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 3) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 4) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 5) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	  if (fabs((peakList[j].mass - peakList[i].mass) - 6) < runManParam->FRAGMENT_ERROR1)
	    peakList[j].used = true;
	}
	
	if (peakList[j].mass - peakList[i].mass > 7) break;
      }
    }
  }
}

// ********************************************************************************************** //


void data::normInt() {

  // les pics sont triés par intensité, mais il se peut que le premier (et quelques
  // suivants) soient la masse parente ou des isotopes de la masse parente.
  // Donc je prend comme intensité la plus élevée le premier pic qui n'a pas été
  // marqué comme masse parente ou isotope de la masse parente
 
  double max=0.0;
  int     i = 0;

/*  while (true && (i < peakNb)) {
    if (peakList[i].used == false) {max = peakList[i].intensity; break;}
    i++;
  } replaced by : */
  for (i = 0; i < peakNb; ++i) {
    if (!peakList[i].used) {
    	max = peakList[i].intensity; 
    	break;
    }
  }

  for (i = 0; i < peakNb; i++) 
    peakList[i].intensity = (100.0 * peakList[i].intensity) / max ;
}  

// ********************************************************************************************** //

void data::putBins() {
  
  binsize = int(parentMassM / 100);  
  
  // trie les pics par intensité
  qsort(peakList, peakNb, sizeof(PEAKS), compar_PEAKS);

  int b = -1;
  for (int i = 0; i < peakNb; i++) {
    if (i%binsize == 0) b++;
    if (b > runManParam->BIN_NB-1) b = runManParam->BIN_NB-1;
    peakList[i].noBin = b;
  }

  qsort(peakList, peakNb, sizeof(PEAKS), compar_PEAK_MASSES);
}

// ********************************************************************************************** //

void data::write(File &fp) {
  
  fprintf(fp, "List of Peaks\n-----------------------------------------------\n");
  fprintf(fp, "%-15s: %i\n", "Number of peaks", peakNb);
  fprintf(fp, "%-15s: %f\n", "Parent_mass_RAW", parentMassRAW);
  fprintf(fp, "%-15s: %i\n", "Charge", charge);
  fprintf(fp, "\n-----------------------------------------------\n");
  fprintf(fp, "%-15s\n", "a) mass");
  fprintf(fp, "%-15s\n", "b) intensity");
  fprintf(fp, "%-15s\n", "c) noBin");
  fprintf(fp, "%-15s\n\n\n", "d) e) f) charge1, charge2, charge3");
  
  for (int i = 0; i < peakNb; i++) {
    fprintf(fp, "%5i", i);
    fprintf(fp, "%10.4f", peakList[i].mass);
    fprintf(fp, "%10.4f", peakList[i].intensity);
    fprintf(fp, "%5i",    peakList[i].noBin);
    fprintf(fp, "%3i",    peakList[i].CHARGE1);
    fprintf(fp, "%3i",    peakList[i].CHARGE2);
    fprintf(fp, "%3i\n",  peakList[i].CHARGE3);
  } 
  
} 

// ********************************************************************************************** //

void data::writeSpectrumListXML(File &fp) {
  fprintf(fp, "  <spectrum id=\"%i\">\n", specID);
  fprintf(fp, "  <peakList>\n<![CDATA[");
  
  for (int i = 0; i < peakNb; i++) {
    fprintf(fp, "%f ", peakList[i].mass);
    fprintf(fp, "%f ", peakList[i].intensity);
    fprintf(fp, "?\n");
  } 
   fprintf(fp, "\n]]></peakList>\n");
   fprintf(fp, "  </spectrum>\n");
} 

// ********************************************************************************************** //

void data::display(File& fp)
{
  fprintf(fp,"%-25s: %i\n", "Spectrum ID", specID);
  fprintf(fp,"%-25s: %s\n", "SpectrumTitle", title);
  fprintf(fp,"%-25s: %11i\n", "initPeakNb", initPeakNb);
  fprintf(fp,"%-25s: %11i\n", "peakNb after proc.", peakNb);
  fprintf(fp,"%-25s: %11.5f\n", "ParentMass raw", parentMassRAW);
  fprintf(fp,"%-25s: %11.5f / %-7i\n", "ParentMass(M)/Charge", parentMassM, charge);
 
}

// ********************************************************************************************** //

void data::displayXML(File& fp)
{
  fprintf(fp, "    <spectrumMatch ref=\"%i\">\n", specID);
  fprintf(fp, "      <title>%s</title>\n", title);
  fprintf(fp, "      <initPeakNb>%i</initPeakNb>\n", initPeakNb);
  fprintf(fp, "      <peakNb>%i</peakNb>\n", peakNb);
  fprintf(fp, "      <precursor>\n");
  fprintf(fp, "        <moz>%f</moz>\n", parentMassRAW);
  fprintf(fp, "        <mass>%f</mass>\n", parentMassM);
  fprintf(fp, "        <charge>%i</charge>\n", charge);
  fprintf(fp, "      </precursor>\n");
}

// ********************************************************************************************** //
