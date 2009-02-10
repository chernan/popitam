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

#ifndef __DEFINES_H__
#define __DEFINES_H__

// ******************************************************************************

#define TRUE       1
#define FALSE      0

// ******************************************************************************

#define MAX_TAXID_NB                     50
#define INTERVAL_NB                      25  // nbre d'intervals pour les histogrammes de stats de scores

#define MAX_SIMILAR_PROT                300
#define MAX_SP_SOURCE                 20000

#define SEQ_LENGHT                      128
#define AC_LENGHT                       256
#define ID_LENGHT                       256
#define DE_LENGHT                       256
#define AC_FILTER_LENTGH             100000 

#define MAX_IONS                         20
#define MAX_BIN_NB                       10
#define MIN_COVBIN                        2
#define MAX_CONNECT                    1000

#define MAX_LENGHT                       64
#define MAX_INIT_PEAKS                10000

//define MAX_PEAKS                       200
#define MIN_PEAKS                        10

#define MAX_SUBSEQ_NB                  5000
#define MAX_SUBSEQ_NB_IN_AN_ARRANGEMENT 100

#define MAX_ARRANGEMENT_NB             3000

#define NB_MODIF	                  0          // uniquemement meth oxidation et carbamidomethylation

#define MAX_DOUBLONS                   1000
#define MAX_COMBI                       100          // max de combinaisons possibles pour les modifs prévues

#define MIN_PROT_MASS                   666
#define MAX_PROT_MASS              10000000

#define MAX_MUT_NB                        3

#define MIN_PRESCORE                      4.0        // (float) PAS UTILISE ratio par rapport au nombre d'aa présumés du peptide parent
                                                     // pour le scoring (2 signifie nbrePrésumé / 2)
#define MAX_MERGED                      500
#define MAX_SUCC1                       100
#define MAX_SUCC2                      1000
#define MAX_SHIFTS                       10

#define MAX_SOURCE_EL                   128

// ********************************************************************************************** //

#define FLOAT_LIMIT_NEG            -3.4e38
#define FLOAT_LIMIT_POS             3.4e38
#define FLOAT_PREC                  0.000001
#define DOUBLE_LIMIT_NEG           -1.7e308  
#define DOUBLE_LIMIT_POS           +1.7e308   
#define DOUBLE_PREC                 0.0000000001     

// ******************************************************************************

struct popipep {
  float          dtbPepMass;
  char           dtbSeq[MAX_LENGHT];
  double         bLayer[MAX_LENGHT];

  int	dtbSeqSZ;
};


struct PATH {
  int   iVertex;
  int   iEdge;
};

// ******************************************************************************

struct SCORE {

  double  lakScore1_a;
  double  modScore1_b;
  double  covScore1_c;                 
  double  covScore2_d;                 
  double  covScore3_e;                 
  double  intScore1_f;                 
  double  perScore1_g;                 
  double  famScore1_h;
  double  errScore1_i;          
  double  redScore1_j;
  double  serScore1_k; 
  double  serScore2_l;
  double  finArrScore;

};

// ******************************************************************************


#endif
