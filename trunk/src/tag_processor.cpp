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
#include "tag_processor.h"
#include "defines.h"
#include "error.h"
#include "memorycheck.h"

#include "clicsearch.h"

using namespace std;

// ********************************************************************************************** //

extern memorycheck memCheck;

// ********************************************************************************************** //


tag_processor::tag_processor() 
{
  runManParam     = NULL;
  specStats       = NULL;
  aaParam         = NULL;
  spectrumData    = NULL;
  popiGraph       = NULL;
  aPeptide        = NULL; 
  popiTags        = NULL;
  funGen          = NULL;
  scenarios[0]    = NULL;
  scenarios[1]    = NULL;
}

// ********************************************************************************************** //

tag_processor::~tag_processor() 
{
  runManParam     = NULL;
  specStats       = NULL;
  aaParam         = NULL;
  spectrumData    = NULL;
  popiGraph       = NULL;
  aPeptide        = NULL; 
  popiTags        = NULL;
  funGen          = NULL; 
  scoreF          = NULL;
  if (scenarios[0] != NULL) {delete scenarios[0]; scenarios[0] = NULL;                            memCheck.scenario--;}
  if (scenarios[1] != NULL) {delete scenarios[1]; scenarios[1] = NULL;                            memCheck.scenario--;}
}

// ********************************************************************************************** //

void tag_processor::init(runManagerParameters* rMP, spectrumstatistics* specS, 
			 aa* aaP, ion* ionP, data* spectrumD, graphe *popiG, 
			 peptide* aPep, subseq* popiT, fun* funG, TREE *F, specresults* specR)
{
  runManParam        = rMP;
  specStats          = specS;
  aaParam            = aaP;
  ionParam           = ionP;
  spectrumData       = spectrumD;
  popiGraph          = popiG;
  aPeptide           = aPep;
  popiTags           = popiT;
  funGen             = funG;
  scoreF             = F;
  specResults        = specR;
  scenarios[0]       = new scenario();                                                            memCheck.scenario++;
  scenarios[1]       = new scenario();                                                            memCheck.scenario++;
  scenarios[0]->Scores.init();
  scenarios[1]->Scores.init();
  bestScenarioIndice = 0;
}

// ********************************************************************************************** //

void tag_processor::processTags()
{
  if (!aPeptide->ISRANDOM) 
    {
      specStats->pepWithAtLeastOneSubSeq++;
      specStats->cumSubSeqNb += popiTags->subSeqNb;
    }
  
  // CREE LE GRAPH DE COMPATIBILITE DE TAGS ET LANCE UNE RECHERCHE DE CLIQUES DESSUS
  compPredTab  = initCompPredTab(popiTags->subSeqNb);
  bool connect = fillCompPredTab(compPredTab);
  
  if ((runManParam->r_CHECK) && (!aPeptide->ISRANDOM))
    {
      char filename[256];
      sprintf(filename, "%sCOMPRED_TAB.txt", runManParam->OUTPUT_DIR);
      File fp(filename, "a");
      writeCompPredTab(compPredTab, popiTags->subSeqNb, fp);
      fp.Close();
    }
  
  // FUN MODE
  if ((runManParam->r_FUN) && (!aPeptide->ISRANDOM))
    {
      if (!strcmp(aPeptide->popiPep.dtbSeq, spectrumData->get_seqAsInDtb())) {
	fprintf(funGen->fpFUN, "NEWPOS %s against %s;\n", spectrumData->get_seqAsInDtb(), aPeptide->popiPep.dtbSeq);
      }
      
      else {
	fprintf(funGen->fpFUN, "NEWNEG %s against %s;\n", spectrumData->get_seqAsInDtb(), aPeptide->popiPep.dtbSeq); 
      } 
    }
  
  clicSearch.findCliques(&tag_processor::CallBackArr, this, compPredTab, popiTags->subSeqNb, connect);
  // --> rdv dans la fonction CallBackArr
  
  if ((runManParam->r_FUN) && (!aPeptide->ISRANDOM)) fprintf(funGen->fpFUN, ";\n");
  
  // MAINTENANT, TOUTES LES CLIQUES ONT ETE SCOREES;
  // LE MEILLEUR SCENARIO EST DANS scenarios[bestScenarioIndice];
  // 3 CAS:
  // 1) AUCUN SCENARIO PROPOSE --> scenarios[bestScenarioIndice].scenarioSeq est vide;
  // 2) AUCUN SCENARIO VALIDE  --> scenarios[bestScenarioIndice].hypoNb != manParam->m_MUTMOD
  // 3) SINON SCENARIO VALIDE A TENTER D'INSERER
  
  // INSERE L'ARRANGEMENT DANS LA RESULTLIST, S'IL EST SUFFISAMMENT BON
  if ((strlen(scenarios[bestScenarioIndice]->scenarioSeq) > 0) && (scenarios[bestScenarioIndice]->hypoNb == runManParam->m_MUTMOD))
    {
      if (!aPeptide->ISRANDOM)
	{
	  specStats->pepWithAtLeastOneScenario++; 
	  updateSpecResultsNEG();
	  if (runManParam->PLOT) {
	    if (strcmp(aPeptide->popiPep.dtbSeq, spectrumData->get_seqAsInDtb()))
	      fprintf(specResults->fpSCORES_NEG, "%.3f\n", scenarios[bestScenarioIndice]->Scores.finArrScore);
	  }
	}
      else 
	{
	  updateSpecResultsRANDOM(); 
	  if (runManParam->PLOT) fprintf(specResults->fpSCORES_RANDOM, "%.3f\n", scenarios[bestScenarioIndice]->Scores.finArrScore);
	}
      
      if   ((!specResults->lastElement->preceding->FILLED) 
	    | ((specResults->lastElement->preceding->FILLED) 
	       &&  (scenarios[bestScenarioIndice]->Scores.finArrScore > specResults->lastElement->preceding->Scenario->Scores.finArrScore)))
	{
	  
	    
	  specResults->add_and_sort(aPeptide, scenarios[bestScenarioIndice]); 
	}
    }         
  freeCompPredTab();
}

