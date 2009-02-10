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

#include <fstream>
#include <iostream>
#include <cstdlib>

#include "error.h"
#include "defines.h"

#include "file.h"


/******************************************************************************/

void fatal_error(char* fileErrorName, int type, char* msg) {
  File fp;
  fp.Open(fileErrorName, "w");

  switch(type) {
  case FILES:  fprintf(fp, "1\ncan not open/find FILE...\n");          break;
  case TEMP:   fprintf(fp, "2\ncan not create temporary file...\n");   break;
  case MEMORY: fprintf(fp, "1\nreading/writing out of memory...\n");   break;
  case LOOK:   fprintf(fp, "1\ndata missing...\n");                    break;
  case DEBUG:  fprintf(fp, "2\ndebug error...\n");                     break;
  case MODE:   fprintf(fp, "2\narguments error...\n");                 break;
  case NANI:   fprintf(fp, "2\na variable has value NAN...\n");        break;
  case INFI:   fprintf(fp, "2\na variable has value INF...\n");        break;
  case BORN:   fprintf(fp, "2\na score is not correctly borned...\n"); break;
  case DATAS:  fprintf(fp, "1\nproblem with input data...\n");         break;
  default:     fprintf(fp, "2\ninternal error");
  }
  fprintf(fp, "--> %s\n", msg);

  fp.Close();

  exit(1);  //termine le programme immédiatement
}


/******************************************************************************/
