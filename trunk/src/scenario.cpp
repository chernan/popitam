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
#include <cstdlib>
#include <cctype>
#include "scenario.h"
#include "defines.h"
#include "memorycheck.h"
#include "gptree.h"
#include "utils.h"

using namespace std;

// ********************************************************************************************** //

// GLOBAL VARIABLES DECLARED ELSEWHERE 

extern memorycheck memCheck;

// ********************************************************************************************** //

struct NODEANDBMASS {
  int    iN;
  float  bM;
};

struct DEUXINT {
  int    pos;
  int    arr;
};

// ********************************************************************************************** //

int compar_nodeAndbMassBybM(const void *a, const void *b) {
  if (((NODEANDBMASS*)a)->bM > ((NODEANDBMASS*)b)->bM) return  1;
  if (((NODEANDBMASS*)a)->bM < ((NODEANDBMASS*)b)->bM) return -1;
                                                       return  0;
}

int comparPOS(const void*a, const void*b) {
  if (((DEUXINT*)a)->pos > ((DEUXINT*)b)->pos) return  1;
  if (((DEUXINT*)a)->pos < ((DEUXINT*)b)->pos) return -1;
                                               return  0;
}


// ********************************************************************************************** //

scenario::scenario() 
{
  runManParam        = NULL;
  aaParam            = NULL;
  ionParam           = NULL;
  spectrumData       = NULL;
  popiGraph          = NULL;
  aPeptide           = NULL;
  popiTags           = NULL;
  funGen             = NULL;
  scoreF             = NULL;
  arr                = NULL;
  shift              = NULL; 
  size               = 0;
  strcpy(scenarioSeq, "");
}

// ********************************************************************************************** //

scenario::~scenario() 
{  
  runManParam        = NULL;
  aaParam            = NULL;
  ionParam           = NULL;
  spectrumData       = NULL;
  popiGraph          = NULL;
  aPeptide           = NULL;
  popiTags           = NULL;
  funGen             = NULL;
  scoreF             = NULL;
 
  if (arr   != NULL) {delete[] arr;   arr   = NULL;                                               memCheck.scenario--;}
  if (shift != NULL) {delete[] shift; shift = NULL;                                               memCheck.scenario--;}  
}

// ********************************************************************************************** //

void scenario::init(int s, int* clic, runManagerParameters* rMP, aa* aaP, ion* ionP, data* spectrumD, 
		    graphe* popiG, peptide* aPep, subseq* popiT, TREE* F, fun* funG) 
{
  runManParam        = rMP;
  aaParam            = aaP;
  ionParam           = ionP;
  spectrumData       = spectrumD;
  popiGraph          = popiG;
  aPeptide           = aPep;
  popiTags           = popiT;
  funGen             = funG;
  shiftNb            = 0;
  size               = s;
  hypoNb             = 0;                                                                        // SERT A COMPTER LE NOMBRE D'HYPOTHESES QU'ON FAIT
  arr                = new int[size];                                                            memCheck.scenario++;
  shift              = new float[size+1];                                                        memCheck.scenario++;
  for (int i = 0; i < size; i++) arr[i] = 0;
  for (int i = 0; i < size+1; i++) shift[i] = 0.0;
  for (int i = 0; i < size; i++) arr[i] = clic[i];

  strcpy(scenarioSeq,  "");
  Scores.init();
  scoreF             = F;
}

// ********************************************************************************************** //

void scenario::free() 
{ 
  runManParam        = NULL;
  aaParam            = NULL;
  ionParam           = NULL;
  spectrumData       = NULL;
  popiGraph          = NULL;
  aPeptide           = NULL;
  popiTags           = NULL;
  funGen             = NULL;
  scoreF             = NULL;
  size               = 0;
  shiftNb            = 0;
  hypoNb             = 0;
  strcpy(scenarioSeq, "");  
  
  Scores.init();
  
  if (arr   != NULL) {delete[] arr;   arr   = NULL;                                               memCheck.scenario--;}
  if (shift != NULL) {delete[] shift; shift = NULL;                                               memCheck.scenario--;}  
}


// ********************************************************************************************** //

