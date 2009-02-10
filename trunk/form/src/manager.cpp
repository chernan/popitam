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

	Company			: Swiss	Intitute of	Bioinformatics
	Author			: Marc Tuloup
	Contact			: Marc.Tuloup@isb-sib.ch
	Created	on			: 24.02.2005
	Last modification		: 25.02.2005
	Comments			: 

***********************************************************/


#define	USE_MANAGER
#define	USE_POPITAMFORM
#define	USE_CGI
#define	USE_HTML
#define	USE_REQUEST
#define	USE_MSG
#define	USE_UTIL
#define	USE_TIME

#include "../path_file.inc"
#include "use.h"

const int BUFF_LENGTH = 256;

//const	char *Paths::m_pszFileName = "/work/expasy/indexes/tools/popitam/data/setup_out/path.txt";
//const	char *Paths::m_pszFileName = "/home/sun-000/Stagiaire/chernand/public_html/indexes/tools/popitam/data/path_lin.txt";
const char *Paths::m_pszFileName = PATH_TXT;

const bool Manager::DEBUG =	true;

/******************************************************

		Constructor
	    

*******************************************************/
Manager::Manager(void) :
	m_iRequest(REQUEST_FORM),
	isBatchMode(false) { }


/******************************************************

		Destructor

*******************************************************/
Manager::~Manager(void)
{
}


/***********************************************************

   Run

***********************************************************/
void Manager::Run(void)
{
	File file(stdout);
    
/*	  try{
		char *pszGlobalVarExpasy = GetEnv("GL_");
		Paths::m_pszFileName = pszGlobalVarExpasy;
	}catch(Error *pError){
		pError->Print();
		delete pError;
	}
*/

	//charge les chemins utilisés
	try{
		Paths::Load();

		inputForm.Load();
    
		if(Cgi::IsDefaultCall()){
			inputForm.SetDefault();
			inputForm.DoDisplay(file);
			return;
		}
	    
		    
		m_iRequest = inputForm.ReadCgi();
    
		try{
    
			switch(m_iRequest){
			    
				case REQUEST_SUBMIT:
					inputForm.CheckCgi();
					RequestSubmit();
					break;
    
				case REQUEST_FORM:
					inputForm.CheckCgi();
					inputForm.DoDisplay(file);
					break;
    
				case REQUEST_EXAMPLE1:
					//inputForm.SetDefault();
					inputForm.SetExample0MODDta();
					inputForm.DoDisplay(file);
					break;
    
				case REQUEST_EXAMPLE2:
					//inputForm.SetDefault();
					inputForm.SetExample0MODMgf();
					inputForm.DoDisplay(file);
					break;
    
				case REQUEST_EXAMPLE3:
					//inputForm.SetDefault();
					inputForm.SetExample1MODMgf();
					inputForm.DoDisplay(file);
					break;
    
				case REQUEST_EXAMPLE4:
					//inputForm.SetDefault();
					inputForm.SetExample2MODMgf();
					inputForm.DoDisplay(file);
					break;
    
				case REQUEST_DEFAULT:
					inputForm.SetDefault();
					inputForm.DoDisplay(file);
					break;
    
				default	:
					ThrowError("Manager::Run", "Unknown	request	id=\"%d\"",	m_iRequest);
			}
    
		}catch(Msg *pMsg){
			inputForm.DoDisplay(file, pMsg->Get());
			delete pMsg;
		}

		Paths::Free();
	    
	}catch(Error *pError){
		pError->Print();
		delete pError;
		return;
	}
}



