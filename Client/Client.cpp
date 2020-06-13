// Client.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Client.h"

#pragma comment(lib,"ws2_32.lib")
#include <CommCtrl.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <string>


#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

SOCKET Client;
static HWND hEdit, hText;
static HWND hBtn;
TCHAR clean_message[1024];
HANDLE hThr;
static HWND hChild;

WCHAR name[100], server_ip[100];
UINT server_port;

static std::vector<std::string> v;

TCHAR ChildClassName[MAX_LOADSTRING] = _T("WinChild");

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    Login(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Разместите код здесь.



	// Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_CLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT));

	MSG msg;

	// Цикл основного сообщения:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



void GetMessageFromServer()
{

	char buffer[1024];


	for (;;)
	{
		int i = 0;
		memset(buffer, 0, 1024);
		int result = recv(Client, buffer, 1024, 0);
		if (result > 0)
		{
			while (buffer[i] != '\0')
			{
				clean_message[i] = (TCHAR)buffer[i];
				i++;
			}
			clean_message[i++] = L'\0';

			v.push_back(buffer); 
			InvalidateRect(hChild, NULL, 1);
			UpdateWindow(hChild);
		}

	}

}


ATOM MyRegisterChildClass()
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = ChildProc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName =ChildClassName ;
	return RegisterClassEx(&wcex);
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 480, 620, nullptr, nullptr, hInstance, nullptr);


	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	switch (message)
	{
	case WM_CREATE:
		hEdit = CreateWindow(L"Edit", NULL, WS_CHILD | WS_VISIBLE |
			WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
			0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);
		hBtn = CreateWindow(L"Button", L"Отправить",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);
		/*hText = CreateWindow(L"Edit", NULL, WS_CHILD | WS_VISIBLE | ES_READONLY |
			WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
			0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);*/
		MyRegisterChildClass();
		hChild = CreateWindow(ChildClassName , NULL, WS_CHILD | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL |
			WS_DLGFRAME | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);

		break;
	case WM_SIZE:
		HIWORD(lParam);
		MoveWindow(hEdit, 0, HIWORD(lParam) - 100, LOWORD(lParam) - 100, 100, TRUE);
		MoveWindow(hBtn, LOWORD(lParam) - 100, HIWORD(lParam) - 100, 100, 50, TRUE);

		//MoveWindow(hText, 0, 0, LOWORD(lParam), HIWORD(lParam)-100, TRUE);

		MoveWindow(hChild, 0, 0, LOWORD(lParam), HIWORD(lParam) - 100, TRUE);

		break;
	case WM_COMMAND:
	{
		if (lParam == (LPARAM)hBtn)
		{
			TCHAR str[1024];
			char buffer[1024];
			GetWindowText(hEdit, str, 1024);
			SetWindowText(hEdit, L"");

			WideCharToMultiByte(CP_ACP, 0, str, sizeof(str), buffer, sizeof(buffer), NULL, NULL);

			int k = strlen(buffer);
			buffer[k] = ('\0');

			////ООООЧЧЧЕЕЕННННЬЬЬЬ    ИНТЕРЕСНО/////
			////////////////////////////////////////////////////////
			if (SOCKET_ERROR == (send(Client, buffer, 1024, 0)))
			{
				// Error...

				// ...
			}



		}

		int wmId = LOWORD(wParam);
		// Разобрать выбор в меню:
		switch (wmId)
		{
		case ID_MENU_LOGIN:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGIN), hWnd, Login);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			TerminateThread(hThr, 0);
			shutdown(Client, 0);
			closesocket(Client);
			v.clear();
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Добавьте сюда любой код прорисовки, использующий HDC...



		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Обработчик сообщений для окна "Login".
INT_PTR CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	WSADATA data;
	WORD version = MAKEWORD(2, 2);
	int res = WSAStartup(version, &data);
	if (res != 0)
	{
		return 1;
	}
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			int result;
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT_NAME, name, 100);
			GetDlgItemText(hDlg, IDC_EDIT_IP, server_ip, 100);//"127.0.0.1"
			server_port =  GetDlgItemInt(hDlg, IDC_EDIT_PORT,NULL,FALSE);//8488
			struct sockaddr_in peer;
			peer.sin_family = AF_INET;
			peer.sin_port = htons(server_port);
			InetPton(AF_INET, server_ip, &peer.sin_addr.s_addr);

			Client = socket(AF_INET, SOCK_STREAM, 0);
			result = connect(Client, (sockaddr*)& peer, sizeof(peer));
			if (result)
			{
				perror("Error calling connect");
				return 0;
			}
			else
			{
				char buffer[100];
				WideCharToMultiByte(CP_ACP, 0, name, sizeof(name), buffer, sizeof(buffer), NULL, NULL);
				send(Client, buffer, 100, 0);
				hThr = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)GetMessageFromServer, NULL, NULL, NULL);
			}

			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}

	}
	return (INT_PTR)FALSE;
}




LRESULT CALLBACK ChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	const int LineHeight = 16;
	std::vector<std::string>::iterator it;
	int y;

	switch (message)
	{
	case WM_CREATE:
		v.push_back("Hello first line");
		break;
	case WM_LBUTTONDOWN:

		break;
	case WM_PAINT:
		
		hdc = BeginPaint(hWnd, &ps);
		for (y = 0, it = v.begin(); it < v.end(); ++it, y += LineHeight)
		{
			char buf[1024];
			strcpy_s(buf, 1024 , it->c_str());
			TextOutA(hdc, 0, y, buf, it->length());
		}
		EndPaint(hWnd, &ps);

		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


