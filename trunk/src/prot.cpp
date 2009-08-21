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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstring>

#include "utils.h"
#include "protEl.h"
#include "prot.h"
#include "defines.h"
#include "memorycheck.h"
#include "params.h"
#include "error.h"
#include "runManagerParameters.h"

using namespace std;

// ********************************************************************************************** //

/* GLOBAL VARIABLES DECLARED ELSEWHERE */

extern memorycheck memCheck;

// ********************************************************************************************** //

/* COMPARISON FUNCTION FOR QSORT */

int compar_SpSource(const void *a, const void *b)
{
  if (((SOURCE*)a)->parentMassM < ((SOURCE*)b)->parentMassM) return -1;
  if (((SOURCE*)a)->parentMassM > ((SOURCE*)b)->parentMassM) return  1;
                                                             return  0;
}

// ********************************************************************************************** //

prot::prot() 
{
  runManParam     = NULL;
}

// ********************************************************************************************** //

void prot::init(runManagerParameters* rMP) 
{
  runManParam                = rMP;
  elementNb                  = 0;
  currentElement             = NULL;
  firstElement               = new protEl();                                                        memCheck.prot++;
  lastElement                = new protEl();                                                        memCheck.prot++;
  firstElement->following    = lastElement;
  lastElement->preceding     = firstElement;
  firstElement->preceding    = NULL;
  lastElement->following     = NULL;
  currentElement              = firstElement;
}

// ********************************************************************************************** //

prot::~prot() {
  
  runManParam     = NULL;

  currentElement = firstElement->following;
  
  while(currentElement != lastElement) {
    removeElement(currentElement);
    currentElement = firstElement->following;
  }
  
  // remove t�te et queue
  delete firstElement;                                                                              memCheck.prot--;
  firstElement = NULL;
  delete lastElement;                                                                               memCheck.prot--;
  lastElement = NULL;
}

// ********************************************************************************************** //

void prot::compile()
{
  computeElCovs();
  sort();
  sortPepByParentMass();
  removeDuplicatedProteins();
}

// ********************************************************************************************** //

void prot::removeElement(protEl* pel){
  // remove the element el 
  pel->preceding->following = pel->following;
  pel->following->preceding = pel->preceding;
  delete pel;                                                                                         memCheck.prot--;
  pel = NULL;
  elementNb--;
  if (elementNb < 0) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in function protResults::removeElement()");
}

// ********************************************************************************************** //



// ********************************************************************************************** //

void prot::update(element* cE, specresults* specR, int ID) 
{
  
  bool    PRESENT     = true;
  
  for (int i = 0; i < cE->Peptide->exemplairesNb; i++) 
    {
      PRESENT = false;
      
      currentElement = firstElement->following;
      while (currentElement != lastElement)
	{
	  //if (   (currentElement->protIds[0].dtbId == cE->Peptide->getProtein(i)->m_reloadDBEntry.m_iDBFileIndex) 
	  //    && (currentElement->protIds[0].offId == cE->Peptide->getProtein(i)->m_reloadDBEntry.m_uiEntryOffset)   )
	    if (currentElement->protIds[0].prot == cE->Peptide->getProtein(i))
		    
	    {
	      PRESENT = true;
	      currentElement->updateIt(cE, i, 
				       specR->spectrumData->get_title(),         specR->spectrumData->get_seqAsInDtb(), 
				       (float)specR->spectrumData->get_parentMassRAW(), (float)specR->spectrumData->get_parentMassM(), ID, runManParam->FILE_ERROR_NAME);
	    }
	  currentElement = currentElement->following;
	}
      if (PRESENT == false)
	{
	  newEl = new protEl();                                                                     memCheck.prot++;
	  newEl->fillIt(cE, i, 
			specR->spectrumData->get_title(),         specR->spectrumData->get_seqAsInDtb(), 
			(float)specR->spectrumData->get_parentMassRAW(), (float)specR->spectrumData->get_parentMassM(), ID, runManParam->FILE_ERROR_NAME);
	  // connections
	  lastElement->preceding->following = newEl;
	  newEl->preceding = lastElement->preceding;
	  lastElement->preceding = newEl;
	  newEl->following = lastElement;
	  elementNb++;
	}
    }
}