/***********************************************************

   RequestSubmit

***********************************************************/
void Manager::RequestSubmit(void)
{
	char *pszKey = StrdupFormat("%ld%d", time(NULL), rand()%1000);

	//ecrit les data qui seront utilisees par tagopop dans un fichier à	part
	char *pszDataFileName =	StrdupFormat("_popdata_%s.txt", pszKey);
	TagFile	dataFile;
	dataFile.Open(StrdupFormat("%s%s", path()->pszTmpDir, pszDataFileName), "w");
	if(	strlen(inputForm.GetDataFileName())	) {
		dataFile.WriteTxt(inputForm.GetDataFile());
	}
	else {
		dataFile.WriteTxt(inputForm.GetDataCopy());
	}
	dataFile.Close();

	//ecrit	la fonction	de score qui sera utilisee par tagopop dans	un fichier à part
	char *pszScoreFileName;
	if(strlen(inputForm.GetScoreFileName()) || strlen(inputForm.GetScoreCopy())) {
		pszScoreFileName = StrdupFormat("_scorefun_%s.txt", pszKey);
		TagFile scoreFile;
		scoreFile.Open(StrdupFormat("%s%s", path()->pszTmpDir, pszScoreFileName), "w");
		if( strlen(inputForm.GetScoreFileName()) ) {
			scoreFile.WriteTxt(inputForm.GetScoreFile());
		}
		else if( strlen(inputForm.GetScoreCopy()) ){
			scoreFile.WriteTxt(inputForm.GetScoreCopy());
		}
		scoreFile.Close();
	}
	else {
		pszScoreFileName = "null";
	}

	if(inputForm.IsMailUsed()){

		//ecrit	les	autres donnees dans	un fichier param
		char *pszParamFileName = StrdupFormat("_param_%s.txt", pszKey);
		TagFile	cgiParamFile;
		cgiParamFile.Open(StrdupFormat("%s%s", path()->pszTmpDir, pszParamFileName), "w");
		inputForm.WriteFile(&cgiParamFile);
		cgiParamFile.Close();

		//appel	form.cgi en mode batch
		char *pszBatchCmdFileName = StrdupFormat("%s_cmd_%s.txt", path()->pszTmpDir, pszKey);
		File command;
		command.Open(pszBatchCmdFileName, "w");
		fprintf(command, "%s \"%s\" \"%s\" \"%s\"", path()->pszCgiFullPath, 
							pszParamFileName,
							pszDataFileName,
							pszScoreFileName);
		command.Close();

		SystemCmd("chmod +x %s", pszBatchCmdFileName);
//		  SystemCmd("%s", pszBatchCmdFileName);
		SystemCmd("batch <%s", pszBatchCmdFileName);

		if(!DEBUG) {
			remove(pszBatchCmdFileName);
		}

		free(pszBatchCmdFileName);
		free(pszParamFileName);
		free(pszKey);


		ThrowMsg("Your request has been	submitted to Popitam.\\n"
			"The results will be sent to you by email : \\n(%s)\\n\\nNB : Your E-mail server should allow you to receive\\nHTML messages containing javascript.\\nIf not, the result may not reach your mailbox.\\n", inputForm.GetMail());


	}else{
		//ouvre	la sortie standard
		File file(stdout);
		WriteHtmlContentType();
	    
		//submit
		Submit(pszDataFileName,	pszScoreFileName, file,	true);

	}


    
}