// ********************************************************************************************** //

void tag_processor::CallBackArr(void *pInstance, int size, int* clic)
{
  //  LA TAILLE DE LA CLIQUE EST SIZE, ET LA CLIQUE EST DANS LE TABLEAU clic
  
  tag_processor *pThis = (tag_processor *)pInstance;  
  if (size >= MAX_SUBSEQ_NB_IN_AN_ARRANGEMENT) fatal_error(pThis->runManParam->FILE_ERROR_NAME, MEMORY, "in function tag_processor::CallBackArr");
 
  pThis->scenarios[1 - pThis->bestScenarioIndice]->init(size, clic,   
							pThis->runManParam, pThis->aaParam,  pThis->ionParam,
							pThis->spectrumData, pThis->popiGraph, 
							pThis->aPeptide, pThis->popiTags, 
	        					pThis->scoreF,       pThis->funGen);
  pThis->processClique();

}

// ********************************************************************************************** //

void tag_processor::processClique()
{
  
  scenarios[1 - bestScenarioIndice]->buildScenario();

  // controle si le scénario est valable

  // condition 1: nombre de gaps (hypoNb)
  // condition 2: taille des modifs
  // condition 3: prescoring couverture
  
  
  if (runManParam->m_MUTMOD != scenarios[1 - bestScenarioIndice]->hypoNb)
    {
      scenarios[1 - bestScenarioIndice]->free();
      return;
    }
  
  float valeur;
  for (int i = 0; i < scenarios[1 - bestScenarioIndice]->hypoNb; i++) {
    if (abs(scenarios[1 - bestScenarioIndice]->shift[i]) > runManParam->FRAGMENT_ERROR2)
      {
	valeur = scenarios[1 - bestScenarioIndice]->shift[i];
	if ((valeur < runManParam->LOW_LIMIT_RANGE_MOD) | (valeur > runManParam->UP_LIMIT_RANGE_MOD))
	  {  
	    scenarios[1 - bestScenarioIndice]->free();
	    return;
	  }
      }
  }
  
  if (!scenarios[1 - bestScenarioIndice]->computeScores())  // si cov. pas atteint, ou problème de NAN ou INF, computeScores renvoie false
    {
      scenarios[1 - bestScenarioIndice]->free();
      return;
    }
  
  if (!aPeptide->ISRANDOM) 
    {
      specStats->cumScenarioNb++; 
      if (runManParam->r_FUN)   scenarios[1 - bestScenarioIndice]->writeFun();
      if (runManParam->r_CHECK) scenarios[1 - bestScenarioIndice]->writeScenario();
    }
      
  // PLACE bestScenarioIndice sur le meilleur scenario
  if (scenarios[1 - bestScenarioIndice]->Scores.finArrScore >= scenarios[bestScenarioIndice]->Scores.finArrScore)
    {
      bestScenarioIndice = 1 - bestScenarioIndice;
    }
  
  scenarios[1 - bestScenarioIndice]->free();
}

