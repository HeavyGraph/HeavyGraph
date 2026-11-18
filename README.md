## HeavyGraph
### Introduction
A graph stream is a dynamic data stream composed of sequential two-dimensional items, forming an evolving graph with nodes and edges. Discovering heavy patterns, including frequent nodes and their key edges, is crucial for applications such as network attack detection, social network analysis, and traffic management. However, existing sketches either handle one-dimensional items or focus on graph-frequency summarization; extending them to diverse node-edge heavy-pattern queries typically requires memory-intensive task-specific auxiliary structures and costly update operations. In this paper, we introduce \emph{HeavyGraph}, a sketch designed for heavy pattern detection in graph streams. Its main contributions are threefold: (1) a two-stage framework consisting of a general filter and a specific heavy detector supports multiple heavy-pattern queries over nodes and their incident edges, e.g., heavy-hitter and Top-$k$ detection; (2) the general filter employs an asymmetric two-dimensional compact matrix with fingerprints to improve memory utilization; and (3) the specific heavy detector uses local in-bucket descending updates to incrementally maintain frequent nodes and their key edges, avoiding global heap maintenance and improving accuracy and throughput. Experiments on a real network trace show that HeavyGraph improves the F1 score by up to 0.73/0.34 and achieves up to $46.2\times$/$22.3\times$ the throughput of GSS-based/DAS-based baselines. A DDoS-aware traffic control case study shows that its key-edge output reduces the collateral damage by about 99.5\% compared with node-only detection solutions.

### Datasets
Our code runs on the following dataset:

-[Caida](./Data/caida.dat)

-[Caida_Weight](./Data/caida_weight.dat)

-[Campus](./Data/campus.dat)

-[Campus_Weight](./Data/campus_weight.dat)

-[DDoS_Caida](./Data/ddos_caida.dat)

-[DDoS_Campus](./Data/ddos_campus.dat)

Please use `git lfs pull` to pull these datasets.

### How to Run
Suppose you have already navigated into the project directory.
```sh
cmake ./
make
./main 
./ddos
```