/***********************************************************

   Submit

***********************************************************/
void Manager::Submit(const char	*pszDataFileName, const	char *pszScoreFileName,	File &output, bool isStdout)
{
	char *pszKey = StrdupFormat("%ld%d", time(NULL), rand()%1000);
	
	
	
	int pathlength = strlen(path()->pszPrgmFullPath);
	char pszTemp[256];
	memset(pszTemp, '\0', 256);
	strncpy(pszTemp, path()->pszPrgmFullPath, pathlength-25);// bin_Linux/popitam/tagopop length
	const char* pszWsubPath = StrdupFormat("%s%s", pszTemp, "wsub.py");//"/work/expasy/www/cgi-bin/popitam/wsub.py";

	//cree le fichier de sortie pour le programme
	const char *pszOutputFileName =	StrdupFormat("_output_%s.txt", pszKey);
	File outputFile( StrdupFormat("%s%s", path()->pszTmpDir, pszOutputFileName) , "w");
	outputFile.Close();

	//ecrit	les parametres dans un fichier param au format tagopop
	char *pszParamFileName = StrdupFormat("_popparam_%s.txt", pszKey);
	TagFile	paramFile;
	paramFile.Open( StrdupFormat("%s%s", path()->pszTmpDir, pszParamFileName) , "w");
	WriteParamFile(&paramFile, pszScoreFileName);
    
	char *pszErrorFileName = StrdupFormat("_error_%s.txt", pszKey);
	File errorFile( StrdupFormat("%s%s", path()->pszTmpDir, pszErrorFileName) , "w");
	errorFile.Close();

	//ecrit	en fin de fichier la ligne de commande qui sera lancee !! seulement verification - a enlever
	paramFile.WriteTxt("\n\nCommand	line will be ");	//path()->pszPrgmFullPath,
	char *pszCmdString = StrdupFormat("tagopop -r=NORMAL -s=UNKNOWN -m=%d -p=%s -d=%s -f=%s -e=%s -o=%s",
					inputForm.GetMutMode(),
					pszParamFileName,
					pszDataFileName,
					inputForm.GetDataFormat(),
					pszErrorFileName,
					pszOutputFileName);
	paramFile.WriteTxt(pszCmdString);
	paramFile.WriteTxt("\n\n");

	const char *pszVitalitJobidFilePath = StrdupFormat("%s_vitalitjid_%s.txt", path()->pszTmpDir, pszKey);
	char *pszCmdWsubString = StrdupFormat("%s --noscp --noauth --user=expasy --jobname=popitam%s --queue=normal --in=%s%s --in=%s%s %s --out=%s%s --out=%s%s \"%s\" > %s",
					pszWsubPath, pszKey,
					path()->pszTmpDir, pszParamFileName, path()->pszTmpDir, pszDataFileName, (strcmp(pszScoreFileName,"null")==0)?"":StrdupFormat("--in=%s%s", path()->pszTmpDir, pszScoreFileName),
					path()->pszTmpDir, pszOutputFileName, path()->pszTmpDir, pszErrorFileName, 
					pszCmdString,
					pszVitalitJobidFilePath);
	paramFile.WriteTxt(pszCmdWsubString);

	paramFile.Close();

	//appel	tagopop	par ligne de commande
	////SystemCmd("cd	%sEXE/", pszDirFullPath);
	//SystemCmd("%s",	pszCmdString); 
	
	/**************************************************************/
	//Write date in log file
	
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	File logFile;
	if( !logFile.OpenIfExist(StrdupFormat("%saldente_log/popitam.log", path()->pszTmpDir) , "a") ) {
		logFile.Open(StrdupFormat("%saldente_log/popitam.log", path()->pszTmpDir) , "w");
	}
	fprintf(logFile, "%02d/%02d/%04d\n", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900);
	logFile.Close();

	
	/**************************************************************/
	//run wsub
	SystemCmd("%s", pszCmdWsubString);
	
	//get jobid	
	File jobidFile(pszVitalitJobidFilePath, "r");
	char szBuffer[BUFF_LENGTH];
	fgets(szBuffer,	BUFF_LENGTH, jobidFile);
	int currentJobid = atoi(szBuffer); 
	jobidFile.Close();
	
	//check that job ws correctly submitted
	if( currentJobid == 0) { //if message in file is not a simple job id
		ThrowMsg("Popitam experiments some connexion problems with Vital-IT. Please resubmit in a few minutes.\nThe problem should persist, please contact expasy tools helpdesk.");
	}
	
	//check job status for a certain amount of time
	//pending running success failure unknown suspend
	const char *pszJobStatusFilePath = StrdupFormat("%s_jobstatus_%s.txt", path()->pszTmpDir, pszKey);
	File statusFile(pszJobStatusFilePath, "w");
	statusFile.Close();
	
	unsigned long int iniTime = time(NULL);
	double currentTime = 0;
	double limitTime = isBatchMode?DBL_MAX:300.0;
	double waitingTime = isBatchMode?30.0:5.0;
	bool isLongRun = true;
	jobidFile.Open(pszVitalitJobidFilePath, "a");
	while ( (currentTime = ((double)(time(NULL)-iniTime))) < limitTime) { //While running time < 300 seconds = 5 minutes
		SystemCmd("%s --noscp  --noauth --user=expasy --status --jid=%d > %s", pszWsubPath, currentJobid, pszJobStatusFilePath);
		statusFile.Open(pszJobStatusFilePath, "r");
		fgets(szBuffer,	BUFF_LENGTH, statusFile);
		fprintf(jobidFile, "%s ; time is %f\n", szBuffer, currentTime);
		if( strncmp(szBuffer, "pending", 7)!=0 ) {
			if( strncmp(szBuffer, "running", 7)!=0 ) {
				isLongRun=false;
				statusFile.Close();		
				break;
			}
		}
		statusFile.Close();
		while ( ((double)(time(NULL)-iniTime)) < currentTime+waitingTime ) {
			for (int i=0; i<100000; i++) {} // 100 000 is ~1 second;
		}
	}
	jobidFile.Close();
	
	if(isLongRun) {
		SystemCmd("%s --noscp  --noauth --user=expasy --stop --jid=%d > %s", pszWsubPath, currentJobid, pszJobStatusFilePath);
		strcpy(szBuffer, "stopped");
	}
	else {
		//Get current status
		SystemCmd("%s --noscp  --noauth --user=expasy --status --jid=%d > %s", pszWsubPath, currentJobid, pszJobStatusFilePath);
		statusFile.Open(pszJobStatusFilePath, "r");
		fgets(szBuffer,	BUFF_LENGTH, statusFile);
		statusFile.Close();
	}

	
	//Test status other than pending, running
	//failure, success, stopped ; others = killed, resumed, submission_error, suspend, unknown
	if ( !strncmp(szBuffer, "failure", 7) ) {

		SystemCmd("%s --noscp --noauth --user=expasy --jid=%d --peek=%s > %s%s", pszWsubPath, currentJobid, pszErrorFileName, path()->pszTmpDir, pszErrorFileName);
		WriteErrorPage(output, StrdupFormat("%s%s", path()->pszTmpDir, pszErrorFileName), isStdout, currentJobid);

	}
	else if ( !strncmp(szBuffer, "success", 7) ) {
		WriteHeader(output, isStdout);
	
		//Check error file, as sometimes a job can be marked as successful but having errors
		SystemCmd("%s --noscp --noauth --user=expasy --jid=%d --peek=%s > %s%s", pszWsubPath, currentJobid, pszErrorFileName, path()->pszTmpDir, pszErrorFileName);
		File errorFile( StrdupFormat("%s%s", path()->pszTmpDir, pszErrorFileName) , "r");
		char* fgetsRetVal=NULL;
		fgetsRetVal = fgets(szBuffer, BUFF_LENGTH, errorFile);
		errorFile.Close();
		if (fgetsRetVal!=NULL) {
			if(!strncmp(fgetsRetVal, "0", 1) || !strncmp(fgetsRetVal, "1", 1) || !strncmp(fgetsRetVal, "2", 1) ) {
				WriteErrorMessage(output, StrdupFormat("%s%s", path()->pszTmpDir, pszErrorFileName), currentJobid);
			}
		} 

		//error checked and displayed, look at output.		
		SystemCmd("%s --noscp --noauth --user=expasy --jid=%d --peek=%s > %s%s", pszWsubPath, currentJobid, pszOutputFileName, path()->pszTmpDir, pszOutputFileName);
		WriteOutput(StrdupFormat("%s%s", path()->pszTmpDir, pszOutputFileName), output, isStdout);
		
		WriteFooter(output, isStdout);

	}
	else if ( !strncmp(szBuffer, "stopped", 7) ) {
		WriteHeader(output, isStdout);
		fprintf(output,	"<h1>Time out - Job too long</h1>\n");
		fprintf(output,	"Your job has been stopped, maybe due to time restrictions.<br>Please consider submitting to popitam giving your e-mail adress in the web form. This will allocate more time to your request, and the coresponding results will be directly sent by e-mail.\n");

		//time out message displayed, look at output.		
		SystemCmd("%s --noscp --noauth --user=expasy --jid=%d --peek=%s > %s%s", pszWsubPath, currentJobid, pszOutputFileName, path()->pszTmpDir, pszOutputFileName);
		WriteOutput(StrdupFormat("%s%s", path()->pszTmpDir, pszOutputFileName), output, isStdout);

		WriteFooter(output, isStdout);
		
	}
	else {
		WriteHeader(output, isStdout);
		fprintf(output,	"<h1>Job %d status error</h1>\nYour job has ended on Vital-It with status \"%s\".<br>Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback_Jobid_%d&tools@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk</a>.\n", currentJobid, szBuffer, currentJobid);
		WriteFooter(output, isStdout);
		
	}
	output.Close();
	
	/*****************************************************************/

	//efface les fichiers devenus inutiles
	if(!DEBUG) {
		remove(pszOutputFileName);
		remove(pszDataFileName);
		remove(pszParamFileName);
		remove(pszErrorFileName);
		remove(pszVitalitJobidFilePath);
	}

}

