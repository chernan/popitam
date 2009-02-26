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
	Created on         : 27.11.2003
  Last modification  : 05.10.2007
	Comments           : 

***********************************************************/

#include "atomicmass.h"
#include "error.h"

#include <string.h>
#include <stdlib.h>

#include <assert.h>


/*

Exact Masses and Isotopic Abundances : http://www.sisweb.com/referenc/source/exactmaa.htm


Ag(107)  106.905095   51.84    Ag(109)  108.904754   48.16                               	
Al(27)    26.981541  100.00                                                             	
Ar(36)    35.967546    0.34    Ar(38)    37.962732   0.063    Ar(40)    39.962383   99.60 	
As(75)    74.921596  100.00                                                             	
Au(197)  196.966560  100.00                                                             	
B(10)     10.012938   19.80    B(11)     11.009305   80.20                               	
Ba(130)  129.906277    0.11    Ba(132)  131.905042    0.10    Ba(134)  133.904490    2.42 Ba(135)  134.905668    6.59    Ba(136)  135.904556    7.85    Ba(137)  136.905816   11.23 Ba(138)  137.905236   71.70                                                             	
Be(9)      9.012183  100.00                                                             	
Bi(209)  208.980388  100.00                                                             	
Br(79)    78.918336   50.69    Br(81)    80.916290   49.31                               	
C(12)     12.000000   98.90    C(13)     13.003355    1.10                               	
Ca(40)    39.962591   96.95    Ca(42)    41.958622    0.65    Ca(43)    42.958770    0.14 Ca(44)    43.955485   2.086    Ca(46)    45.953689   0.004    Ca(48)    47.952532    0.19 	
Cd(106)  105.906461    1.25    Cd(110)  109.903007   12.49    Cd(111)  110.904182   12.80 Cd(112)  111.902761   24.13    Cd(113)  112.904401   12.22    Cd(114)  113.903361   28.73 Cd(116)  115.904758    7.49 	
Ce(136)  135.907140    0.19    Ce(138)  137.905996    0.25    Ce(140)  139.905442   88.48 Ce(142)  141.909249   11.08                                                             	
Cl(35)    34.968853   75.77    Cl(37)    36.965903   24.23                               	
Co(59)    58.933198  100.00                                                             	
Cr(50)    49.946046    4.35    Cr(52)    51.940510   83.79    Cr(53)    52.940651    9.50 Cr(54)    53.938882    2.36                                                             	
Cs(133)  132.905433  100.00                                                             	
Cu(63)    62.929599   69.17    Cu(65)    64.927792   30.83                               	
Dy(156)  155.924287   0.060    Dy(158)  157.924412    0.10    Dy(160)  159.925203    2.34 Dy(161)  160.926939   18.90    Dy(162)  161.926805   25.50    Dy(163)  162.928737   24.90 Dy(164)  163.929183   28.20                                                             	
Er(162)  161.928787    0.14    Er(164)  163.929211    1.61    Er(166)  165.930305   33.60 Er(167)  166.932061   22.95    Er(168)  167.932383   26.80    Er(170)  169.935476   14.90 	
Eu(151)  150.919860   47.80    Eu(153)  152.921243   52.20                               	
F(19)     18.998403  100.00                                                             	
Fe(54)    53.939612    5.80    Fe(56)    55.934939   91.72    Fe(57)    56.935396    2.20 Fe(58)    57.933278    0.28                                                             	
Ga(69)    68.925581   60.10    Ga(71)    70.924701   39.90                               	
Gd(152)  151.919803   0.200    Gd(154)  153.920876    2.18    Gd(155)  154.822629   14.80 Gd(156)  155.922130   20.47    Gd(157)  156.923967   15.65    Gd(158)  157.924111   24.84 Gd(160)  159.927061   21.86                                                             	
Ge(70)    69.924250   20.50    Ge(72)    71.922080   27.40    Ge(73)    72.923464    7.80 Ge(74)    73.921179   36.50    Ge(76)    75.921403    7.80                               	
H(1)       1.007825   99.99    H(2)       2.014102   0.015   	
He(3)      3.016029   .0001    He(4)      4.002603  100.00                               	
Hf(174)  173.940065    0.16    Hf(176)  175.941420    5.20    Hf(177)  176.943233   18.60 Hf(178)  177.943710   27.10    Hf(179)  178.945827   13.74    Hf(180)  179.946561   35.20 	
Hg(196)  195.965812    0.15    Hg(198)  197.966760   10.10    Hg(199)  198.968269   17.00  Hg(200)  199.968316   23.10    Hg(201)  200.970293   13.20    Hg(202)  201.970632   29.65  Hg(204)  203.973481    6.80                                                             	
Ho(165)  164.930332  100.00                                                             	
I(127)   126.904477  100.00                                                             	
In(113)  112.904056    4.30    In(115)  114.903875   95.70                               	
Ir(191)  190.960603   37.30    Ir(193)  192.962942   62.70                               	
K(39)     38.963708   93.20    K(40)     39.963999   0.012    K(41)     40.961825    6.73 	
Kr(78)    77.920397    0.35    Kr(80)    79.916375    2.25    Kr(82)    81.913483   11.60 Kr(83)    82.914134   11.50    Kr(84)    83.911506   57.00    Kr(86)    85.910614   17.30 	
La(138)  137.907114    0.09    La(139)  138.906355   99.91                               	
Li(6)      6.015123    7.42    Li(7)      7.016005   92.58                               	
Lu(175)  174.940785   97.40    Lu(176)  175.942694    2.60                               	
Mg(24)    23.985045   78.90    Mg(25)    24.985839   10.00    Mg(26)    25.982595   11.10 	
Mn(55)    54.938046  100.00                                                             	
Mo(92)    91.906809   14.84    Mo(94)    93.905086    9.25    Mo(95)    94.905838   15.92  Mo(96)    95.904676   16.68    Mo(97)    96.906018    9.55    Mo(98)    97.905405   24.13  Mo(100)   99.907473    9.63                                                             	
N(14)     14.003074   99.63    N(15)     15.000109    0.37                               	
Na(23)    22.989770  100.00                                                             	
Nb(93)    92.906378  100.00                                                             	
Nd(142)  141.907731   27.13    Nd(143)  142.909823   12.18    Nd(144)  143.910096   23.80  Nd(145)  144.912582    8.30    Nd(146)  145.913126   17.19    Nd(148)  147.916901    5.76  Nd(150)  149.920900    5.64                                                             	
Ne(20)    19.992439   90.60    Ne(21)    20.993845    0.26    Ne(22)    21.991384    9.20 	
Ni(58)    57.935347   68.27    Ni(60)    59.930789   26.10    Ni(61)    60.931059    1.13 Ni(62)    61.928346    3.59    Ni(64)    63.927968    0.91                               	
O(16)     15.994915   99.76    O(17)     16.999131   0.038    O(18)     17.999159    0.20 	
Os(184)  183.952514    0.02    Os(186)  185.953852    1.58    Os(187)  186.955762    1.60  Os(188)  187.955850   13.30    Os(189)  188.958156   16.10    Os(190)  189.958455   26.40  Os(192)  191.961487   41.00                                                             	
P(31)     30.973763  100.00                                                             	
Pb(204)  203.973037    1.40    Pb(206)  205.974455   24.10    Pb(207)  206.975885   22.10 Pb(208)  207.976641   52.40                                                             	
Pd(102)  101.905609    1.02    Pd(104)  103.904026   11.14    Pd(105)  104.905075   22.33 Pd(106)  105.903475    27.33   Pd(108)  107.903894   26.46    Pd(110)  109.905169   11.72 	
Pr(141)  140.907657  100.00                                                             	
Pt(190)  189.959937   0.010    Pt(192)  191.961049    0.79    Pt(194)  193.962679   32.90 Pt(195)  194.964785   33.80    Pt(196)  195.964947   25.30    Pt(198)  197.967879    7.20 	
Rb(85)    84.911800   72.17    Rb(87)    86.909184   27.84                               	
Re(185)  184.952977   37.40    Re(187)  186.955765   62.60                               	
Rh(103)  102.905503  100.00                                                             	
Ru(96)    95.907596    5.52    Ru(98)    97.905287    1.88    Ru(99)    98.905937   12.70  Ru(100)   99.904218   12.60    Ru(101)  100.905581   17.00    Ru(102)  101.904348   31.60  Ru(104)  103.905422   18.70                                                             	
S(32)     31.972072   95.02    S(33)     32.971459    0.75    S(34)     33.967868    4.21 S(36)     35.967079   0.020                                                             	
Sb(121)  120.903824   57.30    Sb(123)  122.904222   42.70                               	
Sc(45)    44.955914  100.00                                                             	
Se(74)    73.922477    0.90    Se(76)    75.919207    9.00    Se(77)    76.919908    7.60 Se(78)    77.917304   23.50    Se(80)    79.916521   49.60    Se(82)    81.916709    9.40 	
Si(28)    27.976928   92.23    Si(29)    28.976496    4.67    Si(30)    29.973772    3.10 	
Sm(144)  143.912009    3.10    Sm(147)  146.914907   15.00    Sm(148)  147.914832   11.30  Sm(149)  148.917193   13.80    Sm(150)  149.917285    7.40    Sm(152)  151.919741   26.70  Sm(154)  153.922218   22.70                                                             	
Sn(112)  111.904826  0.97      Sn(114)  113.902784  0.65  Sn(115)  114.903348 0.36	 Sn(116)  115.901744   14.70    Sn(117)  116.902954    7.70    Sn(118)  117.901607   24.30  Sn(119)  118.903310    8.60    Sn(120)  119.902199   32.40    Sn(122)  121.903440    4.60  Sn(124)  123.905271    5.60                                                             
Sr(84)    83.913428    0.56    Sr(86)    85.909273    9.86    Sr(87)    86.908902    7.00 Sr(88)    87.905625   82.58                                                             	
Ta(180)  179.947489   0.012    Ta(181)  180.948014   99.99                               	
Tb(159)  158.925350  100.00                                                             	
Te(122)  121.903055    2.60    Te(123)  122.904278    0.91    Te(124)  123.902825    4.82  Te(125)  124.904435    7.14    Te(126)  125.903310   18.95    Te(128)  127.904464   31.69  Te(130)  129.906229   33.80                                                             	
Th(232)  232.038054  100.00                                                             	
Ti(46)    45.952633    8.00    Ti(47)    46.951765    7.30    Ti(48)    47.947947   73.80 Ti(49)    48.947871    5.50    Ti(50)    49.944786    5.40                               	
Tl(203)  202.972336   29.52    Tl(205)  204.974410   70.48                               	
Tm(169)  168.934225  100.00                                                             	
U(234)   234.040947   0.006    U(235)   235.043925    0.72    U(238)   238.050786   99.27 	
V(50)     49.947161    0.25    V(51)     50.943963   99.75                               	
W(180)   179.946727    0.13    W(182)   181.948225   26.30    W(183)   182.950245   14.30 W(184)   183.950953   30.67    W(186)   185.954377   28.60                               	
Xe(124)  123.905894    0.10    X(126)   125.904281   0.09	 Xe(128)  127.903531    1.91    Xe(129)  128.904780   26.40    Xe(130)  129.903510    4.10  Xe(131)  130.905076   21.20    Xe(132)  131.904148   26.90    Xe(134)  133.905395   10.40  Xe(136)  135.907219    8.90                                                             
Y(89)     88.905856  100.00                                                             	
Yb(168)  167.933908    0.13    Yb(170)  169.934774    3.05    Yb(171)  170.936338   14.30  Yb(172)  171.936393   21.90    Yb(173)  172.938222   16.12    Yb(174)  173.938873   31.80  Yb(176)  175.942576   12.70                                                             	
Zn(64)    63.929145   48.60    Zn(66)    65.926035   27.90    Zn(67)    66.927129    4.10 Zn(68)    67.924846   18.80    Zn(70)    69.925325    0.60                               	
Zr(90)    89.904708   51.45    Zr(91)    90.905644   11.27    Zr(92)    91.905039   17.17 Zr(94)    93.906319   17.33    Zr(96)    95.908272    2.78    	


Ag(Average)	107.868131			
Al(Average)	26.981541				
Ar(Average)	39.948740				
As(Average)	74.921596				
Au(Average)	196.966560			
B	(Average)	10.812024				
Ba(Average)	137.326659			
Be(Average)	9.012183				
Bi(Average)	208.980388			
Br(Average)	79.903527				
C	(Average)	12.011037				
Ca(Average)	40.086465				
Cd(Average)	111.451208			
Ce(Average)	140.114868			
Cl(Average)	35.452738				
Co(Average)	58.933198				
Cr(Average)	51.995926				
Cs(Average)	132.905433			
Cu(Average)	63.545642				
Dy(Average)	162.497541			
Er(Average)	167.255716			
Eu(Average)	151.964582			
F	(Average)	18.998403				
Fe(Average)	55.846815				
Ga(Average)	69.723230				
Gd(Average)	157.237329			
Ge(Average)	72.632251				
H	(Average)	1.008026				
He(Average)	4.002606				
Hf(Average)	178.489791			
Hg(Average)	200.588446			
Ho(Average)	164.930332			
I	(Average)	126.904477			
In(Average)	114.817883			
Ir(Average)	192.216070			
K	(Average)	39.075702				
Kr(Average)	83.800024				
La(Average)	138.905456			
Li(Average)	6.941740				
Lu(Average)	174.966835			
Mg(Average)	24.306852				
Mn(Average)	54.938046				
Mo(Average)	95.931289				
N	(Average)	14.006763				
Na(Average)	22.989770				
Nb(Average)	92.906378				
Nd(Average)	144.242350			
Ne(Average)	20.190941				
Ni(Average)	58.687890				
O	(Average)	15.998985				
Os(Average)	190.239787			
P	(Average)	30.973763				
Pb(Average)	207.216897			
Pd(Average)	106.415325			
Pr(Average)	140.907657			
Pt(Average)	195.080126			
Rb(Average)	85.476363				
Re(Average)	186.206722			
Rh(Average)	102.905503			
Ru(Average)	101.069741			
S	(Average)	32.064389				
Sb(Average)	121.757994			
Sc(Average)	44.955914				
Se(Average)	78.993275				
Si(Average)	28.085510				
Sm(Average)	150.360250			
Sn(Average)	118.548922			
Sr(Average)	87.616658				
Ta(Average)	180.951513			
Tb(Average)	158.925350			
Te(Average)	127.478108			
Th(Average)	232.038054			
Ti(Average)	47.878426				
Tl(Average)	204.383398			
Tm(Average)	168.934225			
U	(Average)	238.019374			
V	(Average)	50.941471				
W	(Average)	183.848908			
Xe(Average)	131.293079			
Y	(Average)	88.905856				
YbAverage)	173.034201			
ZnAverage)	65.396364				
ZrAverage)	91.221349				



******************* Calculate Average masses from first table saved in "atom.txt"   *****************************************   
File file;
file.Open("atom.txt", "r");

File fileOut;
fileOut.Open("average.txt", "w");

char szBuffer[1024];
char *psz;
char *pszAtom;
double dMassMono;
double dPercent;
double	dMassAverage;

while(fgets(szBuffer, 1024, file)){

	pszAtom = strtok(szBuffer, " \t\r\n");
	psz			= pszAtom;
	dMassAverage = 0;

	while(psz){
		psz = strtok(NULL, " \t\r\n");
		sscanf(psz, "%lf", &dMassMono);

		psz = strtok(NULL, " \t\r\n");
		sscanf(psz, "%lf", &dPercent);

		dMassAverage += dMassMono * dPercent;
		
		psz = strtok(NULL, " \t\r\n");				
	}

	pszAtom = strtok(pszAtom, "(");
	fprintf(fileOut, "%s\t%f\n", pszAtom, dMassAverage / 100);
}


file.Close();
fileOut.Close();


*****************************************   Generate code for ReadAtom() *****************************************
File file;
		file.Open("atom.txt", "r");

		File fileOut;
		fileOut.Open("code.txt", "w");

		char szBuffer[1024];
		char *psz;
		char *pszAtom;
		double	dMassAverage, dMassMono, dPercent;
		int			iIsotop;
		int			iPercentMaxIdx;
		double  dPercentMax;
		char		cPreviousFirstLetter = '\0';

		DynamicArray<int>			aIsotop;
		DynamicArray<double>	aMass;


		while(fgets(szBuffer, 1024, file)){

			aIsotop.Reset();
			aMass.Reset();

			dMassAverage	= 0;
			dPercentMax		= 0;


			pszAtom = strtok(szBuffer, " \t\r\n");
			psz			= pszAtom;

			while(psz){

				sscanf(strchr(psz, '(')+1, "%d", &iIsotop);
				aIsotop.Add(iIsotop);

				psz = strtok(NULL, " \t\r\n");
				sscanf(psz, "%lf", &dMassMono);
				aMass.Add(dMassMono);

				psz = strtok(NULL, " \t\r\n");
				sscanf(psz, "%lf", &dPercent);

				if(dPercent > dPercentMax){
					dPercentMax = dPercent;
					iPercentMaxIdx = aIsotop.GetNbElt()-1;
				}

			

				dMassAverage += dMassMono * dPercent;
				
				psz = strtok(NULL, " \t\r\n");				
			}

			dMassAverage /= 100;
			pszAtom = strtok(pszAtom, "(");


			if(cPreviousFirstLetter != pszAtom[0]){
				if(cPreviousFirstLetter != '\0')
					fprintf(fileOut, "\tbreak;\n\n");

				cPreviousFirstLetter = pszAtom[0];
				fprintf(fileOut, "case '%c':\n", pszAtom[0]);
			}

			if(pszAtom[1] == '\0')
				fprintf(fileOut, "\tif(szAtom[1] == '\\0'){\n");
			else
				fprintf(fileOut, "\tif(szAtom[1] == '%c'){\n", pszAtom[1]);

			fprintf(fileOut, "\t\tif(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? %f : %f;\n", aMass[iPercentMaxIdx], dMassAverage);

			for(int i=0; i<aIsotop.GetNbElt(); i++)
				fprintf(fileOut, "\t\tif(iIsotop == %d) return %f;\n", aIsotop[i], aMass[i]);

			fprintf(fileOut, "\t}\n");
			
		}


		file.Close();
		fileOut.Close();
		
*/


