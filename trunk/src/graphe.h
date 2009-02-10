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

#ifndef _GRAPHE_H_
#define _GRAPHE_H_

// ********************************************************************************************** //

#include "defines.h"
#include "runManagerParameters.h"
#include "aa.h"
#include "ion.h"
#include "data.h"
#include "vertex.h"

// ********************************************************************************************** //

// COMPARISON ROUTINE FOR QSORT() 

int compar_vertices_bMass(const void *, const void *);

// ********************************************************************************************** //

class graphe {

 public:
  runManagerParameters    *runManParam;

  aa                      *aaParam;
  ion                     *ionParam;
  data                    *spectrumData;



  int      orVertexNb;
  int      vertexNb;
  int      wantedNodeNb;
  int      minNodeNb;

  vertex  *or_vertexList;
  vertex  *vertexList;
  
  int      IedgeNb;
  int      IIedgeNb;
  int      or_IedgeNb;
  int      or_IIedgeNb;

  double   intSom; // somme des intensités des pics inclus dans le graphe (utilisée lors du scoring)

   graphe();
  ~graphe();
 
  
  inline int     get_vertexNb()                               {return vertexNb;};
  inline int     get_IedgeNb()                                {return IedgeNb;}
  inline int     get_IIedgeNb()                               {return IIedgeNb;};
  inline vertex* get_or_vertexList()                          {return or_vertexList;}
  inline int     get_orVertexNb()                             {return orVertexNb;}
  inline double  get_intSom()                                 {return intSom;}
 
  inline double get_or_iVertex_mean_bMass(int i)              {return or_vertexList[i].mean_bMass;}
  inline int    get_or_iVertex_mergedNb(int i)                {return or_vertexList[i].mergedNb;}
  inline double get_or_iVertex_mbMass(int i, int m)           {return or_vertexList[i].bMass[m];}
  inline int    get_or_iVertex_miHypo(int i, int m)           {return or_vertexList[i].iHypo[m];}
  inline int    get_or_iVertex_miPeak(int i, int m)           {return or_vertexList[i].iPeak[m];}
  inline double get_or_iVertex_mpeakMass(int i, int m)        {return or_vertexList[i].peakMass[m];}
  inline double get_or_iVertex_mpeakInt(int i, int m)         {return or_vertexList[i].peakInt[m];}
  inline int    get_or_iVertex_mpeakBin(int i, int m)         {return or_vertexList[i].peakBin[m];}
  
  inline double get_iVertex_mean_bMass(int i)                 {return vertexList[i].mean_bMass;}
  inline int    get_iVertex_mergedNb(int i)                   {return vertexList[i].mergedNb;}
  inline double get_iVertex_mbMass(int i, int m)              {return vertexList[i].bMass[m];}
  inline int    get_iVertex_miHypo(int i, int m)              {return vertexList[i].iHypo[m];}
  inline int    get_iVertex_miPeak(int i, int m)              {return vertexList[i].iPeak[m];}
  inline double get_iVertex_mpeakMass(int i, int m)           {return vertexList[i].peakMass[m];}
  inline double get_iVertex_mpeakInt(int i, int m)            {return vertexList[i].peakInt[m];}
  inline int    get_iVertex_mpeakBin(int i, int m)            {return vertexList[i].peakBin[m];}
  
 
  inline int    get_iVertexSucc1Nb(int i)                     {return vertexList[i].succ1Nb;}
  inline int    get_iVertexSucc2Nb(int i)                     {return vertexList[i].succ2Nb;}
 
  inline int    get_iVertex_jSucc_iVertex(int i, int j, int t){if (t == 0) return vertexList[i].succ1List[j].iVertexSucc;
                                                               else        return vertexList[i].succ2List[j].iVertexSucc;}
  inline int    get_iVertex_jSucc_iAA(int i, int j, int t)    {if (t == 0) return vertexList[i].succ1List[j].iAA;
                                                               else        return vertexList[i].succ2List[j].iAA;}
  inline int    get_iVertex_jSucc1_iVertex(int i, int j)      {return vertexList[i].succ1List[j].iVertexSucc;};
  inline int    get_iVertex_jSucc1_iAA(int i, int j)          {return vertexList[i].succ1List[j].iAA;};
 
  inline int    get_iVertex_jSucc2_iVertex(int i, int j)      {return vertexList[i].succ2List[j].iVertexSucc;};
  inline int    get_iVertex_jSucc2_iAA(int i, int j)          {return vertexList[i].succ2List[j].iAA;};

  void init(runManagerParameters*, aa*, ion*, data*);
  void buildGraph();
  int  compute_vertexNb();
  void build_vertice();
  void merge();
  bool findClosestNodes(int*, int*);
  void mergeNodes(int, int);
  void cleanGraph();
  void pickNodes();
  void connect(int);
  void connectSucc1(int);
  void connectSucc2(int);
  bool verifyEdges(vertex*, int, int, int);

  void write(File&, int);
  void write_dotGraphSimple(File&);
  void write_dotGraphDouble(File&);
  void display(File&);
  void displayXML(File&);
};


/******************************************************************************/

#endif
