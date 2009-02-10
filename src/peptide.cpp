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

#include "peptide.h"
#include <cstring>
#include <cstdlib>
#include "memorycheck.h"

using namespace std;

// ********************************************************************************************** //

extern memorycheck memCheck; 

// ********************************************************************************************** //

peptide::peptide()
{  
	runManParam = NULL;
	aaParam = NULL;
	myProt = new Protein*[MAX_DOUBLONS];                                          memCheck.digest++;
	for (int i = 0; i < MAX_DOUBLONS; i++) myProt[i] = NULL;
  
}

// ********************************************************************************************** //

peptide::~peptide() 
{
  runManParam = NULL;
  aaParam     = NULL;
  if(myProt) {
    for(int i=0; i<MAX_DOUBLONS; i++) {
      if(myProt[i])	{delete myProt[i]; myProt[i] = NULL;                         memCheck.digest--;}
    }
    delete[] myProt; myProt = NULL;                                                  memCheck.digest--;
    myProt=NULL;
  }
}

// ********************************************************************************************** //

void peptide::init(runManagerParameters* rMP, aa* aaP, float dbM, char* seq,  int start, int end, DBReader *pDBReader, bool IR)
{
  runManParam           = rMP;
  aaParam               = aaP;
  ISRANDOM              = IR;

  popiPep.dtbPepMass    = dbM;
  strcpy(popiPep.dtbSeq,  seq);
  exemplairesNb         = 1;
  posStart[0]           = start;
  posEnd[0]             = end;
  myProt[0] = new Protein;                                                               memCheck.digest++;
  myProt[0]->init(pDBReader);
  computeLayers();
}

// ********************************************************************************************** //

char* peptide::getSeqAAcar() 
{
  return popiPep.dtbSeq;
}

// ********************************************************************************************** //

Sequence peptide::getSeqAA()
{
  Sequence Seq(popiPep.dtbSeq);
  return Seq;
}


// ********************************************************************************************** //


void peptide::computeLayers()
{
  int indice = aaParam->findIndice(popiPep.dtbSeq[0]);
  popiPep.bLayer[0] = aaParam->aaTable[indice].mass + aaParam->H_mass;
  for (int i = 1, n=(int)strlen(popiPep.dtbSeq); i < n; i++) {
    indice = aaParam->findIndice(popiPep.dtbSeq[i]);
    popiPep.bLayer[i] = popiPep.bLayer[i-1] + aaParam->aaTable[indice].mass;
  }
}

// ********************************************************************************************** //

void peptide::randomize()
{
  ISRANDOM = true;

  int  p;
  int  l = (int)strlen(popiPep.dtbSeq)-1;
  char c;
  for (int i = 0;i < l; i++)
    {
      // CHOISIT UN EMPLACEMENT D'ECHANGE
      p = rand()% l;
      c = popiPep.dtbSeq[i];
      popiPep.dtbSeq[i]   = popiPep.dtbSeq[p];
      popiPep.dtbSeq[p]   = c;
    }
  //  myProt[0].init(-1, -1, 0);
  computeLayers();
}

// ********************************************************************************************** //
