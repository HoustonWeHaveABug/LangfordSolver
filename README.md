# LangfordSolver
Generalized Langford problem solver for given range and order

## langford_solver

The program reads the following parameters on standard input:
- Order (2 for pairs, 3 for triplets, etc...)
- Range inferior bound (>= 1)
- Range superior bound (>= Range inferior bound)
- Option flags (0 = search for all solutions / stay silent until end of execution, 1 = stop execution after first solution found, 2 = print current cost and sequence for each solution found, 3 = combine flags 1 and 2)

Langford standard problem L(s, n) is solved using parameters (s, 2, n+1).

Nickerson variant V(s, n) is solved using parameters (s, 1, n).

It will perform the search given the problem parameters and options using Knuth's DLX algorithm.

At the end of execution it will print the final cost (size of the search tree) and the total number of solutions found.

## langford_\*\_first_only.sh

Shell scripts that can be used to find the first solution for each valid length of the Langford sequence, starting from the lowest until a given superior bound.
