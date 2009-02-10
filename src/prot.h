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

#ifndef __PROT_H__
#define __PROT_H__


// ********************************************************************************************** //

#include "protEl.h"
#include "file.h"
#include "data.h"
#include "runManagerParameters.h"
#include "element.h"
#include "specresults.h"

// ********************************************************************************************** //


class prot {
 public:
  runManagerParameters  *runManParam;
 

  int                    elementNb;
  protEl                *firstElement;
  protEl                *lastElement;
  protEl                *newElement;
  protEl                *currentElement;
  protEl                *newEl;

   prot();
  ~prot();
  void init(runManagerParameters*);
  void compile();
  void removeElement(protEl*);
  void update(element*, specresults*, int);
  void computeElCovs();
  void sort();
  void sortPepByParentMass();
  void removeDuplicatedProteins();
  bool compareElements(protEl*, protEl*);
  void compactElements(protEl*, protEl*);
  void writeSimple(File&);
  void write(File&);

};

/******************************************************************************/

#endif
