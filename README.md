# Combinatorial Optimization for Tourist Attraction Selection

[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Course: IIA](https://img.shields.io/badge/Course-Artificial_Intelligence-orange.svg)](https://www.isec.pt)
[![Institution: ISEC](https://img.shields.io/badge/Institution-ISEC_Coimbra-red.svg)](https://www.isec.pt)

> [!NOTE]  
> **Academic Context:**  
> Developed as part of the **Introduction to Artificial Intelligence (Introdução à Inteligência Artificial - IIA)** curricular unit in the **BSc in Computer Science (LEI)** at the **Polytechnic University of Coimbra (ISEC)**, Academic Year 2025/2026.

---

## 📌 Problem Overview

This project tackles the **Maximum Diversity Problem (MDP) / Subset Selection Problem** applied to tourism datasets. Given a set of $ candidate tourist locations and an  \times N$ pairwise distance/dissimilarity matrix, the goal is to select an optimal subset of $ locations that **maximizes total pairwise diversity / distance**, defined by the objective function:

\\max \\sum_{i=1}^{m-1} \\sum_{j=i+1}^{m} \\text{dist}(s_i, s_j)

where , s_j$ are distinct locations included in the selected subset of size $.

---

## 🧠 Implemented Algorithms & Metaheuristics

### 1. Local Search: Hill Climbing (*Trepa-Colinas*)
Iterative improvement search exploring neighboring solutions:
- **Single Swap Neighborhood:** Randomly replaces one selected candidate with an unselected candidate.
- **Double Swap Neighborhood:** Concurrently swaps two selected candidates with two unselected candidates to escape shallow local optima.

### 2. Evolutionary / Genetic Algorithm
Population-based optimization with customizable genetic operators:
- **Selection Operators:**
  - *Tournament Selection:* Compares fitness between randomly sampled individuals.
  - *Roulette Wheel Selection:* Fitness-proportionate probabilistic selection.
- **Crossover Operators:**
  - *Single-Point Crossover:* Cuts and recombines parent chromosomes at a random split point.
  - *Uniform Crossover:* Probabilistically inherits genes from each parent per locus.
- **Mutation & Feasibility Repair:**
  - Swap mutation with chromosome repair mechanisms to guarantee valid subsets (no duplicate locations, strictly $ elements).

### 3. Memetic / Hybrid Algorithms
Combines the global exploration of the Evolutionary Algorithm with localized exploitation via Hill Climbing:
- **Hybrid 1:** Intense local search applied to the best-of-generation individual.
- **Hybrid 2:** Lightweight local search applied across generated offspring.

---

## 📊 Datasets & Experimental Evaluation

The system was evaluated across datasets of varying dimensional scale:
* 	ourism_5.txt
* 	ourism_20.txt
* 	ourism_50.txt
* 	ourism_100.txt
* 	ourism_250.txt
* 	ourism_500.txt

Detailed statistical analyses, convergence plots, and comparison tables across multiple runs (average fitness, best fitness, standard deviation) are documented in [IIA_TP2_Resultados.xlsx](./IIA_TP2_Resultados.xlsx) and the technical report [Relatório_TP2_IIA.pdf](./Relat%C3%B3rio_TP2_IIA.pdf).

---

## 🚀 Compilation and Execution

### Using GCC (Command Line)
`ash
# Navigate to the source folder
cd untitled

# Compile with optimization flags
gcc -O2 main.c -o optimizer

# Run the interactive console
./optimizer
`

### Using CMake / CLion
The project includes a ready-to-use [CMakeLists.txt](./untitled/CMakeLists.txt):
`ash
mkdir build && cd build
cmake ..
cmake --build .
./untitled
`

---

## 👥 Authors

* **Rodrigo Salgueiro Ideia** — Student ID: 2024140892 ([GitHub](https://github.com/rodrigoideia19))
* **Gustavo Vicente Jesus** — Student ID: 2024140853

*BSc in Computer Science (LEI-CE / LEI), Instituto Superior de Engenharia de Coimbra (ISEC)*