void Manager::WriteErrorPage(File &output, const char* pszErrorFilePath, bool isStdout, int jobId) {
	
		//affichage de l'entete et des headers expasy
		fprintf(output, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
		fprintf(output,	"<html lang=\"en-US\">\n<head>\n<title>ExPASy - Popitam output</title>\n");
		inputForm.WriteStyle(output);
		fprintf(output,	"<script LANGUAGE=JAVASCRIPT>\n"
			  "<!--\n");
		char *pszExpFileName = StrdupFormat("%s%s",	path()->pszExpFilesDir,	"expasy_script.txt"); // _script
		File expasyFile;
		if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
			inputForm.WriteExpasyFile(output, expasyFile);
		}
		fprintf(output,	"-->\n</script>\n\n");
	    
		fprintf(output, "</head>\n<body>\n");

		pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_head.txt"); //_head _script _search _foot
		if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
			inputForm.WriteExpasyFile(output, expasyFile);
		}
		pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_search.txt");	// _script
		if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
			inputForm.WriteExpasyFile(output, expasyFile);
		}
	
		//Write error message
		WriteErrorMessage(output, pszErrorFilePath, jobId);
		
		//resubmit
		inputForm.WriteResubmit(output,	!isStdout);
	    
		//end page and close errorFile
		pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_foot.txt"); // _script _search 
		if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
			inputForm.WriteExpasyFile(output, expasyFile);
		}
		WriteHtmlEnd(output);

	
}

void Manager::WriteErrorMessage(File &output, const char* pszErrorFilePath, int jobId) {
	
		//affichage de l'erreur
		//inputForm.WriteLogo(output);
		
		fprintf(output,	"<h1>Popitam Error</h1><br>\n");
		File errorFile(pszErrorFilePath, "r");
		char szBuffer[BUFF_LENGTH];
		fgets(szBuffer,	BUFF_LENGTH, errorFile);
		int readValue = atoi(szBuffer);
		if(readValue==1) { //tagopop external error
			fprintf(output,	"Error in job %d :<br>\n", jobId);
			while( fgets(szBuffer, BUFF_LENGTH, errorFile)!=NULL ) {
				fprintf(output,	"%s<br>\n", szBuffer);
			}
			fprintf(output,	"<br>Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback_Jobid_%d&tools@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk</a>.\n", jobId);
		}
		else if(readValue==0) {
			//szBuffer contient deja le message d'erreur
			fprintf(output,	"Undefined error in job %d.<br>Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback_Jobid_%d&tools@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk</a>.\n", jobId, jobId);
		}
		else if(readValue==2) {
			//szBuffer contient deja le message d'erreur
			fprintf(output,	"Internal error in job %d.<br>Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback_Jobid_%d&tools@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk</a>.\n", jobId, jobId);
		}
		
		errorFile.Close();
}

