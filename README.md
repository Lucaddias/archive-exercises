# ZIP Code Search **Brazil** in C (Sequential vs. Binary) 

This project was developed in **C** to demonstrate, in practice, the manipulation of large binary files and the drastic performance difference between the **Sequential Search** and **Binary Search** algorithms.

The program reads a database file (`.dat`) containing hundreds of thousands of Brazilian address records and retrieves the complete information for a specific ZIP code (CEP) entered by the user.

## The Problem and the Solution

The database file used (`cep_ordenado.dat`) and (`cep.dat`). Each record is a `struct` with exactly 300 bytes.

* **Sequential Search (`SequentialZipSearch.c`):** The algorithm reads the file line by line from the very beginning. In the worst-case scenario (if the ZIP code does not exist or is the last entry), the program performs **699,306 disk reads**, which consumes a significant amount of processing time.
* **Binary Search (`BinaryZipSearch.c`):** The algorithm takes advantage of the fact that the file is sorted. It divides the search space in half at each iteration, using the `fseek` and `fread` commands to "jump" directly to the calculated position. The exact same ZIP code is found (or completely discarded) in a maximum of **20 reads**.
