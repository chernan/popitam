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

#ifndef RESULT_H_
#define RESULT_H_

#include "prot.h"
#include "runManagerParameters.h"
#include "file.h"
#include "data.h"
#include "specresults.h"

// ********************************************************************************************** //


class result {
 public:  
  runManagerParameters *runManParam;
 specresults           *specResults;
  int                  *tabRank;
  prot                 *protList;

  result();
  ~result();
  void init(runManagerParameters*);
  void initTabRank(int);
  void initProtList();
  void addToProtList(specresults*, int);
};


// ********************************************************************************************** //


#endif
