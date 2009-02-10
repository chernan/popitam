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

#ifndef UTILS_H_
#define UTILS_H_

#include <cmath>

#include "vertex.h"

inline double get_ionMass(double PM, double bMass, double ch, double off, char term) {
  if (term == 'N') return (bMass + off + (ch-1))/ch;
  if (term == 'C') return (PM - bMass + off + (ch-1))/ch;
  return -1;
}

inline double get_bMass(double PM, double ionMass, double ch, double off, char term) {
  if (term == 'N') return ch * ionMass - (ch-1) - off;
  if (term == 'C') return PM - ch * ionMass + (ch-1) + off;
  return -1;
}

inline float  float_alea (float, float);
inline int    int_alea (int, int);
int    shoot(float);
bool   compare_aa(char, char);
int    findIndice(char);    // retourne l'indice correspondant à l'acide aminé
int    findIndice(char, char);
int    dichomSearch(double, vertex*, int);
inline bool   isFloatEqual(float a, float b) { return (fabs(a-b) <= FLOAT_PREC); } 
inline bool   isDoubleEqual(double a, double b) { return (fabs(a-b) <= FLOAT_PREC); } 
double normal3(double, char* fileErrorName);


#endif
