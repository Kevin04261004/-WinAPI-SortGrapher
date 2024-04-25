#pragma once

#include "Resource.h"

#define SWAP(first, second) \
    auto temp = (first); \
	(first) = (second); \
	(second) = temp;


typedef enum data_size
{
	TEST1 = 3000,
	TEST2 = 5000,
	TEST3 = 10000,
	TEST_INIT = 500,
} data_size_t;

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

/* for (int count, count < maxSize; ++count) */
/* { */
void bubbleSort(int* ptr, int count, int maxSize)
{
	for (int j = count + 1; j < maxSize; ++j)
	{
		if (ptr[j] < ptr[count])
		{
			SWAP(ptr[count], ptr[j]);
		}
	}
}
/* } */

/* for (int count, count < maxSize; ++count) */
/* { */
void selectionSort(int* ptr, int count, int maxSize)
{
	int minIndex = count;
	for (int j = count + 1; j < maxSize; ++j)
	{
		if (ptr[j] < ptr[minIndex])
		{
			minIndex = j;
		}
	}
	if (minIndex != count)
	{
		SWAP(ptr[count], ptr[minIndex]);
	}
}
/* } */