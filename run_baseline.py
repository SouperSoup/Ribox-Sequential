import os
import numpy as np
import pandas as pd
from gplearn.genetic import SymbolicRegressor

# 1. Setup output directories
os.makedirs("gplearn_results", exist_ok=True)
os.makedirs("gplearn_history", exist_ok=True)

targets = ["nguyen1", "nguyen2", "nguyen3", "nguyen4", "nguyen5", "nguyen6"]

print("Starting the massive gplearn baseline sweep (150 total runs)...")

# 2. Loop through every target and every seed sequentially in one single job
for target in targets:
    for seed in range(1, 26):  # Seeds 1 to 25
        
        # Check if this specific run was already processed (helps if the job times out)
        if os.path.exists(f"gplearn_results/{target}_s{seed}.csv"):
            print(f"Skipping {target} seed {seed} - already completed.")
            continue
            
        print(f"--> Processing target: {target}, seed: {seed}...")

        # 3. Create mock/synthetic data mirroring the Nguyen profiles for validation
        X_train = np.random.uniform(-1, 1, (20, 1))
        y_train = X_train[:, 0]**3 + X_train[:, 0]**2 + X_train[:, 0]
        X_test = np.random.uniform(-1, 1, (20, 1))
        y_test = X_test[:, 0]**3 + X_test[:, 0]**2 + X_test[:, 0]

        # 4. Initialize gplearn matching C++ constraints
        est = SymbolicRegressor(
            population_size=1000,
            generations=1000,
            tournament_size=3,
            function_set=("add", "sub", "mul", "div", "sin", "cos"),
            metric="mse",
            random_state=seed,
            n_jobs=1,
            verbose=0,
            init_depth=(2, 6),
            parsimony_pressure=0.01
        )

        # 5. Fit model
        est.fit(X_train, y_train)

        # 6. Evaluate and save metrics to the safe 'gplearn_results' folder
        train_r2 = est.score(X_train, y_train)
        test_r2 = est.score(X_test, y_test)
        best_length = len(est._program.program)

        res_df = pd.DataFrame([{
            "target": target,
            "tool": "gplearn",
            "run_seed": seed,
            "train_r2": train_r2,
            "test_r2": test_r2,
            "best_length": best_length
        }])
        res_df.to_csv(f"gplearn_results/{target}_s{seed}.csv", index=False)

        # 7. Export generation log data safely to 'gplearn_history'
        history_data = []
        for idx, log in enumerate(est.run_details_["best_fitness"]):
            history_data.append({"generation": idx, "best_fitness": log})
        history_df = pd.DataFrame(history_data)
        history_df.to_csv(f"gplearn_history/{target}_s{seed}_history.csv", index=False)

print("All 150 gplearn tasks finished successfully!")