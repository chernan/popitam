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

#ifndef __GLOBSTAT_H__
#define __GLOBSTAT_H__

/******************************************************************************/

#include "defines.h"

/******************************************************************************/

class globStat {
 public:
  int   goodArrObs;
  int   badArrObs;
  int   allArrObs;
  int   maxGoodArrNb;
  int   maxBadArrNb;
  int   maxAllArrNb;

  float arrGOODStatTab[MAX_ARRANGEMENT_NB];
  float arrBADStatTab[MAX_ARRANGEMENT_NB];
  float arrALLStatTab[MAX_ARRANGEMENT_NB];
    
  int   goodSubSeqObs;
  int   badSubSeqObs;
  int   allSubSeqObs;
  int   maxGoodSubSeqNb;
  int   maxBadSubSeqNb;
  int   maxAllSubSeqNb;

  float subseqGOODStatTab[MAX_SUBSEQ_NB];
  float subseqBADStatTab[MAX_SUBSEQ_NB];
  float subseqALLStatTab[MAX_SUBSEQ_NB];

  // -------------------------------------------

  float lenghtObs;   // pour la normalisation
  float maxLenght;   // pour la limite sup sur l'axe des x du graphe
  float lenghtStatTab[MAX_LENGHT];

  float maxExpectedLenght;  // pour la limite sur sur l'axe des x du graphe
  float nodeStatTab[MAX_LENGHT];
  float nodeStatTabObsNb[MAX_LENGHT];
  float edgeStatTab[MAX_LENGHT];
  float edgeStatTabObsNb[MAX_LENGHT];
   
  // -------------------------------------------

  double  goodScoresTab[MAX_GOOD_PEPTIDES_TO_PROCESS]; // j'y mets uniquement le meilleur arrangement des bons peptides
  int     goodScoresNb;
  double  badScoresTab[MAX_BAD_PEPTIDES_TO_PROCESS];  // j'y mets uniquement le meilleur arrangement des mauvais peptides
  int     badScoresNb;
  double  goodScoresHistTab[INTERVAL_NB];   // à initialiser
  double  badScoresHistTab[INTERVAL_NB];    // à initialiser
  double  goodMax;
  double  badMax;
  double  goodMin;
  double  badMin;
  double  goodMean;
  double  goodVariance;
  double  badMean;
  double  badVariance;
  double  xAxis[INTERVAL_NB];

 // -------------------------------------------

  globStat();
  ~globStat();
  void init();
  void writeStatsForArrNb();
  void writeStatsForSubSeqNb();
  void writeStatsForLenght();
  void writeStatsForNodeNb();
  void writeStatsForEdgeNb();
  void processStatsForScores();
  void processStatsForGoodScores();
  void processStatsForBadScores();
  void computeMaxMin(double*, double*, double*, int);              // max, min, tab, elNb
  void normalizeTab(double, double*, int);                         // max, tab,elNb
  void computeMean(double*, double*, int);                         // mean, tab, elNb
  void computeVariance(double*, double, double*, int);             // variance, mean, tab, elNb
  void buildXaxis(int, double, double, double*);                   // elNb, incremValue, min, tab
  void fillHistTab(double*, int, double*, double, double);         // tab, tabElNb, histTab, incremValue, min
  void writeScores(char*, double*, int, double, double, double);   // str, tab, elNb, mean, variance, incremValue
};

#endif
