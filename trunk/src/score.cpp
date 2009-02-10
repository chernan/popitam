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

#include "aa.h"
#include "error.h"
#include "score.h"
#include "defines.h"


// ********************************************************************************************** //

score::score() {}

// ********************************************************************************************** //

score::~score() {}

// ********************************************************************************************** //

void score::init() 
{
  lakScore1_a = 0;
  modScore1_b = 0;
  covScore1_c = 0;                 
  covScore2_d = 0;                 
  covScore3_e = 0;                 
  intScore1_f = 0;                 
  perScore1_g = 0;                 
  famScore1_h = 0;
  errScore1_i = 0;          
  redScore1_j = 0;
  serScore1_k = 0; 
  serScore2_l = 0;
  finArrScore = FLOAT_LIMIT_NEG;
}

// ********************************************************************************************** //

void score::write(File& fp)
{
  fprintf(fp, "%.4f ", lakScore1_a);
  fprintf(fp, "%.4f ", modScore1_b);
  fprintf(fp, "%.4f ", covScore1_c);
  fprintf(fp, "%.4f ", covScore2_d);
  fprintf(fp, "%.4f ", covScore3_e);
  fprintf(fp, "%.4f ", intScore1_f);
  fprintf(fp, "%.4f ", perScore1_g);
  fprintf(fp, "%.4f ", famScore1_h);
  fprintf(fp, "%.4f ", errScore1_i);  
  fprintf(fp, "%.4f ", redScore1_j);
  fprintf(fp, "%.4f ", serScore1_k);
  fprintf(fp, "%.4f ", serScore2_l);
  fprintf(fp, "%.4f ", finArrScore);
}

// ********************************************************************************************** //
