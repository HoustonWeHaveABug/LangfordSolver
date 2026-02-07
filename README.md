# LangfordSolver
Generalized Langford problem solver for given range and order

## langford_solver

Parameters read on standard input: order range_inf range_sup \[ hooks_n \] \[ settings \] \[ sentinel \] \[ plans_n \]

- order (2 for pairs, 3 for triplets, etc...)
- range_inf: range inferior bound (>= 1)
- range_sup: range superior bound (>= range inferior bound)
- hooks_n: number of hooks (= 0 search for perfect solutions only, > 0 otherwise)
- settings = sum of option settings (colombian solutions only = 1, planar solutions only = 2, first solution only = 4, circular mode = 8, verbose mode = 16)
- sentinel: argument for the colombian variant
- plans_n: number of dimensions, argument for the planar variant

Langford standard problem L(s, n) is solved using parameters (s, 2, n+1, 0, 0).

Nickerson variant V(s, n) is solved using parameters (s, 1, n, 0, 0).

It will perform the search given the problem parameters and settings using Knuth's DLX algorithm. The necessary conditions
for existence of sequences are performed as presented in [this paper](https://pdfs.semanticscholar.org/b999/9a360a3a1d2663149b56338a62eaa113f819.pdf).

At the end of execution it will print the final cost (size of the search tree) and the total number of solutions found.

## langford_solver_count_planars.sh

Bash script that can be used to count the number of planar solutions from the output provided by the solver (launched with option settings planar solutions only and verbose mode).

## langford_pairs.sh / langford_triplets.sh

Bash scripts that can be used to run the solver for each valid length of the Langford sequence, starting from the lowest until a given superior bound.

## Results (langford_\*\.txt files)

Environment: Linux 5.4.12-arch1-1

CPU: Intel Core i5-4430 3.0 GHz - One core used
