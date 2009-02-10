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

#include "Sequence.h"
#include "defines.h"

//using namespace std;

// FONCTIONS CONSTR DESTR
// ***************************************************************************************** //

Sequence::Sequence(char *s): seqAA(s) {}

// ***************************************************************************************** //

Sequence::Sequence()
{
  seqAA=0x0;
}

// ***************************************************************************************** //

Sequence::~Sequence() {}

// ***************************************************************************************** //

// FONCTIONS MEMBRES 
double* Sequence::getBlayer() { //appel fonction qqpart de patricia A VOIR
    double* tab= new double[MAX_LENGHT];
    for (int i=0;i<MAX_LENGHT;i++) {
        tab[i]=0.0;
    }

    return tab;
}

// ***************************************************************************************** //
