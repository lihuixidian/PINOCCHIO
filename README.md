# PINOCCHIO
A movement-aware location selection algorithm (published in TKDE16, ICDE17)

Given the movement history (e.g., historical trajactory/check-in logs) for massive users, how to find a location, from a group of candidates, to place some facility, such that the new facility can probably cover the most number of moving users. We present a novel model, namely PINOCCHIO, that can address the aforementioned problem efficiently. The results of this work have been published in the following papers, please cite the following appropriately if you are using the code for PINOCCHIO.

- PINOCCHIO: Probabilistic Influence-Based Location Selection over Moving Objects.    Wang, M.; Li, H.; Cui, J.; Deng, K.; Bhowmick, S. S.; and Dong, Z.   IEEE Trans. Knowl. Data Eng., 28(11): 3068–3082. 2016.
- PINOCCHIO: Probabilistic Influence-Based Location Selection over Moving Objects.    Wang, M.; Li, H.; Cui, J.; Deng, K.; Bhowmick, S. S.; and Dong, Z.   In 33rd IEEE International Conference on Data Engineering, ICDE 2017, San Diego, CA, USA, April 19-22, 2017, pages 21–22, 2017.

This work is part of our ongoing project, namely Movement-Aware Location Selection (MALOS). Besides PINOCCHIO, given the movement history (e.g., historical trajactory/check-in logs) for massive users, we've also conducted research works on several other problems, including:
- Find the best location to deploy the next branch, if there are *k* existing facilities in the region.
- Find a facility, from *k* existing ones, and relocate it to another place such that the utility can be maximized.
- Find k location to place facilities such that the aggregated utility can be maximized.


Environment
----------
1. These experiments are implementes in C++.<br>
2. IDE is VS 2013.

Dataset
----------
1. There are two datasets which are recorded by two text files(`checkins-10162.txt` and `checkins-2321.txt`).<br>
    * checkins-10162.txt: 10162 users from Gowalla located in California.
    * checkins-2321.txt: 2321 users from Foursquare located in Singapore.
2. The trajectory of user consist of username and a series of check-in points.<br>
3. Candidate sets are randomly generated from check-in points.

Algorithm
----------
1. `NA` : A baseline method that exhaustively computes the cumulative influence probabilities for all pairs of candidate location and moving object, based on which we retrieve the most influential candidate.<br>
2. `PIN` : PINOCCHIO algorithm described in Algorithm 2 in our TKDE paper.<br>
3. `PIN-VO` : PINOCCHIO-VO algorithm described in Algorithm3 in our TKDE paper.<br>

Supplement
----------------

1. `PLS.cpp `: Main function. <br>
2. `pino.cpp`: This file contains the implements of all algorithoms (eg., NA,PIN,PIN-VO) in our TKDE paper.<br>

Usage
----------
1. All data files should be placed in a local folder named as 'Release', e.g., '`D:\Experiment\PLS\Release`'.<br>
2. We should load `boost library` in this project which provides corresponding utilities with respect to R-tree.<br>
3. There are some precompiles in program.<br>
    
    `CANDIDATES_GENERATION` : Generate candidates.<br>
    `GEN_FROM_10162` : From 10162 (1) or 2321 (0) dataset.<br>
    `PICK_FROM_UNIQUE` : In CANDIDATES_GENERATION, pick from unique coordinates (set) but not all check-in logs (vector).<br>
    `CHECKINS_EXCLUDING`: Exclude check-ins that appear in candidates. And generate checkins.txt.<br>
    `DATA_LOADING`  : Load the datas about candidates and users.<br>
    **They are all in PLS.cpp.**<br>


