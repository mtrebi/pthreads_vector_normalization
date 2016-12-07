# Vector normalization

## Introduction
In this project I implemented to programs to normalize a vector (divide each component by the vector's length). The first program is sequetial while the second one is executed in parallel using POSIX threads.

## Sequential
This is straightforward.


![Sequential code for vector normalization](https://github.com/mtrebi/pthreads_vector_normalization/blob/master/images/sequential.png)


## Parallel

This is a little bit more tricky:

1. First of all we split the vector in as many segments as threads we have. So that, each thread works only on one segment of the data and its size is equal to Size/nThreads.
2. Then, in each thread we calculate the sum of the elements for that particular segment.
3. After that, we join the results of the different threads (that is the sum of the elements for that segment) and we add them.
4. We calculate the square root of that sum to get the length of the vector.
5. Again, we split the vector as before.
6. Now, in each thread we divide each element in the vector by the length calculated before.
7. Finally, we join all the segments together to get the final normalized vector.

![Parallel code for vector normalization](https://github.com/mtrebi/pthreads_vector_normalization/blob/master/images/parallel.png)

## Results
As we can see, when the vector is small, it makes no sense to use a parallel algorithm because the overhead splitting the data and joining the results is too high in comparision with the operations that we're doing.
Something similar happens with the number of threads. If we have a lot of threads but the load that they have is small, it's not worth because of the overhead that it means to split and join the data from too many chunks.
However, as the vector size grows, it makes more and more sense to use threads (and more threads) because more operations can then be done in parallel and thus decrease our speed execution time.

![Comparision chart](https://github.com/mtrebi/pthreads_vector_normalization/blob/master/images/benchmark.png)
