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
	Created on         : 06.07.2006
	Last modification  : 06.07.2006
	Comments           : 

***********************************************************/

#include "pimw.h"
#include "math.h"

#include <string.h>


/******************************************************

		Constructor 

Column 1 : pKa for COOH if C-terminal
Column 2 : pKa for NH2  if N-terminal
Column 3 : pKa for side chain, if applicable

*******************************************************/

PiMw::PiMw(TE_MassMode eMode)
{
	//pka pour chaque acide amine
	m_dPka[0][0]=3.55;		m_dPka[0][1]=7.59;		m_dPka[0][2]=0;				//A
	m_dPka[1][0]=3.55;		m_dPka[1][1]=7.50;		m_dPka[1][2]=0;				//B
	m_dPka[2][0]=3.55;		m_dPka[2][1]=7.50;		m_dPka[2][2]=9.00;		//C
	m_dPka[3][0]=4.55;		m_dPka[3][1]=7.50;		m_dPka[3][2]=4.05;		//D
	m_dPka[4][0]=4.75;		m_dPka[4][1]=7.70;		m_dPka[4][2]=4.45;		//E
	m_dPka[5][0]=3.55;		m_dPka[5][1]=7.50;		m_dPka[5][2]=0;				//F
	m_dPka[6][0]=3.55;		m_dPka[6][1]=7.50;		m_dPka[6][2]=0;				//G
	m_dPka[7][0]=3.55;		m_dPka[7][1]=7.50;		m_dPka[7][2]=5.98;		//H
	m_dPka[8][0]=3.55;		m_dPka[8][1]=7.50;		m_dPka[8][2]=0;				//I
	m_dPka[9][0]=0;				m_dPka[9][1]=0;				m_dPka[9][2]=0;				//J
	m_dPka[10][0]=3.55;		m_dPka[10][1]=7.50;		m_dPka[10][2]=10;			//K
	m_dPka[11][0]=3.55;		m_dPka[11][1]=7.50;		m_dPka[11][2]=0;			//L
	m_dPka[12][0]=3.55;		m_dPka[12][1]=7.00;		m_dPka[12][2]=0;			//M
	m_dPka[13][0]=3.55;		m_dPka[13][1]=7.50;		m_dPka[13][2]=0;			//N
	m_dPka[14][0]=0;			m_dPka[14][1]=0;			m_dPka[14][2]=0;			//O
	m_dPka[15][0]=3.55;		m_dPka[15][1]=8.36;		m_dPka[15][2]=0;			//P
	m_dPka[16][0]=3.55;		m_dPka[16][1]=7.50;		m_dPka[16][2]=0;			//Q
	m_dPka[17][0]=3.55;		m_dPka[17][1]=7.50;		m_dPka[17][2]=12.0;		//R
	m_dPka[18][0]=3.55;		m_dPka[18][1]=6.93;		m_dPka[18][2]=0;			//S
	m_dPka[19][0]=3.55;		m_dPka[19][1]=6.82;		m_dPka[19][2]=0;			//T
	m_dPka[20][0]=0;			m_dPka[20][1]=0;			m_dPka[20][2]=0;			//U
	m_dPka[21][0]=3.55;		m_dPka[21][1]=7.44;		m_dPka[21][2]=0;			//V
	m_dPka[22][0]=3.55;		m_dPka[22][1]=7.50;		m_dPka[22][2]=0;			//W
	m_dPka[23][0]=3.55;		m_dPka[23][1]=7.50;		m_dPka[23][2]=0;			//X	
	m_dPka[24][0]=3.55;		m_dPka[24][1]=7.50;		m_dPka[24][2]=10;			//Y
	m_dPka[25][0]=3.55;		m_dPka[25][1]=7.50;		m_dPka[25][2]=0;			//Z	


	//calcul une fois pour toute 10^(-Pka)
	for(int i=0; i<26; i++){
		m_dPka[i][0] = pow(10., - m_dPka[i][0]);
		m_dPka[i][1] = pow(10., - m_dPka[i][1]);
		m_dPka[i][2] = pow(10., - m_dPka[i][2]);
	}

	//masse de acides amines
	SetMassMode(eMode);
}


/***********************************************************

   SetMassMode

 ***********************************************************/
