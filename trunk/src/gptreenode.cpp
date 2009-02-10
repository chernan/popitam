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

//********************************************************************//
//              INCLUDE                                               //
//********************************************************************//
#include <cstdlib>                                                   // exit().
#include <iostream>                                                 // flux io.
#include <fstream>
#include "gptreenode.h"                                                     // NODE.

using namespace std;

//********************************************************************//
//              CONSTRUCTOR                                           //
//********************************************************************//
NODE::NODE(bool type, int value, int argv)
{
  ArgvNb      = argv;
  Type        = type;
  Value       = value;
  Father      = NULL;
  Coeff       = 0.0;
  Children    = new NODE*[ArgvNb];
  for (int i=0; i<ArgvNb; i++)
    Children[i] = NULL;
}

//********************************************************************//
//              DESTRUCTOR                                            //
//********************************************************************//
NODE::~NODE()
{
  if (Children != NULL)
    {
      for (int i=0; i<ArgvNb; i++)
	if (Children[i] != NULL)
	  Children[i] = NULL;
      delete Children;
      Children = NULL;
    }
  if (Father == NULL)
    Father = NULL;
}


//********************************************************************//
//              GETCHILD                                              //
//********************************************************************//
NODE *NODE::getChild(int ID)
{
  if (ID >= ArgvNb)
    {
      cerr<<"\nError 0 in function NODE::getChild().\n";
      exit(1);
    }
  if (Children == NULL)
    {
      cerr<<"\nError 1 in function NODE::getChild().\n";
      exit(1);
    }
  return Children[ID];
}
      
//********************************************************************//
//              SETCHILD                                              //
//********************************************************************//
void  NODE::setChild(NODE *child,int ID)
{
  if (ID >= ArgvNb)
    {
      cerr<<"\nError 0 in function NODE::setChild().\n";
      exit(0);
    }
  if (Children == NULL)
    {
      cerr<<"\nError 1 in function NODE::setChild().\n";
      exit(0);
    }
  Children[ID] = child;
}
      
