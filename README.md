# LangfordSolver
Generalized Langford problem solver for given range and order

## langford_solver

The program reads the following parameters on standard input:
- Order (2 for pairs, 3 for triplets, etc...)
- Range inferior bound (>= 1)
- Range superior bound (>= Range inferior bound)
- Number of hooks (= 0 search for perfect solutions only, > 0 otherwise)
- Sum of option flags (1 = search for planar solutions only, 2 = stop execution after first solution found, 4 = print current cost and sequence for each solution found)

Langford standard problem L(s, n) is solved using parameters (s, 2, n+1, 0, 0).

Nickerson variant V(s, n) is solved using parameters (s, 1, n, 0, 0).

It will perform the search given the problem parameters and options using Knuth's DLX algorithm.

At the end of execution it will print the final cost (size of the search tree) and the total number of solutions found.

## langford_\*\_first_only.sh

Shell scripts that can be used to find the first solution for each valid length of the Langford sequence, starting from the lowest until a given superior bound.

## langford_pairs_count_planars_only.sh

When searching for planar solutions only, the solver will count as a solution every possible ways to link the numbers inside a group. This shell script encapsulates the solver to remove duplicate solutions and gives the expected count for P(s, n) and variants.

## Results

| Order | Range | Planars only / All | First only / All | Time | Output |
| ----- | ----- | ------------------ | ---------------- | ---- | ------ |
| 2 | 1-4 to 1-601 | All | First only | 1m | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_pairs_first_only_601.txt |
|| 2-4 to 2-601 |||||
| 2 | 2-20 | All | All | 156h | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_2_2_20_0_0.txt |
| 3 | 2-20 | All | All | 4m40s | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_3_2_20_0_0.txt |
| 4 | 1-24 | All | All | 1h30m | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_4_1_24_0_4.txt |
| 4 | 1-25 | All | All | 6h35m | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_4_1_25_0_4.txt |
| 4 | 1-32 | All | First only | 3h12m | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_4_1_32_0_6.txt |
| 4 | 1-33 | All | First only | 7m15s | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_4_1_33_0_6.txt |
| 4 | 2-25 | All | All | 50m | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_4_2_25_0_4.txt |
| 4 | 2-32 | All | First only | 1h20m | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_4_2_32_0_6.txt |
| 4 | 2-33 | All | First only | 35m | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_solver_4_2_33_0_6.txt |
| 3 | 1-9 to 1-407 | All | First only | 30s | https://github.com/HoustonWeHaveABug/LangfordSolver/blob/master/langford_triplets_first_only_407.txt |
|| 2-9 to 2-407 |||||