void PiMw::SetMassMode(TE_MassMode eMode)
{
	//masse de acides amines
	const char *ppszAAFormula[26] = {	"C3H5ON",			//A
																		"",						//B
																		"C3H5ONS", 		//C
																		"C4H5O3N", 		//D
																		"C5H7O3N", 		//E
																		"C9H9ON",			//F
																		"C2H3ON", 		//G
																		"C6H7ON3", 		//H
																		"C6H11ON", 		//I
																		"C6H11ON", 		//J
																		"C6H12ON2", 	//K
																		"C6H11ON", 		//L
																		"C5H9ONS",		//M
																		"C4H6O2N2", 	//N
																		"", 					//O
																		"C5H7ON", 		//P
																		"C5H8O2N2", 	//Q
																		"C6H12ON4", 	//R
																		"C3H5O2N", 		//S
																		"C4H7O2N", 		//T
																		"", 					//U
																		"C5H9ON", 		//V
																		"C11H10ON2", 	//W
																		"C9H3", 			//X		moyenne en tenant compte des frequence des AA dans SP.
																		"C9H9O2N",		//Y
																		""						//Z	
																	};

	AtomicMass atomicMass(eMode);

	for(int i=0; i<26; i++)
		m_tdAAMass[i] = atomicMass.GetMass(ppszAAFormula[i]); 

	m_dH2O = atomicMass.GetMass("H2O"); 
}

/******************************************************

		Destructor 

*******************************************************/

PiMw::~PiMw(void)
{	
}



/***********************************************************

   GetMw

***********************************************************/
double PiMw::GetMw(const char *psz)
{
	double dMass = 0;

	while(*psz)
		dMass += m_tdAAMass[*psz++ - 'A'];

	
	return (dMass + m_dH2O);
}



/***********************************************************

   GetPi

***********************************************************/
double PiMw::GetPi(const char *psz)
{
	memset(m_iComp, 0, 26 * sizeof(int));

	char cFirstCharact = *psz;

	while(*psz)
		m_iComp[*psz++ - 'A']++;

	char cLastCharact = *(psz-1);


	//pI
	int	iMin	= 0;		
	int	iMax	= 1400;		
	int iMid;
				
	int		iNbR	= m_iComp[17];	//'R' - 'A'
    int		iNbH	= m_iComp[7];	//'H' - 'A'
    int		iNbK	= m_iComp[10];	//'K' - 'A'
    int		iNbD	= m_iComp[3];	//'D' - 'A'
    int		iNbE	= m_iComp[4];	//'E' - 'A'
    int		iNbC	= m_iComp[2];	//'C' - 'A'
    int		iNbY	= m_iComp[24];	//'Y' - 'A'

	double	dNTerKa = m_dPka[cFirstCharact - 'A'][1];
	double	dCTerKa = m_dPka[cLastCharact - 'A'][0];
	double	dRKa	= m_dPka[17][2];
	double	dHKa	= m_dPka[7][2];
	double	dKKa	= m_dPka[10][2];
	double	dDKa	= m_dPka[3][2];
	double	dEKa	= m_dPka[4][2];
	double	dCKa	= m_dPka[2][2];
	double	dYKa	= m_dPka[24][2];

	
	double	dExpPH;
	double	dNTer, dCTer, dR, dH, dK, dD, dE, dC, dY;

	while(iMin < iMax){
		
		iMid = (iMax + iMin) / 2;

		dExpPH = pow(10., -(double)iMid / 100);	


		dNTer	= dExpPH			/ (dNTerKa + dExpPH);
		dR		= iNbR	* dExpPH	/ (dRKa + dExpPH);
		dH		= iNbH	* dExpPH	/ (dHKa + dExpPH);
		dK		= iNbK	* dExpPH	/ (dKKa + dExpPH);

		dCTer	= dCTerKa			/ (dCTerKa + dExpPH);
		dD		= iNbD	* dDKa		/ (dDKa + dExpPH);
		dE		= iNbE	* dEKa		/ (dEKa + dExpPH);
		dC		= iNbC	* dCKa		/ (dCKa + dExpPH);
		dY		= iNbY	* dYKa		/ (dYKa + dExpPH);

		//calcul du signe de la charge : pour avoir la charge il faudrait multiplier par dExpPH (>0)
		if( (dNTer + dR + dH + dK - (dCTer + dD + dE + dC + dY)) > 0)
			iMin = iMid+1;
		else
			iMax = iMid-1;
	}

	return (iMin / 100.);
}

