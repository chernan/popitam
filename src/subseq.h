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

#ifndef __SUBSEQ_H__
#define __SUBSEQ_H__

/******************************************************************************/

#include <cstdio>
#include "defines.h"
#include "aa.h"
#include "ion.h"
#include "data.h"
#include "runManagerParameters.h"
#include "file.h"


using namespace std;

// ********************************************************************************************** //

struct SUBSEQ { 

  int         posBegin;  
  double      massBegin;
  double      massEnd;
  int         lenght;
  char        parsedSeq[MAX_LENGHT];

  int*        or_pL;             // peaks included 
  int*        or_nL;             // nodes included
  
  int         state;
  int         iNodeNb;
  int         or_iNodesParsed[MAX_LENGHT];
};

/******************************************************************************/


class subseq {

 public:
  runManagerParameters *runManParam;
  data                 *spectrumData;
  aa                   *aaParam;
  ion                  *ionParam;

  
  int     subSeqNb;
  int     minLenght;
  SUBSEQ  subSeqTab[MAX_SUBSEQ_NB];

  subseq();
  ~subseq();
  void    init(runManagerParameters*, aa*, data*, ion*);
  int     comparPosBegin(const void*, const void*);
  int     comparPos(const void*, const void*);
  void    initSubSeqTab(int, int, int);
  void    freeSubSeqTab();
  void    freeSubSeqTab(int);
 
  void    removeSubSubSeqs(char*);
  int     mark_subSeqs();
  int     mark_subSeqs2(char*);
  int     mark_subSeqs3(char*);
  bool    isSubSeq(int, int);
  bool    rulesOK(int,int);
  void    copySubSeq(int, int, int, int);
  
  void    copy(SCORE*, SCORE*);
  void    write(char*, File&, int);
 
  void    writeArrangments();
  void    writeScenarios();
  void    writeScoresForFun(SCORE, int, FILE*);
};

/******************************************************************************/


#endif
