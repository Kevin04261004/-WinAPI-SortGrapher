/* 청강문화산업대학교 202313048 김도윤
*
개요:
문제크기에 따라 다양한 정렬알고리즘의 성능을 분석하고자 한다.성능은 임의의 양의 정수들을 정렬하는데 소요되는 시간으로 측정한다.
성능 분석에 적용되는 정렬알고리즘과 조건이 다음과 같을 때 주어진 문제를 해결하시오.
# 정렬알고리즘 : 선택정렬, 버블정렬, 삽입정렬, 합병정렬, 퀵정렬, 셀정렬(Shell sort), 셈정렬(Count sort)
입력데이터는 양의 정수이며 임의로 생성한다.
# 입력데이터 케이스 : 3, 000개, 5, 000개, 10, 000개
# 각 알고리즘에 적용되는 동일한 케이스의 초기데이터는 값과 순서가 동일해야 한다.
# 출력데이터 : 각 케이스별 정렬된 데이터, 수행시간
*
*/
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>
#include <thread>
#include <assert.h>
#include "resource.h"
#include "SortGrapher.h"

HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("Sort Grapher");
HWND mainHWnd;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = MAKEINTRESOURCE(IDC_MENU);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);
	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_HSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, SW_MAXIMIZE);
	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

static HMENU g_hMenu;
static HMENU g_hOptionMenu = NULL;
static long long Big_O_Count = 0;
static sort_type_t g_sortType;
static clock_t g_startClock, g_endClock;