// ********************************************************************************************** //

bool** tag_processor::initCompPredTab(int n)
{ 
  bool** tab;
  tab = new bool*[n];                                                                             memCheck.tagprocessor++;
  for (int i = 0; i < n; i++) 
    {
      tab[i] = new bool[n];                                                                       memCheck.tagprocessor++;
    }
  
  // PAS BESOIN D'INITIALISER, C'EST FAIT PLUS TARD
  
  return tab;
}

// ********************************************************************************************** //

void tag_processor::freeCompPredTab()
{
  for (int i = 0; i < popiTags->subSeqNb; i++) 
    {
      if (compPredTab[i] != NULL) {delete[] compPredTab[i]; compPredTab[i] = NULL;                memCheck.tagprocessor--;}
    }
  if (compPredTab != NULL)        {delete[] compPredTab;    compPredTab    = NULL;                memCheck.tagprocessor--;}
}

// ********************************************************************************************** //

bool tag_processor::fillCompPredTab(bool** compPredTab) {
  // compPredTab est un graph de compatibilité entre séquences
  // sont incompatibles deux séquences qui partagent un même pic et celles qui ont une incohérence entre
  // leur position de début et leur beginMass (règle de précédence)

  // return true si le graphe est complet (dans ce cas, il n'y a pas à faire de recherche de cliques)
  
  bool connect = true;
  bool CONTROL = true;

  double seqMass1;
  double seqMass2;
  
  int    iBegPos,   jBegPos;
  int    iEndPos,   jEndPos;
  double iBegMass,  jBegMass;
  double iEndMass,  jEndMass;
  char   iSeq[256], jSeq[256];

  if (popiTags->subSeqNb == 0) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function tag_processor::fillCompPredTab()");
  if (popiTags->subSeqNb == 1) {
    compPredTab[0][0] = true;
    return connect;
  }

  for (int i = 0; i < popiTags->subSeqNb; i++) {
    for (int j = i; j < popiTags->subSeqNb; j++) {
      CONTROL = FALSE;
      // toutes les cases dont d'abord initialisées à true puis changées à false si nécessaire
      compPredTab[i][j] = true;
      compPredTab[j][i] = true;
      if (i == j) continue;            // une séquence est toujours compatible avec elle-même...
      
      // ---------------------------------------------------------------------------
      // Mnb         = 0;
      // Cnb         = 0;
      
      iBegPos     = popiTags->subSeqTab[i].posBegin;
      jBegPos     = popiTags->subSeqTab[j].posBegin;
      iEndPos     = popiTags->subSeqTab[i].posBegin+popiTags->subSeqTab[i].lenght;
      jEndPos     = popiTags->subSeqTab[j].posBegin+popiTags->subSeqTab[j].lenght;
      iBegMass    = popiTags->subSeqTab[i].massBegin;
      jBegMass    = popiTags->subSeqTab[j].massBegin;
      iEndMass    = popiTags->subSeqTab[i].massEnd;
      jEndMass    = popiTags->subSeqTab[j].massEnd;
      strcpy(iSeq,  popiTags->subSeqTab[i].parsedSeq);
      strcpy(jSeq,  popiTags->subSeqTab[j].parsedSeq);
      seqMass1    = 0;
      seqMass2    = 0;

      // -----------------------------------------------------------------------------
      
      // COMPATIBILITE DE PRECEDENCE
      
      // -----------------------------------------------------------------------------
      
      // CAS1: LES TAGS SONT COMPLETEMENT CHEVAUCHANTS
      //    ----
      //    ----
      
      if ((iBegPos == jBegPos) && (iEndPos == jEndPos)) 
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j); 
	}

      // -----------------------------------------------------------------------------
      
      // CAS2: TAG i EST INCLU DANS j ET RECIPROQUEMENT
      
      // ------------   i
      //     ----       j
      
      if ((iBegPos <  jBegPos) && (iEndPos >  jEndPos)) 
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j); 
	}
      
      // -----------------------------------------------------------------------------
      
      //     ----       i
      // ------------   j
      
      if ((iBegPos >  jBegPos) && (iEndPos <  jEndPos)) 
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j); 
	}
      
      // -----------------------------------------------------------------------------
      
      // CAS3: LES TAGS SONT PARTIELLEMENT CHEVAUCHANTS
      // TAG i EST DECALE VERS L'AVANT
      
      // ----------        i
      //     ----------    j
      
      if ((iBegPos < jBegPos) && (iEndPos < jEndPos) && (iEndPos > jBegPos)) 
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j); 
	}
      
      // -----------------------------------------------------------------------------
      
      // TAG i EST DECALE VERS L'ARRIERE
      
      //       ----------    i
      //   ----------        j
            
      if ((iBegPos > jBegPos) && (iEndPos > jEndPos) && (iBegPos < jEndPos)) 
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j);
	}
      
      // -----------------------------------------------------------------------------
      
      // CAS 4:
      //    ------------   i
      //    -------        j
      
      if ((iBegPos == jBegPos) && (iEndPos > jEndPos))
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j);
	}
      
      // -----------------------------------------------------------------------------
      
      //    -------        i
      //    ------------   j
      
      if ((iBegPos == jBegPos) && (iEndPos < jEndPos))
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j);
	}
      
      // -----------------------------------------------------------------------------
      
      // CAS 5:
      //    ------------   i
      //         -------   j
      
      if ((iBegPos < jBegPos) && (iEndPos == jEndPos))
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j);
	}
      
      // -----------------------------------------------------------------------------
      
      //         -------   i
      //    ------------   j
      
      if ((iBegPos > jBegPos) && (iEndPos == jEndPos))
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j);
	}
      
      // -----------------------------------------------------------------------------
      
      // CAS 6: TAGS NON CHEVAUCHANTS
      // i AVANT j
      // -----               i
      //           -----     j
      
      if ((iBegPos < jBegPos) && (iEndPos < jEndPos) && (iEndPos < jBegPos)) 
	{
	  BugVerify(&CONTROL);
	  // trouve la masse de la séquence entre les tags grâce à la séquence dtb
	  //	  seqMass1 = computeSeqMass(iEndPos, jBegPos, dtbSeq, &Cnb, &Mnb);
	  seqMass1 = computeSeqMass(iEndPos, jBegPos, aPeptide->popiPep.dtbSeq);

	  if (!runManParam->m_MUTMOD) 
	    {
	      //     if (fabs(iEndMass + seqMass1 - jBegMass) > runManParam->FRAGMENT_ERROR2 + dC*Cnb + dM*Mnb)
	      if (fabs(iEndMass + seqMass1 - jBegMass) > runManParam->FRAGMENT_ERROR2)
		{
		  connect = setCompPredTabToFalse(compPredTab, i,j);
		}
	    }
	  if (runManParam->m_MUTMOD) 
	    {
	      //	      if ((iBegMass > jBegMass) | (iEndMass > jEndMass)) 
	      if (iEndMass > jBegMass)
		{
		  connect = setCompPredTabToFalse(compPredTab, i,j); 
		}
	    }
	}
      
      // -----------------------------------------------------------------------------
      
      // i APRES j
      //            -----    i
      //  -----              j
      
      if ((iBegPos > jBegPos) && (iEndPos > jEndPos) && (iBegPos > jEndPos)) 
	{
	  BugVerify(&CONTROL);
	  // trouve la masse de la séquence entre les tags grâce à la séquence dtb
	  //	  seqMass1 = computeSeqMass(jEndPos, iBegPos, dtbSeq, &Cnb, &Mnb);
	  seqMass1 = computeSeqMass(jEndPos, iBegPos, aPeptide->popiPep.dtbSeq);
	  
	  if (!runManParam->m_MUTMOD) 
	    {
	      //     if (fabs(jEndMass + seqMass1 - iBegMass) > runManParam->FRAGMENT_ERROR2 + dC*Cnb + dM*Mnb)
	      if (fabs(jEndMass + seqMass1 - iBegMass) > runManParam->FRAGMENT_ERROR2)
		{
		  connect = setCompPredTabToFalse(compPredTab, i,j);
		}
	    }
	  if (runManParam->m_MUTMOD) 
	    {
	      //      if ((iBegMass < jBegMass) && (iEndMass < jEndMass))
	      if (jEndMass > iBegMass)
		{
		  connect = setCompPredTabToFalse(compPredTab, i,j);
		}
	    }
	}
      
      // -----------------------------------------------------------------------------
      
      // CAS 7:
      // i CONTIGU A j

      // i AVANT j
      //  -----          i
      //       -----     j

      if ((iBegPos < jBegPos) && (iEndPos < jEndPos) && (iEndPos == jBegPos)) 
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j);
	}
      
      // -----------------------------------------------------------------------------
      // i APRES j
      //       -----   i
      //  -----        j
      
      if ((iBegPos > jBegPos) && (iEndPos > jEndPos) && (iBegPos == jEndPos)) 
	{
	  // CE CAS N'EST PAS AUTORISE
	  BugVerify(&CONTROL);
	  connect = setCompPredTabToFalse(compPredTab, i,j);
	}
      if (compPredTab[i][j] == true) {  // ne le fait que si cela peut changer quelquechose, sinon ça prend du temps pour rien
	//	if (!compaPeaks(i,j)) connect = setCompPredTabToFalse(i,j);
	if (!compaNodes(i,j)) connect = setCompPredTabToFalse(compPredTab, i,j);
      }

     
      if (CONTROL == false) {
      	fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function tag_processor::fillCompPredTab() b.");}
    }  // END FOR j
  }    // END FOR i
  
  return connect;
}