/******************************************************

		Constructor

*******************************************************/
AtomicMass::AtomicMass(TE_MassMode eMode) :
	m_eMode(eMode)
{
	m_dElectron	= 0.000545;
}

/******************************************************

		Destructor

*******************************************************/
AtomicMass::~AtomicMass(void)
{
}


/***********************************************************

	GetMass

***********************************************************/

double AtomicMass::GetMass(const char *psz)
{
	assert(psz);

	double		dMass	= 0;
	const char	*pszCpy = psz;

	try{

		double d;
		int i;

		while( (*psz != '\0') && (*psz != '(') ){
			d = ReadAtom(&psz);
			i = ReadNumber(&psz);
			dMass += d * i;
		}
		
		dMass += ReadCharge(&psz);


	}catch(Error *pError){
		pError->Stack("AtomicMass::GetMass", "Formula \"%s\" is not valid", pszCpy);
	}

	return dMass;
}




/***********************************************************

	ReadNumber

***********************************************************/

int AtomicMass::ReadNumber(const char **ppsz)
{
	int i;
	
	if(sscanf(*ppsz, "%d", &i) != 1)
		return 1;		//par default pas de nombre = 1 fois
	
	//avance le pointeur de ce qui a ete lu
	if(**ppsz == '-')
		(*ppsz)++;

	while( (**ppsz >= '0') && (**ppsz <= '9') )
		(*ppsz)++;

	return i;
}