/*
void scenario::buildScenario()
{
  sortArrByBeginPos();

  // construit le scénario
  // = parcourir les tags, et placer les - et * nécessaires 
  // (- = un gap expliqué par un manque d'info, * = un gap expliqué par une mod/mut)
  
  // PROCEDURE
  // le but est de parcourir l'arrangment, et pour chaque gap, de vérifier si la 
  // masse du gap match la séquence (on est alors dans un cas: manque d'infos) ou ne
  // matche pas la séquence (hypothèse d'une mut/mod)
  
  int    p1, p2;
  

  // on parcourt tous les tags, et pour chacun d'eux, on calcule la moyenne
  // du shift des noeuds du tag
  
  for (int i = 0; i < size; i++) {
    if (i == 0) {
      p1 = 0;
      p2 = popiTags->subSeqTab[arr[0]].posBegin;
    }
    else {
      p1 = popiTags->subSeqTab[arr[i-1]].posBegin + popiTags->subSeqTab[arr[i-1]].lenght;
      p2 = popiTags->subSeqTab[arr[i]].posBegin;
    }
    
    
    // 3 cas:
    // 1. le tag est le premier et il est en position 0: p1=p2=0;
    // 2. le tag est le premier, et il est en position > 0: p1=0 < p2
    // 3. le tag n'est pas le premier, dans ce cas on utilise le tag précédent pour obtenir p1, et p2 = mB
    
    // ensuite, il faut voir s'il y a un gap avant le tag
    // cas particulier: le tag est en bP 0, mais bM > 0 (modif dans le terminal)
    
    
    //  if ((p2 != 0) | ((p2 == 0) && (popiTags->subSeqTab[scenarioTab[a].arr[i]].massBegin > aaParam->H_mass))) { 
    // s'il y a un gap entre le début et le premier tag
    
    if (true) {
      
    shift[shiftNb]  = computeShiftMean(i);
      if (fabs(shift[shiftNb]) > runManParam->FRAGMENT_ERROR2) {

	// HYPO DE modGaps
	hypoNb++;
	
	// if premier tag
	if (p1 == 0) {  // il faut partir de -1 car on inclut le terminal
	  for (int z = -1; z < p2-p1; z++)     strcat(scenarioSeq, "*");
	}
	else {
	  for (int z = 0; z < p2-p1; z++)      strcat(scenarioSeq, "*");
	}
      }
      else {
	if ((p2 == 0)|(p1 == 0)) {
	  for (int z = -1; z < p2-p1; z++)     strcat(scenarioSeq, "-");
	}
	else {
	  for (int z = 0; z < p2-p1; z++)      strcat(scenarioSeq, "-");  
	}
      }
      if (strlen(scenarioSeq) > MAX_LENGHT)  {fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario()");}
    }
    
    shiftNb++;
    if (shiftNb > size+1) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario(); shiftNb is too big");
    strcat(scenarioSeq, popiTags->subSeqTab[arr[i]].parsedSeq);
    if (strlen(scenarioSeq) > MAX_LENGHT) {fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario()");}
  }
  
  // il reste de toute manière un gap à remplir
  // trouve les positions du gap 
  
  p1 = popiTags->subSeqTab[arr[size-1]].posBegin + popiTags->subSeqTab[arr[size-1]].lenght;
  p2 = strlen(aPeptide->popiPep.dtbSeq);
    
  // calcule l'ensemble des mod/mut déjà fait pour la séquence
  float preShift = 0;
  for (int s = 0; s < shiftNb; s++) preShift += shift[s];
  // récupère la masse théorique totale de la séquence (avec les terminaux)
  // en fait, seqM correspond à Mr
  float seqM = aPeptide->popiPep.bLayer[strlen(aPeptide->popiPep.dtbSeq)-1] + aaParam->O_mass + aaParam->H_mass;
  // (c'est la PM théorique = de la dtb)
  
  // calcul le shift pour le dernier lack
  shift[shiftNb] = spectrumData->get_parentMassM() - preShift - seqM;
  if (fabs(shift[shiftNb]) > runManParam->FRAGMENT_ERROR2) {

    // HYPO DE MODIFICATION
    hypoNb++;

    if (p2 == (int)strlen(aPeptide->popiPep.dtbSeq)) 
      {
	for (int z = -1; z < p2-p1; z++) strcat(scenarioSeq, "*");
      }
    else {
      for (int z =  0; z < p2-p1; z++)   strcat(scenarioSeq, "*");
    }
  }
  else 
    {
      if (p2 == (int)strlen(aPeptide->popiPep.dtbSeq)) 
	{
	  for (int z = -1; z < p2-p1; z++) strcat(scenarioSeq, "-");
	}
      else 
	{
	  for (int z =  0; z < p2-p1; z++) strcat(scenarioSeq, "-");
	}
    }
  if (strlen(scenarioSeq) > MAX_LENGHT)  {fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario()");}
  
  shiftNb++;
  if (shiftNb > size+1) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario(); shiftNb is too big");
  

}
*/
// ********************************************************************************************** //

void scenario::buildScenario()
{
  // = parcourir les tags, et placer les - et * nécessaires 
  // (- = un lackGap expliqué par un manque d'info, * = un modGap expliqué par une mod/mut)
 
  //  1) trie les tags par position sur la séquence
  sortArrByBeginPos();
 
  // 2) parcours les tags et calcule les shifts des gaps

  int    p1       = 0;
  int    p2       = 0;
  float  preShift = 0;

  for (int i = 0; i < size; i++) // pour tous les tags 
    {

      // on calcule maintenant les positions p1 et p2 du gap sur la séquence

      if (i == 0) {   // si le tag est le premier
	p1 = 0;
	p2  = popiTags->subSeqTab[arr[0]].posBegin;
      }
      else            // si le tag n'est pas le premier
	{
	  p1 = popiTags->subSeqTab[arr[i-1]].posBegin + popiTags->subSeqTab[arr[i-1]].lenght;
	  p2  = popiTags->subSeqTab[arr[i]].posBegin;
	}
      
         // trois cas possibles: 
      // a) soit le tag est le premier et il commence en position 0  (p1 = p2 = 0)
      // b) soit le tag est le premier et il commence après la position 0 (p1 = 0 et p2 > p1)
      // c) soit le tag n'est pas le premier, et il y a forcément un gap avant! (p1 != 0 et p2 > p1)

      // cas a)
      if ((p1 == 0) && (p2 == 0))
	{
	  poseUnTag(i);
	}
      
      // cas b) et c)
      if (p1 < p2)
	{
	  preShift        = computePreShift();             // somme les shifts antérieurs
	  shift[shiftNb]  = computeShiftMean(preShift, i); // calcul le shift du tag i par rapport à la séquence théorique et aux shifts antérieurs

	  if (fabs(shift[shiftNb]) > runManParam->FRAGMENT_ERROR2)  
	    {
	      poseUnGap(p1, p2, '*'); hypoNb++;
	    }
	  else 
	    {
	      poseUnGap(p1, p2, '-');
	    }

	  poseUnTag(i);
	}
    }

  // une fois qu'on a parcouru tous les tags, il faut vérifier qu'il ne reste pas un gap à remplir
  
  p1 = popiTags->subSeqTab[arr[size-1]].posBegin + popiTags->subSeqTab[arr[size-1]].lenght;
  p2 = (int)strlen(aPeptide->popiPep.dtbSeq);
  if (p1 < p2)  // il y a un gap à remplir
    {
      preShift = computePreShift();     
      shift[shiftNb] = ((float)spectrumData->get_parentMassM()) - preShift - aPeptide->popiPep.dtbPepMass;

      if (fabs(shift[shiftNb]) > runManParam->FRAGMENT_ERROR2) 
	{
	  poseUnGap(p1,p2, '*'); hypoNb++;
	}
      else
	{
	  poseUnGap(p1,p2, '-');
	}
    }
}

// ********************************************************************************************** //

float scenario::computePreShift()
{
  float preShift = 0;
  for (int s = 0; s < shiftNb; s++) preShift += shift[s];
  return preShift;
}

