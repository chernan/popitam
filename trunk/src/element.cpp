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
#include <iostream>

#include "element.h"
#include "memorycheck.h"
#include "data.h"
#include "utils.h"
#include "error.h"
#include "defines.h"


using namespace std;


// ********************************************************************************************** //

/* GLOBAL VARIABLES DECLARED ELSEWHERE */

extern memorycheck    memCheck;

// ********************************************************************************************** //

element::element()
{
  FILLED          = false;    // element non "visité"
  rank            = 0;
  Peptide         = NULL;
  Scenario        = NULL;
  ZSCORE_NEG      = (float)FLOAT_LIMIT_NEG;
  PVALUE_NEG      = FLOAT_LIMIT_POS;
  ECH_SIZE_NEG    = 0;
  ZSCORE_RANDOM   = (float)FLOAT_LIMIT_NEG;
  PVALUE_RANDOM   = FLOAT_LIMIT_POS;
  ECH_SIZE_RANDOM = 0;
  preceding       = NULL;
  following       = NULL;
}

// ********************************************************************************************** //

element::~element() 
{
  if (Peptide   != NULL) {delete Peptide;  Peptide  = NULL;                                       memCheck.peptide--;}
  if (Scenario  != NULL) {delete Scenario; Scenario = NULL;                                       memCheck.scenario--;}
  preceding      = NULL; 
  following      = NULL; 
}

// ------------------------------------------------------------------------

void element::fillElement(peptide* pep, scenario* scen) {

  FILLED               = true;
  rank                 = 0;
  
  // je copie les infos essentielles de pep et scen
  Peptide   = new peptide();                                                                      memCheck.peptide++;
  Peptide->ISRANDOM                  = pep->ISRANDOM;
  Peptide->popiPep.dtbPepMass        = pep->popiPep.dtbPepMass;
  strcpy(Peptide->popiPep.dtbSeq, pep->popiPep.dtbSeq);
  Peptide->exemplairesNb             = pep->exemplairesNb;
 
  for (int i = 0; i < pep->exemplairesNb; i++)
    {
      Peptide->myProt[i] = new Protein;                                                          memCheck.digest++;
      Peptide->myProt[i]->init(pep->myProt[i]->m_reloadDBEntry);
      Peptide->posStart[i]           = pep->posStart[i];
      Peptide->posEnd[i]             = pep->posEnd[i];
    }
  
  Scenario  = new scenario();                                                                     memCheck.scenario++;
  Scenario->shiftNb                   = scen->shiftNb;
  Scenario->shift = new float[scen->shiftNb];                                                     memCheck.scenario++;                                                
  
  for (int i = 0; i < scen->shiftNb; i++)
    {
      Scenario->shift[i]   = scen->shift[i];
    }
  strcpy(Scenario->scenarioSeq, scen->scenarioSeq);
  Scenario->Scores.finArrScore = scen->Scores.finArrScore;
  
  preceding      = NULL;
  following      = NULL; 
}

// ********************************************************************************************** //

void element::update(peptide* pep, char* FileErrorName)
{
  int nbExp = Peptide->exemplairesNb;
  Peptide->myProt[nbExp] = new Protein;   memCheck.digest++;
  
  Peptide->myProt[nbExp]->init( pep->myProt[0]->m_reloadDBEntry );
  Peptide->posStart[nbExp]           = pep->posStart[0];
  Peptide->posEnd[nbExp]             = pep->posEnd[0]; 
  Peptide->exemplairesNb++;
  if (Peptide->exemplairesNb >= MAX_DOUBLONS) fatal_error(FileErrorName, MEMORY, "please increase MAX_DOUBLONS in defines.H");
}

// ********************************************************************************************** //



