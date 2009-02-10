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

//********************************************************************//
//              INCLUDE                                               //
//********************************************************************//
#include <cstring>                                                   // strcpy(), strcat().
#include <fstream>                                                  // ifstream, outstream.                  
#include <iostream>                                                 // flux io.
#include <cstdlib>                                                   // exit().              
#include "gpparameters.h"                                               // PARAMETERS.

using namespace std;

//********************************************************************//
//              CONSTRUCTOR                                           //
//********************************************************************//

PARAMETERS::PARAMETERS() {}


void PARAMETERS::init(char* filename)
{
  ifstream fin;
  fin.open(filename);
  if (!fin) {cerr<<"\nError 0 in function PARAMETERS::PARAMETERS().\n"; exit(1);}

  char temp;
  int j;
  InputDir        = new char[256];
  OutputDir       = new char[256];
  OutDotFilename  = new char[256];
  OutDotFilename  = new char[256];
  DataFilename    = new char[256];
  StatFilename    = new char[256];

  /* MAIN PARAMETERS */
  while (fin.get(temp)) { if (temp == '=') { fin>>InputDir; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>OutputDir; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>OutDotFilename; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>DataFilename; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>StatFilename; break; } }  
  while (fin.get(temp)) { if (temp == '=') { fin>>GenerationNumber; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>PopulationSize; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>DepthInit; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>DepthMax; break; } }
  while (fin.get(temp)) { if (temp == '=') { fin>>RandomInitialPop; break; } }  
  
  /* TERMINALS PARAMETERS */
  while (fin.get(temp)) { if (temp == '=') { fin>>TerminalNumber; break; } }
  TConvertTerminal = new char *[TerminalNumber];
  while (fin.get(temp)) 
    { 
      if (temp == '#') 
	{
	  for (int i=0;i<TerminalNumber; i++) 
	    {
	      TConvertTerminal[i] = new char[50]; 
	      while (fin.get(temp))
		{
		  if (temp == '#') { cerr<<"\nError 1 in function PARAMETERS::PARAMETERS().\n"; exit(1); }
		  if (temp == '"') 
		    { 
		      j=0;
		      while (fin.get(temp))
			{
			  if (j>=48) { cerr<<"\nError 2 in function PARAMETERS::PARAMETERS().\n"; exit(1); }
			  else 
			    {
			      if (temp == '"' ) { TConvertTerminal[i][j]='\0'; break; }
			      else { TConvertTerminal[i][j]=temp; }
			      j++;
			    }
			}
		      break;
		    }
		}
	    }
	  break;
	}
    }
  while (fin.get(temp)) { if (temp == '#') { break;}  }

  /* FUNCTION PARAMETERS */
  while (fin.get(temp)) { if (temp == '=') { fin>>FunctionNumber; break; } }
  FunctionArity = new int[FunctionNumber];
  for (int k=0; k<FunctionNumber; k++)
    while (fin.get(temp)) { if (temp == '=') { fin>>FunctionArity[k]; break;}  }
  TConvertFunction = new char *[FunctionNumber];
  while (fin.get(temp)) 
    { 
      if (temp == '#') 
	{
	  for (int i=0;i<FunctionNumber; i++) 
	    {
	      TConvertFunction[i]=new char[50]; 
	      while (fin.get(temp))
		{
		  if (temp == '#') { cerr<<"\nError 3 in function PARAMETERS::PARAMETERS().\n"; exit(1); }
		  if (temp == '"') 
		    { 
		      j=0;
		      while (fin.get(temp))
			{
			  if (j>=48) { cerr<<"\nError 4 in function PARAMETERS::PARAMETERS().\n"; exit(1); }
			  else 
			    {
			      if (temp == '"' ) { TConvertFunction[i][j]='\0'; break; }
			      else { TConvertFunction[i][j]=temp; }
			      j++;
			    }
			}
		      break;
		    }
		}
	    }
	  break;
	}
    }
  while (fin.get(temp)) { if (temp == '#') { break; } }
  
  /* GENETIC OPERATORS PARAMETERS */
  while (fin.get(temp)) { if (temp== '=') { fin>>MOselection;        break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>TournoiNumber;      break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>ProbaElitism;       break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>ProbaCrossOver;     break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>ProbaMutation;      break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>ProbaPermutation;   break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>ProbaMutationCoeff; break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>ProbaMutConv;       break; } } 
  while (fin.get(temp)) { if (temp== '=') { fin>>ProbaMutTrans;      break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>SelectivePressure;  break; } }
  

  /* COEFF */
  while (fin.get(temp)) { if (temp== '=') { fin>>LowerLimit; break; } }
  while (fin.get(temp)) { if (temp== '=') { fin>>UpperLimit; break; } } 

  /* READ INITIAL SRUCTURES FILES */
  if (RandomInitialPop == 0)
    {
      while (fin.get(temp)) { if (temp== '=') { fin>>InDotFileNumber; break; } } 
      InDotFilename = new char *[InDotFileNumber];
      char *FileName = NULL;                
      char *buffer   = NULL;                     
      for (int i=0; i<InDotFileNumber; i++)        
	{
	  buffer = new char[100];
	  while (fin.get(temp)) {if (temp == ':') {fin>>buffer; break;}} 
	  FileName = new char[256];
	  strcpy(FileName, InputDir);
	  strcat(FileName, buffer);
	  InDotFilename[i] = new char[256];
	  strcpy(InDotFilename[i], FileName);
	  delete buffer;           buffer=NULL;
	  delete FileName;         FileName=NULL;
	}
    }
  else 
    {
      InDotFileNumber = 0;
      InDotFilename   = NULL;
    }
  fin.close(); fin.clear();
}