/***********************************************************

   WriteParamFile

write file containing parameters for the program
pszDirFullPath

***********************************************************/
void Manager::WriteParamFile(TagFile *pFile, const char	*pszScoreFileName)
{
	pFile->WriteTxt("//	FILE NAMES\n\n");
    
	pFile->WriteTxt("PATH_FILE						:default\n");
//	pFile->WriteTxt("PATH_FILE						:");pFile->WriteTxt(path()->pszLibPath);pFile->WriteTxt("\n");
	pFile->WriteTxt("AMINO_ACID_FILE				:default\n");
//	pFile->WriteTxt("AMINO_ACID_FILE				:");pFile->WriteTxt(path()->pszAA);pFile->WriteTxt("aa20.txt\n");
	pFile->WriteTxt("AA_PROPERTIES_FILE				:default\n\n");
//	pFile->WriteTxt("AA_PROPERTIES_FILE				:");pFile->WriteTxt(path()->pszAAProp);pFile->WriteTxt("aa_properties.txt\n\n");

	pFile->WriteTxt("GPPARAMETERS					:default\n");
//	pFile->WriteTxt("GPPARAMETERS					:");pFile->WriteTxt(path()->pszScores);pFile->WriteTxt("functionLoadParam.txt\n");
    
	if(strcmp(pszScoreFileName, "null")==0) {
		pFile->WriteTxt("SCORE_FUN_FUNCTION0		:default\n");
//		pFile->WriteTxt("SCORE_FUN_FUNCTION0		:");pFile->WriteTxt(path()->pszScores);pFile->WriteTxt("funScore0.dot\n");
		pFile->WriteTxt("SCORE_FUN_FUNCTION1		:default\n");
//		pFile->WriteTxt("SCORE_FUN_FUNCTION1		:");pFile->WriteTxt(path()->pszScores);pFile->WriteTxt("funScore1.dot\n");
		pFile->WriteTxt("SCORE_FUN_FUNCTION2		:default\n\n");
//		pFile->WriteTxt("SCORE_FUN_FUNCTION2		:");pFile->WriteTxt(path()->pszScores);pFile->WriteTxt("funScore2.dot\n\n");
	}
	else {
		int	mutMode	= inputForm.GetMutMode();
		if(mutMode==0) {
			pFile->WriteTxt("SCORE_FUN_FUNCTION0	:");pFile->WriteTxt(pszScoreFileName);pFile->WriteTxt("\n");
			//le chemin complet n'est pas dans le nom pszScoreFileName
		}
		else {
			pFile->WriteTxt("SCORE_FUN_FUNCTION0	:default\n");
//			pFile->WriteTxt("SCORE_FUN_FUNCTION0	:");pFile->WriteTxt(path()->pszScores);pFile->WriteTxt("funScore0.dot\n");
		}
		if(mutMode==1) {
			pFile->WriteTxt("SCORE_FUN_FUNCTION1	:");pFile->WriteTxt(pszScoreFileName);pFile->WriteTxt("\n");
		}
		else {
			pFile->WriteTxt("SCORE_FUN_FUNCTION1	:default\n");
//			pFile->WriteTxt("SCORE_FUN_FUNCTION1	:");pFile->WriteTxt(path()->pszScores);pFile->WriteTxt("funScore1.dot\n");
		}
		if(mutMode==2) {
			pFile->WriteTxt("SCORE_FUN_FUNCTION2	:");pFile->WriteTxt(pszScoreFileName);pFile->WriteTxt("\n\n");
		}
		else {
			pFile->WriteTxt("SCORE_FUN_FUNCTION2	:default\n\n");
//			pFile->WriteTxt("SCORE_FUN_FUNCTION2	:");pFile->WriteTxt(path()->pszScores);pFile->WriteTxt("funScore2.dot\n\n");
		}
	}
    
	pFile->WriteTxt("PROBS_TOFTOF1					:default\n");
    pFile->WriteTxt("PROBS_QTOF1                	:default\n");
    pFile->WriteTxt("PROBS_QTOF2                	:default\n");
 	pFile->WriteTxt("PROBS_QTOF3					:default\n\n");
// 	pFile->WriteTxt("PROBS_TOFTOF1					:");pFile->WriteTxt(path()->pszProb);pFile->WriteTxt("TOFTOF_1.prob\n");
//    pFile->WriteTxt("PROBS_QTOF1                	:");pFile->WriteTxt(path()->pszProb);pFile->WriteTxt("QTOF_1.prob\n");
//    pFile->WriteTxt("PROBS_QTOF2                	:");pFile->WriteTxt(path()->pszProb);pFile->WriteTxt("QTOF_2.prob\n");
// 	pFile->WriteTxt("PROBS_QTOF3					:");pFile->WriteTxt(path()->pszProb);pFile->WriteTxt("QTOF_3.prob\n\n");
    
	pFile->WriteTxt("DTB_SP_DIR						:default\n");
	pFile->WriteTxt("DTB_TR_DIR						:default\n");
	pFile->WriteTxt("TAXO_INFILE					:default\n");
//	pFile->WriteTxt("DTB_SP_DIR						:");pFile->WriteTxt(path()->pszDB);pFile->WriteTxt("UniProtSP\n");
//	pFile->WriteTxt("DTB_TR_DIR						:");pFile->WriteTxt(path()->pszDB);pFile->WriteTxt("UniProtTR\n");
//	pFile->WriteTxt("TAXO_INFILE					:");pFile->WriteTxt(path()->pszTaxonomy);pFile->WriteTxt("taxonomy.txt\n");
	pFile->WriteTxt("OUTPUT_DIR						:");pFile->WriteTxt(path()->pszTmpDir);pFile->WriteTxt("\n");			  //!unused!
	pFile->WriteTxt("GEN_OR_FILENAME_SUFF			:");pFile->WriteTxt(path()->pszTmpDir);pFile->WriteTxt("EXE/LS/OR_SPEC\n");	  //!unused!
	pFile->WriteTxt("GEN_NOD_FILENAME_SUFF			:");pFile->WriteTxt(path()->pszTmpDir);pFile->WriteTxt("EXE/LS/NOD_SPEC\n");  //!unused!
	pFile->WriteTxt("SCORE_NEG_FILE					:SCORE_NEG.txt\n\n");
	pFile->WriteTxt("SCORE_RANDOM_FILE				:SCORE_RANDOM.txt\n\n");
    
	pFile->WriteTxt("//	SPECTRUM PARAMETERS\n\n");
    
	pFile->WriteTxt("DB								:");pFile->WriteTxt(inputForm.GetDatabase());		pFile->WriteTxt("\n");
//	pFile->WriteTxt("TAXONOMY						:");pFile->WriteTxt(inputForm.GetTaxonomyStr());	pFile->WriteTxt("\n");
	pFile->WriteTxt("TAXONOMY						:root\n");
//	pFile->WriteTxt("TAX_ID							:");pFile->WriteSplInt(inputForm.GetTaxonomyId());	pFile->WriteTxt("\n");
	pFile->WriteTxt("TAX_ID							:1\n");
	pFile->WriteTxt("AC_FILTER						:");pFile->WriteTxt(inputForm.GetACFilter());		pFile->WriteTxt("\n");
	pFile->WriteTxt("FRAGM_ERROR1					:");pFile->WriteSplDouble(inputForm.GetFragmentError1());	pFile->WriteTxt("\n");
	pFile->WriteTxt("FRAGM_ERROR2					:");pFile->WriteSplDouble(inputForm.GetFragmentError2());	pFile->WriteTxt("\n");
	pFile->WriteTxt("PREC_MASS_ERR					:2\n");
	pFile->WriteTxt("INSTRUMENT						:");pFile->WriteTxt(inputForm.GetInstrument());		pFile->WriteTxt("\n\n");
    
	pFile->WriteTxt("//	DIGESTION PARAMETERS\n\n");
    
	pFile->WriteTxt("MISSED							:");pFile->WriteSplInt(inputForm.GetMissedCleavage());		pFile->WriteTxt("\n\n");
    
	pFile->WriteTxt("//	POPITAM	SPECIFIC PARAMETERS\n\n");
    
	pFile->WriteTxt("PEAK_INT_SEUIL					:5\n");
	pFile->WriteTxt("BIN_NB							:10\n");
	pFile->WriteTxt("COVBIN							:6\n");
	pFile->WriteTxt("EDGE_TYPE						:1\n");
	pFile->WriteTxt("MIN_TAG_LENTGH					:3\n");
	pFile->WriteTxt("RESULT_NB						:");pFile->WriteSplDouble(inputForm.GetDisplayNb());		pFile->WriteTxt("\n");
	pFile->WriteTxt("MIN_PEP_PER_PROT				:2\n");
	pFile->WriteTxt("UP_LIMIT_RANGE_PM				:");pFile->WriteSplDouble(inputForm.GetMaxAddPM());		pFile->WriteTxt("\n");
	pFile->WriteTxt("LOW_LIMIT_RANGE_PM				:");pFile->WriteSplDouble(inputForm.GetMaxLossPM());		pFile->WriteTxt("\n");
	pFile->WriteTxt("UP_LIMIT_RANGE_MOD				:");pFile->WriteSplDouble(inputForm.GetMaxAddModif());		pFile->WriteTxt("\n");
	pFile->WriteTxt("LOW_LIMIT_RANGE_MOD			:");pFile->WriteSplDouble(inputForm.GetMaxLossModif());		pFile->WriteTxt("\n");
	pFile->WriteTxt("MIN_COV_ARR					:0.3\n\n");
	pFile->WriteTxt("PLOT							:0\n\n");
	pFile->WriteTxt("PVAL_ECHSIZE					:0\n\n");

	pFile->WriteTxt("//	********************************************************************************************** //\n\n");

   
	//popitam form
/*	  m_display.WriteFile(pFile);
	m_acList.WriteFile(pFile);
	m_modifications.WriteFile(pFile);
	m_parentMError.WriteFile(pFile);
*/
}