COLORREF GetRGB(int count, int maxCount)
{
	// 최소값 지정
	int minCount = 0;

	// 색상 범위 계산
	double range = maxCount - minCount;

	// 무지개 색상 정의
	COLORREF rainbowColors[] = {
		RGB(255, 0, 0),      // 빨강
		RGB(255, 165, 0),    // 주황
		RGB(255, 255, 0),    // 노랑
		RGB(0, 255, 0),      // 초록
		RGB(0, 0, 255),      // 파랑
		RGB(75, 0, 130)      // 보라
	};

	// 범위 내의 비율 계산
	double percentage = (count - minCount) / range;

	// 현재 색상의 인덱스 계산
	int colorIndex = (int)(percentage * (sizeof(rainbowColors) / sizeof(rainbowColors[0])));

	// 현재 색상과 다음 색상의 비율 계산
	double colorRatio = (percentage * (sizeof(rainbowColors) / sizeof(rainbowColors[0]))) - colorIndex;

	// 보간된 RGB 값 계산
	int red = (int)(GetRValue(rainbowColors[colorIndex]) + colorRatio * (GetRValue(rainbowColors[colorIndex + 1]) - GetRValue(rainbowColors[colorIndex])));
	int green = (int)(GetGValue(rainbowColors[colorIndex]) + colorRatio * (GetGValue(rainbowColors[colorIndex + 1]) - GetGValue(rainbowColors[colorIndex])));
	int blue = (int)(GetBValue(rainbowColors[colorIndex]) + colorRatio * (GetBValue(rainbowColors[colorIndex + 1]) - GetBValue(rainbowColors[colorIndex])));

	return RGB(red, green, blue);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// static const COLORREF DOT_RGB = RGB(255, 255, 255);
	static const int TICKTIME_MS = 1;
	
	static std::thread sortingThread;
	static RECT crt;
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP hBitmap, oldBitmap;

	static int dataSize = 100;
	static bool orderByASC;
	int screenWidth, screenHeight;
	static int* data_ptr = nullptr;

	switch (iMsg)
	{
	case WM_CREATE:
	{
		/* 화면의 크기를 최대화면으로 */
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);

		MoveWindow(hWnd, 0, 0, screenWidth, screenHeight, TRUE);

		/* 클라이언트 Rect 가져오기. */
		GetClientRect(hWnd, &crt);

		/* 데이터 초기화 */
		mainHWnd = hWnd;
		orderByASC = false;
		g_sortType = SELECTION_SORT;
		g_hMenu = GetMenu(hWnd); // 메뉴 hWnd에서 가져오기
		g_hOptionMenu = GetSubMenu(g_hMenu, 1); // 0 번째 메뉴(PopUp) 가져오기
		EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_START, MF_ENABLED);
		/* 초기값 세팅 및 동적할당*/
		assert(data_ptr == nullptr);
		SetRandomDatasIntoArray(data_ptr, dataSize);
		assert(data_ptr != nullptr);

		break;
	}
	case WM_DESTROY:
		/* 동적할당한 데이터들 해제 */
		if (data_ptr != nullptr)
		{
			free(data_ptr);
		}
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		hMemDC = CreateCompatibleDC(hdc);
		hBitmap = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
		oldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

		FillRect(hMemDC, &crt, (HBRUSH)GetStockObject(BLACK_BRUSH));

		for (int i = 0; i < dataSize; ++i)
		{
			switch (dataSize)
			{
			case 100:
				SetPixel(hMemDC, i * 6, data_ptr[i] * 6, GetRGB(data_ptr[i], dataSize));
				break;
			case 500:
				SetPixel(hMemDC, i, data_ptr[i], GetRGB(data_ptr[i], dataSize));
				break;
			case 1000:
				SetPixel(hMemDC, i /2, data_ptr[i] / 2, GetRGB(data_ptr[i], dataSize));
				break;
			case 3000:
				SetPixel(hMemDC, i / 4, data_ptr[i] / 4, GetRGB(data_ptr[i], dataSize));
				break;
			case 5000:
				SetPixel(hMemDC, i / 6, data_ptr[i] / 6, GetRGB(data_ptr[i], dataSize));
				break;
			case 10000:
				SetPixel(hMemDC, i / 11, data_ptr[i] / 11, GetRGB(data_ptr[i], dataSize));
				break;
			case 50000:
				SetPixel(hMemDC, i / 40, data_ptr[i] / 35, GetRGB(data_ptr[i], dataSize));
				break;
			default:
				SetPixel(hMemDC, i, data_ptr[i], GetRGB(data_ptr[i], dataSize));
				break;
			}
		}

		BitBlt(hdc, 0, 0, crt.right, crt.bottom, hMemDC, 0, 0, SRCCOPY);

		DeleteObject(SelectObject(hMemDC, oldBitmap));
		DeleteDC(hMemDC);

		EndPaint(hWnd, &ps);
		break;
	case WM_ERASEBKGND:
		// 배경을 지우지 않고 처리함
		return TRUE;
	case WM_SIZE:
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_MENU_SORT_SELECTIONSORT:
			g_sortType = SELECTION_SORT;
			break;
		case ID_MENU_SORT_BUBBLESORT:
			g_sortType = BUBBLE_SORT;
			break;
		case ID_MENU_SORT_INSERTIONSORT:
			g_sortType = INSERTION_SORT;
			break;
		case ID_MENU_SORT_MERGESORT:
			g_sortType = MERGE_SORT;
			break;
		case ID_MENU_SORT_QUICKSORT:
			g_sortType = QUICK_SORT;
			break;
		case ID_MENU_SORT_SHELLSORT:
			g_sortType = SHELL_SORT;
			break;
		case ID_MENU_SORT_COUNTINGSORT:
			g_sortType = COUNTING_SORT;
			break;
		case ID_MENU_OPTION_RESET_100:
			dataSize = 100;
			SetRandomDatasIntoArray(data_ptr, dataSize);
			break;
		case ID_MENU_OPTION_RESET_500:
			dataSize = 500;
			SetRandomDatasIntoArray(data_ptr, dataSize);
			break;
		case ID_MENU_OPTION_RESET_1000:
			dataSize = 1000;
			SetRandomDatasIntoArray(data_ptr, dataSize);
			break;
		case ID_MENU_OPTION_RESET_3000:
			dataSize = 3000;
			SetRandomDatasIntoArray(data_ptr, dataSize);
			break;
		case ID_MENU_OPTION_RESET_5000:
			dataSize = 5000;
			SetRandomDatasIntoArray(data_ptr, dataSize);
			break;
		case ID_MENU_OPTION_RESET_10000:
			dataSize = 10000;
			SetRandomDatasIntoArray(data_ptr, dataSize);
			break;
		case ID_MENU_OPTION_RESET_50000:
			dataSize = 50000;
			SetRandomDatasIntoArray(data_ptr, dataSize);
			break;
		case ID_MENU_OPTION_START:
			// SetRandomDatasIntoArray(data_ptr, dataSize);
			switch (g_sortType)
			{
			case SELECTION_SORT:
				sortingThread = std::thread(selectionSort, data_ptr, dataSize, orderByASC);
				break;
			case BUBBLE_SORT:
				sortingThread = std::thread(bubbleSort, data_ptr, dataSize, orderByASC);
				break;
			case INSERTION_SORT:
				sortingThread = std::thread(insertionSort, data_ptr, dataSize, orderByASC);
				break;
			case MERGE_SORT:
				sortingThread = std::thread(mergeSort, data_ptr, dataSize, orderByASC);
				break;
			case QUICK_SORT:
				sortingThread = std::thread(quickSort, data_ptr, dataSize, orderByASC);
				break;
			case SHELL_SORT:
				sortingThread = std::thread(shellSort, data_ptr, dataSize, orderByASC);
				break;
			case COUNTING_SORT:
				sortingThread = std::thread(countingSort, data_ptr, dataSize, orderByASC);
				break;
			default:
				break;
			}
			sortingThread.detach();
			break;
		case ID_MENU_OPTION_ORDERBY_ASC:
			orderByASC = true;
			break;
		case ID_MENU_OPTION_ORDERBY_DESC:
			orderByASC = false;
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	default:
		return (DefWindowProc(hWnd, iMsg, wParam, lParam));
	}
	return 0;
}