// ********************************************************************************************** //

void scenario::poseUnTag(int i)
{
  strcat(scenarioSeq, popiTags->subSeqTab[arr[i]].parsedSeq);
  if (strlen(scenarioSeq) > MAX_LENGHT) {fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario()");}
}

// ********************************************************************************************** //

void scenario::poseUnGap(int p1, int p2, char c)
{
  for (int z = 0; z < p2-p1; z++) 
    {
      scenarioSeq[z+p1] = c;
    }
  scenarioSeq[p2] = '\0';
  if (strlen(scenarioSeq) > MAX_LENGHT)  {fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario()");}
  
  shiftNb++;
  if (shiftNb > size+1) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::buildScenario(); shiftNb is too big");
}

// ********************************************************************************************** //

void scenario::sortArrByBeginPos()
{
  //trie les tags par position et construit la séquence scénario pour l'arrangement courant


  DEUXINT arrSorted[MAX_SUBSEQ_NB_IN_AN_ARRANGEMENT];
  for (int i = 0; i < size; i++) 
    {
      arrSorted[i].arr = arr[i];
      arrSorted[i].pos = popiTags->subSeqTab[arr[i]].posBegin;
    }

  qsort(arrSorted, size, sizeof(DEUXINT), comparPOS);

  for (int i = 0; i < size; i++) arr[i] = arrSorted[i].arr;

}


// ********************************************************************************************** //

float scenario::computeShiftMean(float preShift, int i)// renvoie le shift moyen pour le tag i 
{
  
  float  *bMassesExp  = new float[516];                                                           memCheck.scenario++;
  float  *bMassesTheo = new float[516];                                                           memCheck.scenario++;
  float   shiftMean = 0;
  int     nb        = 0;
  float   delta     = 0;


  // pour cela, j'utilise la beginMass et la masse correspondante sur la séquence théorique
  // recherche de la masse théorique par rapport à la séquence théorique (s'il y a déjà eu des
  // scenarios de mod/mut, il faut inclure les shift (=masses des mod/mut) pour correctement prédire la masse théorique)
  // 1 calcul de la masse total des mod/mut sur la séquence prefix à la position du tag 
  
  if (popiTags->subSeqTab[arr[i]].posBegin == 0) 
    {
      delta = float(popiTags->subSeqTab[arr[i]].massBegin) - aaParam->H_mass;
    }
  else   
    {
      delta = float(popiTags->subSeqTab[arr[i]].massBegin - aPeptide->popiPep.bLayer[popiTags->subSeqTab[arr[i]].posBegin - 1]);
    }
  
  //  delta = subSeqTab[arr[i]].massBegin - (aPeptide->popiPep.bLayer[subSeqTab[arr[i]].posBegin - 1] + preShift);
  //  if (subSeqTab[arr[i]].posBegin == 0) delta = 0;
  //  else  delta = subSeqTab[arr[i]].massBegin - (aPeptide->popiPep.bLayer[subSeqTab[arr[i]].posBegin - 1]);
  
  // il faudra tenir compte du delta lors du *matching* des theo et exp
  
  fillExpTheoTabs(i, delta, bMassesExp, bMassesTheo, &nb, '-');  // le charactere '-' signifie de faire le remplissage pour les ions Cterm comme
  // les Nterm; sinon, on met un 'C' ou un 'N', ce qui est fait pour le scoring, car la régression doit alors être plus précise (c'est à cause de la
  // masse parente qui est utilisée pour les ions Cterm et qui peut être peu précise)
  
 
  for (int n = 0; n <  nb; n++) {
    shiftMean+= bMassesExp[n]- (bMassesTheo[n] + preShift);
  }
  shiftMean /= (float)nb;  // maintenant, shiftMean est computé.
  

  delete[] bMassesExp;                                                                            memCheck.scenario--;
  delete[] bMassesTheo;                                                                           memCheck.scenario--;

  return shiftMean;  

}

// ********************************************************************************************** //
/*

double subseq::computeShiftReg(int a, int i) {  
  // compute la droite de régression à partir des valeurs des noeuds (il faut au moins 3 noeuds sinon
  // la fonction renvoie une moyenne, et renvoie la valeur attendue sur la régression pour
  // la beginMass
  
  
  double* massesExperimentales  = new double[516];                                                  memCheckscenario++;
  double* massesAttendues = new double[516];                                                        memCheckscenario++;
  double  shiftReg  = 0;
  double  preShift  = 0;
  int     nb        = 0;
  double  delta     = 0;

  // FORMATION DES COUPLES
  // pour cela, j'utilise la beginMass et la masse correspondante sur la séquence théorique
  // recherche de la masse théorique par rapport à la séquence théorique (s'il y a déjà eu des
  // scenarios de mod/mut, il faut inclure les shift (=masses des mod/mut) pour correctement prédire la masse théorique)
  // 1 calcul de la masse total des mod/mut sur la séquence prefix à la position du tag

  for (int z = 0; z < arrTab[a].shiftNb; z++) preShift += arrTab[a].shift[z];
  
  // delta est une valeur temporaire, qui représente le shift entre le beginMass du tag et la masse correspondante
  // sur la séquence théorique; il est utilisé pour correctement matcher les masses theo et exp lors du calcul
  // de la régression linéaire. Ensuite, le shift est déduit de la droite de régression.
  if (subSeqTab[arrTab[a].arr[i]].posBegin == 0)  delta = subSeqTab[arrTab[a].arr[i]].massBegin - aaParam->H_mass;
  else delta = subSeqTab[arrTab[a].arr[i]].massBegin - (popiPep->bLayer[subSeqTab[arrTab[a].arr[i]].posBegin - 1]);
    
  fillExpTheoTabs(a, i, delta, massesExperimentales, massesAttendues, &nb, '-');  // le charactere '-' signifie de faire le remplissage pour les ions Cterm comme
  // les Nterm; sinon, on met un 'C' ou un 'N'
  for (int n = 0; n < nb; n++) massesAttendues[n] += preShift;
  
  // REGRESSION SI NB > 2
  if (nb < 2) {fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "the number of couple is below 3 for regression; please set min_tag_lenght to 3 in tags.cc");}

  double pente = 0, ordOr = 0;
  find_a_and_b(&pente, &ordOr, massesExperimentales, massesAttendues, nb);
  double expYvalue=pente*subSeqTab[arrTab[a].arr[i]].massBegin+ordOr;
  shiftReg = expYvalue-subSeqTab[arrTab[a].arr[i]].massBegin;
 
  delete[] massesExperimentales;                                                                  memCheck.scenario--;
  delete[] massesAttendues;                                                                       memCheck.scenario--;

  return shiftReg;  
}
*/

// ********************************************************************************************** //

void scenario::fillExpTheoTabs(int i, float delta, 
			       float* massesExperimentales, float* massesAttendues, 
			       int* nb, char c) {
  
  float  bMasseExp;
  int    or_iNode;
  int    iHypo;
  int    iSubIndice = arr[i];
  
  for (int j = 0; j < popiTags->subSeqTab[iSubIndice].iNodeNb; j++) {   
    
    // retrouve et store dans bMassExp la bMass du noeud courant à partir des infos dans le noeud originel 
    or_iNode   = popiTags->subSeqTab[iSubIndice].or_iNodesParsed[j];

    for (int m = 0; m < popiGraph->get_or_iVertex_mergedNb(or_iNode); m++) {
      iHypo = popiGraph->get_or_iVertex_miHypo(or_iNode, m);
      
      if (c != '-') {if (c != ionParam->get_term(iHypo)) continue;}

      massesExperimentales[(*nb)] = (float)popiGraph->get_or_iVertex_mbMass(or_iNode, m);
      
      // trouve la masse théorique correspondante à bMasseExp dans les bLayers
      bMasseExp = (float)popiGraph->get_or_iVertex_mbMass(or_iNode,m);
      massesAttendues[(*nb)] = FindClosestTheoMass(bMasseExp, iSubIndice, delta);

      // rajoute les deltas aux deux masses pour séparer les masses théo
      massesExperimentales[(*nb)] += (float)ionParam->get_delta(iHypo);
      massesAttendues[(*nb)]      += (float)ionParam->get_delta(iHypo);
      
      (*nb)++; if ((*nb) >= 516) {fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "more than 516 runnedNodes in function tag_processor::computeErrScore()");} 
    }
  }
}

// ********************************************************************************************** //


float scenario::FindClosestTheoMass(float bMasseExp, int iSubIndice, float delta) 
{
  float  smallestDiff = 1000000.0;
  float  bestTheoMass = 0.0;
  
  for (int k = popiTags->subSeqTab[iSubIndice].posBegin-1; k < popiTags->subSeqTab[iSubIndice].posBegin + (int)strlen(popiTags->subSeqTab[iSubIndice].parsedSeq); k++)
    {
      if (k == -1) {
	if (fabs(bMasseExp - aaParam->H_mass - delta) < smallestDiff)
	  {
	    bestTheoMass = aaParam->H_mass;
	    smallestDiff = fabs(bMasseExp - aaParam->H_mass - delta);
	  }
      }
      else {
	if (fabs(bMasseExp - aPeptide->popiPep.bLayer[k] - delta) < smallestDiff) 
	  {
	    bestTheoMass = (float)aPeptide->popiPep.bLayer[k];
	    smallestDiff = fabs(bMasseExp - ((float)aPeptide->popiPep.bLayer[k]) - delta);
	  }
      }
    }
  return bestTheoMass;
}

// ********************************************************************************************** //

bool scenario::computeScores()
{
  float s1, s2, s3;
  
  s1=0; s2=0; 
  if (!computeGapScores(&s1, &s2)) return false;
  Scores.lakScore1_a       = s1; // entier, non borné (mais reste < qqls unités)
  Scores.modScore1_b       = s2; // entier, non borné (mais reste < qqls unités)

  s1=0; s2=0; s3=0;
  if (!computeCovScores(&s1, &s2, &s3)) return false;  // ordre = simple, glo (=simple+double), all (= avec lacks)
  Scores.covScore1_c       = s1;
  Scores.covScore2_d       = s2;
  Scores.covScore3_e       = s3;
  if (Scores.covScore3_e < runManParam->MIN_COV_ARR) return false;

  s1=0;
  if (!computeIntScore(&s1)) return false;
  Scores.intScore1_f       = s1;  // moyenne des intensités normalisées à 1;
  // entre 0 et 1, mais pourrait aussi dépasser 1 car plusieures fois le même pic possible
  
  s1=0;
  if (!computePerScore(&s1)) return false;
  Scores.perScore1_g       = s1;  // somme des prob. d'occurence des prob. ioniques des différents pics utilisés
                                       // (non borné) de l'ordre de qqls unités
  // plus il y a de pics, mieux c'est (car cela veut dire plus de pics mergés et meilleure couverture)
  // plus les probs ioniques des pics sont hautes mieux c'est (car cela veut dire bonnes hypothèses ioniques, comme b, y)
  
  s1=0;
  if (!computeFamScore(&s1)) return false;           // rapport au nombre de noeud de la taille des familles au carré
  Scores.famScore1_h       = s1;  // (non borné, mais reste de l'ordre < qqls unités à qqls dizaines)
    
  s1=0; s2=0;
  if (!computeErrScore(&s1, 'N')) return false;      // erreur sur les ions Nterm;
  if (!computeErrScore(&s2, 'C')) return false;      // erreur sur les ions Cterm;
  if (  isFloatEqual(s1, -1000000.0))                                     Scores.errScore1_i = s2;
  if (  isFloatEqual(s2, -1000000.0))                                     Scores.errScore1_i = s1;
  if ((!isFloatEqual(s1, -1000000.0)) && (!isFloatEqual(s2, -1000000.0))) Scores.errScore1_i = (s1+s2)/2;
  if (( isFloatEqual(s1, -1000000.0)) && ( isFloatEqual(s2, -1000000.0))) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "errscores are both -1000000;");
  
  s1=0;
  if (!computeRedScore(&s1)) return false;    // peakScore; il s'agit de la proportion parmi les pics des pics qui sont
  Scores.redScore1_j       = s1;              // utilisés plusieures fois
  
  s1=0; s2=0;
  if (!computeSerScore(&s1, &s2)) return false;
  Scores.serScore1_k         = s1;  //% par rapport à la longueur de la dtb de la série b la plus longue
  Scores.serScore2_l         = s2;  //% par rapport à la longueur de la dtb de la série y la plus longue
  

  bool NAN_ = false;
  bool INF_ = false;
  
  NODE* root = scoreF->getRoot();
  Scores.finArrScore = scoreF->ParseTree(runManParam->FILE_ERROR_NAME, root, Scores, &NAN_, &INF_);
  //  if (!isFloatEqual(Scores.finArrScore,   Scores.finArrScore))  fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeScores");
  //  if ( isFloatEqual(Scores.finArrScore-1, Scores.finArrScore))  fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeScores");
  if (!isDoubleEqual(Scores.finArrScore,   Scores.finArrScore)) return false;
  if ( isDoubleEqual(Scores.finArrScore-1, Scores.finArrScore)) return false;
  
  return true;
}

