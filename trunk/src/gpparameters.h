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

#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

//********************************************************************//
//              INCLUDE                                               //
//********************************************************************//
#include <fstream>                                                  // ifstream, ofstream.
using namespace std;

//********************************************************************//
//              CLASS PARAMETERS                                      //
//********************************************************************//
class PARAMETERS
{
 public:
  // main parameters.
  char  *InputDir;                                                    // directory of input files.
  char  *OutputDir;                                                   // directory of output files.
  char  *OutDotFilename;                                              // name of output dotfile.
  char  *DataFilename;                                                // name of input datafile. 
  char  *StatFilename;                                                // name of output statfile.
  int   GenerationNumber;                                             // number of generations.
  int   PopulationSize;                                               // size of the populations.
  bool  RandomInitialPop;                                             // 1 -> random creation of the initial population. 
                                                                      // 0 -> loading of the initial population.
  // functions and terminals.
  int   DepthInit;                                                    // maximum depth of the initial individuals. 
  int   DepthMax;                                                     // maximum depth of the individuals.
  int   TerminalNumber;                                               // number of terminals.
  int   FunctionNumber;                                               // number of functions.
  int   InDotFileNumber;                                              // number of input dotfiles that must be loaded.
  int   *FunctionArity;                                               // number of arguments for each function.
  char  **TConvertTerminal;                                           // convertion table of the terminals.
  char  **TConvertFunction;                                           // convertion tables of the functions.
  char  **InDotFilename;                                              // names of the input dotfiles.
  float LowerLimit;                                                   // lower limit coefficient.
  float UpperLimit;                                                   // upper limit of coefficient.
  
  // genetic operators.
  bool  MOselection;                                                  // 0 -> selection par tournoi sur FitnessObj1.
                                                                      // 1 -> selection par Multi-Objectif.
  int   TournoiNumber;                                                // number of adversaries for tournament operator.
  float ProbaElitism;                                                 // probability of elitism.                                 
  float ProbaCrossOver;                                               // probability of crossover.
  float ProbaMutation;                                                // probability of mutation.
  float ProbaPermutation;                                             // probability of permutation.
  float ProbaMutationCoeff;                                           // probability of coefficient mutation.
  float ProbaMutConv;                                                 // probability of mutation by conversion.
  float ProbaMutTrans;                                                // probability of mutation by transition.
  float SelectivePressure;                                            // selection pressure (biais to elitisme).

 public:
  ~PARAMETERS();                                                      // destructor.
  PARAMETERS();                                                       // constructor.
  void init(char*);
  void DisplayParameters();                                           // displays parameters.
  void WriteParameters(ofstream &);                                   // writes parameters.
};

#endif