// ********************************************************************************************** //

void tag_processor::BugVerify(bool* CONTROL) {
  if (*CONTROL == true) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function tag_processor::BugVerify()");
  *CONTROL = true;
}

// ********************************************************************************************** //


bool tag_processor::setCompPredTabToFalse(bool** compPredTab, int i, int j) {
  compPredTab[i][j] = false;
  compPredTab[j][i] = false;
  return false;   // valeur pour connect
}

// ********************************************************************************************** //

double tag_processor::computeSeqMass(int from, int to, char* seq) {
  int    iaa;
  double mass = 0;

  for (int k = from; k < to; k++) 
    {
      iaa = aaParam->findIndice(seq[k]);
      mass += aaParam->aaTable[iaa].mass;
    }
  return mass;
}

// ********************************************************************************************** //

bool tag_processor::compaPeaks(int i, int j) {
  
  for (int k = 0; k < spectrumData->get_peakNb(); k++) 
    {
      if ((popiTags->subSeqTab[i].or_pL[k] != 0) && (popiTags->subSeqTab[j].or_pL[k] != 0)) return false;
    }
  return true;
}

// ********************************************************************************************** //

bool tag_processor::compaNodes(int i, int j) {
  
  for (int k = 0; k < popiGraph->get_orVertexNb(); k++) 
    {
      if ((popiTags->subSeqTab[i].or_nL[k] != 0) && (popiTags->subSeqTab[j].or_nL[k] != 0)) return false;
    }
  return true;
}