// ********************************************************************************************** //


void scenario::write(File& fp)
{
  fprintf(fp, "\n%6s","");
  fprintf(fp, "%6s%s\n", "", aPeptide->popiPep.dtbSeq);
  fprintf(fp, "%11s%s", "",  scenarioSeq);
  fprintf(fp, "         ");
  for (int f = 0; f < shiftNb; f++) {
    fprintf(fp, "%10.2f ", shift[f]);
  }
  fprintf(fp, "\n\n");
}

// ********************************************************************************************** //

bool scenario::computeGapScores(float* lack, float* modMut) 
{
  for (unsigned int i = 0, n=(unsigned int)strlen(scenarioSeq); i < n; i++) {
    switch(scenarioSeq[i]) {
    case '*': {(*modMut)++; while ((scenarioSeq[i] == '*') && (i < strlen(scenarioSeq))) i++; break;}
    case '-': {(*lack)++;   while ((scenarioSeq[i] == '-') && (i < strlen(scenarioSeq))) i++; break;}
    }
  }
  return true;
}
 
// ********************************************************************************************** //


bool scenario::computeCovScores(float* sim, float* glo, float* all) {

  for (unsigned int i = 0, n=(unsigned int)strlen(scenarioSeq); i < n; i++) {
    if (scenarioSeq[i] != '*') {
      (*all)++;
      if (scenarioSeq[i] != '-') {
	(*glo)++;
	if (toupper(scenarioSeq[i]) == scenarioSeq[i]) (*sim)++;
      }
    }
  }

  // si il y a une hypo de modif, je rajoute un à chacun des scores (la place de la modif)
  if (Scores.modScore1_b == 1) {(*sim)++; (*glo)++; (*all)++;}
  if (Scores.modScore1_b == 2) {(*sim)+=2; (*glo)+=2; (*all)+=2;}

  (*sim) /= (float(strlen(scenarioSeq)));
  (*glo) /= (float(strlen(scenarioSeq)));
  (*all) /= (float(strlen(scenarioSeq)));

  //  if (!isFloatEqual(*sim, *sim))     fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeCovScores a.");
  //  if (!isFloatEqual(*glo, *glo))     fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeCovScores b.");
  //  if (!isFloatEqual(*all, *all))     fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeCovScores c.");
  //  if ( isFloatEqual((*sim)-1, *sim)) fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeCovScores a.");
  //  if ( isFloatEqual((*glo)-1, *glo)) fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeCovScores b.");
  //  if ( isFloatEqual((*all)-1, *all)) fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeCovScores c.");

  if (!isFloatEqual(*sim, *sim))     return false;
  if (!isFloatEqual(*glo, *glo))     return false;
  if (!isFloatEqual(*all, *all))     return false;
  if ( isFloatEqual((*sim)-1, *sim)) return false;
  if ( isFloatEqual((*glo)-1, *glo)) return false;
  if ( isFloatEqual((*all)-1, *all)) return false;

  if (((*sim) < 0) | ((*sim) > 1)) fatal_error(runManParam->FILE_ERROR_NAME, BORN, "in function scenario::computCovScores a.");
  if (((*glo) < 0) | ((*glo) > 1)) fatal_error(runManParam->FILE_ERROR_NAME, BORN, "in function scenario::computCovScores b.");
  if (((*all) < 0) | ((*all) > 1)) fatal_error(runManParam->FILE_ERROR_NAME, BORN, "in function scenario::computCovScores c.");
  return true;
}

