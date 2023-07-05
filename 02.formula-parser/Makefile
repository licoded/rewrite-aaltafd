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
CFLAGS				=	-Wall $(CFLAG_INCLUDE_DIRS) $(CFLAG_nonconforming) $(CFLAG_ignore_Warn)
DEBUGFLAGS			=	-D DEBUG -g -pg

# ===	MINISAT		===
MINISAT_SOLVER_FILE	=	minisat/core/Solver.cc
MINISAT_TARGETS	=	minisat_solver.o

# ===	LTLPARSER	===
LTL_FORMULA_FILE	=	ltl_formula.c
LTL_PARSER_FILE		=	ltlparser.c
LTLPARSER_TARGETS	=	ltl_formula.o ltl_parser.o

# g++ formula/aalta_formula.cpp -I./ -c -o tmp/aalta_formula.o

.PHONY : clean hello depend

all: depend hello

hello:
	echo $(OBJS)

main:			$(OBJS)
	$(CC) main.cpp $(CFLAGS) -c -o $(TARGET_DIR)/main.o
	$(CC) $(TARGET_DIR)/main.o $(addprefix $(TARGET_DIR)/, $(MINISAT_TARGETS)) $(addprefix $(TARGET_DIR)/, $(LTLPARSER_TARGETS)) $^ $(CFLAGS) $(DEBUGFLAGS) -lz -o aaltafd

# ===	MINISAT		===
minisat_build:	$(MINISAT_TARGETS:.o=)

minisat_solver:	$(MINISAT_SOLVER_FILE)
	$(CC) $^ $(CFLAGS) -c -o $(TARGET_DIR)/$@.o


# ===	LTLPARSER	===
ltlparser_build: $(LTLPARSER_TARGETS:.o=)

# ltl_formula:	$(TARGET_DIR)/ltl_formula
# $(TARGET_DIR)/ltl_formula: $(LTLPARSER_DIR)/$(LTL_FORMULA_FILE)
# 	$(CC) $^ $(CFLAGS) -c -o $@.o

ltl_formula:	$(LTLPARSER_DIR)/$(LTL_FORMULA_FILE)
	$(CC) $^ $(CFLAGS) -c -o $(TARGET_DIR)/$@.o

# ltl_parser:	$(TARGET_DIR)/ltlparser
# $(TARGET_DIR)/ltlparser: $(LTLPARSER_DIR)/$(LTL_PARSER_FILE)
# 	$(CC) $^ $(CFLAGS) -c -o $@.o

ltl_parser:		$(LTLPARSER_DIR)/$(LTL_PARSER_FILE)
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

clean :
	rm $(PROG) $(OBJS)

aaltasolver.o: aaltasolver.cpp 