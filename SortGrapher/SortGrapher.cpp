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

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static const COLORREF DOT_RGB = RGB(255, 255, 255);
	static const int TICKTIME_MS = 1;
	
	
	static RECT crt;
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HBITMAP hBitmap, oldBitmap;

	static data_size_t data_size = TEST_INIT;

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

		/* �ʱⰪ ���� �� �����Ҵ�*/
		srand((unsigned)time(NULL));
		data_ptr = (int*)malloc(sizeof(int) * data_size);
		assert(data_ptr != NULL);
		for (int i = 0; i < data_size; ++i)
		{
			data_ptr[i] = rand() % data_size + 1;
		}

		// ���ο� �����忡�� ���� �˰��� ����
		std::thread sortingThread(selectionSort, data_ptr, data_size);
		sortingThread.detach(); // �����带 ��Ÿġ�Ͽ� ���� ������� ���������� ����

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

		for (int i = 0; i < data_size; ++i)
		{
			SetPixel(hMemDC, i, data_ptr[i], DOT_RGB);
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
		break;
	default:
		return (DefWindowProc(hWnd, iMsg, wParam, lParam));
	}
	return 0;
}

void SortUpdateWindowTick()
{
	InvalidateRect(mainHWnd, NULL, true);
	// UpdateWindow(mainHWnd);
	// Sleep(1);
}

void bubbleSort(int* ptr, int maxSize)
{
	for (int i = 0; i < maxSize; ++i)
	{
		for (int j = i + 1; j < maxSize; ++j)
		{
			SortUpdateWindowTick();

			if (ptr[j] < ptr[i])
			{
				SWAP(ptr[i], ptr[j]);
			}
		}
	}
}
void selectionSort(int* ptr, int maxSize)
{
	for (int i = 0; i < maxSize; ++i)
	{
		int minIndex = i;
		for (int j = i + 1; j < maxSize; ++j)
		{
			SortUpdateWindowTick();

			if (ptr[j] < ptr[minIndex])
			{
				minIndex = j;
			}
		}
		if (minIndex != i)
		{
			SWAP(ptr[i], ptr[minIndex]);
		}
	}
}