// ********************************************************************************************** //

void tag_processor::writeCompPredTab(bool** tab, int n, File &fp) {
  
  fprintf(fp, "\n----------------------------\n%s\n\n   ", aPeptide->popiPep.dtbSeq);
  
  for (int i = 0; i < n; i++) fprintf(fp, "%2i", i);
  fprintf(fp, "\n");

  for (int i = 0; i < n; i++) {
    fprintf(fp, "%3i ", i);
    for (int j = 0; j < n; j++) {
      if (tab[i][j] == true) fprintf(fp, "x ");
      else                           fprintf(fp, "  "); 
    }  
    fprintf(fp, "\n");
  }
}

// ********************************************************************************************** //

void tag_processor::updateSpecResultsNEG()
{
  // UPDATE LA MOYENNE ET L'ECART TYPE POUR LE CALCUL DE LA P-VALUE (POUR LE SPECTRE COURANT)
  specResults->MEAN_NEG     += (float)scenarios[bestScenarioIndice]->Scores.finArrScore;
  specResults->VAR_NEG      += (float)((scenarios[bestScenarioIndice]->Scores.finArrScore) * (scenarios[bestScenarioIndice]->Scores.finArrScore));
  specResults->ECH_SIZE_NEG ++;
}

// ********************************************************************************************** //

void tag_processor::updateSpecResultsRANDOM()
{
  // UPDATE LA MOYENNE ET L'ECART TYPE POUR LE CALCUL DE LA P-VALUE (POUR LE SPECTRE COURANT)
  specResults->MEAN_RANDOM     += (float)scenarios[bestScenarioIndice]->Scores.finArrScore;
  specResults->VAR_RANDOM      += (float)((scenarios[bestScenarioIndice]->Scores.finArrScore) * (scenarios[bestScenarioIndice]->Scores.finArrScore));
  specResults->ECH_SIZE_RANDOM ++;
}

// ********************************************************************************************** //

