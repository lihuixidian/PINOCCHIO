# PINOCCHIO
A movement-aware location selection algorithm (published in TKDE16, ICDE17)

Given the movement history (e.g., historical trajactory/check-in logs) for massive users, how to find a location, from a group of candidates, to place some facility, such that the new facility can probably cover the most number of moving users. We present a novel model, namely PINOCCHIO, that can address the aforementioned problem efficiently. The results of this work have been published in the following papers, please cite the following appropriately if you are using the code for PINOCCHIO.

- PINOCCHIO: Probabilistic Influence-Based Location Selection over Moving Objects.    Wang, M.; Li, H.; Cui, J.; Deng, K.; Bhowmick, S. S.; and Dong, Z.   IEEE Trans. Knowl. Data Eng., 28(11): 3068–3082. 2016.
- PINOCCHIO: Probabilistic Influence-Based Location Selection over Moving Objects.    Wang, M.; Li, H.; Cui, J.; Deng, K.; Bhowmick, S. S.; and Dong, Z.   In 33rd IEEE International Conference on Data Engineering, ICDE 2017, San Diego, CA, USA, April 19-22, 2017, pages 21–22, 2017.

This work is part of our ongoing project, namely Movement-Aware Location Selection (MALOS). Besides PINOCCHIO, given the movement history (e.g., historical trajactory/check-in logs) for massive users, we've also conducted research works on several other problems, including:
- Find the best location to deploy the next branch, if there are *k* existing facilities in the region.
- Find a facility, from *k* existing ones, and relocate it to another place such that the utility can be maximized.
- Find k location to place facilities such that the aggregated utility can be maximized.