//********************************************************************//
//              DESTRUCTOR                                            //
//********************************************************************//
PARAMETERS::~PARAMETERS()
{
  if (TConvertTerminal!=NULL) 
    {
      for (int i=0; i<TerminalNumber; i++) { if (TConvertTerminal[i]!=NULL) delete  [] TConvertTerminal[i]; }
      delete TConvertTerminal;
    }
  if (TConvertFunction!=NULL) 
    {
      for (int i=0; i<FunctionNumber; i++) { if (TConvertFunction[i]!=NULL) delete [] TConvertFunction[i]; }
      delete TConvertFunction;
    }  
  if (InDotFilename !=NULL)
    {
      for (int i=0; i<InDotFileNumber; i++)
	{
	  delete [] InDotFilename[i];
	}
      delete InDotFilename;
    }
  if (FunctionArity != NULL)
    {
      delete FunctionArity;
      FunctionArity = NULL;
    }
  if (InputDir != NULL)
    {
      delete InputDir;
      InputDir = NULL;
    }
  if (OutputDir != NULL)
    {
      delete OutputDir;
      OutputDir = NULL;
    }
  if (OutDotFilename != NULL)
    {
      delete OutDotFilename;
      OutDotFilename = NULL;
    }
  if (DataFilename != NULL)
    {
      delete DataFilename;
      DataFilename = NULL;
    }
  if (StatFilename != NULL)
    {
      delete StatFilename;
      StatFilename = NULL;
    }
}