// ********************************************************************************************** //

bool scenario::computeIntScore(float* s1) {
  
  float usedIntSom = 0;
  int    or_iNode_indice;
  
  // puis calcul la somme des intensités des pics inclus dans le scenario
  for (int i = 0; i < size; i++) {                                              // pour chaque sous-seq de l'arrangement
    for (int j = 0; j < popiTags->subSeqTab[arr[i]].iNodeNb; j++) {                    // pour chaque élément source de la sous-séquence courante
      or_iNode_indice = popiTags->subSeqTab[arr[i]].or_iNodesParsed[j];
      if ((or_iNode_indice == 0) | (or_iNode_indice == popiGraph->get_orVertexNb()-1)) continue;
      
      for (int m = 0; m < popiGraph->get_or_iVertex_mergedNb(or_iNode_indice); m++) {
	usedIntSom += (float)popiGraph->get_or_iVertex_mpeakInt(or_iNode_indice, m);
      }
    }
  }
  
  (*s1) = (float)(usedIntSom / popiGraph->get_intSom());
  
  // tests NAN et INF
  //  if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeIntScore");
  //  if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeIntScore");
  if (!isFloatEqual(*s1 , *s1))    return false;
  if ( isFloatEqual((*s1)-1, *s1)) return false;
  return true;
}

// ********************************************************************************************** //

