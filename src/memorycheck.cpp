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

#include "memorycheck.h"

// ********************************************************************************************** //


memorycheck::memorycheck()
{
  runManParam        = 0;
  popitam            = 0;
  prot               = 0;
  aa                 = 0;
  data               = 0;
  ion                = 0;
  tagoScore          = 0;
  graph              = 0;
  subseq             = 0;
  tagprocessor       = 0;
  tagextractor       = 0;
  clicsearch         = 0;
  tagoResults        = 0;
  results            = 0;
  sufftree           = 0;
  digest             = 0;
  compare            = 0;
  functionScore      = 0;
  fun                = 0;
  scenario           = 0;
  peptide            = 0;
  allrunstatistics   = 0;
  spectrumstatistics = 0;
  treeword           = 0;
}


// ********************************************************************************************** //

memorycheck::~memorycheck() {}

// ********************************************************************************************** //


