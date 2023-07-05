SRCS				= 	$(wildcard *solver.cpp) $(wildcard *checker.cpp) main.cpp
TARGET_DIR			= 	tmp
LTLPARSER_DIR		=	ltlparser
_OBJS				= 	$(SRCS:.cpp=.o)
OBJS				=	$(addprefix $(TARGET_DIR)/, $(_OBJS))

PROG				=	aaltaf
CC					=	g++
CFLAG_INCLUDE_DIRS	=	-I ./ -I ./minisat
CFLAG_nonconforming	=	-fpermissive
CFLAG_ignore_Warn	=	-isystem minisat -isystem ltlparser
DEBUGFLAGS			=	-D DEBUG -g -pg
CFLAGS				=	-Wall $(CFLAG_INCLUDE_DIRS) $(CFLAG_nonconforming) $(CFLAG_ignore_Warn)
CFLAG_HJSON			=	-lhjson

# ===	MINISAT		===
MINISAT_SOLVER_FILE	=	minisat/core/Solver.cc
MINISAT_TARGETS	=	minisat_solver.o

# ===	LTLPARSER	===
LTL_FORMULA_FILE	=	ltl_formula.c
LTL_PARSER_FILE		=	ltlparser.c
LTL_TRANS_FILE		=	trans.c
LTLPARSER_TARGETS	=	ltl_formula.o ltl_parser.o ltl_trans.o
PARSER_FILES		=	ltlparser/ltl_formula.c ltlparser/ltllexer.c ltlparser/ltlparser.c ltlparser/trans.c 

# ===	FORMULA		===
FORMULA_FILE	=	formula/aalta_formula.cpp
FORMULA_TARGETS	=	aalta_formula.o

# ===	MYHJSON		===
MYHJSON_FILE	=	myhjson.cpp
MYHJSON_TARGETS	=	myhjson.o

# g++ formula/aalta_formula.cpp -I./ -c -o tmp/aalta_formula.o

.PHONY : clean depend

# first run `make depend` and paste/replace in the end of this file (Makefile)
all: main

maind:			$(OBJS) $(PARSER_FILES)
	$(CC)	\
		$(addprefix $(TARGET_DIR)/, $(MINISAT_TARGETS))		\
		$(addprefix $(TARGET_DIR)/, $(FORMULA_TARGETS))		\
		$(addprefix $(TARGET_DIR)/, $(MYHJSON_TARGETS))		\
		$^ $(CFLAGS) $(CFLAG_HJSON) $(DEBUGFLAGS) -lz -o aaltafd

main:			$(SRCS) $(PARSER_FILES) $(FORMULA_FILE) $(MYHJSON_FILE) $(MINISAT_SOLVER_FILE)
	$(CC)	\
		$^ $(CFLAGS) $(CFLAG_HJSON) -lz -o aaltaf

# test aalta_formula
test-af-main:		tests/formula/main.cpp formula_build
	$(CC)	\
		$(addprefix $(TARGET_DIR)/, $(FORMULA_TARGETS))		\
		$< $(PARSER_FILES) $(CFLAGS) -lz -o $@

test-af-tostring:	tests/formula/to_string.cpp formula_build
	$(CC)	\
		$(addprefix $(TARGET_DIR)/, $(FORMULA_TARGETS))		\
		$< $(PARSER_FILES) $(CFLAGS) -lz -o $@

# ===	MINISAT		===
minisat_build:	$(MINISAT_TARGETS:.o=)

minisat_solver:	$(MINISAT_SOLVER_FILE)
	$(CC) $^ $(CFLAGS) -c -o $(TARGET_DIR)/$@.o


# ===	FORMULA		===
formula_build:	$(FORMULA_TARGETS:.o=)

aalta_formula:	$(FORMULA_FILE)
	$(CC) $^ $(CFLAGS) -c -o $(TARGET_DIR)/$@.o


# ===	MYHJSON		===
myhjson_build:	$(MYHJSON_TARGETS:.o=)

myhjson:	$(MYHJSON_FILE)
	$(CC) $^ $(CFLAGS) -c -o $(TARGET_DIR)/$@.o


ltlparser/ltllexer.c :
	ltlparser/grammar/ltllexer.l
	flex ltlparser/grammar/ltllexer.l

ltlparser/ltlparser.c :
	ltlparser/grammar/ltlparser.y
	bison ltlparser/grammar/ltlparser.y


depend: $(SRCS)
	rm -f "$@"
	$(CC) $(CFLAGS) -MM $^ > ".$@"
	sed -i '/^[^ ]/ s/^/tmp\//' ".$@"

clean :
	rm $(PROG) $(OBJS)


tmp/aaltasolver.o: aaltasolver.cpp aaltasolver.h minisat/core/Solver.h \
 formula/aalta_formula.h ltlparser/ltl_formula.h
	$(CC) $< $(CFLAGS) -c -o $@
tmp/carsolver.o: carsolver.cpp carsolver.h solver.h aaltasolver.h \
 minisat/core/Solver.h formula/aalta_formula.h ltlparser/ltl_formula.h \
 transition.h
	$(CC) $< $(CFLAGS) -c -o $@
tmp/invsolver.o: invsolver.cpp invsolver.h aaltasolver.h \
 minisat/core/Solver.h formula/aalta_formula.h ltlparser/ltl_formula.h
	$(CC) $< $(CFLAGS) -c -o $@
tmp/solver.o: solver.cpp solver.h aaltasolver.h minisat/core/Solver.h \
 formula/aalta_formula.h ltlparser/ltl_formula.h transition.h
	$(CC) $< $(CFLAGS) -c -o $@
tmp/ltlfchecker.o: ltlfchecker.cpp ltlfchecker.h formula/aalta_formula.h \
 ltlparser/ltl_formula.h solver.h aaltasolver.h minisat/core/Solver.h \
 transition.h
	$(CC) $< $(CFLAGS) -c -o $@
tmp/carchecker.o: carchecker.cpp carchecker.h ltlfchecker.h \
 formula/aalta_formula.h ltlparser/ltl_formula.h solver.h aaltasolver.h \
 minisat/core/Solver.h transition.h carsolver.h invsolver.h
	$(CC) $< $(CFLAGS) -c -o $@
tmp/main.o: main.cpp formula/aalta_formula.h ltlparser/ltl_formula.h \
 ltlfchecker.h solver.h aaltasolver.h minisat/core/Solver.h transition.h \
 carchecker.h carsolver.h invsolver.h
	$(CC) $< $(CFLAGS) -c -o $@