bool scenario::computePerScore(float* s1) {
  
  int   or_iNode_indice, iHypo, iBin;
  // int   nb = 0;

  for (int i = 0; i < size; i++) {                                              // pour chaque sous-seq de l'arrangement
    for (int j = 0; j < popiTags->subSeqTab[arr[i]].iNodeNb; j++) {                    // pour chaque élément source de la sous-séquence courante
      // recherche de la prob ionique correspondant au noeud
      or_iNode_indice = popiTags->subSeqTab[arr[i]].or_iNodesParsed[j];
      if ((or_iNode_indice == 0) | (or_iNode_indice == popiGraph->get_orVertexNb()-1)) continue;

      
      for (int m = 0; m < popiGraph->get_or_iVertex_mergedNb(or_iNode_indice); m++) {
	iHypo  = popiGraph->get_or_iVertex_miHypo(or_iNode_indice, m);
	iBin   = popiGraph->get_or_iVertex_mpeakBin(or_iNode_indice, m);
	(*s1) += (float)(ionParam->get_prob(iHypo, iBin)/100);
	// nb++;
      }
    }
  }
  
  //  (*s1) /= (float)nb;  // je ne prends pas la moyenne, mais simplement la somme
  
  // tests NAN et INF
  // if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computePerScore");
  // if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computePerScore");
  if (!isFloatEqual(*s1 , *s1))    return false;
  if ( isFloatEqual((*s1)-1, *s1)) return false;
  return true;
}

// ********************************************************************************************** //

bool scenario::computeFamScore(float* s1) {
  
  int nb = 0;
  int or_iNode_indice;
  
  for (int i = 0; i < size; i++) {                                                     // pour chaque sous-seq de l'arrangement
    for (int j = 0; j < popiTags->subSeqTab[arr[i]].iNodeNb; j++) {                    // pour chaque élément source de la sous-séquence courante
      or_iNode_indice = popiTags->subSeqTab[arr[i]].or_iNodesParsed[j];
      if ((or_iNode_indice == 0) | (or_iNode_indice == popiGraph->get_orVertexNb()-1)) continue;
      (*s1) += (popiGraph->get_or_iVertex_mergedNb(or_iNode_indice) * popiGraph->get_or_iVertex_mergedNb(or_iNode_indice));
      nb++;
    }
  }
  //(*s1) /= (float)nb;
  
  // tests
  // if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeFamScore");
  // if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeFamScore");
  if (!isFloatEqual(*s1 , *s1))    return false;
  if ( isFloatEqual((*s1)-1, *s1)) return false;
  return true;
}

// ********************************************************************************************** //

bool scenario::computeSerScore(float* s1, float* s2) {

  int          or_iNode_indice, ionIndice;
  NODEANDBMASS allNodes[512];
  int          nb       = 0;
  int          countB   = 0;
  int          countY   = 0;
  int          maxB     = 0;
  int          maxY     = 0;
  bool         ISbHere  = false;
  bool         ISyHere  = false;
  

  for (int i = 0; i < size; i++) {                                              // pour chaque sous-seq de l'arrangement
    nb = 0;
    // PREPARATION DES NOEUDS A PARCOURIR POUR RECHERCHER LES MEILLEURES SERIES
    for (int j = 0; j < popiTags->subSeqTab[arr[i]].iNodeNb; j++) {                    // pour chaque élément source de la sous-séquence courante
      or_iNode_indice = popiTags->subSeqTab[arr[i]].or_iNodesParsed[j];
      if ((or_iNode_indice == 0) | (or_iNode_indice == popiGraph->get_orVertexNb()-1)) continue;
     
      allNodes[nb].iN = or_iNode_indice;
      allNodes[nb].bM = (float)popiGraph->get_or_iVertex_mean_bMass(or_iNode_indice);
      nb++; if (nb >= 512) fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function scenario::computeSerScore()");
    }
  }
  
  // tri des noeuds par leur bMasses
  qsort(allNodes, nb, sizeof(NODEANDBMASS), compar_nodeAndbMassBybM);
    
  // -------------------------------------------------------------------------
    
  countB     = 0;
  countY     = 0;
  maxB       = 0;
  maxY       = 0;
    
  // recherche si les ions sont présents en respectant l'ordre des bMasses
  // pour chaque noeud
  for (int j = 0; j < nb; j++) {  
    
    ISbHere    = false;
    ISyHere    = false;

    or_iNode_indice = allNodes[j].iN;
    // pas de problem de pseudonoeuds, car ils ne sont pas présent dans allNodes
      
    for (int m = 0; m < popiGraph->get_or_iVertex_mergedNb(or_iNode_indice); m++) {
      ionIndice = popiGraph->get_or_iVertex_miHypo(or_iNode_indice, m);
	
      if (!strcmp(ionParam->get_name(ionIndice), "b")) {ISbHere = true; continue;}
      if (!strcmp(ionParam->get_name(ionIndice), "y")) {ISyHere = true; continue;}
    }
    // pour les ions présents, incrémente la série, et pour les absents, remet-là à 0
    
    if (ISbHere == true)  countB++;
    else {
      if (countB > maxB) maxB = countB;
      countB = 0;
    }
    if (ISyHere == true)  countY++;
    else {
      if (countY > maxY) maxY = countY;
      countY = 0;
    }
  }
  if (countB > maxB) maxB = countB; // nécessaire
  if (countY > maxY) maxY = countY;
  
  (*s1) = (float)maxB;
  (*s2) = (float)maxY;

  
  // normalise selon le nombre de noeuds parcourus 
  
  if ((*s1) > strlen(aPeptide->popiPep.dtbSeq)+2) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function scenario::computeSerScore; this should not occur a.");
  if ((*s2) > strlen(aPeptide->popiPep.dtbSeq)+2) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function scenario::computeSerScore; this should not occur b.");
  // le +2, c'est pour le cas ou toute la séquence a été parcourue, mais que les premiers et dernier noeuds ne sont
  // pas des pseudonoeuds 

  //  (*s1) = (*s1)/(float)nb;
  //  (*s2) = (*s2)/(float)nb;

  //  if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeSerScore a.");
  //  if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeSerScore a.");
  //  if (!isFloatEqual(*s2 , *s2))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeSerScore b.");
  //  if ( isFloatEqual((*s2)-1, *s2))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeSerScore b.");

  if (!isFloatEqual(*s1 , *s1))    return false;
  if ( isFloatEqual((*s1)-1, *s1)) return false;
  if (!isFloatEqual(*s2 , *s2))    return false;
  if ( isFloatEqual((*s2)-1, *s2)) return false;
  return true;
} 

