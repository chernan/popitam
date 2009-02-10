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

#ifndef __TAG_EXTRACTOR_H__
#define __TAG_EXTRACTOR_H__

// ****************************************************************************

#include "defines.h"
#include "runManagerParameters.h"
#include "aa.h"
#include "data.h"
#include "graphe.h"
#include "peptide.h"
#include "subseq.h"
#include "gnb.h"
#include "sufftab.h"
#include "ants.h"

// ****************************************************************************

#define AASIZE  64

class tag_extractor {
  public :

  runManagerParameters *runManParam;
  aa                   *aaParam;
  data                 *spectrumData;
  graphe               *popiGraph;
  peptide              *aPeptide;
  sufftab              *suffTab;
  subseq               *popiTags;
  
  // LL: try these
  char* pepstr;
  TAB*  tab1ptr;
  TAB2*  tab2ptr;
  bool  aaFlag[AASIZE];
  bool  aa2Flag[AASIZE][AASIZE];
  vertex* vlst;
  vertex* ovlst;

  tag_extractor();
  ~tag_extractor();  
  void  init(runManagerParameters*, aa*, data*, graphe*, peptide*, subseq*);
  void  getTheTags();
  void  buildSuffTree();
  void  buildSuffTab();
  void  writeSuffTab();
  void  parseGraph();
  void  explore(ants, int, int, int, bool);  
  void  moveLili(ants*, int, int, int);
  void  keepTag(ants);
  void  lookOcc(int, bool*);
  bool  match1(vertex*, int*, int*, int);
  bool  match2(vertex*, int*, int*, int);
  void  writeAllTags(char*);
};


// ****************************************************************************

#endif
