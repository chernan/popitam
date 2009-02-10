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

#ifndef __RUN_MANAGER_PARAMETERS_H__
#define __RUN_MANAGER_PARAMETERS_H__

// ********************************************************************************************** //

#include "file.h"
#include "util.h"
#include "defines.h"

//using namespace std;


// ********************************************************************************************** //


class runManagerParameters {


 public:
   
  // STDIN STDOUT 
  
  File  FILEOUT;               // STDOUT
  File  FILEOUTXML;
  File  FILEOUTSHORT;
  File  FILEIN;                // FICHIER MSMS SOURCE
  char  FILEINNAME[256];
  File  FILEERROR;
  

  // PARAMETRES DE RUN (DONNES EN ARGUMENTS)
  
  int   r_NORMAL;
  int   r_CHECK;
  int   r_FUN; 
  int   s_UNKNOWN;
  int   s_IDSET;
  int   s_MIXSET;
  int   m_MUTMOD;

  char  FORMAT[16];
  
  // FILE NAMES
  char   FILE_ERROR_NAME[256];
  char   PATH_INFILE[256];
  char   AMINO_ACID_INFILE[256];       
  char   GPPARAMETERS_INFILE[256];
  char   SCORE_FUNCTION0_INFILE[256];
  char   SCORE_FUNCTION1_INFILE[256];
  char   SCORE_FUNCTION2_INFILE[256];
  char   PROBS_TOFTOF1_INFILE[256];
  char   PROBS_QTOF1_INFILE[256];
  char   PROBS_QTOF2_INFILE[256];
  char   PROBS_QTOF3_INFILE[256];
  char   DB1_PATH[256];
  char   DB2_PATH[256];
  int    TAXID[MAX_TAXID_NB];
  int    TAXID_NB;
  char   AC_FILTER[AC_FILTER_LENTGH];
  char   ENZYME[256];
  char   OUTPUT_DIR[256];    
  
  char   GEN_OR_FILENAME_SUFF[256];
  char   GEN_NOD_FILENAME_SUFF[256];
  char   SCORE_NEG_OUTFILE[256];
  char   SCORE_RANDOM_OUTFILE[256];


  // SPECTRUM PARAMETERS
  // These values are used for all input spectra

 

  float     FRAGMENT_ERROR1;
  float     FRAGMENT_ERROR2;
  float     PREC_MASS_ERROR;
  char      INSTRUMENT[256];
  int       GLOBAL_CHARGE;
  
  // DIGESTION PARAMETERS 

  int    MISSED;
  
  // POPITAM SPECIFIC PARAMETERS 
  
  int    SPECTRUM_NB;
  float  PEAK_INT_SEUIL;
  int    BIN_NB;
  int    COVBIN;
  int    EDGES_TYPE;
  int    MIN_TAG_LENTGH;
  int    RESULT_NB;
  int    MIN_PEP_PER_PROT;
  float  UP_LIMIT_RANGE_PM;
  float  LOW_LIMIT_RANGE_PM;
  float  UP_LIMIT_RANGE_MOD;
  float  LOW_LIMIT_RANGE_MOD;
  float  MIN_COV_ARR;
  int    PLOT;
  int    PVAL_ECHSIZE;
 
 
  // METHODS
   runManagerParameters();
  ~runManagerParameters();
  void init(int, char**);
  void initIN_OUT(char*, char*, char*);
  void initArguments(int, char**);
  void checkArguments(int, char**);
  void displayArguments(File&);
  void loadParameters(File&, char**);
  char* convertSpectra(char*);
  void findSpectrumNb(File&);
  void display(File&);
  void displayXML(File&);
};


// ********************************************************************************************** //


#endif  
