# Computer Architecture Projects (CS6600)

This repository contains projects done as part of CS6600 Computer Architecture Course at IIT Madras offered in Jul-Nov 2024.

## 1. Cache Simulator 
- A generic cache and memory hierarchy simulator designed to evaluate and compare various memory hierarchy configurations.
- Key parameters such as like Miss Rate, Average Access Time(AAT), Energy Delay Product(EDP) are analyzed and how they vary across different Cache parameters like Size, Blocksize, Associativity and trade offs is explored.
- [View detailed report](CacheSimulator/Report.pdf)

## 2. Branch Predictor
- A generic branch predictor to understand the behaviour of the actual branch predictors which are used in actual hardware.
- [View detailed report](BranchPredictor/Report.pdf)

## 3. Dynamic Scheduling
- A simulator for an Out-of-Order superscalar processor based on Tomasulo’s algorithm (discussed in class) that fetches, dispatches, and issues N instructions per cycle.
- The primary goal is to only model the dynamic scheduling mechanism in detail. Hence, perfect caches and perfect branch prediction are assumed.
- [View detailed report](DynamicScheduling/Report.pdf)
