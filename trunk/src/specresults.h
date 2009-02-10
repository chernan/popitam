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

#ifndef __SPEC_RESULTS_H__
#define __SPEC_RESULTS_H__

/******************************************************************************/

#include "element.h"
#include "runManagerParameters.h"
#include "data.h"
#include "peptide.h"
#include "scenario.h"

// ********************************************************************************************** //

class specresults {
  public :
  runManagerParameters *runManParam;
  data                 *spectrumData;
  
  int                   elementNb;
  int                   rankNb;
  element              *firstElement;
  element              *lastElement;
  element              *newElement;
  element              *currentElement;

  // STATISTIQUES DE SCORES POUR LE SPECTRE
  File           fpSCORES_NEG;
  File           fpSCORES_RANDOM;
  int            ECH_SIZE_NEG;
  float          MEAN_NEG;
  float          VAR_NEG;

  //  peptide*       randPep;
  //  int            iRandCount;
  int            ECH_SIZE_RANDOM;
  float          MEAN_RANDOM;
  float          VAR_RANDOM;

   specresults();
   ~specresults();
  inline int      get_elementNb()          {return elementNb;};
  inline element* get_firstElement()       {return firstElement;};
  inline element* get_lastElement()        {return lastElement;};
  inline element* get_currentElement()     {return currentElement;};
  

  void    init(runManagerParameters*, data*);
  void    add_and_sort(peptide*, scenario*);
  void    addDoublon(peptide*);
  void    insert(peptide*, scenario*);
  void    removeElement(element*);
  void    putRanks(); 
  int     giveRankOfCorrectPeptide();  
  //  int     giveRankMixt();
  void    write(File&);
  void    writeXML(File&);
  void    writeShort(File&);
  void    writeSimple(File&);
  void    addToProtList();
  void    computePValuesNEG();
  void    computePValuesRANDOM();
  void    createPopScoresNEG(int);
  void    createPopScoresRANDOM(int);
};

// ********************************************************************************************** //

#endif