/***********************************************************

	ReadCharge

***********************************************************/

double AtomicMass::ReadCharge(const char **ppsz)
{
	//pas de charge
	if(**ppsz == '\0')
		return 0;

	if(**ppsz != '(')
		ThrowError("AtomicMass::ReadCharge", "Ion charge should be between parenthesis");

	(*ppsz)++;

	int iNbCharge = ReadNumber(ppsz);

	switch(**ppsz){
		case '+':
			iNbCharge *= -1;
			break;

		case '-':
			break;

		default :
			ThrowError("AtomicMass::ReadCharge", "Ion charge last character must be '+' or '-'");
	}

	(*ppsz)++;
	
	if(**ppsz != ')')
		ThrowError("AtomicMass::ReadCharge", "Ion charge should be between parenthesis");

	(*ppsz)++;

	if(**ppsz)
		ThrowError("AtomicMass::ReadCharge", "Ion charge should end the formula");

	return m_dElectron * iNbCharge;
}

/***********************************************************

   ReadIsotop

 ***********************************************************/
int	AtomicMass::ReadIsotop(const char **ppsz)
{
	if(**ppsz != '[')
		return 0;

	if(m_eMode == MASS_AVERAGE)
		ThrowError("AtomicMass::ReadIsotop", "Isotop informations and Average mass are incompatible");

	(*ppsz)++;

	int iIsotop;
	if(sscanf(*ppsz, "%d", &iIsotop) != 1)
		ThrowError("AtomicMass::ReadIsotop", "Isotop bad format");

	//avance le pointeur de ce qui a ete lu
	while( (**ppsz >= '0') && (**ppsz <= '9') )
		(*ppsz)++;

	if(**ppsz != ']')
		ThrowError("AtomicMass::ReadIsotop", "Isotop should be between braket");;

	(*ppsz)++;

	return iIsotop;
}



