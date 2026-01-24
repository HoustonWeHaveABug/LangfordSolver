LANGFORD_SOLVER_C_FLAGS=-c -fPIE -fsanitize=bounds -fsanitize-undefined-trap-on-error -fstack-clash-protection -fstack-protector-strong -O2 -std=c89 -Waggregate-return -Wall -Walloca -Warith-conversion -Warray-bounds=2 -Wbad-function-cast -Wcast-align=strict -Wcast-qual -Wconversion -Wduplicated-branches -Wduplicated-cond -Werror -Wextra -Wfloat-equal -Wformat=2 -Wformat-overflow=2 -Wformat-security -Wformat-signedness -Wformat-truncation=2 -Wimplicit-fallthrough=3 -Winline -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro -Wl,-z,separate-code -Wlogical-op -Wlong-long -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wnull-dereference -Wold-style-definition -Wpedantic -Wpointer-arith -Wredundant-decls -Wshadow -Wshift-overflow=2 -Wstack-protector -Wstack-usage=1000000 -Wstrict-overflow=4 -Wstrict-prototypes -Wstringop-overflow=4 -Wswitch-default -Wswitch-enum -Wtraditional-conversion -Wtrampolines -Wundef -Wvla -Wwrite-strings
LANGFORD_SOLVER_OBJS=langford_solver.o

langford_solver: ${LANGFORD_SOLVER_OBJS}
	gcc -o langford_solver ${LANGFORD_SOLVER_OBJS}

langford_solver.o: langford_solver.c langford_solver.make
	gcc ${LANGFORD_SOLVER_C_FLAGS} -o langford_solver.o langford_solver.c

clean:
	rm -f langford_solver ${LANGFORD_SOLVER_OBJS}