// ********************************************************************************************** //

bool scenario::computeRedScore(float* s1) {

  float redondance = 0;
  float usedPeakNb = 0;
  int    peakIndice;
  int    or_iNode_indice;

  int*    allpL;
  allpL = new int[spectrumData->get_peakNb()];                                                    memCheck.scenario++; 
  for (int i = 0; i < spectrumData->get_peakNb(); i++) allpL[i] = 0;
  
  // combine tous les pics des différents tags dans pL
  for (int i = 0; i < size; i++) {                                              // pour chaque sous-seq de l'arrangement
    for (int j = 0; j < popiTags->subSeqTab[arr[i]].iNodeNb; j++) {                    // pour chaque élément source de la sous-séquence courante
      or_iNode_indice = popiTags->subSeqTab[arr[i]].or_iNodesParsed[j];
      if ((or_iNode_indice == 0) | (or_iNode_indice == popiGraph->get_orVertexNb()-1)) continue;

      for (int m = 0; m < popiGraph->get_or_iVertex_mergedNb(or_iNode_indice); m++) {
	peakIndice = popiGraph->get_or_iVertex_miPeak(or_iNode_indice, m);
	allpL[peakIndice] += popiTags->subSeqTab[arr[i]].or_pL[peakIndice];
      }
    }
  }
 
  // fait les incrémentations
  for (int i = 0; i < spectrumData->get_peakNb(); i++) {
    if (allpL[i] > 0) usedPeakNb++;
    if (allpL[i] > 1) redondance += (allpL[i] * allpL[i]);
  }
  
  (*s1) = redondance/usedPeakNb;

  //  if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeRedScore");
  //  if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeRedScore");

  if (!isFloatEqual(*s1 , *s1))    {delete[] allpL; allpL = NULL; return false; memCheck.scenario--;}
  if ( isFloatEqual((*s1)-1, *s1)) {delete[] allpL; allpL = NULL; return false; memCheck.scenario--;}

  delete[] allpL; allpL = NULL;                                                                   memCheck.scenario--; 
  return true;
}

// ********************************************************************************************** //


bool scenario::computeLenScore(float* s1) { 
    for (int i = 0; i < size; i++) {
    (*s1) += popiTags->subSeqTab[arr[i]].lenght;
  }
  (*s1) /= (float)size;
  (*s1) /= float(strlen(aPeptide->popiPep.dtbSeq));

  //  if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeLenScore");
  //  if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeLenScore");

  if (!isFloatEqual(*s1 , *s1))    return false;
  if ( isFloatEqual((*s1)-1, *s1)) return false;

  if (((*s1) < 0) | ((*s1) > 1)) fatal_error(runManParam->FILE_ERROR_NAME, BORN, "in function scenario::computeLenScore");
  return true;
}

// ********************************************************************************************** //


bool scenario::computeLexScore(float* s1) { 
  
  for (int i = 0; i < size; i++) {
    if (popiTags->subSeqTab[arr[i]].lenght > (*s1))
      (*s1) = (float)popiTags->subSeqTab[arr[i]].lenght;
  }
  (*s1) /= float(strlen(aPeptide->popiPep.dtbSeq));

  //  if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeLexScore");
  //  if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeLexScore");

  if (!isFloatEqual(*s1 , *s1))    return false;
  if ( isFloatEqual((*s1)-1, *s1)) return false;

  if (((*s1) < 0) | ((*s1) > 1)) fatal_error(runManParam->FILE_ERROR_NAME, BORN, "in function scenario::computeLexScore");
  return true;

}

// ********************************************************************************************** //

