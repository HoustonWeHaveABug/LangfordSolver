LANGFORD_SOLVER_C_FLAGS=-O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

langford_solver: langford_solver.o
	gcc -o langford_solver langford_solver.o

langford_solver.o: langford_solver.c langford_solver.make
	gcc -c ${LANGFORD_SOLVER_C_FLAGS} -o langford_solver.o langford_solver.c

clean:
	rm -f langford_solver langford_solver.o