void SortGraphMessage(sort_type_t sortType, long long big_O_Count, int duration_ms)
{
	const TCHAR* sort_type_str;
	switch (sortType)
	{
	case SELECTION_SORT:
		sort_type_str = TEXT("Selection Sort");
		break;
	case BUBBLE_SORT:
		sort_type_str = TEXT("Bubble Sort");
		break;
	case INSERTION_SORT:
		sort_type_str = TEXT("Insertion Sort");
		break;
	case MERGE_SORT:
		sort_type_str = TEXT("Merge Sort");
		break;
	case QUICK_SORT:
		sort_type_str = TEXT("Quick Sort");
		break;
	case SHELL_SORT:
		sort_type_str = TEXT("Shell Sort");
		break;
	case COUNTING_SORT:
		sort_type_str = TEXT("Counting Sort");
		break;
	default:
		sort_type_str = TEXT("Unknown Sort");
		break;
	}
	TCHAR msgBuffer[256];
	double duration = duration_ms / (double)CLOCKS_PER_SEC;
	_stprintf_s(msgBuffer, TEXT("반복한 횟수: %lld(번), 걸린 시간: %.3f(sec)"), big_O_Count - 1, duration);

	MessageBox(mainHWnd, msgBuffer, sort_type_str, MB_OK);
}

void SetRandomDatasIntoArray(int*& ptr, int count)
{
	ptr = (int*)realloc(ptr, sizeof(int) * count);
	assert(ptr != nullptr);
	srand(10);
	for (int i = 0; i < count; ++i)
	{
		ptr[i] = rand() % count + 1;
	}
	DataReseted();
}

/* 이 함수를 사용하여 윈도우 화면을 다시 그리고, 시간 복잡도를 판별하세요. */
void SortUpdateWindowTick()
{
	InvalidateRect(mainHWnd, NULL, true);
	++Big_O_Count;
	// UpdateWindow(mainHWnd);
	// Sleep(1);
}

void DataReseted()
{
	Big_O_Count = 0;
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_START, MF_ENABLED);
}

void DataSortStarted()
{
	Big_O_Count = 0;
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_START, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_100, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_500, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_1000, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_3000, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_5000, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_10000, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_50000, MF_DISABLED);
}

void DataSortFinish(int duration)
{
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_START, MF_DISABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_100, MF_ENABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_500, MF_ENABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_1000, MF_ENABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_3000, MF_ENABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_5000, MF_ENABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_10000, MF_ENABLED);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_RESET_50000, MF_ENABLED);
	SortGraphMessage(g_sortType, Big_O_Count, duration);
	EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_START, MF_ENABLED);
}

/* 선택정렬 */
void selectionSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		if (orderByASC)
		{
			for (int i = 0; i < maxSize; ++i)
			{
				int minIndex = i;
				for (int j = i + 1; j < maxSize; ++j)
				{
					if (ptr[j] < ptr[minIndex])
					{
						minIndex = j;
					}
					SortUpdateWindowTick();
				}
				if (minIndex != i)
				{
					SWAP(ptr[i], ptr[minIndex]);
				}
			}
		}
		else
		{
			for (int i = 0; i < maxSize; ++i)
			{
				int maxIndex = i;
				for (int j = i + 1; j < maxSize; ++j)
				{
					if (ptr[j] > ptr[maxIndex])
					{
						maxIndex = j;
					}
					SortUpdateWindowTick();
				}
				if (maxIndex != i)
				{
					SWAP(ptr[i], ptr[maxIndex]);
				}
			}
		}
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}

