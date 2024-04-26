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

void SortUpdateWindowTick();

void bubbleSort(int* ptr, int maxSize);

void selectionSort(int* ptr, int maxSize);
