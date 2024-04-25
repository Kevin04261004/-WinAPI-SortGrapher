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
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>
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
	static bool sorted;
	static int count;
	switch (iMsg)
	{
	case WM_CREATE:
		/* 화면의 크기를 최대화면으로 */
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);

		MoveWindow(hWnd, 0, 0, screenWidth, screenHeight, TRUE);

		/* 클라이언트 Rect 가져오기. */
		GetClientRect(hWnd, &crt);

		/* 데이터 초기화 */
		mainHWnd = hWnd;
		count = 0;
		sorted = false;

		/* 초기값 세팅 및 동적할당*/
		srand((unsigned)time(NULL));
		data_ptr = (int*)malloc(sizeof(int) * data_size);
		assert(data_ptr != NULL);
		for (int i = 0; i < data_size; ++i)
		{
			data_ptr[i] = rand() % data_size + 1;
		}

		/* 타이머 세팅 */
		SetTimer(hWnd, 1, TICKTIME_MS, NULL);
		SendMessage(hWnd, WM_TIMER, 1, 0);

		break;
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

		for (int i = 0; i < data_size; ++i)
		{
			SetPixel(hMemDC, i, data_ptr[i], DOT_RGB);
		}

		BitBlt(hdc, 0, 0, crt.right, crt.bottom, hMemDC, 0, 0, SRCCOPY);

		DeleteObject(SelectObject(hMemDC, oldBitmap));
		DeleteDC(hMemDC);

		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		if (!sorted)
		{
			selectionSort(data_ptr, count, data_size);
			count++;
		}
		InvalidateRect(hWnd, NULL, true);
	case WM_ERASEBKGND:
		// 배경을 지우지 않고 처리함
		return TRUE;
	case WM_LBUTTONDOWN:
		sorted = !sorted;
		break;
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