// ********************************************************************************************** //

void prot::computeElCovs() 
{
  currentElement = firstElement->following;
  while (currentElement != lastElement)
    {
      currentElement->computeCov();
      currentElement = currentElement->following;
    }
}

/******************************************************************************/

void prot::sort()
{
  protEl *pE, *maxE, *iE, *tempo, *pEc, *nextE;

  iE = maxE = pE = nextE = firstElement->following;
  
  while (nextE != lastElement)
    {
      maxE = pE;
      iE   = nextE = pE->following;

      while (iE != lastElement)
	{
	  if (pE->cov < iE->cov) 
	    {
	      if (iE->cov > maxE->cov) maxE = iE;  // store le max
	    }
	  iE = iE->following;
	}
      // permute maxE avec pE
      if (maxE != pE)
	{
	  pEc                         = pE;
	  maxE->preceding->following  = pEc;
	  pEc->preceding->following   = maxE;
	  tempo                       = maxE->preceding;
	  maxE->preceding             = pEc->preceding;
	  pEc->preceding              = tempo;
	  
	  pEc->following->preceding   = maxE;
	  maxE->following->preceding  = pEc;
	  tempo                       = pEc->following;
	  pEc->following              = maxE->following;
	  maxE->following             = tempo;
	}
      pE = nextE;
    }
}

/******************************************************************************/

void prot::sortPepByParentMass()
{
  currentElement = firstElement->following;
  while (currentElement != lastElement)
    {
      qsort(currentElement->SpSource, currentElement->SpSourceNb, sizeof(SOURCE), compar_SpSource);
      currentElement = currentElement->following;
    }
}

/******************************************************************************/

void prot::removeDuplicatedProteins()  // n'appeler cette fonction qu'� la fin
{
  currentElement = firstElement->following;
  while (currentElement->following != lastElement)
    {
      while ((compareElements(currentElement, currentElement->following)) && (currentElement->following != lastElement))
	{
	  compactElements(currentElement, currentElement->following);
	}
    
      currentElement = currentElement->following;
      if (currentElement == lastElement) break;
    }
}

/******************************************************************************/

bool prot::compareElements(protEl* el1, protEl* el2)
{
  if (!isFloatEqual(el1->cov, el2->cov))  return false;
  if (el1->SpSourceNb != el2->SpSourceNb) return false;
  for (int i = 0; i < el1->SpSourceNb; i++)
    {
      if (strcmp(el1->SpSource[i].dtbSeq, el2->SpSource[i].dtbSeq)) return false;
    }
  return true;
}

/******************************************************************************/

void prot::compactElements(protEl* el1, protEl* el2)
{
  //el1->protIds[el1->similarProtNb].dtbId = el2->protIds[0].dtbId;  // el2 sera toujours � similarProtNb =0;
  //el1->protIds[el1->similarProtNb].offId = el2->protIds[0].offId;
  el1->protIds[el1->similarProtNb].prot = el2->protIds[0].prot;
  
  strcpy(el1->protIds[el1->similarProtNb].AC, el2->protIds[0].AC);
  strcpy(el1->protIds[el1->similarProtNb].ID, el2->protIds[0].ID);
  strcpy(el1->protIds[el1->similarProtNb].DE, el2->protIds[0].DE);
  el1->similarProtNb ++;
  if (el1->similarProtNb >= MAX_SIMILAR_PROT){
    fatal_error(runManParam->FILE_ERROR_NAME, MEMORY, "in function prot::compactElements(); please increase MAX_SIMILAR_PROT in defines.h"); 
  }
  removeElement(el2);
}

/******************************************************************************/

