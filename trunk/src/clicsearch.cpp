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
   
===============================================================================

    April 22, 1997
    this program implements Algorithm 4.5 for solving Problem 4.1  All Cliques
*/

// ------------------------------------------------------

#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <cstdlib>

#include "defs.h"
#include "clicsearch.h"
#include "setlib0.h"

#include "error.h"
#include "defines.h"
#include "memorycheck.h"

using namespace std;

// ********************************************************************************************** //

extern memorycheck memCheck;

// ********************************************************************************************** //

clicsearch::clicsearch()   {}

clicsearch::~clicsearch()  {}

// ********************************************************************************************** //

void clicsearch::findCliques(void(*cbArr)(void *pInstance, int size, int* clic),void *pInstance, bool** compPred, int subN, bool CONNECT) {

  m_pfctCallBackArr   = cbArr;
  m_pCallBackInstance = pInstance;
  
  // la fonction de recherche de cliques rapide nécessite que le graphe contienne au moins 3 noeuds
  // et qu'il ne soit pas complet
  // si le graph est complet, je lance une recherche force brute
  if (CONNECT) searchBrutForce(compPred, subN);
  else 
    {
      if (subN == 2)
	{
	  for (int i = 0; i < 2; i++)
	    CallBack(1, &i);
	}
      else
	{
	  searchQuick(compPred, subN);
	}
    }
}

// ********************************************************************************************** //

void clicsearch::searchQuick(bool** compPred, int subN)
{
  //f=fopen("beubeu.txt","r");
  //ReadGraphEdges(f, &A,&n);
  //fclose(f);
  ReadGraphEdges(&A, &n, compPred, subN);
  SetInit(n);
  B=NewSetList(n);
  C=NewSetList(n);
  N=NewSetList(n+1);
  X=(int *)calloc(n,sizeof(int));                                                                  memCheck.clicsearch++;
  GetSet(B[0],V);
  SetDelete(0,B[0]);
  for (int v = 1; v < n; v++) {
    GetSet(B[v],B[v-1]);
    SetDelete(v,B[v]);
  }
  
  AllCliques(0);
  free(X);                                                                                         memCheck.clicsearch--;
  FreeSetList(n, &B);
  FreeSetList(n, &C);
  FreeSetList(n+1, &N);
  ClearSet(n);
  FreeGraph(n, A);
}


// ********************************************************************************************** //

void clicsearch::searchBrutForce(bool** compPred, int subN)
{
  int clic[MAX_SUBSEQ_NB_IN_AN_ARRANGEMENT];       // clic[] va contenir les différentes cliques extraites du graphe compPredTab[][]    
                                                   // au fur et à mesure qu'elles seront trouvées
 
  // dès qu'une nouvelle clique sera trouvée, elle sera ajoutée dans le tableau membre scenarioTab
  
  for (int size = subN; size > 0; size--)
    // on va rechercher toutes les cliques de compatibilité de toutes les tailles possibles
    recFindCliques(compPred, subN, clic, 0, 0, size);
  
}

// ********************************************************************************************** //

void clicsearch::recFindCliques(bool** compPred, int subN, int* clic, int i, int j, int size) {
  // on recherche toutes les cliques de taille size parmi le graphe de compatibilité des sous-séquences
  // je recherche d'abord une combinaison possible, puis je teste pour savoir si c'est une clique
    
  bool test = TRUE;
  
  if (i == size) {                        // une combinaison de taille size a été trouvée
    if (isComplete(compPred, clic, size)) {    // je teste pour savoir si tous ses membres de la combinaison sont connectés dans le graphe 
                                          // de compatibilité en d'autres mots, si le sous-graphe représenté par la combinaison est 
                                          // un sous-graphe complet, soit une clique      
      // clic contient une clique possible de compatibilité de taille size
      // on la stocke dans le tableau scenarioTab[]
     
      CallBack(size, clic);
    }
  }
  else {
    for (int k = j; k <= subN - size + i; k++) {
      test  = true;
      for (int l = 0; l < i; l++) {
        if (!compPred[clic[l]][k]) {
          test = FALSE;
          break;
	}
      }
      
      clic[i] = k;
      
      if (test == TRUE)
        recFindCliques(compPred, subN, clic, i+1, k+1, size);
    }
  }
}

// ********************************************************************************************** //

bool clicsearch::isComplete(bool** tab, int* clic, int size) {
  
  if (size == 1) return TRUE;
  // il n'y a qu'une seule sous-séquence, et donc le graphe représenté par une seule
  // sous séquence est forcément complet
  
  for (int i = 0; i < size; i++) {
    for (int j = i+1; j < size; j++) {
      if (!tab[clic[i]][clic[j]]) return FALSE;
    }
  }
  return TRUE;
}

// ********************************************************************************************** //


void clicsearch::AllCliques(int ell) {
  /*
    Algorithm 4.4. 
    This algorithm returns a list of all the cliques in the graph without repetition.
  */
  int start,i,x;
  if (ell==0) {
    GetSet(C[ell],V);
    GetSet(N[ell],V);
    start=0;
  }
  else {
  
    for(i=0;i<WORDS;i++) {
      C[ell][i]=A[X[ell-1]][i]&B[X[ell-1]][i]&C[ell-1][i];
    }
    start=X[ell-1];
    // ON A UNE CLIQUE DANS X[]
    /*
      printf("["); 
      for(i=0;i<ell;i++)printf(" %d",X[i]); 
      printf(" ]");
    }
    */
    
    CallBack(ell, X);                                                                            // CALLBACK !!!
    Intersection(A[X[ell-1]],N[ell-1],N[ell]);   
  }
  for(x=start;x<n;x++) if(MemberOfSet(x,C[ell])) {
    X[ell]=x;
    AllCliques(ell+1);
  }
}

// ********************************************************************************************** //

graph clicsearch::NewGraph(int n)      // Allocate storage for a graph on order n
{
  int i;
  graph A;
  A = (graph)calloc(n, sizeof(set));                                                             memCheck.clicsearch++;
  for(i=0;i<n;i++) {
    A[i] = NewSet();
  }
  return(A);
}

// ********************************************************************************************** //

void clicsearch::FreeGraph(int n, graph A)  // Deallocate storage for a graph on order n
{
  int i;
  for(i=0;i<n;i++) FreeSet(&A[i]);
  free(A);                                                                                       memCheck.clicsearch--;
}

// ********************************************************************************************** //


// Read and allocate storage for a graph from the file F which contains a list 
// of edges. The order of the graph is returned in n.

void clicsearch::ReadGraphEdges(graph *A, int *n, bool** compPred, int subN) {
 
  //  int u,v;
  //  int i ,m;
  //  fscanf(F,"%d %d", &m, n);
  //  ComputeWords(*n);
  //  *A=NewGraph(*n);
  //  for(i=0;i<m;i++) {
  //  fscanf(F,"%d %d",&u,&v);
  //  SetInsert(u,(*A)[v]);
  //  SetInsert(v,(*A)[u]);
  //  }
  
  
  // HOME
  int u,v;
  *n = subN;            
  ComputeWords(*n);
  *A = NewGraph(*n);
  
  for (u = 0; u < *n; u++) {
    for (v = u; v < *n; v++) {
      if (compPred[u][v]) {
	SetInsert(u, (*A)[v]);
	SetInsert(v, (*A)[u]);
      }
    }
  }
}

// ********************************************************************************************** //

void clicsearch::CallBack(int size, int* clic)
{
  m_pfctCallBackArr(m_pCallBackInstance, size, clic);
}

// ********************************************************************************************** //