bool scenario::computeErrScore(float* s1, char term)
{
  float* massesExperimentales  = new float[516];                                                  memCheck.scenario++;
  for(int i=0; i<516; i++) massesExperimentales[i] = 0.0;
  float* massesAttendues       = new float[516];                                                  memCheck.scenario++;
  for(int i=0; i<516; i++) massesAttendues[i] = 0.0;
  int     nb, lastnb;
  float  delta     = 0;
  float  d         = 0;
 
  nb = 0; 
  for (int i = 0; i < size; i++) {                           // pour chaque sous-seq de l'arrangement
    lastnb = nb;
    // compute le delta du tag i courant
    delta = 0;
    for (int z = 0; z <= i; z++) delta += shift[z];
    
    // créé les couples <exp;theo>
    fillExpTheoTabs(i, delta, massesExperimentales, massesAttendues, &nb, term);
    for (int z = lastnb; z < nb; z++) massesAttendues[z] += delta;
  }
  // maintenant, les tableaux massesExp et massesAttendues contiennent les masses pour tous les tags
  // de l'arrangement a
  
  // calcul le nombre de masses théoriques représentées dans le plot de régression
  int theoMassDiffNb = 1;
  for (int n = 1; n < nb; n++) {if (!isFloatEqual(massesAttendues[n-1], massesAttendues[n])) theoMassDiffNb++;}

  // Les cas suivants peuvent se présenter:
  // 1) nb = 0 (pas d'ion du terminal term); dans ce cas le score n'a plus de sens et prend la valeur de -1000'000
  // 3) nb > 0 et theoMassDiffNb < 3: Il y a en tout cas deux points, mais pas suffisamment de points différents pour calculer
  // 4) theoMassDiffNb >= 3 : regression normale
  
  if (nb == 0) {
    (*s1) = -1000000.0;
    delete[] massesExperimentales;                                                                memCheck.scenario--;
    delete[] massesAttendues;                                                                     memCheck.scenario--;
    return true;
  }
    
  if (theoMassDiffNb < 5) {      // ne fait pas une régression, mais compute la moyenne des erreurs
    
    for (int q = 0; q <  nb; q++) {d += fabs(massesExperimentales[q] - massesAttendues[q]);}	

    d /= (float)nb;
    (*s1) += d;
  
    //    if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeErrScore a.");
    //    if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeErrScore a.");
    if ((!isFloatEqual(*s1 , *s1)) |  ( isFloatEqual((*s1)-1, *s1)))
      {
	delete[] massesExperimentales;                                                                memCheck.scenario--;
	delete[] massesAttendues;                                                                     memCheck.scenario--;
	return false;
      }
  }
  
  if (theoMassDiffNb >= 5) {
    
    float pente = 0, ordOr = 0;
    find_a_and_b(&pente, &ordOr, massesExperimentales, massesAttendues, nb);
    
    // test des NAN et INF
    //    if (!isFloatEqual(pente,  pente))     fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeErrScore b.");
    //    if ( isFloatEqual(pente-1,pente))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeErrScore b.");
    //    if (!isFloatEqual(ordOr,  ordOr))     fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeErrScore c.");
    //    if ( isFloatEqual(ordOr-1,ordOr))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeErrScore c.");
    if ((!isFloatEqual(pente,  pente)) | ( isFloatEqual(pente-1,pente)) | (!isFloatEqual(ordOr,  ordOr)) | ( isFloatEqual(ordOr-1,ordOr)))
      {
	delete[] massesExperimentales;                                                                memCheck.scenario--;
	delete[] massesAttendues;                                                                     memCheck.scenario--;
	return false;
      }

    d = compute_the_variance(pente, ordOr, massesExperimentales, massesAttendues, nb);
    (*s1) += d;
    
    //    if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeErrScore d.");
    //    if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeErrScore d.");
    if ((!isFloatEqual(*s1 , *s1)) | ( isFloatEqual((*s1)-1, *s1)))
      {
       	delete[] massesExperimentales;                                                                memCheck.scenario--;
	delete[] massesAttendues;                                                                     memCheck.scenario--;
	return false;
      }    
  }
  
  // fin du calcul pour s1
  (*s1) /= (float)nb;
  
  if ((*s1) < 0) {fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function scenario::computeErrScore; this should not occur");}
  
  // *s1 est très petit. Je prends une racine pour l'augmenter, sans pour autant passer la borne du 1
  (*s1) = sqrt(*s1);
  (*s1) = sqrt(*s1);
  (*s1) = sqrt(*s1);
  
  // tests NAN et INF
  //  if (!isFloatEqual(*s1 , *s1))        fatal_error(runManParam->FILE_ERROR_NAME, NANI, "in function scenario::computeErrScore e.");
  //  if ( isFloatEqual((*s1)-1, *s1))     fatal_error(runManParam->FILE_ERROR_NAME, INFI, "in function scenario::computeErrScore e.");
  if ((!isFloatEqual(*s1 , *s1)) | ( isFloatEqual((*s1)-1, *s1)))
    {
      delete[] massesExperimentales;                                                                memCheck.scenario--;
      delete[] massesAttendues;                                                                     memCheck.scenario--;
      return false;
    }    
  
  delete[] massesExperimentales;                                                                  memCheck.scenario--;
  delete[] massesAttendues;                                                                       memCheck.scenario--;
  return true;
}


// ********************************************************************************************** //

void scenario::find_a_and_b(float* a, float* b, float* bMassesExp, float* bMassesTheo, int n) {
  
 // calcul la droite y = ax + b qui permet de passer des valeurs theoriques aux valeurs observees
  float sx = 0, sy = 0, sxy = 0, sxx = 0; 
  float masstheo, massobs;
  float del;
  
  //pas assez de points 
  if(!n || (n == 1)){
    *b = 0;
    *a = 0;
    fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "not enough points for linear regression in function scenario::find_a_and_b()");
  }
  
  for (int i = 0; i < n; i++) {
    masstheo = bMassesTheo[i];
    massobs  = bMassesExp[i];
    sx	+= masstheo;
    sy	+= massobs;
    sxy	+= masstheo * massobs;
    sxx	+= masstheo * masstheo;
  }
  
  del = n * sxx - sx * sx;
  
  *b = (float)((sxx * sy - sx * sxy)/del);
  *a = (float)((n * sxy - sx * sy)/del); 
}

// ********************************************************************************************** //

float scenario::compute_the_variance(float a, float b, float* bMassesExp, float* bMassesTheo, int n) {
  // Calcul la distance moyenne d'un point a la droite issue de la regression lineaire 
  //   y = ax + b:	fait la somme pour chaque point (a * masseTheo + b) - masseObs au carre et divise par 
  //   le nbre de points 
    float variance = 0;
  
  for(int i = 0; i < n; i++){
    variance += get_deviation(i, a, b, bMassesExp, bMassesTheo, n);
  }
  variance /= n;
  
  return variance;
  
}

// ********************************************************************************************** //

float scenario::get_deviation(int i, float a, float b, float* bMassesExp, float* bMassesTheo, int n) {
  
  float dev;
  dev = a * bMassesTheo[i] + b - bMassesExp[i];	
  dev = fabs(dev);
  // dev *= dev;
  // dev /= ((a*a)+(b*b));
  //  if (dev < 0.000001) return 0.000001;
  return dev;
  
}


// ********************************************************************************************** //

void scenario::writeScenario()
{ 
  char filename[256];
  sprintf(filename, "%sSCENARIOS.txt", runManParam->OUTPUT_DIR);
  File fp(filename, "a");
  write(fp);
  fp.Close();
}

// ********************************************************************************************** //

void scenario::writeFun() 
{  
  fprintf(funGen->fpFUN, "> ");
  if (!strcmp(aPeptide->popiPep.dtbSeq, spectrumData->get_seqAsInDtb())) fprintf(funGen->fpFUN, "1 ");
  else                                                                   fprintf(funGen->fpFUN, "0 ");
  Scores.write(funGen->fpFUN);
  fprintf(funGen->fpFUN, ";\n");
}


// ********************************************************************************************** //


