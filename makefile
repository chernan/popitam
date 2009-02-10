CC			= g++
CFLAGS			= -Wall 

# default option value; -g is for debugging
OPT			= -g
#remplace la valeur de $OPT
#option permettant de compiler tagopop en statique (en incluant toutes les bibliotheques necessaires)
#dans ce cas, lancer le make avec 'make OPTION=sttc' par exemple
# choose 'static' or 's' to include all required libraries in final exe (necessary when compiling on expasy3)
ifeq ($(OPTION), static)
        OPT = -static -O3
endif
ifeq ($(OPTION), s)
        OPT = -static -O3
endif

# choose 'profile' or 'p' option to generate profiling information for gprofile
ifeq ($(OPTION), profile)
	OPT = -pg
endif
ifeq ($(OPTION), p)
        OPT = -pg
endif

# choose 'optimize' or 'o' to optimize normal compilation
ifeq ($(OPTION), optimize)
	OPT = -O3	# O3 to optimize, but not to use it with -g (debug)
endif
ifeq ($(OPTION), o)
        OPT = -O3
endif

POP_VERSION		= '"v4.0"'

SRC_DIR			= ./src ./src/DIGEST ./src/DIGEST/base ./src/DIGEST/database ./src/DIGEST/digest
# macro qui indique ou chercher pour toutes les sources files
VPATH			= ${SRC_DIR}
# data folder - don't forget the final /
DATA 			= '"./data/"'
# db folder - don't forget the final /
# linux /home/sun-000/Stagiaire/chernand/public_html/indexes/tools/popitam/
# windows H:\\Work\\Projects\\cpp\\DTBS\\WINDB\\
DB				= '"../DB/"'

INCLUDE_DIR		= -I ./src -I ./src/DIGEST -I ./src/DIGEST/base -I ./src/DIGEST/database -I ./src/DIGEST/digest
OBJ_DIR			= ./obj
OUT_DIR         	= ./bin
OUT             	= $(OUT_DIR)/popitam
DIST_DIR		= ./dist

OBJECTS 		= atomicmass file MATerror tagfile txtfile util \
			  actree dbentry  dbfile dbfilereader dbptm dbreader \
			  digest mcmanager modifmanager peptidebase peptidedigest ptmmanager  \
			  ioparam ioversion \
			  aa about allrunstatistics ants clicsearch compare data element error \
			  fun gnb gpparameters gptree gptreenode graphe ion main memorycheck peptide \
			  prot Protein protEl result runManager runManagerParameters scenario \
			  score Sequence setlib0 specresults spectrumstatistics subseq sufftab \
			  tag_extractor tag_processor Tree usage utils vertex

RM			= /bin/rm -rf
MKDIR			= /bin/mkdir -p

all:	$(OBJ_DIR) $(OUT) make_subprojects

clean:	clean_obj clean_subprojects
	$(RM) $(OUT)
	$(RM) $(DIST_DIR)

clean_obj:	clean_subprojects_obj
	$(RM) $(OBJ_DIR)

clean_subprojects_obj:
	@make -C ./form clean_obj
	@make -C ./DB clean_obj
	@make -C ./CREATE_DB clean_obj

clean_subprojects:
	@make -C ./form clean
	@make -C ./DB clean
	@make -C ./CREATE_DB clean

make_subprojects:
	@make -C ./form
	@make -C ./DB
	@make -C ./CREATE_DB

help: 
	@echo "Three options modify default compilation, i.e. debug (use OPTION=...)"
	@echo "    static   or s for static compiling "
	@echo "    profile  or p for profiling informations "
	@echo "    optimize or o for optimized compilation "
	@echo "Some paths can also be modified during compilation " 
	@echo "    DATA  folder containing data files "
	@echo "    DB    folder containing databases, i.e. UniProtSP and UniProtTR folders"
	
dist:	all dist_init
	cp $(OUT) $(DIST_DIR)/
