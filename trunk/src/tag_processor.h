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

#ifndef __TAG_PROCESSOR_H__
#define __TAG_PROCESSOR_H__

// ****************************************************************************

#include "defines.h"
#include "runManagerParameters.h"
#include "spectrumstatistics.h"
#include "aa.h"
#include "ion.h"
#include "data.h"
#include "graphe.h"
#include "peptide.h"
#include "subseq.h"
#include "ants.h"
#include "clicsearch.h"
#include "fun.h"
#include "scenario.h"
#include "gptree.h"
#include "specresults.h"

using namespace std;

/******************************************************************************/

class tag_processor {
  public :

  runManagerParameters *runManParam;
  spectrumstatistics   *specStats;
  aa                   *aaParam;
  ion                  *ionParam;
  data                 *spectrumData;
  graphe               *popiGraph;
  peptide              *aPeptide;   // JE FILE UN POINTEUR DE POINTEUR, CAR LA VALEUR DE APEPTIDE DOIT EGALEMENT ETRE MODIFIEE DANS UNE FONCTION DE TAGPROCESSOR
  subseq               *popiTags;
  specresults          *specResults;

  TREE                 *scoreF;

  fun                  *funGen;

  clicsearch            clicSearch;
  bool                **compPredTab;
  int                   bestScenarioIndice;
  scenario             *scenarios[2];
  
  
  tag_processor();
  ~tag_processor(); 

  static  void CallBackArr(void *pInstance, int size, int* clic);

  void    init(runManagerParameters*, spectrumstatistics*, aa*, ion*, data*, 
	       graphe*, peptide*, subseq*, fun*, TREE*, specresults*);
  void    processTags(); 
  void    processClique();
  void    writeScenario();
  void    writeScenarioScoresForFUN(File&);
  bool**  initCompPredTab(int);
  void    freeCompPredTab();
  bool    fillCompPredTab(bool**);
  void    BugVerify(bool*);
  double  computeSeqMass(int, int, char*);
  bool    setCompPredTabToFalse(bool**, int, int);
  bool    compaPeaks(int, int);
  bool    compaNodes(int, int);
  void    writeCompPredTab(bool**, int, File&);
  void    lookForCliques(bool**);
  void    findCliques(bool**, int*, int, int, int);
  bool    isComplete(bool**, int*, int);
  void    updateSpecResultsNEG();
  void    updateSpecResultsRANDOM();
};


// ****************************************************************************

#endif
