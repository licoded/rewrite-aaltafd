
FORMULAFILES =	formula/aalta_formula.cpp
	
PARSERFILES  =	ltlparser/ltl_formula.c ltlparser/ltllexer.c ltlparser/ltlparser.c ltlparser/trans.c 

ALLFILES     =	$(FORMULAFILES) $(PARSERFILES) main.cpp
TESTFILES     =	$(FORMULAFILES) $(PARSERFILES) tests/formula/main.cpp


CC	    =   g++
FLAG    = -I./  -I./minisat/  -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS -fpermissive -isystem minisat -isystem ltlparser
DEBUGFLAG   =	-D DEBUG -g -pg
RELEASEFLAG = -O2

aaltaf :	release

ltlparser/ltllexer.c :
	ltlparser/grammar/ltllexer.l
	flex ltlparser/grammar/ltllexer.l

ltlparser/ltlparser.c :
	ltlparser/grammar/ltlparser.y
	bison ltlparser/grammar/ltlparser.y
	
	

.PHONY :    release debug clean

release :   $(ALLFILES)
	    $(CC) $(FLAG) $(RELEASEFLAG) $(ALLFILES) -lz -o aaltaf

debug :	$(ALLFILES)
	$(CC) $(FLAG) $(DEBUGFLAG) $(ALLFILES) -lz -o aaltafd

test : 	$(ALLFILES)
	$(CC) $(FLAG) $(DEBUGFLAG) $(TESTFILES) -lz -o test

clean :
	rm -f *.o *~ aaltaf
