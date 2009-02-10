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

#ifndef _CLICSEARCH_H_
#define _CLICSEARCH_H_

#include "defs.h"
#include "setlib0.h"

// ********************************************************************************************** //

typedef set* graph;

// ********************************************************************************************** //


class clicsearch
{
 public:

   
    // pointeur sur fonction
  void (*m_pfctCallBackArr)(void *pInstance, int size, int* clic);
  void  *m_pCallBackInstance;

  int n;
  graph A;
  setlist C,B,N;
  int * X;
  FILE *f;
  
  clicsearch();
  ~clicsearch();
  void findCliques(void(*pfctCallBackArr)(void *pInstance, int size, int* clic), void *pInstance, bool**, int, bool);
  void searchBrutForce(bool**, int);
  void recFindCliques(bool**, int, int*, int, int, int);
  bool isComplete(bool**, int*, int);
  void searchQuick(bool**, int);
  void AllCliques(int);
  graph NewGraph(int);
  void FreeGraph(int, graph);
  void ReadGraphEdges(graph*, int*, bool**, int);


  void CallBack(int, int*);

};

// ********************************************************************************************** //

#endif
