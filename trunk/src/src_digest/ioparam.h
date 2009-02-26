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

/***********************************************************

  Company            : Swiss Intitute of Bioinformatics
	Author             : Marc Tuloup
	Contact            : Marc.Tuloup@isb-sib.ch
	Created on         : 01.02.2007
	Last modification  : 01.02.2007
	Comments           : 

***********************************************************/


#ifndef __IOPARAM_H__
#define __IOPARAM_H__


#include "dynamicarray.h"
#include "tagfile.h"
#include "massdefine.h"
#include "kwmodedefine.h"



/******************************************************

Class IOParamDB


*******************************************************/

class IOParamDB 
{

private :
	
	char					*m_pszLabel;
	char					*m_pszPath;
	unsigned int	m_uiTime;


public :

	IOParamDB(void);
	IOParamDB(const char *pszLabel, const char *pszPath, unsigned int	uiTime=0);

	~IOParamDB(void);

	inline const char *GetLabel(void)
	{ return m_pszLabel; }

	inline const char *GetPath(void)
	{ return m_pszPath; }

	inline unsigned int GetTime(void)
	{ return m_uiTime; }

	void Copy(IOParamDB *pIOParamDB);

	void ReadVersion1And2(TagFile &file);
	void Read(TagFile &file);
	void Write(TagFile &file);

private:

	void Reset(void);
};




/******************************************************

Class IOParamModif


*******************************************************/

#define MODIF_FIXED			false
#define MODIF_VARIABLE	true

extern const char *pszModifMode[2];

class IOParamModif 
{

private :
	
	char		*m_pszName;
	char		*m_pszLabel;
	char		*m_pszLocus;
	char		*m_pszFormula;
	bool		m_bMode;
	int			m_iTolerance;
	double	m_dScoring;

public :

	IOParamModif(void);
	IOParamModif(const char *m_pszName, const char *pszLabel, const char *pszLocus, const char *pszFormula, 
				bool bMode, int iTolerance, double dScoring);

	~IOParamModif(void);


	void Copy(IOParamModif *pIOParamModif);

	void Read(TagFile &file);
	void Write(TagFile &file);

	inline const char *GetName(void)
	{ return m_pszName; }

	inline const char *GetLabel(void)
	{ return m_pszLabel; }

	inline const char *GetLocus(void)
	{ return m_pszLocus; }

	inline const char *GetFormula(void)
	{ return m_pszFormula; }

	inline bool GetMode(void)
	{ return m_bMode; }

	inline int GetTolerance(void)
	{ return m_iTolerance; }

	inline double GetScoring(void)
	{ return m_dScoring; }


	void SetName(const char *pszName);
	void SetLabel(const char *pszLabel);
	void SetLocus(const char *pszLocus);
	void SetFormula(const char *pszFormula);

	inline void SetMode(bool bMode)
	{ m_bMode = bMode; }

	void SetTolerance(int iTolerance);
	void SetScoring(double dScoring);
};







/******************************************************

Class IOParam


*******************************************************/


typedef enum{
	ION_MODE_M			= 0,
	ION_MODE_MPLUS,
	ION_MODE_MMINUS,
	NB_ION_MODE
}TE_IonMode;

extern const char *pszIonMode[NB_ION_MODE];

class IOParam 
{

public :
	
	//protein filters
	DynamicArray<IOParamDB *>			m_aDB;
	DynamicArray<int>							m_aTaxId;
	int														m_iMwMin;
	int														m_iMwMax;
	double												m_dPiMin;
	double												m_dPiMax;
	bool 													m_bVarsplices;
	bool 													m_bFragments;
	DynamicArray<int>							m_aKW;
	TE_KwMode											m_eKwMode;
	DynamicArray<char *>					m_aAC;

	//peptides
	char													*m_pszEnzyme;
	DynamicArray<IOParamModif *>	m_aModif;		
	int														m_iMissedClevage;
	TE_MassMode										m_eResolution;
	TE_IonMode										m_eIonMode;
	bool													m_bPTM;

		//Threshold
	double												m_dShift;
	int														m_iSlope;
	int														m_iError;
	int														m_iHitsMin;

	
	//output
	int														m_iDisplayMax;


private :

	static const int iVersion;

public :

	IOParam(void);
	~IOParam(void);

	void Read(const char *pszFileName);
	void Write(const char *pszFileName);

	void SetEnzyme(const char *psz);

	void Copy(IOParam *pParam);

private :

	void ReadVersion1(TagFile &file);
	void ReadVersion2(TagFile &file);
	void Read(TagFile &file);
};



#endif