/***********************************************************

   WriteOutput

***********************************************************/
void Manager::WriteOutput(const	char *pszOutputFileName, File &toFile, bool isStdout)
{
	//output results
	TagFile	file;
	file.Open(pszOutputFileName, "r");
	ResSummary allResults;
	bool parseOK = allResults.ParseTXT(file, inputForm.GetDisplayNb());
	if(parseOK)	{

		if(allResults.iSpectraNb==0) {
			fprintf(toFile,	"<p>Sorry, but there is not enough information to display even the results concerning one spectrum.<br>If your job stopped due to time out limits, please consider restricting your request.<br>"
							 "We are really sorry for the disturbance.</p>");
		}
		else {
			if(allResults.iSpectraNb==allResults.iMaxSpectraNb) {
				fprintf(toFile,	"<h1> Popitam results </h1>\n(Version 06/15/2006.)\n");
			}
			else {
				fprintf(toFile,	"<p>An error occured while processing your data (see error message above). Part of your results (%d from %d spectra) have been retrieved and are displayed below :</p>\n", allResults.iSpectraNb, allResults.iMaxSpectraNb);
				fprintf(toFile,	"<h2> Popitam results </h2>\n(Version 06/15/2006.)\n");	
			}
			//inputForm.SetRunMode(allResults.iAllowedGap==0?0:1);
			inputForm.SetGapMax(allResults.iAllowedGap);
			inputForm.SetUnusedAC(allResults.GetUnusedAC(), allResults.GetUnusedACnb());
	
			//Parameters
			inputForm.WriteOutputParam(toFile, isStdout);
			//if sent by mail, some	paths must be changed in uniprot links
			allResults.WriteHTML(toFile, isStdout);
			//if sent by mail, some	paths must be changed in Resubmit button
			if(isStdout) {inputForm.WriteResubmit(toFile, !isStdout);}
		}
	}
	else {
		fprintf(toFile,	"<p>A problem occured while running popitam : we were not able to retrieve your results.<br>"
		 "Please <a href=\"/cgi-bin/mailform/rt_tools?Tools_Helpdesk&Popitam_feedback&tools@expasy.org\" target=\"_new\">contact ExPASy tools helpdesk</a> and tell them what you were requesting, and with which parameters.<br>"
		 "We are really sorry for the disturbance.</p>");
	}
	file.Close();
}

