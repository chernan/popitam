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

#ifndef __DATA_H__
#define __DATA_H__

/******************************************************************************/

#include "ion.h"
#include "error.h"
#include "file.h"
#include "runManagerParameters.h"

/******************************************************************************/

/* GLOBAL VARIABLE DECLARED IN SEQMSMS */


/******************************************************************************/

/* COMPARISON ROUTINE FOR QSORT */

int compar_PEAKS(const void*, const void*);
int compar_PEAK_MASSES(const void*, const void*);

/******************************************************************************/


struct PEAKS {
  
  double             mass;                
  double             intensity;               
  int                noBin;
  bool               used;
  bool               CHARGE1;
  bool               CHARGE2;
  bool               CHARGE3;
};


/******************************************************************************/


class data {
  public :
  runManagerParameters *runManParam;
  
  int                   specID;
  char                  title[256];
  int                   initPeakNb;            // nbre de pics initiaux (totaux) (pour mémoire)
  int                   peakNb;                
  PEAKS                *peakList;
  double                parentMassRAW;
  double                parentMassM;
  int                   charge;
  int                   binsize;



  // pour les spectres identifiés:
  
  char   seqSpec[256];             // séquence telle qu'elle est dans le spectre:
                                   // les modif. prévues sont indiquées avec O (MOX) ou X (CAM), les modifs
                                   // non prévues sont indiquées par un * qui suit l'acide aminé modifié
                                   // les mutations sont indiquées par une minuscule

  char   seqAsInDtb[256];          // séquence telle qu'elle apparait dans la dtb
                                   // sans MOX ou CAM, sans autre modification et sans mutation
  
 


   data();
  ~data();
  
  inline int     getSpecID()                  {return specID;}
  inline char*   get_title()                  {return title;};
  inline int     get_initPeakNb()             {return initPeakNb;};
  inline int     get_peakNb()                 {return peakNb;};
  inline double  get_parentMassRAW()          {return parentMassRAW;};
  inline double  get_parentMassM()            {return parentMassM;};
  inline int     get_charge()                 {return charge;};
  inline char*   get_seqSpec()                {return seqSpec;};
  inline char*   get_seqAsInDtb()             {return seqAsInDtb;};
  inline int     get_binsize()                {return binsize;};
 
  inline PEAKS*  get_peak_i(int i)            
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return &peakList[i];
  }

  inline double  get_peak_i_mass(int i)       
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return peakList[i].mass;
  }

  inline double   get_peak_i_intensity(int i)  
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return peakList[i].intensity;
  }

  inline int     get_peak_i_noBin(int i)      
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return peakList[i].noBin;
  }

  inline bool    get_peak_i_used(int i)       
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return peakList[i].used;
  }

  inline bool    get_peak_i_CHARGE1(int i)         
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return peakList[i].CHARGE1;
  }
  
  inline bool    get_peak_i_CHARGE2(int i)  
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return peakList[i].CHARGE2;
  }
  
  inline bool    get_peak_i_CHARGE3(int i)    
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    return peakList[i].CHARGE3;
  }
  
  inline void    set_peak_i_used(int i, bool bobo) 
  {
    if (i >= peakNb) fatal_error(runManParam->FILE_ERROR_NAME, DEBUG, "in data.h");
    peakList[i].used = bobo;
  }
  
  void   init(runManagerParameters*);
  bool   load();
  bool   loadPeaksPOP(File&);
  bool   loadPeaksMGF(File&);
  bool   loadPeaksDTA(File&);
  void   preprocessPeaks();
  void   markParentMass();
  void   normInt();
  void   putBins();
  void   write(File&);
  void   writeSpectrumListXML(File&);
  void   display(File&);
  void   displayXML(File&);
};


#endif
