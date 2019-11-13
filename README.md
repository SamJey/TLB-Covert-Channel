# TLB-Covert-Channel
Exploitation of architecture of TLB to allow two processes to communicate course grained information based on access 
times of page-mappings in TLB.

# Task1
Internal structure(number of levels, sets, ways) are first reverse engineered using AMAT of page accesses.

# Task2
Using information from Task 1, wo processes on the same core try to access the same set simultaneously to establish a communication channel.

# Task3
The two processes take up separate reader/writer roles, and information is passes along the channel.

