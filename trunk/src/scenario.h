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

#ifndef __SCENARIO_H__
#define __SCENARIO_H__

/******************************************************************************/

#include "runManagerParameters.h"
#include "aa.h"
#include "ion.h"
#include "data.h"
#include "graphe.h"
#include "peptide.h"
#include "subseq.h"
#include "file.h"
#include "score.h"
#include "gptree.h"
#include "fun.h"

using namespace std;

// ********************************************************************************************** //

class scenario {
  
 public:

  runManagerParameters *runManParam;
  aa            *aaParam;
  ion           *ionParam;
  data          *spectrumData;
  graphe        *popiGraph;
  peptide       *aPeptide;
  subseq        *popiTags;
  fun           *funGen;

  int            size;
  int           *arr;             // arr contient les indices des sous-seq inclues dans l'arrangement
  
  float         *shift;
  int            shiftNb;
  char           scenarioSeq[MAX_LENGHT];
  int            hypoNb;
  score          Scores;
  TREE          *scoreF;
 
  
  scenario();
  ~scenario();

  void    init(int, int*, runManagerParameters*, aa*, ion*, data*, graphe*, peptide*, subseq*, TREE*, fun*);
  void    free();
  void    process();
  void    buildScenario();
  float   computePreShift();
  void    sortArrByBeginPos();
  void    poseUnGap(int, int, char);
  void    poseUnTag(int);

  float   computeShiftMean(float, int);
  void    fillExpTheoTabs(int i, float delta, float* bMassesExp, float* bMassesTheo, int* nb, char c);
  float   FindClosestTheoMass(float bMasseExp, int iSubIndice, float delta);

  bool    computeScores();
  bool    computeGapScores(float*, float*); 
  bool    computeCovScores(float*, float*, float*);
  bool    computeIntScore(float*);
  bool    computePerScore(float*);
  bool    computeFamScore(float*);
  bool    computeSerScore(float*, float*);
  bool    computeRedScore(float*);
  bool    computeLenScore(float*);
  bool    computeLexScore(float*);
  bool    computeErrScore(float*, char);
  void    find_a_and_b(float*, float*, float*, float*, int);
  float   compute_the_variance(float, float, float*, float*, int);
  float   get_deviation(int, float, float, float*, float*, int);
  void    writeScenario();
  void    write(File&);
  void    writeFun();
};
 
// ********************************************************************************************** //

#endif
