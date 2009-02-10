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

#define USE_POPIRESULT
#include "use.h"

const int BUFF_LENGTH = 512;

/******************************************************
******************************************************

        Constructor
        
*******************************************************/
ResSummary::ResSummary(void) :
    iSpectraNb(0),
    iAllowedGap(0),
//    iRunMode(0),
    pResults(NULL),
    unusedAC(NULL)
{
}

/******************************************************

        Destructor

*******************************************************/
ResSummary::~ResSummary(void)
{ }

/******************************************************

        Read/Write

*******************************************************/
bool ResSummary::ParseTXT(TagFile &file, int maxResNb)
{
    char szBuffer[BUFF_LENGTH];
    char* fgetsRetVal=NULL;
    
    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //Popitam is now running; run parameters are... (... allowed gaps)
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    int iNbGapsPos = (int)strcspn(szBuffer, "012");
    iAllowedGap = atoi(szBuffer+iNbGapsPos);

    do {
        fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
    }while(fgetsRetVal!=NULL && szBuffer!=NULL && szBuffer[0]!='_');

    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    
    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //empty
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //File processed
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //Initial number of spectra
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    int ptSzBuffer = 0;
    if(strchr(szBuffer, ':')==NULL) return false;
    for( ;szBuffer[ptSzBuffer]!=':' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
    
    ptSzBuffer++;//arrive après le : (il reste des espaces mais strtoul ne s'en occupe pas)
    iSpectraNb = atoi(szBuffer+ptSzBuffer);
	iMaxSpectraNb = iSpectraNb;

/*    fgets(szBuffer, BUFF_LENGTH, file); //Instrument
    fgets(szBuffer, BUFF_LENGTH, file); //Min covbin
    fgets(szBuffer, BUFF_LENGTH, file); //Wanted number of coverage
    fgets(szBuffer, BUFF_LENGTH, file); //Minimal tag lenght
    fgets(szBuffer, BUFF_LENGTH, file); //Minimal coverage for a valuable arrangement
    fgets(szBuffer, BUFF_LENGTH, file); //
    fgets(szBuffer, BUFF_LENGTH, file); //Graph parameters
    fgets(szBuffer, BUFF_LENGTH, file); // - edges_type :
    fgets(szBuffer, BUFF_LENGTH, file); // - peptide error :
    fgets(szBuffer, BUFF_LENGTH, file); // - fragment error1 :
    fgets(szBuffer, BUFF_LENGTH, file); // - fragment error2 :
    fgets(szBuffer, BUFF_LENGTH, file); //
    
    fgets(szBuffer, BUFF_LENGTH, file); //DB
    fgets(szBuffer, BUFF_LENGTH, file); //DTB_FILE_SP 
    fgets(szBuffer, BUFF_LENGTH, file); //DTB_FILE_TR 
    fgets(szBuffer, BUFF_LENGTH, file); //AC FILTER?
*/
/*Si open search, de nouvelles lignes sont affichées
Modification/Mutation parameters              
max positive delta for all mod/mut           :   0.00
max negative delta for all mod/mut           :   0.00
max delta for a positive mod/mut             :   0.00
max delta for a negative mod/mut             :   0.00
*/

    do {
        fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
    }while(fgetsRetVal!=NULL && szBuffer!=NULL && szBuffer[0]!='_');

    if(fgetsRetVal==NULL || szBuffer==NULL) return false;

    pResults = new PopiResult[iSpectraNb];
    
    bool parseOK = true;
    int i=0;
    for( ; i<iSpectraNb && parseOK;i++){
        pResults[i].SetMaxResult( maxResNb );
        parseOK = pResults[i].ParseTXT(file);
    }
    
    if(!parseOK && i>1) {//i>1 means that at least one spectrum could be read
    	iSpectraNb = i-1;
    	return true;
    }
    return parseOK;
}

void ResSummary::WriteHTML(File &toFile, bool isStdout)
{
    fprintf(toFile, "<a name=\"upstart\"> </a>");
    if(iSpectraNb==0) {
        fprintf(toFile, "<h2>Result summary (no processed spectrum)</h2><br>\n");
    }
    else if(iSpectraNb==1) {
        fprintf(toFile, "<h2>Result summary (1 processed spectrum)</h2><br>\n");
    }
    else {
        fprintf(toFile, "<h2>Result summary (%d processed spectra)</h2><br>\n", iSpectraNb);
    }

    char * server = "";
    if(!isStdout) server = path()->pszServer;
	fprintf(toFile, "<table border=\"1\" cellpadding=\"5\" bgcolor=\"#FFFFEA\"><tbody><tr>"
                "<td>#</td><td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+outputscore\" target=\"_new\">"
                "Scenario score</a></td>"
                "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+deltascore\" target=\"_new\">"
                "Delta score</a></td>"
                "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+pvalue\" target=\"_new\">"
                "p-value</a></td>"
                "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+outputmass\" target=\"_new\">"
                "Mass</a></td>"
                "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+outputdelta\" target=\"_new\">"
                "Delta mass</a></td>"
                "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+result\" target=\"_new\">"
                "Peptide / scenario (shifts)</a></td>"
                "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+idaclist\" target=\"_new\">"
                "Found in ID(AC)</a></td></tr>\n"
                , server, path()->pszRelativeServer, server, path()->pszRelativeServer
                , server, path()->pszRelativeServer, server, path()->pszRelativeServer
                , server, path()->pszRelativeServer, server, path()->pszRelativeServer
                , server, path()->pszRelativeServer);

    for(int i=0; i<iSpectraNb; i++) {
        fprintf(toFile, "<tr>");
        fprintf(toFile, "<td><a href=\"#spectrum%d\">%d</a></td>", i, i+1);
        pResults[i].WriteFirstScenario(toFile, isStdout);
        fprintf(toFile, "</tr>\n");
    }
    
    fprintf(toFile, "</tbody></table><br><br>\n");
    
    for(int i=0; i<iSpectraNb; i++) {
        pResults[i].WriteHTML(toFile, isStdout);
    }

}

/******************************************************
******************************************************

        Constructor
        
*******************************************************/
PopiResult::PopiResult(void) :
    iSpectrumId(0),
    sComment(NULL),
    iPeakNb(0),
    iPeakNbAfterProc(0),
    dParentMassRaw(0.0),
    dParentMass(0.0),
    iParentCharge(0),
    iNodeNb(0),
    sEdgeNb(NULL),
    
    iProtProcessed(0),
    iProtInRange(0),
    iPepAfterDigestion(0),
    iPepPresented(0),
    iPepWithSubseq(0),
    iSubseqNb(0),
    iPepWithScenario(0),
    iScenarioNb(0),
    iSampleStatSize(0),
    dPTime(0.0),
    
    iMaxResult(0),
    pScenarii(NULL),
    unusedACnb(0),
    unusedAC(NULL)
{ }

/******************************************************

        Destructor

*******************************************************/
PopiResult::~PopiResult(void)
{ }

/******************************************************

        WriteHTML

*******************************************************/
bool PopiResult::ParseTXT(TagFile &file)
{
    char szBuffer[BUFF_LENGTH];
    char* fgetsRetVal=NULL;
    
    do {
        fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
    }while(fgetsRetVal!=NULL && szBuffer!=NULL && szBuffer[0]=='\n');

    if(fgetsRetVal==NULL || szBuffer==NULL) return false;

    /////////////////////////////////////////////
    //En tête
    
    //Spectrum ID
    SetSpectrumId( ReadInt(szBuffer) );//SpectrumId

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //SpectrumTitle 
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetComment( ReadString(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //initPeakNb
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetPeakNb( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //peakNb after proc.
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetPeakNbAfterProc( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //ParentMass raw
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetParentMassRaw( ReadDouble(szBuffer) );
    
    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //ParentMass(M)/Charge
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetParentMass( atof( Strdup( strtok(ReadString(szBuffer), "/") ) ) );
    SetParentCharge( atoi( Strdup( strtok(NULL, "/") ) ) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //NodeNb
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetNodeNb( ReadInt(szBuffer) );
    
    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //EdgeNb (simple/double)
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetEdgeNb( ReadString(szBuffer) );
    
    do {
        fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
    }while(fgetsRetVal!=NULL && szBuffer!=NULL && szBuffer[0]!='_');

    if(fgetsRetVal==NULL || szBuffer==NULL) return false;

    /////////////////////////////////////////////
    //Resultat
    
    do {
        fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
    }while(fgetsRetVal!=NULL && szBuffer !=NULL && szBuffer[0]=='\n');

    if(fgetsRetVal==NULL || szBuffer==NULL) return false;

    //Total number of proteins processed
    SetProteinProcessedNb( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Number of protein processed that went through the protein range filter
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetProtInRangeNb( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Peptide obtained after digestion
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetPepAfterDigestion( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Candidate peptides presented for the analysis (that passed throught the PM filter)
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetPepPresented( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Candidate peptides with one or more subsequence
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetPepWithSubseq( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Cumulated number of subsequences
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetSubseqNb( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Candidate peptides with one or more valuable scenario (= that lead to an identification score)
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetPepWithScenario( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Cumulated number of valuable scenario
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetScenarioNb( ReadInt(szBuffer) );

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//empty line
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    
    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//SAMPLE SIZES for statistics: xx peptides neg
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    //go after separator ':'
    int ptSzBuff=0;
    if(strchr(szBuffer, ':')==NULL) return false;
    for( ;szBuffer[ptSzBuff]!=':' && szBuffer[ptSzBuff] !='\0'; ptSzBuff++) {  }
    ptSzBuff++;
    SetSampleStatSize( atoi(szBuffer+ptSzBuff) );
    
    do {
        fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
    }while(fgetsRetVal!=NULL && szBuffer!=NULL && szBuffer[0]=='\n');
    
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;

    //ici szBuffer contient  # |        score (deltaS; pValue)       |  mass   | access |        id       | dtbSeq / scenario (shifts)
    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);// -- | ------------------ | ------- | ------ | --------------- | --------------------
    //ces deux lignes sont présentes même quand il n'y a pas de resultats
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//première ligne supposée contenant un scenario
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    if(szBuffer[0]=='\n') { //si pas de resultat
        SetMaxResult(0);
    }
    else { //si ligne non vide, on commemce à parser
        int maxResNb = GetMaxResult();
        pScenarii = new Scenario[maxResNb];
        Scenario* pScenario;
        int ptSzBuffer, indexScenario=0;

        do {
            //new scenario
            pScenario = new Scenario;
            pScenario->dParentMass = dParentMass; //initialize its parent mass
            ptSzBuffer = 0;
            //id
            pScenario->iId = atoi(szBuffer);
            //go after separator '('
            if(strchr(szBuffer, '(')==NULL) return false;
            for( ;szBuffer[ptSzBuffer]!='(' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
            ptSzBuffer++;
            int nbAcId = atoi(szBuffer+ptSzBuffer);
            pScenario->iNbAcId = nbAcId;
            pScenario->sAccess = new char*[nbAcId];
            pScenario->sIdentifier = new char*[nbAcId];
            int currentAcId = 0;
            //go after separator '|'
            if(strchr(szBuffer+ptSzBuffer, '|')==NULL) return false;
            for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
            if(szBuffer[ptSzBuffer] !='\0') {
                ptSzBuffer++;
                //score
                pScenario->dScore = atof(szBuffer+ptSzBuffer);
                //go after separator '('
                if(strchr(szBuffer+ptSzBuffer, '(')==NULL) return false;
                for( ;szBuffer[ptSzBuffer]!='(' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                ptSzBuffer++;
                pScenario->dDeltaS = atof(szBuffer+ptSzBuffer);
                //go after separator ';'
                if(strchr(szBuffer+ptSzBuffer, ';')==NULL) return false;
                for( ;szBuffer[ptSzBuffer]!=';' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                ptSzBuffer++;
                pScenario->dPValue = atof(szBuffer+ptSzBuffer);
                //go after separator '|'
                if(strchr(szBuffer+ptSzBuffer, '|')==NULL) return false;
                for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                if(szBuffer[ptSzBuffer] !='\0') {
                    ptSzBuffer++;
                    //mass
                    pScenario->dMass = atof(szBuffer+ptSzBuffer);
                    //go after separator '|'
                    if(strchr(szBuffer+ptSzBuffer, '|')==NULL) return false;
                    for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                    if(szBuffer[ptSzBuffer] !='\0') {
                        ptSzBuffer++;
                        //access
                        pScenario->sAccess[currentAcId] = Strdup( strtok(szBuffer+ptSzBuffer, "|") );
                        //identifier
                        pScenario->sIdentifier[currentAcId] = Strdup( strtok(NULL, "|") );
                        currentAcId++;
                        //interpretation
                        pScenario->sInterpretation = Strdup( strtok(NULL, "|") );
                    }
                }
            }
            
            fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file); //next line in current scenario
            if(fgetsRetVal==NULL || szBuffer==NULL) return false;
            pScenario->sInterpretation2 = Strdup( strrchr(szBuffer, '|')+1);//strcat(pScenario->sInterpretation, Strdup( strrchr(szBuffer+ptSzBuffer, '|')

            //from the beginning, go after empty separators '|'
            if(currentAcId<nbAcId) {
                ptSzBuffer = 0;
                for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                ptSzBuffer++;
                for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                ptSzBuffer++;
                for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                ptSzBuffer++;
                //access ?
                pScenario->sAccess[currentAcId] = Strdup( strtok(szBuffer+ptSzBuffer, "|") );
                //identifier
                pScenario->sIdentifier[currentAcId] = Strdup( strtok(NULL, "|") );
                currentAcId++;
            }
            
            do {
                // supposed separation line -- | ------------------ | ------- | ------ | --------------- | --------------------
                fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
                if(fgetsRetVal==NULL || szBuffer==NULL) return false;
                //if not a separation line try to read another acces/id
                if(currentAcId<nbAcId) { //if(szBuffer[0]!='-' && szBuffer[0]!='\n') {
                    ptSzBuffer = 0;
                    //go after separator '|'
                    if(strchr(szBuffer+ptSzBuffer, '|')==NULL) return false;
                    for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                    ptSzBuffer++;
                    if(strchr(szBuffer+ptSzBuffer, '|')==NULL) return false;
                    for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                    ptSzBuffer++;
                    if(strchr(szBuffer+ptSzBuffer, '|')==NULL) return false;
                    for( ;szBuffer[ptSzBuffer]!='|' && szBuffer[ptSzBuffer] !='\0'; ptSzBuffer++) {  }
                    ptSzBuffer++;
                    //access
                    pScenario->sAccess[currentAcId] = Strdup( strtok(szBuffer+ptSzBuffer, "|") );
                    //identifier
                    pScenario->sIdentifier[currentAcId] = Strdup( strtok(NULL, "|") );
                    currentAcId++;

                }
            } while(szBuffer[0]!='-'); //until there is a separation line

            fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//store in buffer next result line
            if(fgetsRetVal==NULL || szBuffer==NULL) return false;
            
            //store current scenario and increment index
            pScenarii[indexScenario++] = *pScenario;
        }while(szBuffer[0]!='\n');
        
        if(fgetsRetVal==NULL || szBuffer==NULL) return false;
        SetMaxResult(indexScenario);
    }

    fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Processing time was:
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    SetPTime( ReadDouble(szBuffer) );
    
    if(!iSpectrumId) {
	fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//Unused AC : nb ACifnb>0
    	if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    	ptSzBuff=0;
    	if(strchr(szBuffer, ':')==NULL) return false;
    	for( ;szBuffer[ptSzBuff]!=':' && szBuffer[ptSzBuff] !='\0'; ptSzBuff++) {  }
    	ptSzBuff++;
    	
	//get unused AC number
	unusedACnb = atoi(szBuffer+ptSzBuff);
	//if is not 0 
    	if(unusedACnb) {
		//initialisation of the list of unused AC 
            	unusedAC = new char*[unusedACnb];
	    	for(int i=0; i<unusedACnb; i++) {
	    		unusedAC[i] = new char[7];
			memset(unusedAC[i], '\0', 7);
	    	}

		//copy of ACs
	    	strtok(szBuffer+ptSzBuff, " #"); //passer le chiffre que l'on vient de lire
	    	char* tempPtr = NULL;
		int lengthTempPtr=0;
	    	for(int i=0 ; i<unusedACnb; i++ ) {
			tempPtr=strtok(NULL, " #");
			if(!tempPtr) {//end of buffer when not all ACs have been read (i still < unusedACnb)
                                fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);//reading another line
                                if(fgetsRetVal==NULL || szBuffer==NULL) return false;
                                unusedAC[i] = strcpy(unusedAC[i], Strdup(strtok(szBuffer, " #")) );//resetting strtok
				continue;
			}
			lengthTempPtr = (int)strlen(tempPtr);
			if(tempPtr && lengthTempPtr==6 ) {
	    			unusedAC[i] = strncpy(unusedAC[i], tempPtr, 6);
			}
			else if(lengthTempPtr<6) { //end of line and AC not read entirely
				strncpy(unusedAC[i], tempPtr, lengthTempPtr);
				tempPtr = strtok(NULL, " #");
				if(!tempPtr) {

                                	fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
                                	if(fgetsRetVal==NULL || szBuffer==NULL) return false;
                                	strncpy(unusedAC[i]+lengthTempPtr, Strdup(strtok(szBuffer, " #")), 6-lengthTempPtr );
				}
				else {
					strncpy(unusedAC[i]+lengthTempPtr, tempPtr, 6-lengthTempPtr);
				}

			}
			else {//what if lengthTempPtr>6 ?? we only take 6...
                                unusedAC[i] = strncpy(unusedAC[i], tempPtr, 6);
                        }
 
	    	}
    	}
    	else {
	    unusedAC = NULL;
        }
    }

    do {
        fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, file);
    }while(fgetsRetVal!=NULL && szBuffer!=NULL && !(szBuffer[0]=='N' && szBuffer[1]=='E' && szBuffer[2]=='X' && szBuffer[3]=='T'));
    
    if(fgetsRetVal==NULL || szBuffer==NULL) return false;
    return true;
}

void PopiResult::WriteHTML(File &toFile, bool isStdout)
{
    char * server = "";
    if(!isStdout) server = path()->pszServer;

    fprintf(toFile, "<a name=\"spectrum%d\"></a>\n", iSpectrumId);
    fprintf(toFile, "<table class=spectrum rules=\"none\" cellpadding=\"5\" bgcolor=\"#CCCCFF\" border=\"2\"> <tbody>\n");

    /////////////////////////////////////////////
    //En tête
    
    //Spectrum ID
	fprintf(toFile, "<tr bgcolor=\"#FFFFEA\">\n<td align=\"center\"><h2>Spectrum %d</h2></td>\n</tr>\n", iSpectrumId+1);

    fprintf(toFile, "<tr>\n<td colspan=\"2\"><table><tbody>\n");
    fprintf(toFile, "<tr><td>Comment</td><td align=\"right\">%s</td></tr>\n", sComment);
	fprintf(toFile, "<tr><td>Initial peak number</td><td align=\"right\">%d</td></tr>\n", iPeakNb);
    fprintf(toFile, "<tr><td>Raw precursor mass</td><td align=\"right\">%.2f</td></tr>\n", dParentMassRaw);
    fprintf(toFile, "<tr><td>Precursor mass</td><td align=\"right\">%.2f</td></tr>\n", dParentMass);
    fprintf(toFile, "<tr><td>Charge</td><td align=\"right\">%d</td></tr>\n", iParentCharge);
    fprintf(toFile, "</tbody></table>\n</td></tr>\n");

    /////////////////////////////////////////////
    //Resultat
    fprintf(toFile, "<tr><td colspan=\"2\"><h3>Results</h3></td></tr>\n");
    
    fprintf(toFile, "<tr><td colspan=\"2\"><table border=\"0\"> <tbody>\n");

    fprintf(toFile, "<tr><td><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+processed\" target=\"_new\">", server, path()->pszRelativeServer);
    fprintf(toFile, "Total number of proteins processed</a></td><td align=\"right\">%d</td>\n", iProtProcessed);
    
    fprintf(toFile, "</tr><tr><td><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+pdigestion\" target=\"_new\">", server, path()->pszRelativeServer);
    fprintf(toFile, "Peptides obtained after digestion</a></td><td align=\"right\">%d</td>\n", iPepAfterDigestion);
    
    fprintf(toFile, "</tr><tr><td><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+panalyze\" target=\"_new\">", server, path()->pszRelativeServer);
    fprintf(toFile, "Candidate peptides presented for further analysis</a></td><td align=\"right\">%d</td>\n", iPepPresented);

    fprintf(toFile, "</tr><tr><td><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+pscenario\" target=\"_new\">", server, path()->pszRelativeServer);
    fprintf(toFile, "Candidate peptides with one or more valuable scenarios</a></td><td align=\"right\">%d</td>\n", iPepWithScenario);

    fprintf(toFile, "</tr><tr><td><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+statsize\" target=\"_new\">", server, path()->pszRelativeServer);
    fprintf(toFile, "Sample size for statistics</a></td><td align=\"right\">%d</td>\n", iSampleStatSize);
    
    fprintf(toFile, "</tr></tbody></table></td></tr>\n");
    
    if(iMaxResult==0) {
        fprintf(toFile, "<tr><td colspan=\"2\"><a href=\"%s%scgi-bin/popitam/help.pl?help.html+noresult\">No result found...</a></td></tr>\n", server, path()->pszRelativeServer);
    }
    else {
		fprintf(toFile, "<tr><td colspan=\"2\"><table border=\"1\" bgcolor=\"#FFFFEA\"><tbody>"
                        "<tr>"
                        "<td>#</td><td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+outputscore\" target=\"_new\">"
                        "Scenario score</a></td>"
                        "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+deltascore\" target=\"_new\">"
                        "Delta score</a></td>"
                        "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+pvalue\" target=\"_new\">"
                        "p-value</a></td>"
                        "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+outputmass\" target=\"_new\">"
                        "Mass</a></td>"
                        "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+outputdelta\" target=\"_new\">"
                        "Delta mass</a></td>"
                        "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+result\" target=\"_new\">"
                        "Peptide / scenario (shifts)</a></td>"
                        "<td align=\"center\"><a class=blacklink href=\"%s%scgi-bin/popitam/help.pl?help.html+idaclist\" target=\"_new\">"
                        "Found in ID(AC)</a></td></tr>\n"
                        , server, path()->pszRelativeServer, server, path()->pszRelativeServer
                        , server, path()->pszRelativeServer, server, path()->pszRelativeServer
                        , server, path()->pszRelativeServer, server, path()->pszRelativeServer, server, path()->pszRelativeServer);
        int index =0;
        do {
            pScenarii[index].WriteHTML(toFile, isStdout);
        }while((++index)<iMaxResult);

        fprintf(toFile, "</tbody></table></td></tr>\n");
    }

    /*if(unusedAC) {
	fprintf(toFile, "<tr><td>Unused AC : ");
	for(int i=0; i<unusedACnb; i++) {
            fprintf(toFile, "%s ", unusedAC[i]);
	}
	fprintf(toFile, "</td></tr>");
    }*/

    fprintf(toFile, "</tbody></table>\n");

    fprintf(toFile, "<a href=\"#upstart\">Top</a><br><br>");
}


void PopiResult::WriteFirstScenario(File &toFile, bool isStdout) {
    if(iMaxResult!=0) {
        pScenarii[0].WriteSimpleHTML(toFile, isStdout);
    }
    else {
        char* server = "";
        if(!isStdout) server = path()->pszServer;    
	fprintf(toFile, "<td><a href=\"%s%scgi-bin/popitam/help.pl?help.html+noresult\" target=\"_new\">No result found...</a></td>\n", server, path()->pszRelativeServer);
    }
}

/******************************************************
******************************************************

        Constructor
        
*******************************************************/
Scenario::Scenario(void) :
    dParentMass(0),
    iId(0),
    dScore(0.0),
    dDeltaS(0.0),
    dPValue(0.0),
    dMass(0.0),
    sAccess(NULL),
    sIdentifier(NULL),
    sInterpretation(NULL),
    sInterpretation2(NULL),
    iNbAcId(0)
{
}
/******************************************************

        Destructor

*******************************************************/
Scenario::~Scenario(void)
{ }

/******************************************************

        Read/Write

*******************************************************/
void Scenario::WriteHTML(File &toFile, bool isStdout)
{
    fprintf(toFile, "<tr>");
    fprintf(toFile, "<td>%d</td>\n", iId); //#
    WriteSimpleHTML(toFile, isStdout);
    fprintf(toFile, "</tr>");
}
void Scenario::WriteSimpleHTML(File &toFile, bool isStdout)
{
    fprintf(toFile, "<td align=\"right\">%.2f</td>", dScore);//score
    if(dDeltaS==1000.0) {
        fprintf(toFile, "<td align=\"right\"> - </td>");
    }
    else {
        fprintf(toFile, "<td align=\"right\">%.2f </td>", dDeltaS);//deltas
    }
    if(dPValue==1000.0) {
        fprintf(toFile, "<td align=\"right\"> - </td>\n");
    }
    else {
        fprintf(toFile, "<td align=\"right\">%.2e</td>\n", dPValue);//pvalue
    }
    fprintf(toFile, "<td align=\"right\">%.2f</td>\n", dMass);//mass
    fprintf(toFile, "<td align=\"right\">%.2f</td>\n", dParentMass - dMass);//mass
    fprintf(toFile, "<td><pre>%s%s</pre></td>\n", sInterpretation, sInterpretation2);//dtbSeq / scenario
    fprintf(toFile, "<td>");
    for(int i=0; i<iNbAcId; i++) {
	int length = (int)strlen(sAccess[i]);
	for(int j=length-1; j>0; j--) {
		if(sAccess[i][j]==' ') sAccess[i][j] = '\0';
	}
        if( strncmp(sIdentifier[i], sAccess[i], strlen(sAccess[i]))==0 ) { //display ID only (TrEMBL)
            if(isStdout) {
                fprintf(toFile, "<a href=\"%suniprot/%s\" target=\"_new\">%s</a><br>", path()->pszRelativeServer, sAccess[i]+1, sIdentifier[i]);
            }
            else {
                fprintf(toFile, "<a href=\"%s/uniprot/%s\" target=\"_new\">%s</a><br>", path()->pszServer, sAccess[i]+1, sIdentifier[i]);
            }
        }
        else { //Swiss-Prot, so display ID(AC)
            char * vPos = strrchr(sIdentifier[i], '_');
            char* variant = NULL;
            if(vPos!=NULL && strlen(vPos)>2 && vPos[1]=='V' && strspn(vPos+2, "0123456789 ")==(strlen(vPos)-2) ) {
                variant = StrdupFormat("-%s", vPos+2);
            }
            else {
                variant="";
            }
            if(isStdout) {
                fprintf(toFile, "<a href=\"%suniprot/%s%s\" target=\"_new\">%s</a>(%s)<br>", path()->pszRelativeServer, sAccess[i]+1, variant, sIdentifier[i], sAccess[i]);
            }
            else {
                fprintf(toFile, "<a href=\"%s/uniprot/%s%s\" target=\"_new\">%s</a>(%s)<br>", path()->pszServer, sAccess[i]+1, variant, sIdentifier[i], sAccess[i]);
            }
        }
    }
    fprintf(toFile, "</td>\n");
}


