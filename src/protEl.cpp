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
#include <fstream>

#include "protEl.h"
#include "element.h"
#include "error.h"
#include "defines.h"
#include "data.h"
#include "compare.h"

using namespace std;

// ********************************************************************************************** //


/* GLOBAL VARIABLES DECLARED ELSEWHERE */


// ********************************************************************************************** //


protEl::protEl(){
  
  FILLED               = FALSE;
  cov                  =  0;
  
  similarProtNb        =  0;
  SpSourceNb           =  0;
  
  preceding      = NULL;
  following      = NULL;
}

// ------------------------------------------------------------------------

/* DESTRUCTOR */

protEl::~protEl() {
  preceding = NULL; 
  following = NULL; 
}

// ------------------------------------------------------------------------

void protEl::fillIt(element* cE, int i, char* title, char* seqAsInDtb, float PMraw, float PM, int specID, char* fileErrorName) 
{
  FILLED           = TRUE;

	protIds[0].dtbId = cE->Peptide->getProtein(i)->m_reloadDBEntry.m_iDBFileIndex; 
  protIds[0].offId = cE->Peptide->getProtein(i)->m_reloadDBEntry.m_uiEntryOffset;
  
  strcpy(protIds[0].AC, cE->Peptide->getProtein(i)->AC);
  strcpy(protIds[0].ID, cE->Peptide->getProtein(i)->ID);
  strcpy(protIds[0].DE, cE->Peptide->getProtein(i)->DE);
  similarProtNb ++;
  if (similarProtNb >= MAX_SIMILAR_PROT) fatal_error(fileErrorName, MEMORY, "in function prot::compactElements(); please increase MAX_SIMILAR_PROT in defines.h"); 

  strcpy(SpSource[SpSourceNb].title,   title);
  strcpy(SpSource[SpSourceNb].dtbSeq,  cE->Peptide->popiPep.dtbSeq);
  SpSource[SpSourceNb].specID        = specID;
  SpSource[SpSourceNb].rank          = cE->rank;
  SpSource[SpSourceNb].finScore      = (float)cE->Scenario->Scores.finArrScore;
  SpSource[SpSourceNb].Zscore        = cE->ZSCORE_NEG;
  SpSource[SpSourceNb].parentMassRaw = PMraw;
  SpSource[SpSourceNb].parentMassM   = PM;
  SpSource[SpSourceNb].dtbPepMass    = cE->Peptide->popiPep.dtbPepMass;
  SpSource[SpSourceNb].delta         = SpSource[SpSourceNb].parentMassM - SpSource[SpSourceNb].dtbPepMass;
  SpSource[SpSourceNb].startPos      = cE->Peptide->posStart[i];
  SpSource[SpSourceNb].endPos        = cE->Peptide->posEnd[i];
  
  if (!strcmp(cE->Peptide->popiPep.dtbSeq, seqAsInDtb))      SpSource[SpSourceNb].IS_CORRECT = true;
  else                                                       SpSource[SpSourceNb].IS_CORRECT = false;

  SpSourceNb++;  
  if (SpSourceNb >= MAX_SP_SOURCE) {fatal_error(fileErrorName, MEMORY, "in function protEl::fillIt(); please increase MAX_SP_SOURCE in defines.h");}
}

// ********************************************************************************************** //

void protEl::updateIt(element* cE, int i, char* title, char* seqAsInDtb, float PMraw, float PM, int specID, char* fileErrorName) 
{
  
  strcpy(SpSource[SpSourceNb].title,   title);
  strcpy(SpSource[SpSourceNb].dtbSeq,  cE->Peptide->popiPep.dtbSeq);
  SpSource[SpSourceNb].specID        = specID;
  SpSource[SpSourceNb].rank          = cE->rank;
  SpSource[SpSourceNb].finScore      = (float)cE->Scenario->Scores.finArrScore;
  SpSource[SpSourceNb].Zscore        = cE->ZSCORE_NEG;
  SpSource[SpSourceNb].parentMassRaw = PMraw;
  SpSource[SpSourceNb].parentMassM   = PM;
  SpSource[SpSourceNb].dtbPepMass    = cE->Peptide->popiPep.dtbPepMass;
  SpSource[SpSourceNb].delta         = SpSource[SpSourceNb].parentMassM - SpSource[SpSourceNb].dtbPepMass;
  SpSource[SpSourceNb].startPos      = cE->Peptide->posStart[i];
  SpSource[SpSourceNb].endPos        = cE->Peptide->posEnd[i];

  if (!strcmp(cE->Peptide->popiPep.dtbSeq, seqAsInDtb))      SpSource[SpSourceNb].IS_CORRECT = true;
  else                                                       SpSource[SpSourceNb].IS_CORRECT = false;

  SpSourceNb++;  
  if (SpSourceNb >= MAX_SP_SOURCE) {fatal_error(fileErrorName, MEMORY, "in function protEl::updateIt(); please increase MAX_SP_SOURCE in defines.h");}
}

// ------------------------------------------------------------------------

void protEl::computeCov()
{
  for (int k = 0; k < SpSourceNb; k++) 
    {
      cov = cov + ((SpSource[k].endPos - SpSource[k].startPos)/ SpSource[k].rank);
    }
}

// ------------------------------------------------------------------------
