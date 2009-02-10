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
#include <cmath>
#include <cctype>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#include "subseq.h"
#include "memorycheck.h"
#include "utils.h"
#include "error.h"

using namespace std;

/******************************************************************************/

/* GLOBAL VARIABLES DECLARED ELSEWHERE */

extern memorycheck memCheck; 


// ********************************************************************************************** //

int compar_subSeq_iNodeNb(const void *a, const void *b) {
  if (((SUBSEQ*)a)->iNodeNb < ((SUBSEQ*)b)->iNodeNb) return  1;
  if (((SUBSEQ*)a)->iNodeNb > ((SUBSEQ*)b)->iNodeNb) return -1;
                                                     return  0;
}

int compar_subSeq_state(const void *a, const void *b) {
  if (((SUBSEQ*)a)->state > ((SUBSEQ*)b)->state) return  1;
  if (((SUBSEQ*)a)->state < ((SUBSEQ*)b)->state) return -1;
                                                 return  0;
}

// ********************************************************************************************** //


subseq::subseq() 
{
  spectrumData = NULL;
  aaParam      = NULL;
}

//---------------------------------------------------------------------------------------------//

subseq::~subseq() {  
  
  spectrumData = NULL;
  aaParam      = NULL;

  for (int i = 0; i < subSeqNb; i++) {
    if (subSeqTab[i].or_pL != NULL)   {delete[] subSeqTab[i].or_pL; subSeqTab[i].or_pL = NULL;      memCheck.subseq--;}
    if (subSeqTab[i].or_nL != NULL)   {delete[] subSeqTab[i].or_nL; subSeqTab[i].or_nL = NULL;      memCheck.subseq--;}
  }
  
}

/******************************************************************************/

void subseq::init(runManagerParameters* rMP, aa* aaP, data* spectrumD, ion* ionP) 
{
  runManParam   = rMP;
  aaParam       = aaP;
  ionParam      = ionP;
  spectrumData  = spectrumD;

  subSeqNb = 0;
  //  minLenght = int((spectrumData->get_parentMassM()/111)/MIN_TAG_LENGHT_RATIO);
  //  if (minLenght < 3) minLenght = 3;
  //  if (minLenght > 5) minLenght = 5;
  minLenght = runManParam->MIN_TAG_LENTGH;

 
}

//---------------------------------------------------------------------------------------------//

void subseq::initSubSeqTab(int i, int peakNb, int orV) {
  subSeqTab[i].posBegin           = -1;
  subSeqTab[i].lenght             =  0;
  subSeqTab[i].massBegin          =  0;
  subSeqTab[i].massEnd            =  0;
  subSeqTab[i].state              = -1;
  subSeqTab[i].iNodeNb            =  0;

  strcpy(subSeqTab[i].parsedSeq, "");
  subSeqTab[i].or_pL = new int[peakNb];                                                          memCheck.subseq++;  
  subSeqTab[i].or_nL = new int[orV];                                                             memCheck.subseq++; 

  memset(subSeqTab[i].or_pL, 0, peakNb*sizeof(int));
  memset(subSeqTab[i].or_nL, 0, orV*sizeof(int));
}

// ********************************************************************************************* //

void subseq::freeSubSeqTab() {
    for (int i = 0; i < subSeqNb; i++) {
      if (subSeqTab[i].or_pL != NULL) {delete[] subSeqTab[i].or_pL; subSeqTab[i].or_pL = NULL;   memCheck.subseq--; }
    if (subSeqTab[i].or_nL != NULL) {delete[] subSeqTab[i].or_nL; subSeqTab[i].or_nL = NULL;     memCheck.subseq--; }
  }
}

// ********************************************************************************************* //

void subseq::freeSubSeqTab(int i) {
  if (subSeqTab[i].or_pL != NULL) {delete[] subSeqTab[i].or_pL; subSeqTab[i].or_pL = NULL;       memCheck.subseq--; }
  if (subSeqTab[i].or_nL != NULL) {delete[] subSeqTab[i].or_nL; subSeqTab[i].or_nL = NULL;       memCheck.subseq--; }
}

// ********************************************************************************************* //


