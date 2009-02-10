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

#ifndef __AA_PARAM_H__
#define __AA_PARAM_H__

#include "runManagerParameters.h"


/******************************************************************************/

struct TAB {
  float  mass;
  int    indice;
  char   code[2];        // 1 letter code
  char   name[4];        // 3 letters code
};


//------------------------------------------------------------------------------


struct TAB2 {
  float  mass;                 // two amino acids weight
  int    indice;
  int    indice1;
  int    indice2;
  char   code[3];              // two amino acids one letter name    
  char   name[7];
};



/******************************************************************************/

/* COMPARISON ROUTINES FOR QSORT() */

int compar_TAB2(const void*, const void*);
int compar_TAB(const void*, const void*);


/******************************************************************************/

class aa {  

  public :
  runManagerParameters *runManParam;
 
  char   aaFile[256];       // file containing the amino acids
  TAB   *aaTable;
  TAB2  *aa2Table;
  int    aa1Nb;
  int    aa2Nb;
  float  H_mass;
  float  C_mass;
  float  N_mass;
  float O_mass;

   aa();
  ~aa();
  void init(runManagerParameters*);
  void build_aaTable();
  void build_aa2Table();
  void write(File&);
  int  findIndice(char);
};

/******************************************************************************/


#endif
