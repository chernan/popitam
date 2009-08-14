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

#include "usage.h"
#include <iostream>
#include <cstdlib>
#include "file.h"


void usage()
{
  File fp(stdout);

  fprintf(fp, "Popitam - version %s\n\n", POP_VERSION);
  fprintf(fp, "USAGE: \n");
  fprintf(fp, "tagopop -r=runMode -s=soluceMode -m=modNb -p=paramFile -d=dataFile -f=dataFileFormat -e=errorFile -o=outputFile [-noxml] [-nosht]\n\n");
  fprintf(fp, "REQUIRED arguments:\n");
  fprintf(fp, "runMode:           NORMAL  for a \"normal\" use of tagopop\n");
  fprintf(fp, "                   CHECK   if you are debugging (only for single spectrum input data) and you whish to see all output files\n");
  fprintf(fp, "                   FUN     if you want produce output for function learning by Genetic Programming (only with identified spectra\n");
  fprintf(fp, "soluceMode:        UNKNOWN for a \"normal\" use of tagopop (you don't know the answers)\n");
  fprintf(fp, "                   IDSET   if the source data are already identified and you whish to see tagopop's performance\n");
  fprintf(fp, "                   MIXSET  if the source data were obtained from a mixture (not finished, please change the AC list in the code)\n");
  fprintf(fp, "modifNb:           0       if you don't want to allow scenarios with modifications/mutations\n");
  fprintf(fp, "                   1       if you want to allow scenarios with a maximum of 1 modification hypotheses (more slow)\n");
  fprintf(fp, "                   2       if you want to allow scenarios with a maximum of 2 modification hypotheses (yet more slow)\n");
  fprintf(fp, "paramFile          filename.txt (put here the filename of the paramFile you want to use)\n");
  fprintf(fp, "dataFile           filename     (put here the MS/MS datafile you want to submit)\n");
  fprintf(fp, "dataFileFormat     mgf     for mascot format\n");
  fprintf(fp, "                   dta     for dta format\n");
  fprintf(fp, "errorFile          filename\n");
  fprintf(fp, "outputFile         filename or stdout\n\n");
  fprintf(fp, "OPTIONAL arguments:\n");
  fprintf(fp, "noxml              tell Popitam to not generate a XML file output\n");
  fprintf(fp, "nosht              tell Popitam to not generate a SHT file output\n");
  fprintf(fp, "_________________________________________________________________________________\n\n");

  fp.Close();
  exit(1);
  
}
