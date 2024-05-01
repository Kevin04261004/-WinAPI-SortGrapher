#pragma once

#include "Resource.h"

#define SWAP(first, second) \
    auto temp = (first); \
	(first) = (second); \
	(second) = temp;

typedef enum sort_type
{
	SELECTION_SORT,
	BUBBLE_SORT,
	INSERTION_SORT,
	MERGE_SORT,
	QUICK_SORT,
	SHELL_SORT,
	COUNTING_SORT,
	HEAP_SORT,
} sort_type_t;

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

void SortGraphMessage(sort_type_t sortType, long long big_O_Count, int duration_ms);

void SortUpdateWindowTick();

void DataReseted();

void DataSortStarted();

void DataSortFinish(int duration);

void SetRandomDatasIntoArray(int*& ptr, int count);

void selectionSort(int* ptr, int maxSize, bool orderByASC);

void bubbleSort(int* ptr, int maxSize, bool orderByASC);

void insertionSort(int* ptr, int maxSize, bool orderByASC);

void mergeSort(int* ptr, int maxSize, bool orderByASC);

void quickSort(int* ptr, int maxSize, bool orderByASC);

void shellSort(int* ptr, int maxSize, bool orderByASC);

void countingSort(int* ptr, int maxSize, bool orderByASC);

void heapSort(int* ptr, int maxSize, bool orderByASC);
