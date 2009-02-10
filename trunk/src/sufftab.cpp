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

#include <cstring>

#include "sufftab.h"
#include "memorycheck.h"
#include "defines.h"
#include "aa.h"

using namespace std;

// ********************************************************************************************** //

// GLOBAL VARIABLE DECLARED 

extern memorycheck memCheck;

// ********************************************************************************************** //

int sufftab::size;

// ********************************************************************************************** //

sufftab::sufftab() 
{
  succ  = NULL;
};

// ********************************************************************************************** //

sufftab::~sufftab()
{
  if (succ != NULL) {delete succ; succ = NULL;                                                    memCheck.sufftree--;}
};

// ********************************************************************************************** //

void sufftab::init(aa* aaP) 
{
  succ = new int[aaP->aa1Nb+1];                                                                    memCheck.sufftree++; 
  // NE PAS OUBLIER LE +1, CAR IL FAUDRA POUVOIR ACCEDER AU CARACTERE ~ STOCKE AVEC LES AA 

  pos1           = 0; // feuilles: position du début du suffix dans la séquence mère (les noeuds précédents sont pris en compte)
  pos2           = 0; // feuilles: position de la séquence de la feuille dans la séquence mère (la séquence de la feuille
                      //           n'est pas le suffix, car il faut encore inclure les aa des noeuds précédents)
  longu          = 0; // longueur du suffix complet
  strcpy(subseq, ""); // séquence de la feuille d'un suffixe (n'est pas égal au suffixe complet)
  state          = -1;// etat feuille/noeud

  for (int i = 0; i < aaP->aa1Nb+1; i++) succ[i] = -1;
  indPere        = 0;
}

// ********************************************************************************************** //

