#!/usr/bin/env python3
"""
gplearn baseline on the same 1-D Nguyen datasets the LGP engine used.
Writes results/gplearn_<target>_seed<seed>.csv (one row each, no header),
matching the C++ schema: target,tool,run_seed,train_r2,test_r2,best_length

Confirmed there is no elitism parameter in gplearn
"""
import os
import numpy as np
import pandas as pd
from gplearn.genetic import SymbolicRegressor

TARGETS = ["nguyen1", "nguyen2", "nguyen3", "nguyen4", "nguyen5", "nguyen6"]
SEEDS   = list(range(25))

# Match d1 where the concept exists; gplearn defaults elsewhere (see 2.1).
POP_SIZE     = 1000
GENERATIONS  = 1000
TOURNAMENT   = 3
FUNCTION_SET = ("add", "sub", "mul", "div", "sin", "cos")  # = d1 ISA ops used
# it seems that lt and gt are not in the function set of gplearn upon basic search, although this was not confirmed within the terminal itself

os.makedirs("results", exist_ok=True)


def r2(y, yhat):
    """Coefficient of determination — matches d1's evaluator."""
    ss_res = np.sum((y - yhat) ** 2)
    ss_tot = np.sum((y - np.mean(y)) ** 2)
    if ss_tot <= 0:
        return 0.0
    return 1.0 - ss_res / ss_tot


def program_length(est):
    """gplearn program length = number of nodes in the expression tree."""
    return int(est._program.length_)


for target in TARGETS:
    train = pd.read_csv(f"datasets/{target}_train.csv")
    test  = pd.read_csv(f"datasets/{target}_test.csv")
    Xtr, ytr = train[["x"]].values, train["y"].values
    Xte, yte = test[["x"]].values,  test["y"].values

    for seed in SEEDS:
        est = SymbolicRegressor(
            population_size=POP_SIZE,
            generations=GENERATIONS,
            tournament_size=TOURNAMENT,
            function_set=FUNCTION_SET,
            metric="mse",        # gplearn optimises MSE internally;
            random_state=seed,   # we compute & report R² ourselves below
            n_jobs=1,
            verbose=0,
            init_depth=(2,5),
            parsimony_coefficient=0.001,
            #added init depth and parsimony coefficient to match d1's settings, see doc 2.1. note that parsimony coefficient punishes trees that grow excessively large (bloat)
            # crossover / mutation rates: gplearn defaults (no LGP analogue)
            # no elitism knob exists on SymbolicRegressor (see doc 2.1) --confirmed by Spencer
        )
        est.fit(Xtr, ytr)

        train_r2 = r2(ytr, est.predict(Xtr))
        test_r2  = r2(yte, est.predict(Xte))
        length   = program_length(est)

        out = f"results/gplearn_{target}_seed{seed}.csv"
        with open(out, "w") as f:
            f.write(f"{target},gplearn,{seed},{train_r2},{test_r2},{length}\n")
        print(f"{target} seed {seed}: train_r2={train_r2:.4f} test_r2={test_r2:.4f}")
        