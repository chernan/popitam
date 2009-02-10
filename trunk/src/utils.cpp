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

#include <cstdlib>
#include <cctype>
#include <cmath>
#include "error.h"
#include "vertex.h"

#include "utils.h"

using namespace std;

/***********************************************************************/

inline float float_alea (float inf, float sup ) {
  return (rand()/(RAND_MAX / (sup - inf)) + inf);
}

/***********************************************************************/

inline int int_alea (int inf, int sup) {
  return (inf + (rand() % (sup - inf +1)));
}

/***********************************************************************/

int shoot(float prob) {
  if (fabs(prob - 1.0) < FLOAT_PREC) 
    return true;
  else 
    return (float_alea(0.0, 1.0) < prob);
}

/***********************************************************************/

bool compare_aa(char aa1, char aa2) {
  if (aa1 == 'O') aa1 = 'M';
  if (aa1 == 'X') aa1 = 'C';
  if (aa1 == 'Z') aa1 = 'C';
  
  if (aa2 == 'O') aa2 = 'M';
  if (aa2 == 'X') aa2 = 'C';
  if (aa2 == 'Z') aa2 = 'C';

  if (aa1 == aa2) return true;
  return false;
}

/***********************************************************************/

int dichomSearch(double val, vertex* tab, int ElNb) {
  int bornSup = ElNb-1;
  int bornInf = 0;
  // recherche de la valeur val dans le tableau trié tab de ElNb éléments
  int i = (int)((bornSup-bornInf)/2);
  // on s'arrête quand i ne bouge plus (cela signifie qu'on est prêt du plus ressemblant)
  int last = 0;
  while (last != i) {
    last = i;
    if (tab[i].mean_bMass > val) {
      bornSup = i;
      i = int((bornSup-bornInf)/2) + bornInf;
    }
    if (tab[i].mean_bMass < val) {
      bornInf = i;
      i = int((bornSup-bornInf))/2 + bornInf;
    }
  }
  // trouve le plus près en partant un peu avant et en finissant un peu après
  int s = i-1;
  int min_i=0;
  if (s < 0) s = 0;
  double min = 1000;
  while ((s < i+2) && (s < ElNb)) {
    if (fabs(tab[s].mean_bMass - val) < min) {
      min   = fabs(tab[s].mean_bMass - val);
      min_i = s;
    }
    s++;
  }
  return tab[min_i].or_indice;  // je retourne l'indice du pic dans peakList
}

//---------------------------------------------------------------------------------------------//
/*
inline bool isFloatEqual(float a, float b)
{ return (fabs(a-b) <= FLOAT_PREC); }
*/
//---------------------------------------------------------------------------------------------//
/*
inline bool isDoubleEqual(double a, double b) 
{ return (fabs(a-b) <= FLOAT_PREC); }
*/
//-----------------------------------------------------------------------------------------

double normal3(double x, char* fileErrorName){
  
  double x2;
  double P;
  x2=x*x;
  
  P = 0.5 * sqrt(1-(0.033333)*(7*pow(2.7183,x2/-2) + 16*pow(2.7183,-.586*x2) + (7+.7854*x2)*pow(2.7183,-x2)));
  
  // la modif ci-dessous évite que l'analyse soit bêtement stoppée par un NAN ou un INF
  // la pvalue reportée sera 1
  //if (!isDoubleEqual(P,   P)) {fatal_error(fileErrorName, NANI, "in function normal3()");}
  //if ( isDoubleEqual(P-1, P)) {fatal_error(fileErrorName, INFI, "in function normal3()");}
  if (!isDoubleEqual(P,   P)) return 0;
  if ( isDoubleEqual(P-1, P)) return 0;
  
  return P;
  
}


//-----------------------------------------------------------------------------------------

