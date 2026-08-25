## HeavyGraph
### Introduction
In this paper, we introduce Heavy-
Graph, a novel sketch designed for heavy pattern detection in graph
streams. The main contributions of HeavyGraph are threefold: (1)
we use a two-stage framework that integrates a general filter with
a specific heavy detector for identifying various heavy patterns,
such as heavy hitters and Top-𝑘 patterns for nodes and edges; (2)
the general filter employs an asymmetric two-dimensional compact
matrix with fingerprints, achieving high memory utilization; (3)
the specific heavy detector applies a local in-bucket descending
update that incrementally maintains the frequent nodes and their
key edges, improving both accuracy and throughput. Extensive ex-
periments across a series of datasets demonstrate that HeavyGraph
significantly outperforms the strawman solution in both error and
throughput performance, achieving improvements of up to two
orders of magnitude. In addition, we integrate HeavyGraph into a
DDoS-aware traffic control simulation system, where it achieves
near-perfect attack traffic coverage while significantly reducing
collateral damage to legitimate traffic.

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