/* 버블정렬 */
void bubbleSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		if (orderByASC)
		{
			for (int i = 0; i < maxSize; ++i)
			{
				bool sorted = true;
				for (int j = i + 1; j < maxSize; ++j)
				{
					if (ptr[j] < ptr[i])
					{
						SWAP(ptr[i], ptr[j]);
						sorted = false;
					}
					SortUpdateWindowTick();
				}
				if (sorted)
				{
					break;
				}
			}
		}
		else
		{
			for (int i = 0; i < maxSize; ++i)
			{
				bool sorted = true;
				for (int j = i + 1; j < maxSize; ++j)
				{
					if (ptr[j] > ptr[i])
					{
						SWAP(ptr[i], ptr[j]);
						sorted = false;
					}
					SortUpdateWindowTick();
				}
				if (sorted)
				{
					break;
				}
			}
		}
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}

/* 삽입정렬 */
void insertionSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		if (orderByASC)
		{
			for (int i = 1; i < maxSize; ++i)
			{
				int current = i;
				int pickData = ptr[current];
				for (; (current > 0) && (ptr[current - 1] > pickData); --current)
				{
					ptr[current] = ptr[current - 1];
					SortUpdateWindowTick();
				}
				ptr[current] = pickData;
			}
		}
		else
		{
			for (int i = 1; i < maxSize; ++i)
			{
				int current = i;
				int pickData = ptr[current];
				for (; (current > 0) && (ptr[current - 1] < pickData); --current)
				{
					ptr[current] = ptr[current - 1];
					SortUpdateWindowTick();
				}
				ptr[current] = pickData;
			}
		}
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}

/* 합병정렬 */
void mergeSortRecursive(int* data, int first, int last, int size, bool orderByASC);
void merge(int* data, int f, int m, int l, int size, bool orderByASC);

void mergeSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		mergeSortRecursive(ptr, 0, maxSize - 1, maxSize, orderByASC);
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}

void mergeSortRecursive(int* data, int first, int last, int size, bool orderByASC)
{
	if (first < last)
	{
		int middle = (first + last) / 2;
		mergeSortRecursive(data, first, middle, size, orderByASC);
		mergeSortRecursive(data, middle + 1, last, size, orderByASC);
		merge(data, first, middle, last, size, orderByASC);
	}
}

void merge(int* data, int f, int m, int l, int size, bool orderByASC)
{
	int* sorted = (int*)malloc(sizeof(int) * size);
	int first1 = f;
	int last1 = m;
	int first2 = m + 1;
	int last2 = l;

	int index = first1;
	if (orderByASC)
	{
		while (first1 <= last1 && first2 <= last2)
		{
			if (data[first1] < data[first2])
			{
				sorted[index++] = data[first1++];
			}
			else
			{
				sorted[index++] = data[first2++];
			}
			SortUpdateWindowTick();
		}
	}
	else
	{
		while (first1 <= last1 && first2 <= last2)
		{
			if (data[first1] >= data[first2])
			{
				sorted[index++] = data[first1++];
			}
			else
			{
				sorted[index++] = data[first2++];
			}
			SortUpdateWindowTick();
		}
	}
	for (; first1 <= last1; ++first1, ++index)
	{
		sorted[index] = data[first1];
		SortUpdateWindowTick();
	}
	for (; first2 <= last2; ++first2, ++index) {
		sorted[index] = data[first2];
		SortUpdateWindowTick();
	}
	for (index = f; index <= l; ++index) {
		data[index] = sorted[index];
		SortUpdateWindowTick();
	}

	if (sorted != nullptr)
	{
		free(sorted);
	}
}

/* 퀵정렬 */
void quickSortRecursive(int* ptr, int low, int high, bool orderByASC);
int partition(int* ptr, int low, int high, bool orderByASC);

void quickSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		quickSortRecursive(ptr, 0, maxSize - 1, orderByASC);
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}

