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

#ifndef __ION_PARAM_H__
#define __ION_PARAM_H__


#include "defines.h"
#include "aa.h"
#include "file.h"

/******************************************************************************/

struct IONS {  
  char   name[16];
  double delta;
  double probs[MAX_BIN_NB+1];        // sert à stocker les probabilités pour chaque bin et la prob. totale
  int    charge;
  char   term;   
};


struct TRIBIN {
  double prob;
  int    iIon;
  int    jBin;
};

struct TRIALL {
  double prob;
  int    iIon;
};


/******************************************************************************/

class ion {
  
  char     ionFile[256];
  int      binNb;
  int      totIonNb;
  int      NionNb;
  int      CionNb;
  IONS     ions[MAX_IONS];
  TRIBIN   triBin[MAX_IONS * MAX_BIN_NB];
  TRIALL   triAll[MAX_IONS];

  public :
  ion();
  void init(char*, char*, aa*);
  ~ion();
  inline int    get_NionNb()           {return NionNb;};
  inline int    get_CionNb()           {return CionNb;};
  inline int    get_totIonNb()         {return totIonNb;};
  inline IONS*  get_iIon(int i)        {return &ions[i];};
  inline int    get_iTriBin_ion(int i) {return triBin[i].iIon;};
  inline int    get_iTriBin_bin(int i) {return triBin[i].jBin;};
  inline double get_iTriBin_prob(int i){return triBin[i].prob;};
  inline double get_delta(int i)       {if (i == -1) return -1; else return ions[i].delta;};
  inline char*  get_name(int i)        {if (i == -1) return "-";else return ions[i].name;};
  inline int    get_charge(int i)      {if (i == -1) return -1; else return ions[i].charge;};
  inline char   get_term(int i)        {if (i == -1) return '-';else return ions[i].term;};
  inline double get_prob(int i, int j) {if (i == -1) return -1; else return ions[i].probs[j];};
  void readInputIons(char*);
  void reComputeDeltas(aa*);
  void sortProbs();
  void write(File&);
  void writeSorted(File&);
};

/******************************************************************************/

#endif
