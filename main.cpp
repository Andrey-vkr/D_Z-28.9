#include <iostream>
#include <cstdlib>
#include <future>
#include <mutex>
#include <chrono>



void randNumb(int* arrMerge, long size)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int i = 0; i < size; i++)
    {
        arrMerge[i] = std::rand() % 100;
    }
}

void merge(int* array, int left, int middle, int right)
{
    int size_left = middle - left + 1;
    int size_right = right - middle;

    int* arrLeft = new int[size_left];
    int* arrRight = new int[size_right];

    for (int i = 0; i < size_left; ++i)
        arrLeft[i] = array[left + i];
    for (int j = 0; j < size_right; ++j)
        arrRight[j] = array[middle + 1 + j];

    int i = 0, j = 0, k = left;

    while (i < size_left && j < size_right)
    {
        if (arrLeft[i] <= arrRight[j])
        {
            array[k] = arrLeft[i];
            ++i;
        }
        else
        {
            array[k] = arrRight[j];
            ++j;
        }
        ++k;
    }

    while (i < size_left)
    {
        array[k] = arrLeft[i];
        ++i;
        ++k;
    }

    while (j < size_right)
    {
        array[k] = arrRight[j];
        ++j;
        ++k;
    }

    delete[] arrLeft;
    delete[] arrRight;
}

void mergeSort(int* array, int left, int right)
{
    if (left < right)
    {
        int middle = left + (right - left) / 2;
        mergeSort(array, left, middle);
        mergeSort(array, middle + 1, right);
        merge(array, left, middle, right);
    }
}

void asyncMergeSort(int* array, int left, int right, int& maxThreads, int& actThreads)
{
    std::mutex mutex_;

    if (left < right) 
    {
        int middle = left + (right - left) / 2;

        std::unique_lock<std::mutex> mutex_lock(mutex_);
        if (actThreads < maxThreads && right - left > 10000)
        {
            ++actThreads;
            mutex_lock.unlock();

            std::future<void> futureLeft = std::async(std::launch::async, [&]
                {
                    asyncMergeSort(array, left, middle, maxThreads, actThreads);
                });
            std::future<void> futureRight = std::async(std::launch::async, [&]
                {
                    asyncMergeSort(array, middle + 1, right, maxThreads, actThreads);
                });

            futureLeft.get();
            futureRight.get();

            mutex_lock.lock();
            --actThreads;
        }
        else
        {
            mutex_lock.unlock();

            asyncMergeSort(array, left, middle, maxThreads, actThreads);
            asyncMergeSort(array, middle + 1, right, maxThreads, actThreads);
        }

        merge(array, left, middle, right);
    }
}

int main()
{
    long size = 100000000;
    int* mergeArray = new int[size];

    randNumb(mergeArray, size);

   /* std::cout << "Random array: ";
    for (int i = 0; i < size; i++)
        std::cout << mergeArray[i] << " ";
    std::cout << '\n';*/

    int maxThreads = std::thread::hardware_concurrency();
    int activeThreads = 0;

    auto startAsync = std::chrono::high_resolution_clock::now();
    asyncMergeSort(mergeArray, 0, size - 1, maxThreads, activeThreads);
    auto finishAsync = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedAsync = finishAsync - startAsync;

   /* std::cout << "Merge sort array: ";
    for (int i = 0; i < size; i++)
       std::cout << mergeArray[i] << " ";
    std::cout << '\n' << '\n';*/

    std::cout << "Elapsed time with multi-threading: " << elapsedAsync.count() << " sec" << std::endl;

    randNumb(mergeArray, size);

    auto start = std::chrono::high_resolution_clock::now();
    mergeSort(mergeArray, 0, size - 1);
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    std::cout << "Elapsed time with single threading: " << elapsed.count() << " sec" << std::endl;

    delete[] mergeArray;

    return 0;
}

