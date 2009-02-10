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

#ifndef __RUN_MANAGER_H__
#define __RUN_MANAGER_H__

// ********************************************************************************************** //

#include "runManagerParameters.h" 
#include "compare.h"
#include "aa.h"
#include "ion.h"
#include "result.h"
#include "gpparameters.h"
#include "gptree.h"
#include "data.h"


// ********************************************************************************************** //

class runManager
{
 public: 
  

  runManagerParameters *runManParam;

  Compare              *popitam;

  aa                   *aaParam;               // ACIDES AMINES
  ion                  *ionParamTOFTOF1;
  ion                  *ionParamQTOF1;
  ion                  *ionParamQTOF2;
  ion                  *ionParamQTOF3;
  
  data                 *spectrumData;
  result               *popiResults;            // RESULTATS DE TOUS LES RUNS
  
  PARAMETERS           *gp_parameters;                // PARAMETRES POUR LA FONCTION DE SCORE 
  TREE                 *scoreFunction[3];             // FONCTIONS DE SCORE POUR 0, 1 et 2 GAPS

  allrunstatistics     *allRunStats;

  runManager();
  ~runManager();
  void init(int, char**);
  void initRunManParam(int, char**);
  void initAaParam();
  void initIonParam();
  void initPopiResult();
  void initAllRunStats();
  void initScoreFunctions();
  void displayArg();
  void displayHead();
  void prepareOutDir();
  void initTagorun(int);
  void run();
  void endTagorun();
  void endRun();
  void writePerformanceIDSET(File&);
  void writePerformanceMIXSET(File&);
};


#endif
