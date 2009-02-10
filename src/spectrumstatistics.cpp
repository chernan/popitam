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

#include "spectrumstatistics.h"
#include "defines.h"
#include "error.h"



// ********************************************************************************************** //

spectrumstatistics::spectrumstatistics() {}

// ********************************************************************************************** //

spectrumstatistics::~spectrumstatistics() {}

// ********************************************************************************************** //

void spectrumstatistics::init()
{
  protNbInDatabases          = 0;  
  protNbInRange              = 0;
  pepGeneratedByDigestion    = 0;  
  pepCandidate               = 0;  
  pepWithAtLeastOneSubSeq    = 0;  
  cumSubSeqNb                = 0;
  pepWithAtLeastOneScenario  = 0;
  cumScenarioNb              = 0;
  
  IN_DTB          = false;
  MANQUE          = false;
}

// ********************************************************************************************** //

void spectrumstatistics::display(File& fp)
{
  fprintf(fp, "\n\n________________________________________________________________________________\n\n");
  fprintf(fp, "\n");
  fprintf(fp, "%-95s: %9i\n",   "Total number of proteins in databases", protNbInDatabases);
  fprintf(fp, "%-95s: %9i\n",   "Number of proteins after filter (taxonomy, ACs...)", protNbInRange);  
//  fprintf(fp, "%-95s: %9i\n",   "Number of peptides obtained from above proteins", pepGeneratedByDigestion); 
  fprintf(fp, "%-95s: %9i\n",   "Number of peptides after filter (PM...)", pepCandidate);
//  fprintf(fp, "%-95s: %9i\n",   "Candidate peptides with one or more subsequence", pepWithAtLeastOneSubSeq);
//  fprintf(fp, "%-95s: %9i\n",   "Cumulated number of subsequences", cumSubSeqNb);
  fprintf(fp, "%-95s: %9i\n",   "Number of peptides with one or more scenario", pepWithAtLeastOneScenario);
  fprintf(fp, "%-95s: %9i\n\n", "Cumulated number of scenarios", cumScenarioNb);

}

// ********************************************************************************************** //

void spectrumstatistics::displayXML(File& fp)
{
	fprintf(fp, "      <dbSearch>\n");
	fprintf(fp, "         <totalNbProtein>%i</totalNbProtein>\n", protNbInDatabases);
	fprintf(fp, "         <protNbAfterFilter>%i</protNbAfterFilter>\n", protNbInRange);  
	fprintf(fp, "         <pepNbAfterFilter>%i</pepNbAfterFilter>\n", pepCandidate);
	fprintf(fp, "         <pepNbWithOneMoreScenarios>%i</pepNbWithOneMoreScenarios>\n", pepWithAtLeastOneScenario);
	fprintf(fp, "         <cumulNbOfScenarios>%i</cumulNbOfScenarios>\n", cumScenarioNb);
}

// ********************************************************************************************** //