void Manager::WriteHeader(File &toFile, bool isStdout) {
	
	fprintf(toFile, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
	fprintf(toFile,	"<html lang=\"en-US\">\n<head>\n<title>ExPASy - Popitam output</title>\n");
    inputForm.WriteStyle(toFile);
         
	if(isStdout) {      
            fprintf(toFile, "<script LANGUAGE=JAVASCRIPT type=\"text/javascript\">\n"
                      "<!--\n");
            char *pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_script.txt"); // _script
            File expasyFile;
            if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
                    inputForm.WriteExpasyFile(toFile, expasyFile);
            }
            fprintf(toFile, "-->\n</script>\n\n");

			fprintf(toFile, "<link rel=\"shortcut icon\" href=\"/favicon.ico\" type=\"image/x-icon\">");
			fprintf(toFile, "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">");
			fprintf(toFile, "<link rel=\"stylesheet\" href=\"/base.css\" type=\"text/css\">");
            
			fprintf(toFile, "</head>\n<body>\n");

            pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_head.txt"); //_head _script _search _foot
            if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
                    inputForm.WriteExpasyFile(toFile, expasyFile);
            }
            pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_search.txt");     // _script
            if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
                    inputForm.WriteExpasyFile(toFile, expasyFile);
            }
	}
	else {
			fprintf(toFile, "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">");
            fprintf(toFile, "</head>\n<body>\n");
	}
	
	//Header
	//dont't write logo if output is sent by mail;
	//don't write logo at all
	//if(isStdout) {
	//	inputForm.WriteLogo(toFile);
	//}
	
}