#	cp test/TEST_* $(DIST_DIR)/test/
	cp ./bin/com.txt $(DIST_DIR)/
	cp ./bin/popParamTest.txt $(DIST_DIR)/popParam.txt
	dos2unix $(DIST_DIR)/popParam.txt
	cp ./documents/readme.txt $(DIST_DIR)/readme.txt
	cp ./bin/testPopitam.pl $(DIST_DIR)/
	cp ./data/*.prob $(DIST_DIR)/resources/
	cp ./data/aa20.txt $(DIST_DIR)/resources/
	cp ./data/functionLoadParam.txt $(DIST_DIR)/resources/
	cp ./data/*.dot $(DIST_DIR)/resources/
	cp ./CREATE_DB/testDBs/demoDB_BASE_FORWARDandDECOY.fasta $(DIST_DIR)/dbs/
#	cp ./CREATE_DB/testDBs/demoDB_BASE_FORWARDandDECOY.bin $(DIST_DIR)/dbs/
	cp ./CREATE_DB/testDBs/demoDB_CAMHEADER.fasta $(DIST_DIR)/dbs/
	cp ./CREATE_DB/testDBs/demoDB_PHENHEADER_FORWARDandDECOY.fasta $(DIST_DIR)/dbs/
	cp ./CREATE_DB/bin/createDB $(DIST_DIR)/dbs/
	cp ./rescoring/auto_popitam_rescored2pidres.pl $(DIST_DIR)/
	cp -r ./rescoring/preprocess/ $(DIST_DIR)/rescoring/
	cp ./rescoring/01_extractscenario/popitam_scenarioextractor.pl $(DIST_DIR)/rescoring/01_extractscenario/
	cp ./rescoring/02_pm-rescoring/esquire3000+.scoring.xml $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/02_pm-rescoring/FT-ICRnormal_5.0ppm_gb-is.scoring.xml $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/02_pm-rescoring/toftof.scoring.xml $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/02_pm-rescoring/qtof.scoring.xml $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/02_pm-rescoring/gpsprot-peptides.dico $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/02_pm-rescoring/gpsprot-peptides-3.mm_links_binary $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/02_pm-rescoring/insilicodef.xml $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/02_pm-rescoring/phenyx-peptmatchscore $(DIST_DIR)/rescoring/02_pm-rescoring/
	cp ./rescoring/03_pidres/Import2PIDRes.pm $(DIST_DIR)/rescoring/03_pidres/
	cp ./rescoring/03_pidres/popitam_xml2pidres.pl $(DIST_DIR)/rescoring/03_pidres/
	cp ./rescoring/03_pidres/popitam_xml2pidres.pl.properties $(DIST_DIR)/rescoring/03_pidres/
	cp ./rescoring/data/dummy-phenyx.conf $(DIST_DIR)/rescoring/data/
	cp -r ./rescoring/perllib/ $(DIST_DIR)/rescoring/
	dos2unix $(DIST_DIR)/testPopitam.pl
	dos2unix $(DIST_DIR)/auto_popitam_rescored2pidres.pl
	chmod +rwx $(DIST_DIR)/testPopitam.pl
	chmod +rwx $(DIST_DIR)/auto_popitam_rescored2pidres.pl
	chmod +rwx $(DIST_DIR)/rescoring/02_pm-rescoring/

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

dist_init:
	mkdir -p $(DIST_DIR)/resources/
	mkdir -p $(DIST_DIR)/dbs/
#	mkdir -p $(DIST_DIR)/test/
	mkdir -p $(DIST_DIR)/rescoring/preprocess/
	mkdir -p $(DIST_DIR)/rescoring/01_extractscenario/
	mkdir -p $(DIST_DIR)/rescoring/02_pm-rescoring/
	mkdir -p $(DIST_DIR)/rescoring/03_pidres/
	mkdir -p $(DIST_DIR)/rescoring/data/
	mkdir -p $(DIST_DIR)/rescoring/perllib/
    
$(OUT): $(foreach x,$(OBJECTS),$(OBJ_DIR)/$(x).o)   # met chaque object dans x, et créer le nom complet 
	$(CC) $(OPT) -o $@ $^

$(OBJ_DIR)/%.o: %.cpp      # prend tous les .cpp dans les chemins indiqués dans VPATH et cree les .o
	$(CC) -o $@ $(INCLUDE_DIR) $(CFLAGS) -DPOP_VERSION=$(POP_VERSION) -DDEF_DATA_FOLDER=$(DATA) -DDEF_DB_PATH=$(DB) $(OPT) -c $< 

# le $@ réfère à la target courante, donc $(OBJ_DIR)/%.o
# le $< réfère au prerequis courant qui a été modifiée plus récemment que la target courante
