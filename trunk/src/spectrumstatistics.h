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

#ifndef __SPECTRUM_STATISTICS_H__
#define __SPECTRUM_STATISTICS_H__


#include "file.h"

// ********************************************************************************************** //


// ********************************************************************************************** //


class spectrumstatistics {  

  public :
 
  int                   protNbInDatabases;         // nombre de protéines processées
  int                   protNbInRange;             // nombre de protéines qui ont passé le filtre sur la masse parente (normalement toutes)
  int                   pepGeneratedByDigestion;   // nombre de peptides qui ont été générés par la digestion
  int                   pepCandidate;              // nombre de peptides qui ont passé le filtre de la masse parente
  int                   pepWithAtLeastOneSubSeq;   // nombre de peptides qui ont obtenu au moins 1 subsequence
  int                   cumSubSeqNb;
  int                   pepWithAtLeastOneScenario; // nombre de peptides qui ont eu au moins 1 arrangement à scorer
  int                   cumScenarioNb;

  bool                  IN_DTB;
  bool                  MANQUE;

   spectrumstatistics();
  ~spectrumstatistics();
  void init();
  void display(File&);
  void displayXML(File&);
};

// ********************************************************************************************** //


#endif
