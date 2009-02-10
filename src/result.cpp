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

#include "result.h"
#include "memorycheck.h"

// ********************************************************************************************** //

// GLOBAL VARIABLES DECLARED ELSEWHERE 


extern memorycheck memCheck;


// ********************************************************************************************** //


result::result() {
  runManParam  = NULL;
  tabRank      = NULL;
  protList     = NULL;
}


// ********************************************************************************************** //


result::~result() {
  if (tabRank    != NULL) {delete tabRank;    tabRank    = NULL;                                   memCheck.results--;}
  if (protList   != NULL) {delete protList;   protList   = NULL;                                   memCheck.results--;}
}


// ********************************************************************************************** //


void result::init(runManagerParameters* rPM)
{
  runManParam = rPM;

  if ( runManParam->r_NORMAL)
    {
      initProtList(); 
    }
  if ( runManParam->s_IDSET)
    {
      initTabRank(runManParam->RESULT_NB);         // tabRank mémorise les rangs des peptides corrects
    }
}


// ********************************************************************************************** //

void result::initTabRank(int n)
{
  tabRank = new int[n];                                                                          memCheck.results++;
  for (int i = 0; i < n; i++) tabRank[i] = 0; 
}

// ********************************************************************************************** //


void result::initProtList() 
{
  protList = new prot;                                                                            memCheck.results++;
  protList->init(runManParam);
}

// ********************************************************************************************** //


void result::addToProtList(specresults* specR, int ID) 
{ 
  specR->currentElement = specR->firstElement->following;
  while ((specR->currentElement != specR->lastElement) && (specR->currentElement->FILLED))
    {
      if (specR->currentElement->PVALUE_NEG < 0.01) protList->update(specR->currentElement, specR, ID);
  
      specR->currentElement = specR->currentElement->following;
    }
}

// ********************************************************************************************** //

