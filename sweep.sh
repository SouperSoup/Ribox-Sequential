#!/bin/bash
#SBATCH --job-name=lgp_sweep
#SBATCH --array=0-149
                                     # 6 targets × 3 seeds = 18 cells (indices 0..17)
#SBATCH --time=00:10:00              # generous; each cell is seconds-to-minutes
#SBATCH --mem=512M                   # 20k-program buffers are tiny
#SBATCH --cpus-per-task=1            # engine is single-threaded
#SBATCH --output=logs/%A_%a.out      # %A = array job id, %a = task id

set -euo pipefail                    # fail loudly: unset var or failed command aborts the cell

# --- sweep definition ---
targets=(nguyen1 nguyen2 nguyen3 nguyen4 nguyen5 nguyen6)
seeds=(0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 42 100 99 67 1000)

# --- map the 1-D array index to a (target, seed) cell ---
n_seeds=${#seeds[@]}
t_idx=$(( SLURM_ARRAY_TASK_ID / n_seeds ))   # integer division -> target
s_idx=$(( SLURM_ARRAY_TASK_ID % n_seeds ))   # remainder        -> seed

target=${targets[$t_idx]}
seed=${seeds[$s_idx]}

# --- ensure output dirs exist (main.cpp also creates them, belt-and-suspenders) ---
mkdir -p results programs logs

echo "cell ${SLURM_ARRAY_TASK_ID}: target=${target} seed=${seed} on $(hostname)"

# --- run one cell ---
./lgp_run \
    datasets/${target}_train.csv \
    datasets/${target}_test.csv \
    ${target} \
    ${seed} \
    results/${target}_seed${seed}.csv \
    programs/${target}_seed${seed}.txt