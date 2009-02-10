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

#ifndef _NODE_H_
#define _NODE_H_

//************************************************************************//
//              CLASS NODE                                                //
//************************************************************************//
class NODE
{
 private:
  int    ArgvNb;                                                           // number of children.
  int    Value;                                                            // value of the node. 
  bool   Type;                                                             // 0->terminal   1->function.
  float  Coeff;                                                            // coefficient.
  NODE  *Father;                                                           // father of the node.
  NODE **Children;                                                        // children of the node.
 
 public:
  NODE(bool, int, int);                                                   // constructor.
  ~NODE();                                                                // destructor.
  inline void    setFather(NODE *father)     { Father = father; }
  inline void    setArgvNb(int nb)           { ArgvNb = nb; }
  inline void    setType(bool type)          { Type = type; }
  inline void    setValue(int value)         { Value = value; }
  inline void    setCoeff(float coeff)       { Coeff = coeff; }
  void           setChild(NODE *,int);

  inline NODE   *getFather()       const     { return Father; }
  inline float   getCoeff()        const     { return Coeff; }
  inline bool    getType()         const     { return Type; }
  inline int     getValue()        const     { return Value; }
  inline int     getArgvNb()       const     { return ArgvNb; }
  NODE           *getChild(int);
};


#endif