void subseq::removeSubSubSeqs(char* seq) {

  // il se peut qu'une sous-séquence soit une sous-sous-séquence d'une sous-séquence plus longue
  // la vérification se fait en triant par indice de noeud parcouru puis en comparant les indice
  qsort(subSeqTab, subSeqNb, sizeof(SUBSEQ), compar_subSeq_iNodeNb);

  // look for subtags 
  int c = mark_subSeqs();
  qsort(subSeqTab, subSeqNb, sizeof(SUBSEQ), compar_subSeq_state);
  // il faut libérer l'espace mémoire des sous-séquences qu'on retire!!!
  for (int i = subSeqNb-c; i < subSeqNb; i++) freeSubSeqTab(i);
  subSeqNb = subSeqNb - c;
  qsort(subSeqTab, subSeqNb, sizeof(SUBSEQ), compar_subSeq_iNodeNb);

  
  // en mode !MUTMOD, on n'autorise pas les sous-seq dont la begin mass n'est pas correcte
  if (!runManParam->m_MUTMOD) 
    {
      c = mark_subSeqs2(seq); 
      qsort(subSeqTab, subSeqNb, sizeof(SUBSEQ), compar_subSeq_state);
      // il faut libérer l'espace mémoire des sous-séquences qu'on retire!!!
      for (int i = subSeqNb-c; i < subSeqNb; i++) freeSubSeqTab(i);
      subSeqNb = subSeqNb - c;
      qsort(subSeqTab, subSeqNb, sizeof(SUBSEQ), compar_subSeq_iNodeNb);
    }

  if (runManParam->m_MUTMOD) 
    {
      c = mark_subSeqs3(seq); 
      qsort(subSeqTab, subSeqNb, sizeof(SUBSEQ), compar_subSeq_state);
      // il faut libérer l'espace mémoire des sous-séquences qu'on retire!!!
      for (int i = subSeqNb-c; i < subSeqNb; i++) freeSubSeqTab(i);
      subSeqNb = subSeqNb - c;
      qsort(subSeqTab, subSeqNb, sizeof(SUBSEQ), compar_subSeq_iNodeNb);
    }

}

//---------------------------------------------------------------------------------------------//

int subseq::mark_subSeqs() {
  int i;
  int dep = 0, tempdep = 0, n = 0;
  int count = 0;
  do {n++;}while ((subSeqTab[n-1].iNodeNb == subSeqTab[n].iNodeNb) && (n < subSeqNb-1));
  

  // arr se trouve au changement de nombre de noeuds;
 
  while (n < subSeqNb) {
    dep = tempdep;
    
    if (subSeqTab[dep].iNodeNb == subSeqTab[n].iNodeNb) break;  // il n'y a que des tags de même longueur dans la liste
    tempdep = n;
    
    do {
      for (i = dep; i < tempdep; i++) {
	if (isSubSeq(n, i))  {
	  subSeqTab[n].state = 1000000;   // met toutes ces séquences à la fin
	  count++;
	  break;
	}
      }  
      n++;
    } while ((subSeqTab[n-1].iNodeNb == subSeqTab[n].iNodeNb) && (n < subSeqNb));
  }
  return count;
}
 
//---------------------------------------------------------------------------------------------//

int subseq::mark_subSeqs2(char* seq) {

  int count = 0;
  
  for (int n = 0; n < subSeqNb; n++) 
    {
      // LA POSITION DU TAG DANS LA SEQUENCE DOIT MATCHER SA BEGIN MASS
      // TROUVE LA MASSE DE LA SEQUENCE PREFIX
      double prefMass    = aaParam->H_mass;
      int    iaa;
      
      for (int k = 0; k < subSeqTab[n].posBegin; k++) {
	iaa = aaParam->findIndice(seq[k]);
	prefMass += aaParam->aaTable[iaa].mass;
      }
      
      if (fabs(subSeqTab[n].massBegin - prefMass) > runManParam->FRAGMENT_ERROR2) {
	subSeqTab[n].state = 1000000;   // met toutes ces séquences à la fin
	count++;
      }
    }
  return count;
}

//---------------------------------------------------------------------------------------------//

int subseq::mark_subSeqs3(char* seq) {

  int count = 0;
 
  for (int n = 0; n < subSeqNb; n++) 
    {
      // FAIT MATCHER LES POSITIONS EXTREMES
      // SI LE TAG A UNE BEGINPOS A 0, IL DOIT AVOIR UNE BEGINMASS A H
      // S'IL A UNE ENDPOS À L, IL DOIT AVOIR UNE ENDMASS A PM-OH

      double NtermMass    = aaParam->H_mass;
      double pepMass      = spectrumData->get_parentMassM() - aaParam->O_mass - aaParam->H_mass;

      if ( (subSeqTab[n].posBegin == 0) && (fabs(subSeqTab[n].massBegin - NtermMass) > runManParam->FRAGMENT_ERROR2) ) {
	subSeqTab[n].state = 1000000;   // met toutes ces séquences à la fin
	count++;
	continue;
      }

      if ( (subSeqTab[n].posBegin + subSeqTab[n].lenght == (int)strlen(seq)) && (fabs(subSeqTab[n].massEnd - pepMass) > runManParam->FRAGMENT_ERROR2) ) {
	subSeqTab[n].state = 1000000;   // met toutes ces séquences à la fin
	count++;
	continue;
      }
    }
  return count;
}

