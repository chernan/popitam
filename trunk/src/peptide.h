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

#ifndef _PEPTIDE_H_
#define _PEPTIDE_H_

// ********************************************************************************************** //

#include "defines.h"
#include "runManagerParameters.h"
#include "aa.h"
#include "Protein.h"
#include "Sequence.h"
#include "Tree.h"
#include "dbreader.h"

// ********************************************************************************************** //


class peptide {
 public :

  runManagerParameters   *runManParam;
  aa                     *aaParam;
  bool                    ISRANDOM;
  popipep                 popiPep;

  int                     exemplairesNb;

  int                     posStart[MAX_DOUBLONS];
  int                     posEnd[MAX_DOUBLONS];
  int                    iChainStart[MAX_DOUBLONS];
  int                    iChainEnd[MAX_DOUBLONS];
  int                    iEntryEnd[MAX_DOUBLONS];
  
  peptide();  
  ~peptide();

  Sequence getSeqAA();  
  char* getSeqAAcar();                 //donne la seq sous forme char*
  void init(runManagerParameters*, aa*, float, char*, int, int, DBReader *pDBReader, bool); 
  void update();
  void computeLayers();
  //void updateResult();
  void randomize();
  
  void allocProtein(int i) { myProt[i] = new Protein; }
  Protein* getProtein(int i) { return myProt[i]; }
  
private:
  Protein**               myProt;             
};
    
// ********************************************************************************************** //

#endif