/***********************************************************

   ReadAtom

***********************************************************/
double AtomicMass::ReadAtom(const char **ppsz)
{	
	if( (**ppsz < 'A') || (**ppsz > 'Z') )
		ThrowError("AtomicMass::ReadAtom", "First letter of atom should be upercase");

	char szAtom[3];
	szAtom[0] = **ppsz;
	(*ppsz)++;


	if( (**ppsz >= 'a') && (**ppsz <= 'z') ){
		szAtom[1] = **ppsz;
		szAtom[2] = '\0';
		(*ppsz)++;
	}else{
		szAtom[1] = '\0';
	}


	//Isotop
	int iIsotop = ReadIsotop(ppsz);


	switch(szAtom[0]){

		case 'A':
			if(szAtom[1] == 'g'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 106.905095 : 107.868131;
				if(iIsotop == 107) return 106.905095;
				if(iIsotop == 109) return 108.904754;
			}
			if(szAtom[1] == 'l'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 26.981541 : 26.981541;
				if(iIsotop == 27) return 26.981541;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 39.962383 : 39.948740;
				if(iIsotop == 36) return 35.967546;
				if(iIsotop == 38) return 37.962732;
				if(iIsotop == 40) return 39.962383;
			}
			if(szAtom[1] == 's'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 74.921596 : 74.921596;
				if(iIsotop == 75) return 74.921596;
			}
			if(szAtom[1] == 'u'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 196.966560 : 196.966560;
				if(iIsotop == 197) return 196.966560;
			}
			break;

		case 'B':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 11.009305 : 10.812024;
				if(iIsotop == 10) return 10.012938;
				if(iIsotop == 11) return 11.009305;
			}
			if(szAtom[1] == 'a'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 137.905236 : 137.326659;
				if(iIsotop == 130) return 129.906277;
				if(iIsotop == 132) return 131.905042;
				if(iIsotop == 134) return 133.904490;
				if(iIsotop == 135) return 134.905668;
				if(iIsotop == 136) return 135.904556;
				if(iIsotop == 137) return 136.905816;
				if(iIsotop == 138) return 137.905236;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 9.012183 : 9.012183;
				if(iIsotop == 9) return 9.012183;
			}
			if(szAtom[1] == 'i'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 208.980388 : 208.980388;
				if(iIsotop == 209) return 208.980388;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 78.918336 : 79.903527;
				if(iIsotop == 79) return 78.918336;
				if(iIsotop == 81) return 80.916290;
			}
			break;

		case 'C':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 12.000000 : 12.011037;
				if(iIsotop == 12) return 12.000000;
				if(iIsotop == 13) return 13.003355;
			}
			if(szAtom[1] == 'a'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 39.962591 : 40.086465;
				if(iIsotop == 40) return 39.962591;
				if(iIsotop == 42) return 41.958622;
				if(iIsotop == 43) return 42.958770;
				if(iIsotop == 44) return 43.955485;
				if(iIsotop == 46) return 45.953689;
				if(iIsotop == 48) return 47.952532;
			}
			if(szAtom[1] == 'd'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 113.903361 : 111.451208;
				if(iIsotop == 106) return 105.906461;
				if(iIsotop == 110) return 109.903007;
				if(iIsotop == 111) return 110.904182;
				if(iIsotop == 112) return 111.902761;
				if(iIsotop == 113) return 112.904401;
				if(iIsotop == 114) return 113.903361;
				if(iIsotop == 116) return 115.904758;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 139.905442 : 140.114868;
				if(iIsotop == 136) return 135.907140;
				if(iIsotop == 138) return 137.905996;
				if(iIsotop == 140) return 139.905442;
				if(iIsotop == 142) return 141.909249;
			}
			if(szAtom[1] == 'l'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 34.968853 : 35.452738;
				if(iIsotop == 35) return 34.968853;
				if(iIsotop == 37) return 36.965903;
			}
			if(szAtom[1] == 'o'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 58.933198 : 58.933198;
				if(iIsotop == 59) return 58.933198;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 51.940510 : 51.995926;
				if(iIsotop == 50) return 49.946046;
				if(iIsotop == 52) return 51.940510;
				if(iIsotop == 53) return 52.940651;
				if(iIsotop == 54) return 53.938882;
			}
			if(szAtom[1] == 's'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 132.905433 : 132.905433;
				if(iIsotop == 133) return 132.905433;
			}
			if(szAtom[1] == 'u'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 62.929599 : 63.545642;
				if(iIsotop == 63) return 62.929599;
				if(iIsotop == 65) return 64.927792;
			}
			break;

		case 'D':
			if(szAtom[1] == 'y'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 163.929183 : 162.497541;
				if(iIsotop == 156) return 155.924287;
				if(iIsotop == 158) return 157.924412;
				if(iIsotop == 160) return 159.925203;
				if(iIsotop == 161) return 160.926939;
				if(iIsotop == 162) return 161.926805;
				if(iIsotop == 163) return 162.928737;
				if(iIsotop == 164) return 163.929183;
			}
			break;

		case 'E':
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 165.930305 : 167.255716;
				if(iIsotop == 162) return 161.928787;
				if(iIsotop == 164) return 163.929211;
				if(iIsotop == 166) return 165.930305;
				if(iIsotop == 167) return 166.932061;
				if(iIsotop == 168) return 167.932383;
				if(iIsotop == 170) return 169.935476;
			}
			if(szAtom[1] == 'u'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 152.921243 : 151.964582;
				if(iIsotop == 151) return 150.919860;
				if(iIsotop == 153) return 152.921243;
			}
			break;

		case 'F':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 18.998403 : 18.998403;
				if(iIsotop == 19) return 18.998403;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 55.934939 : 55.846815;
				if(iIsotop == 54) return 53.939612;
				if(iIsotop == 56) return 55.934939;
				if(iIsotop == 57) return 56.935396;
				if(iIsotop == 58) return 57.933278;
			}
			break;

		case 'G':
			if(szAtom[1] == 'a'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 68.925581 : 69.723230;
				if(iIsotop == 69) return 68.925581;
				if(iIsotop == 71) return 70.924701;
			}
			if(szAtom[1] == 'd'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 157.924111 : 157.237329;
				if(iIsotop == 152) return 151.919803;
				if(iIsotop == 154) return 153.920876;
				if(iIsotop == 155) return 154.822629;
				if(iIsotop == 156) return 155.922130;
				if(iIsotop == 157) return 156.923967;
				if(iIsotop == 158) return 157.924111;
				if(iIsotop == 160) return 159.927061;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 73.921179 : 72.632251;
				if(iIsotop == 70) return 69.924250;
				if(iIsotop == 72) return 71.922080;
				if(iIsotop == 73) return 72.923464;
				if(iIsotop == 74) return 73.921179;
				if(iIsotop == 76) return 75.921403;
			}
			break;

		case 'H':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 1.007825 : 1.008026;
				if(iIsotop == 1) return 1.007825;
				if(iIsotop == 2) return 2.014102;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 4.002603 : 4.002606;
				if(iIsotop == 3) return 3.016029;
				if(iIsotop == 4) return 4.002603;
			}
			if(szAtom[1] == 'f'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 179.946561 : 178.489791;
				if(iIsotop == 174) return 173.940065;
				if(iIsotop == 176) return 175.941420;
				if(iIsotop == 177) return 176.943233;
				if(iIsotop == 178) return 177.943710;
				if(iIsotop == 179) return 178.945827;
				if(iIsotop == 180) return 179.946561;
			}
			if(szAtom[1] == 'g'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 201.970632 : 200.588446;
				if(iIsotop == 196) return 195.965812;
				if(iIsotop == 198) return 197.966760;
				if(iIsotop == 199) return 198.968269;
				if(iIsotop == 200) return 199.968316;
				if(iIsotop == 201) return 200.970293;
				if(iIsotop == 202) return 201.970632;
				if(iIsotop == 204) return 203.973481;
			}
			if(szAtom[1] == 'o'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 164.930332 : 164.930332;
				if(iIsotop == 165) return 164.930332;
			}
			break;

		case 'I':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 126.904477 : 126.904477;
				if(iIsotop == 127) return 126.904477;
			}
			if(szAtom[1] == 'n'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 114.903875 : 114.817883;
				if(iIsotop == 113) return 112.904056;
				if(iIsotop == 115) return 114.903875;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 192.962942 : 192.216070;
				if(iIsotop == 191) return 190.960603;
				if(iIsotop == 193) return 192.962942;
			}
			break;

		case 'K':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 38.963708 : 39.075702;
				if(iIsotop == 39) return 38.963708;
				if(iIsotop == 40) return 39.963999;
				if(iIsotop == 41) return 40.961825;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 83.911506 : 83.800024;
				if(iIsotop == 78) return 77.920397;
				if(iIsotop == 80) return 79.916375;
				if(iIsotop == 82) return 81.913483;
				if(iIsotop == 83) return 82.914134;
				if(iIsotop == 84) return 83.911506;
				if(iIsotop == 86) return 85.910614;
			}
			break;

		case 'L':
			if(szAtom[1] == 'a'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 138.906355 : 138.905456;
				if(iIsotop == 138) return 137.907114;
				if(iIsotop == 139) return 138.906355;
			}
			if(szAtom[1] == 'i'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 7.016005 : 6.941740;
				if(iIsotop == 6) return 6.015123;
				if(iIsotop == 7) return 7.016005;
			}
			if(szAtom[1] == 'u'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 174.940785 : 174.966835;
				if(iIsotop == 175) return 174.940785;
				if(iIsotop == 176) return 175.942694;
			}
			break;

		case 'M':
			if(szAtom[1] == 'g'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 23.985045 : 24.306852;
				if(iIsotop == 24) return 23.985045;
				if(iIsotop == 25) return 24.985839;
				if(iIsotop == 26) return 25.982595;
			}
			if(szAtom[1] == 'n'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 54.938046 : 54.938046;
				if(iIsotop == 55) return 54.938046;
			}
			if(szAtom[1] == 'o'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 97.905405 : 95.931289;
				if(iIsotop == 92) return 91.906809;
				if(iIsotop == 94) return 93.905086;
				if(iIsotop == 95) return 94.905838;
				if(iIsotop == 96) return 95.904676;
				if(iIsotop == 97) return 96.906018;
				if(iIsotop == 98) return 97.905405;
				if(iIsotop == 100) return 99.907473;
			}
			break;

		case 'N':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 14.003074 : 14.006763;
				if(iIsotop == 14) return 14.003074;
				if(iIsotop == 15) return 15.000109;
			}
			if(szAtom[1] == 'a'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 22.989770 : 22.989770;
				if(iIsotop == 23) return 22.989770;
			}
			if(szAtom[1] == 'b'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 92.906378 : 92.906378;
				if(iIsotop == 93) return 92.906378;
			}
			if(szAtom[1] == 'd'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 141.907731 : 144.242350;
				if(iIsotop == 142) return 141.907731;
				if(iIsotop == 143) return 142.909823;
				if(iIsotop == 144) return 143.910096;
				if(iIsotop == 145) return 144.912582;
				if(iIsotop == 146) return 145.913126;
				if(iIsotop == 148) return 147.916901;
				if(iIsotop == 150) return 149.920900;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 19.992439 : 20.190941;
				if(iIsotop == 20) return 19.992439;
				if(iIsotop == 21) return 20.993845;
				if(iIsotop == 22) return 21.991384;
			}
			if(szAtom[1] == 'i'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 57.935347 : 58.687890;
				if(iIsotop == 58) return 57.935347;
				if(iIsotop == 60) return 59.930789;
				if(iIsotop == 61) return 60.931059;
				if(iIsotop == 62) return 61.928346;
				if(iIsotop == 64) return 63.927968;
			}
			break;

		case 'O':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 15.994915 : 15.998985;
				if(iIsotop == 16) return 15.994915;
				if(iIsotop == 17) return 16.999131;
				if(iIsotop == 18) return 17.999159;
			}
			if(szAtom[1] == 's'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 191.961487 : 190.239787;
				if(iIsotop == 184) return 183.952514;
				if(iIsotop == 186) return 185.953852;
				if(iIsotop == 187) return 186.955762;
				if(iIsotop == 188) return 187.955850;
				if(iIsotop == 189) return 188.958156;
				if(iIsotop == 190) return 189.958455;
				if(iIsotop == 192) return 191.961487;
			}
			break;

		case 'P':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 30.973763 : 30.973763;
				if(iIsotop == 31) return 30.973763;
			}
			if(szAtom[1] == 'b'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 207.976641 : 207.216897;
				if(iIsotop == 204) return 203.973037;
				if(iIsotop == 206) return 205.974455;
				if(iIsotop == 207) return 206.975885;
				if(iIsotop == 208) return 207.976641;
			}
			if(szAtom[1] == 'd'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 105.903475 : 106.415325;
				if(iIsotop == 102) return 101.905609;
				if(iIsotop == 104) return 103.904026;
				if(iIsotop == 105) return 104.905075;
				if(iIsotop == 106) return 105.903475;
				if(iIsotop == 108) return 107.903894;
				if(iIsotop == 110) return 109.905169;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 140.907657 : 140.907657;
				if(iIsotop == 141) return 140.907657;
			}
			if(szAtom[1] == 't'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 194.964785 : 195.080126;
				if(iIsotop == 190) return 189.959937;
				if(iIsotop == 192) return 191.961049;
				if(iIsotop == 194) return 193.962679;
				if(iIsotop == 195) return 194.964785;
				if(iIsotop == 196) return 195.964947;
				if(iIsotop == 198) return 197.967879;
			}
			break;

		case 'R':
			if(szAtom[1] == 'b'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 84.911800 : 85.476363;
				if(iIsotop == 85) return 84.911800;
				if(iIsotop == 87) return 86.909184;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 186.955765 : 186.206722;
				if(iIsotop == 185) return 184.952977;
				if(iIsotop == 187) return 186.955765;
			}
			if(szAtom[1] == 'h'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 102.905503 : 102.905503;
				if(iIsotop == 103) return 102.905503;
			}
			if(szAtom[1] == 'u'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 101.904348 : 101.069741;
				if(iIsotop == 96) return 95.907596;
				if(iIsotop == 98) return 97.905287;
				if(iIsotop == 99) return 98.905937;
				if(iIsotop == 100) return 99.904218;
				if(iIsotop == 101) return 100.905581;
				if(iIsotop == 102) return 101.904348;
				if(iIsotop == 104) return 103.905422;
			}
			break;

		case 'S':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 31.972072 : 32.064389;
				if(iIsotop == 32) return 31.972072;
				if(iIsotop == 33) return 32.971459;
				if(iIsotop == 34) return 33.967868;
				if(iIsotop == 36) return 35.967079;
			}
			if(szAtom[1] == 'b'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 120.903824 : 121.757994;
				if(iIsotop == 121) return 120.903824;
				if(iIsotop == 123) return 122.904222;
			}
			if(szAtom[1] == 'c'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 44.955914 : 44.955914;
				if(iIsotop == 45) return 44.955914;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 79.916521 : 78.993275;
				if(iIsotop == 74) return 73.922477;
				if(iIsotop == 76) return 75.919207;
				if(iIsotop == 77) return 76.919908;
				if(iIsotop == 78) return 77.917304;
				if(iIsotop == 80) return 79.916521;
				if(iIsotop == 82) return 81.916709;
			}
			if(szAtom[1] == 'i'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 27.976928 : 28.085510;
				if(iIsotop == 28) return 27.976928;
				if(iIsotop == 29) return 28.976496;
				if(iIsotop == 30) return 29.973772;
			}
			if(szAtom[1] == 'm'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 151.919741 : 150.360250;
				if(iIsotop == 144) return 143.912009;
				if(iIsotop == 147) return 146.914907;
				if(iIsotop == 148) return 147.914832;
				if(iIsotop == 149) return 148.917193;
				if(iIsotop == 150) return 149.917285;
				if(iIsotop == 152) return 151.919741;
				if(iIsotop == 154) return 153.922218;
			}
			if(szAtom[1] == 'n'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 119.902199 : 118.548922;
				if(iIsotop == 112) return 111.904826;
				if(iIsotop == 114) return 113.902784;
				if(iIsotop == 115) return 114.903348;
				if(iIsotop == 116) return 115.901744;
				if(iIsotop == 117) return 116.902954;
				if(iIsotop == 118) return 117.901607;
				if(iIsotop == 119) return 118.903310;
				if(iIsotop == 120) return 119.902199;
				if(iIsotop == 122) return 121.903440;
				if(iIsotop == 124) return 123.905271;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 87.905625 : 87.616658;
				if(iIsotop == 84) return 83.913428;
				if(iIsotop == 86) return 85.909273;
				if(iIsotop == 87) return 86.908902;
				if(iIsotop == 88) return 87.905625;
			}
			break;

		case 'T':
			if(szAtom[1] == 'a'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 180.948014 : 180.951513;
				if(iIsotop == 180) return 179.947489;
				if(iIsotop == 181) return 180.948014;
			}
			if(szAtom[1] == 'b'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 158.925350 : 158.925350;
				if(iIsotop == 159) return 158.925350;
			}
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 129.906229 : 127.478108;
				if(iIsotop == 122) return 121.903055;
				if(iIsotop == 123) return 122.904278;
				if(iIsotop == 124) return 123.902825;
				if(iIsotop == 125) return 124.904435;
				if(iIsotop == 126) return 125.903310;
				if(iIsotop == 128) return 127.904464;
				if(iIsotop == 130) return 129.906229;
			}
			if(szAtom[1] == 'h'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 232.038054 : 232.038054;
				if(iIsotop == 232) return 232.038054;
			}
			if(szAtom[1] == 'i'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 47.947947 : 47.878426;
				if(iIsotop == 46) return 45.952633;
				if(iIsotop == 47) return 46.951765;
				if(iIsotop == 48) return 47.947947;
				if(iIsotop == 49) return 48.947871;
				if(iIsotop == 50) return 49.944786;
			}
			if(szAtom[1] == 'l'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 204.974410 : 204.383398;
				if(iIsotop == 203) return 202.972336;
				if(iIsotop == 205) return 204.974410;
			}
			if(szAtom[1] == 'm'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 168.934225 : 168.934225;
				if(iIsotop == 169) return 168.934225;
			}
			break;

		case 'U':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 238.050786 : 238.019374;
				if(iIsotop == 234) return 234.040947;
				if(iIsotop == 235) return 235.043925;
				if(iIsotop == 238) return 238.050786;
			}
			break;

		case 'V':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 50.943963 : 50.941471;
				if(iIsotop == 50) return 49.947161;
				if(iIsotop == 51) return 50.943963;
			}
			break;

		case 'W':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 183.950953 : 183.848908;
				if(iIsotop == 180) return 179.946727;
				if(iIsotop == 182) return 181.948225;
				if(iIsotop == 183) return 182.950245;
				if(iIsotop == 184) return 183.950953;
				if(iIsotop == 186) return 185.954377;
			}
			break;

		case 'X':
			if(szAtom[1] == 'e'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 131.904148 : 131.293079;
				if(iIsotop == 124) return 123.905894;
				if(iIsotop == 126) return 125.904281;
				if(iIsotop == 128) return 127.903531;
				if(iIsotop == 129) return 128.904780;
				if(iIsotop == 130) return 129.903510;
				if(iIsotop == 131) return 130.905076;
				if(iIsotop == 132) return 131.904148;
				if(iIsotop == 134) return 133.905395;
				if(iIsotop == 136) return 135.907219;
			}
			break;

		case 'Y':
			if(szAtom[1] == '\0'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 88.905856 : 88.905856;
				if(iIsotop == 89) return 88.905856;
			}
			if(szAtom[1] == 'b'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 173.938873 : 173.034201;
				if(iIsotop == 168) return 167.933908;
				if(iIsotop == 170) return 169.934774;
				if(iIsotop == 171) return 170.936338;
				if(iIsotop == 172) return 171.936393;
				if(iIsotop == 173) return 172.938222;
				if(iIsotop == 174) return 173.938873;
				if(iIsotop == 176) return 175.942576;
			}
			break;

		case 'Z':
			if(szAtom[1] == 'n'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 63.929145 : 65.396364;
				if(iIsotop == 64) return 63.929145;
				if(iIsotop == 66) return 65.926035;
				if(iIsotop == 67) return 66.927129;
				if(iIsotop == 68) return 67.924846;
				if(iIsotop == 70) return 69.925325;
			}
			if(szAtom[1] == 'r'){
				if(iIsotop == 0) return (m_eMode==MASS_MONOISOTOPIC)? 89.904708 : 91.221349;
				if(iIsotop == 90) return 89.904708;
				if(iIsotop == 91) return 90.905644;
				if(iIsotop == 92) return 91.905039;
				if(iIsotop == 94) return 93.906319;
				if(iIsotop == 96) return 95.908272;
			}
	}
	

	if(iIsotop)
		ThrowError("AtomicMass::ReadAtom", "Unknown atom %s[%d]", szAtom, iIsotop);
	else
		ThrowError("AtomicMass::ReadAtom", "Unknown atom %s", szAtom);

	return 0;
}


