LANGFORD_SOLVER_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings
LANGFORD_SOLVER_OBJS=langford_solver.o mp_utils.o

langford_solver: ${LANGFORD_SOLVER_OBJS}
	gcc -o langford_solver ${LANGFORD_SOLVER_OBJS}

langford_solver.o: langford_solver.c langford_solver.make
	gcc -c ${LANGFORD_SOLVER_C_FLAGS} -o langford_solver.o langford_solver.c

mp_utils.o: mp_utils.h mp_utils.c langford_solver.make
	gcc -c ${LANGFORD_SOLVER_C_FLAGS} -o mp_utils.o mp_utils.c

clean:
	rm -f langford_solver ${LANGFORD_SOLVER_OBJS}