void prot::writeSimple(File &fp) 
{
  bool GOT_RESULTS = false;

  fprintf(fp, "\n\nRESULTS BY PROTEIN (MINIMUM PEPTIDE COVERAGE REQUIRED FOR DISPLAY IS %i): \n\n",runManParam->MIN_PEP_PER_PROT );
  
  currentElement = firstElement->following;
  while (currentElement != lastElement)
    { 
      if (currentElement->SpSourceNb < runManParam->MIN_PEP_PER_PROT) 
	{
	  currentElement = currentElement->following;
	  continue;
	}

      GOT_RESULTS = true;
      fprintf(fp,"COVERAGE =%.1f         MATCHED PEPTIDE NB=%i\n\n", currentElement->cov, currentElement->SpSourceNb);
      for (int l = 0; l < currentElement->similarProtNb; l++)
	{
	  fprintf(fp,"%s, %s\n", currentElement->protIds[l].ID, currentElement->protIds[l].DE);
	}
      fprintf(fp,"\n");

      fprintf(fp, "%5s %15s %10s %10s %10s %s \n", "rank  ", "score      ", "delta   ", "start   ", "end   ", "  seq");

      for (int k = 0; k < currentElement->SpSourceNb; k++) 
	{
	  fprintf(fp,"%3i %15.2f %10.2f %10i %10i      %s ", currentElement->SpSource[k].rank, currentElement->SpSource[k].finScore, currentElement->SpSource[k].delta, currentElement->SpSource[k].startPos, currentElement->SpSource[k].endPos, currentElement->SpSource[k].dtbSeq);
	  
	  if (runManParam->s_IDSET)
	    {
	      if (currentElement->SpSource[k].IS_CORRECT) fprintf(fp,"(YES!)");
	      else          		                  fprintf(fp,"(BEUH)");
	    }
	  fprintf(fp,"\n");
	}
      
      fprintf(fp, "\n---------------------------------------------------------------------------\n");
      currentElement = currentElement->following;
    }
  if (GOT_RESULTS == false) fprintf(fp, "NO CANDIDATE PROTEIN \n");
}

/******************************************************************************/

void prot::write(File &fp) 
{

  fprintf(fp, "\n\nRESULTS BY PROTEIN: \n\n");
  
  currentElement = firstElement->following;
  while (currentElement != lastElement)
    { 
      if (currentElement->SpSourceNb < runManParam->MIN_PEP_PER_PROT) 
	{
	  currentElement = currentElement->following;
	  continue;
	}
      
      fprintf(fp,"COVERAGE=%.2f MATCHED PEPTIDE NB=%i\n\n", currentElement->cov, currentElement->SpSourceNb);
      for (int l = 0; l < currentElement->similarProtNb; l++)
	{
	  fprintf(fp,"%s, %s\n", currentElement->protIds[l].ID, currentElement->protIds[l].DE);
	}
      fprintf(fp,"\n");

      fprintf(fp, "rank\tscore\tzScore\tdelta\tstart\tend\tseq\n");

      for (int k = 0; k < currentElement->SpSourceNb; k++) 
	{
	  fprintf(fp,"%i\t%.2f\t%.2f\t%.2f\t%i\t%i\t%s ", currentElement->SpSource[k].rank, currentElement->SpSource[k].finScore, currentElement->SpSource[k].Zscore, currentElement->SpSource[k].delta, currentElement->SpSource[k].startPos, currentElement->SpSource[k].endPos, currentElement->SpSource[k].dtbSeq);
	  
	  if (runManParam->s_IDSET)
	    {
	      if (currentElement->SpSource[k].IS_CORRECT) fprintf(fp,"(YES!)");
	      else          		                  fprintf(fp,"(BEUH)");
	    }
	  fprintf(fp,"\n");
	}
      
      fprintf(fp, "\n---------------------------------------------\n");
      currentElement = currentElement->following;
    }
}

/******************************************************************************/

