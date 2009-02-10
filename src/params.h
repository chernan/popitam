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

#ifndef __PARAMS_H__
#define __PARAMS_H__

#include "file.h"
#include <cstdio>

using namespace std;

class params {


 public:

  // **************************** RUNNING MODES //
  
  int   r_NORMAL;
  int   r_CHECK;
  int   r_FUN; 
  int   s_UNKNOWN;
  int   s_IDSET;
  int   s_MIXSET;
  int   m_MUTMOD;

  
  // *********************** FILES IN ARGUMENTS //

  
  char   POP_FILE[256];
  char   DATA_FILE[256];
  char   OUTPUT_FILE[256];
  File   FILEOUT;

  // ******************* DIRECTORIES AND FILES //

  char   OUTPUT_DIR[256];   
  char   DTB_DIR[256];
  char   IONIC_OCC_PROB_DIR[256];
  char   SCORE_FUN_FUNCTION0[256];
  char   SCORE_FUN_FUNCTION1[256];
  char   SCORE_FUN_FUNCTION2[256];

  char   AMINO_ACID_FILE[256];        
  char   GEN_OUTPUT_FILE[256];
   
  // **************** SPECTRUM GLOBAL PARAMETERS //

  char   DB[256];
  char   TAXONOMY[256];
  int    TAX_ID;
  float  FRAGMENT_ERROR1;
  float  FRAGMENT_ERROR2;
  float  PREC_MASS_ERROR;
  char   INSTRUMENT[256];

  // ********************** DIGESTION PARAMETERS //

  int    MISSED;
  
  // ***************"******* POPITAM PARAMETERS //
  
  int    SPECTRUM_NB;
  int    BIN_NB;
  int    COVBIN;
  int    EDGES_TYPE;
  int    RESULT_NB;
  float  MAX_ADD_PM;
  float  MAX_LOS_PM;
  float  MAX_ADD_MOD;
  float  MAX_LOS_MOD;

  // ********************************** METHODS //

  params();
  ~params();
  void init(int, char**);
  void processArguments(int, char**);
  void displayArg();
  void displayHead();
  void findSpectraNb();
  void processParamFile();

};

// ******************************************** //


#endif  
