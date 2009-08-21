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

VERSION			= "4.0.6 (beta)"
VERSION_STR		= '$(VERSION)'

SRC_DIR			= ./src ./src/src_digest ./src/src_digest/base ./src/src_digest/database ./src/src_digest/digest
# macro qui indique ou chercher pour toutes les sources files
VPATH			= ${SRC_DIR}

# data folder - don't forget the final /
DATA			= "./data/"
DATA_STR		= '$(DATA)'

# db folder - don't forget the final /
# linux /home/sun-000/Stagiaire/chernand/public_html/indexes/tools/popitam/
# windows H:\\Work\\Projects\\cpp\\DTBS\\WINDB\\
DB				= '"../DB/"'

INCLUDE_DIR		= -I ./src -I ./src/src_digest -I ./src/src_digest/base -I ./src/src_digest/database -I ./src/src_digest/digest
POPITAM_OBJDIR		= ./obj
POPITAM            	= ./bin/popitam
POPITAM_NAME		= popitam-$(VERSION)
DIST_BASEDIR		= ./dist
DIST			= $(DIST_BASEDIR)/popitam-$(VERSION)

OBJECTS 		= atomicmass file MATerror tagfile txtfile util \
			  actree dbentry  dbfile dbfilereader dbptm dbreader \
			  digest mcmanager modifmanager peptidebase peptidedigest ptmmanager  \
			  ioparam ioversion \
			  aa about allrunstatistics ants clicsearch compare data element error \
			  fun gnb gpparameters gptree gptreenode graphe ion main memorycheck peptide \
			  prot protEl result runManager runManagerParameters scenario \
			  score Sequence setlib0 specresults spectrumstatistics subseq sufftab \
			  tag_extractor tag_processor Tree usage utils vertex

RM			= /bin/rm -rf
MKDIR			= /bin/mkdir -p

all:	$(POPITAM_OBJDIR) $(POPITAM) make_subprojects

# Generate the Popitam distribution
dist:	all
	@mkdir -p $(DIST)
	@cp -r ./bin $(DIST)
	@cp -r ./data $(DIST)
	@cp -r ./docs $(DIST)
	@cp -r ./examples $(DIST)
	@mkdir $(DIST)/dbs
	@tar --exclude=.svn -C $(DIST_BASEDIR) -a -cvf $(DIST_BASEDIR)/$(POPITAM_NAME).tar.gz $(POPITAM_NAME)
	@/bin/rm -rf $(DIST_BASEDIR)/$(POPITAM_NAME)

clean:	clean_obj clean_subprojects
	$(RM) $(POPITAM)
	$(RM) $(DIST_BASEDIR)

clean_obj:	clean_subprojects_obj
	$(RM) $(POPITAM_OBJDIR)

clean_subprojects_obj:
	@make -C ./createDB clean_obj

clean_subprojects:
	@make -C ./createDB clean

make_subprojects:
	@make -C ./createDB

help: 
	@echo "Three options modify default compilation, i.e. debug (use OPTION=...)"
	@echo "    static   or s for static compiling "
	@echo "    profile  or p for profiling informations "
	@echo "    optimize or o for optimized compilation "
	@echo "Some paths can also be modified during compilation " 
	@echo "    DATA  folder containing data files "
	@echo "    DB    folder containing databases, i.e. UniProtSP and UniProtTR folders"
	
$(POPITAM_OBJDIR):
	mkdir -p $(POPITAM_OBJDIR)

$(POPITAM): $(foreach x,$(OBJECTS),$(POPITAM_OBJDIR)/$(x).o)   # met chaque object dans x, et cr�er le nom complet 
	$(CC) $(OPT) -o $@ $^
	/usr/bin/strip -s $(POPITAM)

$(POPITAM_OBJDIR)/%.o: %.cpp      # prend tous les .cpp dans les chemins indiqu�s dans VPATH et cree les .o
	$(CC) -o $@ $(INCLUDE_DIR) $(CFLAGS) -DPOP_VERSION=$(VERSION_STR) -DDEF_DATA_FOLDER=$(DATA_STR) -DDEF_DB_PATH=$(DB) $(OPT) -c $< 

# le $@ r�f�re � la target courante, donc $(POPITAM_OBJDIR)/%.o
# le $< r�f�re au prerequis courant qui a �t� modifi�e plus r�cemment que la target courante
