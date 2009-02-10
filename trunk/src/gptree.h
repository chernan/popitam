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

#ifndef _TREE_H_
#define _TREE_H_

//********************************************************************//
//              INCLUDE                                               //
//********************************************************************//
#include <fstream>                                                  // ifstream, ofstream.
#include "gptreenode.h"                                                     // class NODE.
#include "gpparameters.h"
#include "score.h"

using namespace std;

//********************************************************************//
//              CLASS TREE                                            //
//********************************************************************//

class TREE
{
 private:
  NODE   *Root;                                                         // root of the tree.
  int     NbNode;                                                        // number of nodes of the tree.
  int     NbTerminal;                                                    // number of terminal of the tree.
  int     NbFunction;                                                    // number of function of the tree.
  int     NbCoeff;                                                       // number of coeff of the tree.
  int     Depth;                                                         // Depth of the tree.
  bool    State;                                                         // 0 => not evaluated, 1 => evaluated. 
  
 public:
  inline NODE   *getRoot()       const   { return Root; }             // returns the root.
  inline int    getNbNode()      const   { return NbNode; }           // returns the number of node.
  inline int    getNbTerminal()  const   { return NbTerminal; }       // returns the number of terminal.
  inline int    getNbFunction () const   { return NbFunction ; }      // returns the number of function.
  inline int    getNbCoeff()     const   { return NbCoeff; }          // returns the number of coefficient.
  inline int    getDepth()       const   { return Depth; }            // returns the depth.
  inline bool   getState()       const   { return State; }            // returns the state of the tree evaluation.


  inline void   setRoot(NODE *root)      { Root = root; }             // puts the root.
  inline void   setNbNode(int nb)        { NbNode = nb; }             // puts the number of node.
  inline void   setNbTerminal(int nb)    { NbTerminal = nb; }         // puts the number of terminal.
  inline void   setNbFunction (int nb)   { NbFunction  = nb; }        // puts the number of function.
  inline void   setNbCoeff(int nb)       { NbCoeff = nb; }            // puts the number of coefficient.
  inline void   setState(bool boolean)   { State = boolean; }         // puts the state od the tree evaluation.

  // destructor and constructor.
                                                      
  TREE();                                                             // constructor.        
 ~TREE();                                                             // destructor.
  void DeleteSubTree(NODE *);                                         // deletes a sub-tree.
  
  // chargement arbre et sous-arbre a partir d'une structure dot.
  void LoadTree(PARAMETERS*, char*, char*);                                               // charge un arbre.
  void LoadTree(PARAMETERS*, NODE *,ifstream &);                                   // charge un sous-arbre.
  void LoadNode(PARAMETERS*, ifstream &,int &,bool &,int &,float &);               // charge un noeud.
  void UpdateTree(char*);
  void UpdateTree(NODE *, int);
  // draw tree
  void DrawTree();                                                    // draws a tree.
  void DrawTreeRec(ofstream &, NODE *, int &);                        // draws a sub-tree.
  
  // copy a tree
  void CopyTree(TREE *);                                              // copy a tree
  void CopyNode(NODE *, NODE *,int);                                  // copy a sub-tree.

  // parse tree or sub-tree
  double ParseTree(char*, NODE*, score, bool*, bool*);                            // parses tree.
};

#endif
