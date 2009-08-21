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

#ifndef _PROTEL_H_
#define _PROTEL_H_

/******************************************************************************/

#include "defines.h"
#include "protEl.h"
#include "data.h"
#include "element.h"
#include "file.h"

/******************************************************************************/

struct FICHE
{
  //int           dtbId;
  //unsigned int	offId;
  DBEntry*	prot;
  char          AC[AC_LENGHT];
  char          ID[ID_LENGHT];
  char          DE[DE_LENGHT];
};


struct SOURCE 
{
  int    rank;
  float  finScore;
  float  Zscore;
  int    specID;
  char   title[DE_LENGHT];
  char   dtbSeq[SEQ_LENGHT];
  float  parentMassRaw;
  float  parentMassM;
  float  dtbPepMass;
  float  delta;
  bool   missedCleaved;
  int    startPos;
  int    endPos;
  bool   IS_CORRECT;
};

// ********************************************************************************************** //

class protEl {
  public :

  bool          FILLED;
  float         cov;

  FICHE         protIds[MAX_SIMILAR_PROT];
  int           similarProtNb;
 
  int           SpSourceNb;
  SOURCE        SpSource[MAX_SP_SOURCE];

  
  protEl* preceding;
  protEl* following;
    
  protEl();  
  ~protEl();
  void fillIt(element*, int, char*, char*, float, float, int, char*);
  void updateIt(element*, int, char*, char*, float, float, int, char*);
  void computeCov();
};
    
/******************************************************************************/

#endif
