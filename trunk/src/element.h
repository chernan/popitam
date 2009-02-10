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

#ifndef _ELEMENT_H_
#define _ELEMENT_H_

// ********************************************************************************************** //

#include "peptide.h"
#include "scenario.h"

// ********************************************************************************************** //

class element {
  public :

  bool          FILLED;    
  int           rank;

  peptide*      Peptide;
  scenario*     Scenario;

  float         ZSCORE_NEG;
  double        PVALUE_NEG; 
  int           ECH_SIZE_NEG;

  float         ZSCORE_RANDOM;
  double        PVALUE_RANDOM; 
  int           ECH_SIZE_RANDOM;

  element* preceding;
  element* following;
    
  element();  
  ~element();
  void fillElement(peptide*, scenario*);
  void update(peptide*, char*);
};
    
// ********************************************************************************************** //

#endif
