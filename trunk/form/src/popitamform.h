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

#ifndef __POPITAMFORM_H__
#define __POPITAMFORM_H__



/***********************************************************

Class PopitamForm

***********************************************************/

class PopitamForm
{

private :

    //    Popitam form elements
    FileField m_dataFile;
    AreaField m_dataCopy;
    ItemList  m_formatList;
    
    ItemList  m_database;
//    ItemList  m_taxonomy;
    Taxonomy m_taxonomyObj;
    AreaField m_acList;
    
    ItemList  m_fixedModifs;
    ItemList  m_variableModifs;
    
    ItemList  m_instrument;
    DoubleField m_fragmentError1;

    ItemList m_enzyme;
    ItemList m_missedcleav;

    FileField m_scoreFile;
    AreaField m_scoreCopy;

    RadioGroup m_popitamRunMode;
    DoubleField m_parentMError;

    ItemList m_gapMax;
    DoubleField m_maxAddPM;
    DoubleField m_maxLossPM;
    DoubleField m_maxAddModif;
    DoubleField m_maxLossModif;

    IntField m_nbDisplay;
    Display         m_display;

    const char *pszFormName;

    static const char *pszRequest;
    static const char *pszCgi;

    char** unusedAC;
    int unusedACnb;

public :

    PopitamForm(const char* pszName="myform");
    ~PopitamForm(void);


    void SetDefault(void);
    void SetExample0MODDta(void);
    void SetExample0MODMgf(void);
    void SetExample1MODMgf(void);
    void SetExample2MODMgf(void);
    int ReadCgi(void);
    void ReadFile(TagFile *pFile);

    void Load(void);
    void CheckCgi(void);
    void CheckScoring(char* scoringFunction);
    void CheckMGF(char* data);
    void CheckDTA(char* data);
    void CheckMZDATA(char* data);

    void DoDisplay(File &file, const char *pszMsg=NULL);
    void WriteForm(File &file, const char *pszMsg=NULL);
    void WriteLogo(File &file);
    void WriteStyle(File &file);
    void WriteOutputParam(File &file, bool isStdout);
    void WriteExpasyFile(File &file, File &expasyFile);
    void WriteResubmit(File &file, bool isByMail);
    void WriteFile(TagFile *pFile);

    bool IsMailUsed(void);
    const char* GetMail(void);
    const char* GetDataFile(void);
    const char* GetDataFileName(void);
    const char* GetDataCopy(void);
    void SetDataCopy(const char*);
    const char* GetDataFormat(void);
    const char* GetDatabase(void);
    const char* GetTaxonomyStr(void);
    int GetTaxonomyId(void);
    const char* GetACFilter(void);
    const char* GetInstrument(void);
    int GetMutMode(void);
    const char* GetScoreFile(void);
    const char* GetScoreFileName(void);
    const char* GetScoreCopy(void);
    void SetScoreCopy(const char*);
    const char* GetRunMode(void);
//    void SetRunMode(int);
    int GetMissedCleavage(void);
    double GetFragmentError1(void);
    const char* GetFormattedFragmentError1(void);
    double GetFragmentError2(void);
    int GetDisplayNb(void);
    int GetGapMax(void);
    void SetGapMax(int);
    double GetMaxAddPM(void);
    double GetMaxLossPM(void);
    double GetMaxAddModif(void);
    double GetMaxLossModif(void);
    inline void SetUnusedAC(char** ptr, int nb) { unusedAC = ptr;unusedACnb= nb; } 


private :

    void WriteFormHead(File &file);
    void WriteFormCheckLimit(File &file);
    void WriteFormParamMode(File &file);
    void WriteFormScoringMode(File &file);
    void WriteFormRunningMode(File &file);
    void WriteFormOutputMode(File &file);
    

};


#endif
