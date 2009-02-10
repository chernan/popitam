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

#ifndef __POPIRESULT_H__
#define __POPIRESULT_H__


/***********************************************************

Class Scenario

***********************************************************/

class Scenario {

public:
    double dParentMass;
    int iId;
    double dScore;
    double dDeltaS;
    double dPValue;
    double dMass;
    char** sAccess;
    char** sIdentifier;
    char* sInterpretation;
    char* sInterpretation2;
    int iNbAcId;
    
    Scenario(void);
    ~Scenario(void);

    void WriteHTML(File &toFile, bool isStdout);
    void WriteSimpleHTML(File &toFile, bool isStdout);
};


/***********************************************************

Class PopiResult

***********************************************************/

class PopiResult
{

private :

    int iSpectrumId;
    char* sComment;
    int iPeakNb;
    int iPeakNbAfterProc;
    double dParentMassRaw;
    double dParentMass;
    int iParentCharge;
    int iNodeNb;
    char* sEdgeNb;
    
    int iProtProcessed;
    int iProtInRange;
    int iPepAfterDigestion;
    int iPepPresented;
    int iPepWithSubseq;
    int iSubseqNb;
    int iPepWithScenario;
    int iScenarioNb;
    int iSampleStatSize;
    double dPTime;
    
    int iMaxResult;
    Scenario* pScenarii;
    int unusedACnb;
    char** unusedAC;

public :

    PopiResult(void);
    ~PopiResult(void);

    bool ParseTXT(TagFile &file);
    void WriteHTML(File &toFile, bool isStdout);
    void WriteFirstScenario(File& toFile, bool isStdout);
    
    //Accessors
    inline void SetSpectrumId(int id) { iSpectrumId = id; }
    inline void SetComment(char* comm) { sComment = comm; }
    inline void SetPeakNb(int pNb) { iPeakNb = pNb; }
    inline void SetPeakNbAfterProc(int pNbAP) { iPeakNbAfterProc = pNbAP; }
    inline void SetParentMassRaw(double pmass) { dParentMassRaw = pmass; }
    inline void SetParentMass(double pmass) { dParentMass = pmass; }
    inline void SetParentCharge(int nb) { iParentCharge = nb; }
    inline void SetNodeNb(int nodeNb) { iNodeNb = nodeNb; }
    inline void SetEdgeNb(char* edgeNb) { sEdgeNb = edgeNb; }
    
    inline void SetProteinProcessedNb(int nb) { iProtProcessed = nb; }
    inline void SetProtInRangeNb(int nb) { iProtInRange = nb; }
    inline void SetPepAfterDigestion(int nb) { iPepAfterDigestion = nb; }
    inline void SetPepPresented(int nb) { iPepPresented = nb; }
    inline void SetPepWithSubseq(int nb) { iPepWithSubseq = nb; }
    inline void SetSubseqNb(int nb) { iSubseqNb = nb; }
    inline void SetPepWithScenario(int nb) { iPepWithScenario = nb; }
    inline void SetScenarioNb(int nb) { iScenarioNb = nb; }
    inline void SetScenarii(Scenario* psc) { pScenarii = psc; }
    inline void SetSampleStatSize(int nb) { iSampleStatSize = nb; }
    inline void SetPTime(double time) { dPTime = time; }

    inline void SetMaxResult(int nb) { iMaxResult = nb; }
    inline int GetMaxResult(void) { return iMaxResult; }

    inline char** GetUnusedAC(void) { return unusedAC; }
    inline int GetUnusedACnb(void) { return unusedACnb; }
};

/***********************************************************

Class ResSummary

***********************************************************/

class ResSummary {

public:
    int iSpectraNb;
    int iMaxSpectraNb;
    int iAllowedGap;
//    int iRunMode;
    //Instrument
    //Min covbin
    //Wanted number of coverage
    //Minimal ratio for tag lenght
    //Minimal coverage for a valuable arrangement
    //
    //Graph parameters
    // - edges_type :
    // - peptide error :
    // - fragment error1 :
    // - fragment error2 :
    //
    //DB
    
    /*Si open search, de nouvelles lignes sont affichées
Modification/Mutation parameters              
max positive delta for all mod/mut           :   0.00
max negative delta for all mod/mut           :   0.00
max delta for a positive mod/mut             :   0.00
max delta for a negative mod/mut             :   0.00
*/
    PopiResult* pResults;
    char** unusedAC;

    ResSummary(void);
    ~ResSummary(void);

    bool ParseTXT(TagFile &file, int maxResNb);
    void WriteHTML(File &toFile, bool isStdout);

    inline char** GetUnusedAC() {
		if(pResults) {
		    return pResults[0].GetUnusedAC();
		}
		return NULL;
    }
    inline int GetUnusedACnb() {
        if(pResults) {
            return pResults[0].GetUnusedACnb();
        }
        return 0;
    } 
};


#endif
