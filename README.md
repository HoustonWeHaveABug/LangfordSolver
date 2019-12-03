# LangfordSolver
Langford problem solver for given range and order

The program reads the following parameters on standard input:
- Range inferior bound (= 1 for standard problem, > 1 otherwise)
- Range superior bound (>= Range inferior bound)
- Order (2 for pairs, 3 for triplets, etc...)
- Verbose flag (1: print all solutions, 0: print only first solution then stay silent until the end of execution)

It will search for all the solutions given the problem parameters using Knuth's DLX algorithm.

At the end of execution it will print the cost (size of the search tree) and the total number of solutions found.