//********************************************************************//
//              DISPLAY PARAMETRES                                    //
//********************************************************************//
void PARAMETERS::DisplayParameters()
{
  cout<<endl<<endl;
  cout<<"Inputdir                     :"<<InputDir<<endl;
  cout<<"Outputdir                    :"<<OutputDir<<endl;
  cout<<"Output dotfilename           :"<<OutDotFilename<<endl;
  cout<<"Data filename                :"<<DataFilename<<endl;
  cout<<"Stat filename                :"<<StatFilename<<endl;
  cout<<"Number of generations        :"<<GenerationNumber<<endl;
  cout<<"Size of population           :"<<PopulationSize<<endl;
  cout<<"Maximum initial depth        :"<<DepthInit<<endl;
  cout<<"Maximum depth                :"<<DepthMax<<endl;
  cout<<"Initial population           :";
  if (RandomInitialPop == 1) cout<<"random"<<endl;
  else cout<<"loaded"<<endl;
  cout<<"Number of terminals          :"<<TerminalNumber<<endl;
  cout<<"Convertion table             :\n";
  for (int i=0; i<TerminalNumber; i++)
    {
      cout<<"\tTerminal n^"<<i<<" = \""<<TConvertTerminal[i]<<"\"\n";
    }
  cout<<"Number of functions          :"<<FunctionNumber<<endl;
  cout<<"Arity table:\n";
  for (int i=0; i<FunctionNumber; i++)
    {
      cout<<"\tArity fonction n^"<<i<<" = "<<FunctionArity[i]<<endl;
    }
  cout<<"Convertion table:\n";
  for (int i=0; i<FunctionNumber; i++)
    {
      cout<<"\tFunction n^"<<i<<" = \""<<TConvertFunction[i]<<"\"\n";
    }
  cout<<"Méthode de sélection         :";
  if (MOselection == 0) 
    cout<<"sélection par tournoi sur la fonctionnalité."<<endl;
  else
    cout<<"sélection multi-objectif."<<endl;
  cout<<"Number of adversaries        :"<<TournoiNumber<<endl;
  cout<<"Selective pressure           :"<<SelectivePressure<<endl;
  cout<<"Probability of elitism       :"<<ProbaElitism<<endl;
  cout<<"Probability of crossover     :"<<ProbaCrossOver<<endl;
  cout<<"Probability of mutation      :"<<ProbaMutation<<endl;
  cout<<"Probability of conversion    :"<<ProbaMutConv<<endl;
  cout<<"Probability of transversion  :"<<ProbaMutTrans<<endl;
  cout<<"Probability of permutation   :"<<ProbaPermutation<<endl;
  cout<<"Probability of coef mutation :"<<ProbaMutationCoeff<<endl;
  cout<<"Lower limit of coeff         :"<<LowerLimit<<endl;
  cout<<"Upper limit of coeff         :"<<UpperLimit<<endl;
  if (RandomInitialPop == 0) 
    {
      cout<<"\nNumber of dotfiles loaded    :"<<InDotFileNumber;
      for (int i=0; i<InDotFileNumber; i++)
	{
	  cout<<"\n\tDot filename "<<i<<"       :"<<InDotFilename[i];
	}
    }  
  cout<<endl<<endl;
}


//********************************************************************//
//              ECRIRE PARAMETRES                                     //
//********************************************************************//
void PARAMETERS::WriteParameters(ofstream & fout)
{
  fout<<"Parameters:";
  fout<<"\n\tNumber of generations         :"<<GenerationNumber;
  fout<<"\n\tSize of population            :"<<PopulationSize;
  fout<<"\n\tInitial population            :";
  if (RandomInitialPop == 1) cout<<"random";
  else cout<<"loaded";
  fout<<"\n\tMaximum initial depth         :"<<DepthInit;
  fout<<"\n\tMaximum depth                 :"<<DepthMax;
  fout<<"\n\tSelective pressure            :"<<SelectivePressure;
  fout<<"\n\tMéthode de sélection          :";
  if (MOselection == 0)
    fout<<"sélection par tournoi";
  else 
    fout<<"sélection multi-objectif";
  fout<<"\n\tNumber of adversaries         :"<<TournoiNumber;
  fout<<"\n\tProbability of elitism        :"<<ProbaElitism;   
  fout<<"\n\tProbability of crossover      :"<<ProbaCrossOver;
  fout<<"\n\tProbability of mutation       :"<<ProbaMutation;
  fout<<"\n\tProbability of permutation    :"<<ProbaPermutation;
  fout<<"\n\tProbability of coef mutation  :"<<ProbaMutationCoeff;
}