//---------------------------------------------------------------------------------------------//

bool subseq::isSubSeq(int j, int i) {
  for (int k = subSeqTab[j].iNodeNb; k > 0; k--) {
    if (subSeqTab[j].or_iNodesParsed[k-1] != subSeqTab[i].or_iNodesParsed[k])
      return 0;
  }
  return 1;
}

//---------------------------------------------------------------------------------------------//

void subseq::copySubSeq(int dest, int source, int peakNb, int orV) {

  // la posBegin sera mise ailleurs
  subSeqTab[dest].massBegin            = subSeqTab[source].massBegin;
  subSeqTab[dest].massEnd              = subSeqTab[source].massEnd; 
  subSeqTab[dest].lenght               = subSeqTab[source].lenght;
  strcpy(subSeqTab[dest].parsedSeq,      subSeqTab[source].parsedSeq);
  subSeqTab[dest].iNodeNb              = subSeqTab[source].iNodeNb;
  
  for (int i = 0; i < peakNb; i++) 
    subSeqTab[dest].or_pL[i]           =  subSeqTab[source].or_pL[i];

  for (int i = 0; i < orV; i++)    
    subSeqTab[dest].or_nL[i]           =  subSeqTab[source].or_nL[i];

  for (int i = 0; i < MAX_LENGHT; i++) 
    subSeqTab[dest].or_iNodesParsed[i] =  subSeqTab[source].or_iNodesParsed[i];
 
}


// ********************************************************************************************** //

void subseq::write(char* seq, File &fp, int peakNb) {
  
  fprintf(fp, "\n%s:\na) I \nb) SEQ \nc) INODENB \nd) POSBEGIN \ne) LENGHT \nf) MASSBEGIN \ng) MASSEND \nh) INODES \ni) IPEAKS\n\n", seq);
  for (int i = 0; i < subSeqNb; i++) {
    fprintf(fp, "%3i  ", i);
    fprintf(fp, "%-10s  ",   subSeqTab[i].parsedSeq);
    fprintf(fp, "%3i ", subSeqTab[i].iNodeNb);
    fprintf(fp, "%3i  ",   subSeqTab[i].posBegin);
    fprintf(fp, "%3i  ",   subSeqTab[i].lenght);
    fprintf(fp, "%8.2f  ", subSeqTab[i].massBegin);
    fprintf(fp, "%8.2f  ", subSeqTab[i].massEnd);
    for (int j = 0; j < subSeqTab[i].iNodeNb; j++) fprintf(fp, "%4i ", subSeqTab[i].or_iNodesParsed[j]);
    fprintf(fp, "\n%52s", ""); 
    for (int j = 0; j < peakNb; j++) if (subSeqTab[i].or_pL[j]) fprintf(fp, "%i ", j);
    fprintf(fp, "\n");
  }
  fprintf(fp, "----------------------------------------------------------\n");
}


// ********************************************************************************************** //


//---------------------------------------------------------------------------------------------//




//----------------------------------------------------------------------------------------------------


void subseq::copy(SCORE* dest, SCORE* source) {

  dest->lakScore1_a     = source->lakScore1_a;
  dest->modScore1_b     = source->modScore1_b;
  dest->covScore1_c     = source->covScore1_c;
  dest->covScore2_d     = source->covScore2_d;
  dest->covScore3_e     = source->covScore3_e;
  dest->intScore1_f     = source->intScore1_f;
  dest->perScore1_g     = source->perScore1_g;
  dest->famScore1_h     = source->famScore1_h;
  dest->errScore1_i     = source->errScore1_i;
  dest->redScore1_j     = source->redScore1_j;
  dest->serScore1_k     = source->serScore1_k;
  dest->serScore2_l     = source->serScore2_l;
  dest->finArrScore     = source->finArrScore;
}


// ********************************************************************************************** //