void quickSortRecursive(int* ptr, int low, int high, bool orderByASC)
{
	int pivotPoint;
	if (high > low)
	{
		pivotPoint = partition(ptr, low, high, orderByASC);
		quickSortRecursive(ptr, low, pivotPoint - 1, orderByASC);
		quickSortRecursive(ptr, pivotPoint + 1, high, orderByASC);
	}
}

int partition(int* ptr, int low, int high, bool orderByASC)
{
	int pivotPoint;
	if (orderByASC)
	{
		int pivotItem = ptr[low];
		pivotPoint = low;
		for (int cur = low + 1; cur <= high; ++cur)
		{
			if (ptr[cur] < pivotItem)
			{
				pivotPoint++;
				SWAP(ptr[cur], ptr[pivotPoint]);
			}
			SortUpdateWindowTick();
		}
		SWAP(ptr[low], ptr[pivotPoint]);
	}
	else
	{
		int pivotItem = ptr[low];
		pivotPoint = low;
		for (int cur = low + 1; cur <= high; ++cur)
		{
			if (ptr[cur] > pivotItem)
			{
				pivotPoint++;
				SWAP(ptr[cur], ptr[pivotPoint]);
			}
			SortUpdateWindowTick();
		}
		SWAP(ptr[low], ptr[pivotPoint]);
	}
	return pivotPoint;
}

/* 셸정렬 */
void shellSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		// 셸 정렬에 사용할 간격 시퀀스 배열
		int gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };
		int numGaps = sizeof(gaps) / sizeof(gaps[0]);
		if (orderByASC)
		{
			// 간격 시퀀스 배열을 순회하면서 정렬 수행
			for (int g = 0; g < numGaps; ++g)
			{
				int gap = gaps[g];

				// 삽입 정렬을 해당 간격에 맞게 적용
				for (int i = gap; i < maxSize; ++i)
				{
					int temp = ptr[i];
					int j;
					for (j = i; j >= gap && ptr[j - gap] > temp; j -= gap)
					{
						ptr[j] = ptr[j - gap];
						SortUpdateWindowTick();
					}
					ptr[j] = temp;
				}
			}
		}
		else
		{
			// 간격 시퀀스 배열을 순회하면서 정렬 수행
			for (int g = 0; g < numGaps; ++g)
			{
				int gap = gaps[g];

				// 삽입 정렬을 해당 간격에 맞게 적용
				for (int i = gap; i < maxSize; ++i)
				{
					int temp = ptr[i];
					int j;
					for (j = i; j >= gap && ptr[j - gap] < temp; j -= gap)
					{
						ptr[j] = ptr[j - gap];
						SortUpdateWindowTick();
					}
					ptr[j] = temp;
				}
			}
		}
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}

/* 셈 정렬 */
void countingSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		// 입력 데이터의 최대값 찾기
		int maxVal = ptr[0];
		int minVal = ptr[0];
		for (int i = 1; i < maxSize; ++i)
		{
			if (ptr[i] > maxVal)
			{
				maxVal = ptr[i];
			}
			if (ptr[i] < minVal)
			{
				minVal = ptr[i];
			}
			SortUpdateWindowTick();
		}

		// 누적 빈도수 계산을 위한 배열 생성 및 초기화
		int range = maxVal - minVal + 1;
		int* count = (int*)calloc(range, sizeof(int));

		// 입력 데이터의 빈도수 세기
		for (int i = 0; i < maxSize; ++i)
		{
			count[ptr[i] - minVal]++;
			SortUpdateWindowTick();
		}
		int* sorted = (int*)malloc(sizeof(int) * maxSize);

		int index = 0;
		// 원래 배열을 순회하면서 정렬된 결과를 임시 배열에 저장
		if (orderByASC)
		{
			for (int i = 0; i < range; ++i)
			{
				while (count[i] > 0)
				{
					sorted[index++] = i + minVal;
					count[i]--;
					SortUpdateWindowTick();
				}
			}
		}
		else
		{
			for (int i = range - 1; i >= 0; --i)
			{
				while (count[i] > 0)
				{
					sorted[index++] = i + minVal;
					count[i]--;
					SortUpdateWindowTick();
				}
			}
		}
		// 임시 배열의 내용을 원래 배열에 복사
		for (int i = 0; i < maxSize; ++i)
		{
			ptr[i] = sorted[i];
			SortUpdateWindowTick();
		}
		// 동적 할당한 메모리 해제
		free(count);
		free(sorted);
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}