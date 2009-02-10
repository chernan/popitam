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

#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "defines.h"

/******************************************************************************/

struct SUCC1 {
  int     iVertexSucc;
  int     iAA;       
  int     iAAi;       
};

struct SUCC2 {
  int     iVertexSucc;
  int     iAA;
  int     iAAi;
  int     iAAj;
};


/******************************************************************************/

class vertex {                                    
 
  public :
  int      or_indice;
  int      mergedNb;
  double   mean_bMass;
  double   bMass[MAX_MERGED];
  int      iHypo[MAX_MERGED];                    // indice de l'hypothèse ionique utilisée pour construire la bMass
  int      iPeak[MAX_MERGED];
  double   peakMass[MAX_MERGED];
  double   peakInt[MAX_MERGED];
  int      peakBin[MAX_MERGED];

  int      succ1Nb;
  int      succ2Nb;
  SUCC1    succ1List[MAX_SUCC1];
  SUCC2    succ2List[MAX_SUCC2];

  public :
  
  vertex();
  ~vertex();
     
  inline int getSucc(int i)       {if (i < succ1Nb) return succ1List[i].iVertexSucc;
                                   else             return succ2List[i-succ1Nb].iVertexSucc;}  
  inline int getIAA(int i)        {if (i < succ1Nb) return succ1List[i].iAA;
                                   else             return succ2List[i-succ1Nb].iAA;}

};


/******************************************************************************/

#endif