void Manager::WriteFooter(File &toFile, bool isStdout) {
	if(isStdout) {
            File expasyFile;
            char *pszExpFileName = StrdupFormat("%s%s", path()->pszExpFilesDir, "expasy_foot.txt"); // _script _search 
            if( expasyFile.OpenIfExist(pszExpFileName, "r") ) {
                	inputForm.WriteExpasyFile(toFile, expasyFile);
        	}
	}
	WriteHtmlEnd(toFile);
	
}
/***********************************************************

   BatchMode

***********************************************************/
void Manager::BatchMode(const char *pszParamFileName, const	char *pszDataFileName, const char *pszScoreFileName)
{
	//charge les chemins utilisés, comme path()->pszTmpDir
	try{
		Paths::Load();
		isBatchMode = true;
		
		//Load
		inputForm.Load();
	 
		//charge les parametres	de l'utilisateur et	efface le fichier ou ils etaient stockes
		TagFile	paramFile;
		paramFile.Open(StrdupFormat("%s%s", path()->pszTmpDir, pszParamFileName), "r");
		inputForm.ReadFile(&paramFile);
		paramFile.Close();
		if(!DEBUG) {
			remove(paramFile.GetFileName());
		}
	    
		//re-assigne le	contenu	des	data au	champ correspondant	(nécessaire	pour que le	resubmit du	fichier	mail attaché fonctionne)
		const int	BUFF_LENGTH	= 256;
		int	BIG_BUFF_LENGTH	= 50*BUFF_LENGTH;
		char szBuffer[BUFF_LENGTH];
		char* szData = new char[BIG_BUFF_LENGTH];
		memset(szData, '\0', BIG_BUFF_LENGTH);
		char* szTemp = NULL;
		File dataFile;
		dataFile.Open(StrdupFormat("%s%s", path()->pszTmpDir, pszDataFileName), "r");
		int	currentLength=0;
		while( fgets(szBuffer, BUFF_LENGTH, dataFile)!=NULL ) {
			currentLength+=(int)strlen(szBuffer);
			if(currentLength>BIG_BUFF_LENGTH) {
				BIG_BUFF_LENGTH*=2;
				szTemp = new char[BIG_BUFF_LENGTH];
				strcpy(szTemp, szData);
				delete[] szData;
				szData = szTemp;
			}
			strcat(szData, szBuffer);
		}
		dataFile.Close();
		inputForm.SetDataCopy(szData);

    		//printf("%s\nEnd of file content\n", szData); fflush(stdout);
                //printf("%s\nEnd of score file name\n", pszScoreFileName); fflush(stdout);

		if(strcmp(pszScoreFileName,	"null")!=0)	{
			//re-assigne le	contenu	de la fonction de score	au champ correspondant (nécessaire pour	que	le resubmit	du fichier mail	attaché	fonctionne)
			BIG_BUFF_LENGTH	= 50*BUFF_LENGTH;
			char* szFunction = new char[BIG_BUFF_LENGTH];
			memset(szFunction, '\0', BIG_BUFF_LENGTH);
			char szBuffer2[BUFF_LENGTH];
			File functionFile;
			functionFile.Open(StrdupFormat("%s%s", path()->pszTmpDir, pszScoreFileName),	"r");
			currentLength=0;
			while( fgets(szBuffer2, BUFF_LENGTH, functionFile)!=NULL) {
				currentLength+=(int)strlen(szBuffer2);
				if(currentLength>BIG_BUFF_LENGTH) {
					BIG_BUFF_LENGTH*=2;
					szTemp = new char[BIG_BUFF_LENGTH];
					strcpy(szTemp, szFunction);
					delete[] szFunction;
					szFunction = szTemp;
				}
                //printf("%s\nEnd of szBuffer\n", szBuffer2); fflush(stdout);
 
				strcat(szFunction, szBuffer2);
			}
                //printf("%s\nEnd of function content\n", szFunction); fflush(stdout);
 
			functionFile.Close();
			inputForm.SetScoreCopy(szFunction);
		}
    
		//nom des fichiers
		char *pszKey = StrdupFormat("%ld%d", time(NULL), rand()%1000);

                //printf("%s\nEnd of key content\n", pszKey); fflush(stdout);
    
		//genere le	fichier	a joindre au mail
		char *pszAttachedFilePath = StrdupFormat("%s_mail_%s.html", path()->pszTmpDir, pszKey);
		File attachedFile;
		attachedFile.Open(pszAttachedFilePath, "w");
    
                //printf("%s\nEnd of htlm file name\n", pszAttachedFilePath); fflush(stdout);

		//submit les data de l'utilisateur
		Submit(pszDataFileName,	pszScoreFileName, attachedFile,	false);
	    
		//send mail
		SystemCmd( "%s \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"",
					path()->pszSendMail, 
					inputForm.GetMail(),
					"Popitam results",
					"Your submission results. Save Attached file on your disk before opening it in a browser with Javascript enabled.",
					pszAttachedFilePath,
					"results.html");
		
	    
		if(!DEBUG) {
			remove(pszAttachedFilePath);
		}
		free(pszAttachedFilePath);

		Paths::Free();
	    
	}catch(Error *pError){
		pError->Print();
		delete pError;
		return;
	}
}
