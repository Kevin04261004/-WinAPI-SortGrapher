/* û����ȭ������б� 202313048 �赵��
*
����:
����ũ�⿡ ���� �پ��� ���ľ˰����� ������ �м��ϰ��� �Ѵ�.������ ������ ���� �������� �����ϴµ� �ҿ�Ǵ� �ð����� �����Ѵ�.
���� �м��� ����Ǵ� ���ľ˰���� ������ ������ ���� �� �־��� ������ �ذ��Ͻÿ�.
# ���ľ˰��� : ��������, ��������, ��������, �պ�����, ������, ������(Shell sort), ������(Count sort)
�Էµ����ʹ� ���� �����̸� ���Ƿ� �����Ѵ�.
# �Էµ����� ���̽� : 3, 000��, 5, 000��, 10, 000��
# �� �˰��� ����Ǵ� ������ ���̽��� �ʱⵥ���ʹ� ���� ������ �����ؾ� �Ѵ�.
# ��µ����� : �� ���̽��� ���ĵ� ������, ����ð�
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
	// �ּҰ� ����
	int minCount = 0;

	// ���� ���� ���
	double range = maxCount - minCount;

	// ������ ���� ����
	COLORREF rainbowColors[] = {
		RGB(255, 0, 0),      // ����
		RGB(255, 165, 0),    // ��Ȳ
		RGB(255, 255, 0),    // ���
		RGB(0, 255, 0),      // �ʷ�
		RGB(0, 0, 255),      // �Ķ�
		RGB(75, 0, 130)      // ����
	};

	// ���� ���� ���� ���
	double percentage = (count - minCount) / range;

	// ���� ������ �ε��� ���
	int colorIndex = (int)(percentage * (sizeof(rainbowColors) / sizeof(rainbowColors[0])));

	// ���� ����� ���� ������ ���� ���
	double colorRatio = (percentage * (sizeof(rainbowColors) / sizeof(rainbowColors[0]))) - colorIndex;

	// ������ RGB �� ���
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
		/* ȭ���� ũ�⸦ �ִ�ȭ������ */
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);

		MoveWindow(hWnd, 0, 0, screenWidth, screenHeight, TRUE);

		/* Ŭ���̾�Ʈ Rect ��������. */
		GetClientRect(hWnd, &crt);

		/* ������ �ʱ�ȭ */
		mainHWnd = hWnd;
		orderByASC = false;
		g_sortType = SELECTION_SORT;
		g_hMenu = GetMenu(hWnd); // �޴� hWnd���� ��������
		g_hOptionMenu = GetSubMenu(g_hMenu, 1); // 0 ��° �޴�(PopUp) ��������
		EnableMenuItem(g_hOptionMenu, ID_MENU_OPTION_START, MF_ENABLED);
		/* �ʱⰪ ���� �� �����Ҵ�*/
		assert(data_ptr == nullptr);
		SetRandomDatasIntoArray(data_ptr, dataSize);
		assert(data_ptr != nullptr);

		break;
	}
	case WM_DESTROY:
		/* �����Ҵ��� �����͵� ���� */
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
		// ����� ������ �ʰ� ó����
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
	_stprintf_s(msgBuffer, TEXT("�ݺ��� Ƚ��: %lld(��), �ɸ� �ð�: %.3f(sec)"), big_O_Count - 1, duration);

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

/* �� �Լ��� ����Ͽ� ������ ȭ���� �ٽ� �׸���, �ð� ���⵵�� �Ǻ��ϼ���. */
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

/* �������� */
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

/* �������� */
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

/* �������� */
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

/* �պ����� */
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

/* ������ */
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

/* ������ */
void shellSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		// �� ���Ŀ� ����� ���� ������ �迭
		int gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };
		int numGaps = sizeof(gaps) / sizeof(gaps[0]);
		if (orderByASC)
		{
			// ���� ������ �迭�� ��ȸ�ϸ鼭 ���� ����
			for (int g = 0; g < numGaps; ++g)
			{
				int gap = gaps[g];

				// ���� ������ �ش� ���ݿ� �°� ����
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
			// ���� ������ �迭�� ��ȸ�ϸ鼭 ���� ����
			for (int g = 0; g < numGaps; ++g)
			{
				int gap = gaps[g];

				// ���� ������ �ش� ���ݿ� �°� ����
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

/* �� ���� */
void countingSort(int* ptr, int maxSize, bool orderByASC)
{
	DataSortStarted();
	g_startClock = clock();
	{
		// �Է� �������� �ִ밪 ã��
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

		// ���� �󵵼� ����� ���� �迭 ���� �� �ʱ�ȭ
		int range = maxVal - minVal + 1;
		int* count = (int*)calloc(range, sizeof(int));

		// �Է� �������� �󵵼� ����
		for (int i = 0; i < maxSize; ++i)
		{
			count[ptr[i] - minVal]++;
			SortUpdateWindowTick();
		}
		int* sorted = (int*)malloc(sizeof(int) * maxSize);

		int index = 0;
		// ���� �迭�� ��ȸ�ϸ鼭 ���ĵ� ����� �ӽ� �迭�� ����
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
		// �ӽ� �迭�� ������ ���� �迭�� ����
		for (int i = 0; i < maxSize; ++i)
		{
			ptr[i] = sorted[i];
			SortUpdateWindowTick();
		}
		// ���� �Ҵ��� �޸� ����
		free(count);
		free(sorted);
		SortUpdateWindowTick();
	}
	g_endClock = clock();
	double duration = g_endClock - g_startClock;
	DataSortFinish(duration);
}