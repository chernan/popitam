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

#include "runManager.h"
#include "about.h"
#include "usage.h"
#include "memorycheck.h"
#include "MATerror.h"

// ******************************************************************************************** //

memorycheck memCheck;

// ******************************************************************************************** //

int main(int argc, char** argv) 
{
	int status;
 
	// ou: time_t t;   srand((unsigned)time(&t))
	srand(0);
	//about();
  
	try {
		runManager* runM = new runManager();
		runM->init(argc, argv);
		runM->run();
		runM->endRun();
		delete runM;
		runM = NULL;
		status = 0;

	} catch(Error *pError){
		pError->Print();
		delete pError;
		status = 1;
	}
  
	return status;
}

