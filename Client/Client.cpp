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
HANDLE g_ClientStopEvent = INVALID_HANDLE_VALUE;

WCHAR name[100] = L"";
WCHAR server_ip[100] = L"";
UINT server_port = 0;

//static std::vector<std::string> chat;

//TCHAR ChildClassName[MAX_LOADSTRING] = _T("WinChild");

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    Login(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ChildProc(HWND, UINT, WPARAM, LPARAM);
void AppendText(HWND, LPCTSTR);
int trim(char[]);



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

	while (WaitForSingleObject(g_ClientStopEvent, 0) != WAIT_OBJECT_0)
	{
		int i = 0;
		memset(buffer, 0, 1024);
		int result = recv(Client, buffer, 1024, 0);
		if (result > 0)
		{
			TCHAR message[1024];
			MultiByteToWideChar(CP_ACP, 0, buffer, 1024, message, 1024);
			AppendText(hText, message);
			/*chat.push_back(buffer);
			InvalidateRect(hChild, NULL, 1);
			UpdateWindow(hChild);*/
		}

	}

}


/*ATOM MyRegisterChildClass()
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = ChildProc;
	wcex.hInstance = hInst;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = ChildClassName;
	return RegisterClassEx(&wcex);
}*/

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
		hBtn = CreateWindow(L"Button", L"Send",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);
		hText = CreateWindow(L"Edit", NULL, WS_CHILD | WS_VISIBLE | ES_READONLY |
			WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
			0, 0, 0, 0, hWnd, (HMENU)1, hInst, NULL);
		/*MyRegisterChildClass();
		hChild = CreateWindow(ChildClassName, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
			ES_MULTILINE | ES_AUTOVSCROLL,
			0, 0, 0, 0, hWnd, NULL, hInst, NULL);*/
		break;
	case WM_SIZE:
		HIWORD(lParam);
		MoveWindow(hEdit, 0, HIWORD(lParam) - 100, LOWORD(lParam) - 100, 100, TRUE);
		MoveWindow(hBtn, LOWORD(lParam) - 100, HIWORD(lParam) - 100, 100, 50, TRUE);

		MoveWindow(hText, 0, 0, LOWORD(lParam), HIWORD(lParam) - 100, TRUE);

		//MoveWindow(hChild, 0, 0, LOWORD(lParam)-100, HIWORD(lParam) - 100, TRUE);

		break;
	case WM_COMMAND:
	{
		if (lParam == (LPARAM)hBtn)
		{
			TCHAR str[924];
			_tccpy(str, name);
			std::string str1;
			char buffer[924];
			//GetWindowText(hEdit, str, 924);
			GetWindowTextA(hEdit, buffer, 924);
			SetWindowText(hEdit, L"");

			//WideCharToMultiByte(CP_ACP, 0, str, sizeof(str), buffer, sizeof(buffer), NULL, NULL);

			char message[1024] = "";
			WideCharToMultiByte(CP_ACP, 0, name, sizeof(name), message, sizeof(message), NULL, NULL);
			if (trim(buffer) > 0)
			{

				strcat_s(message, ": ");
				strcat_s(message, buffer);
				strcat_s(message, "\r\n");

				////ООООЧЧЧЕЕЕННННЬЬЬЬ    ИНТЕРЕСНО/////
				////////////////////////////////////////////////////////
				if (SOCKET_ERROR == (send(Client, message, 1024, 0)))
				{
					// Error...

					// ...
				}
			}


		}

		int wmId = LOWORD(wParam);
		// Разобрать выбор в меню:
		switch (wmId)
		{
		case ID_MENU_LOGIN:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGIN), hWnd, Login);
			break;
		case ID_MENU_DISCONNECT:

			SetEvent(g_ClientStopEvent);
			CloseHandle(g_ClientStopEvent);
			shutdown(Client, 0);
			closesocket(Client);
			_tccpy(name, L"");
			_tccpy(server_ip, L"");
			server_port = 0;
			AppendText(hText, L"Disconnect from Server");
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			if (server_port != 0)
			{
				CloseHandle(g_ClientStopEvent);
				shutdown(Client, 0);
				closesocket(Client);
			}
			//chat.clear();
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
		if (server_port != 0)
		{
			CloseHandle(g_ClientStopEvent);
			shutdown(Client, 0);
			closesocket(Client);
		}
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

			SetDlgItemText(hDlg, IDC_ERROR, L"");

			GetDlgItemText(hDlg, IDC_EDIT_NAME, name, 100);
			GetDlgItemText(hDlg, IDC_EDIT_IP, server_ip, 100);//"127.0.0.1"
			server_port = GetDlgItemInt(hDlg, IDC_EDIT_PORT, NULL, FALSE);//8488
			if (!_tccmp(name, L"") || !_tccmp(server_ip, L"") || server_port == 0)
			{
				SetDlgItemText(hDlg, IDC_ERROR, L"Enter true data");
				break;
			}

			struct sockaddr_in peer;
			peer.sin_family = AF_INET;
			peer.sin_port = htons(server_port);
			InetPton(AF_INET, server_ip, &peer.sin_addr.s_addr);
			Client = socket(AF_INET, SOCK_STREAM, 0);

			result = connect(Client, (sockaddr*)& peer, sizeof(peer));
			if (result)
			{
				wchar_t error[10] = L"";
				_itow_s(GetLastError(), error, 10);
				wchar_t buffer[100] = L"Can't connect to the server, winsock error: ";
				wcscat_s(buffer, error);
				SetDlgItemText(hDlg, IDC_ERROR, buffer);

			}
			else
			{
				char buffer[100];
				WideCharToMultiByte(CP_ACP, 0, name, sizeof(name), buffer, sizeof(buffer), NULL, NULL);
				send(Client, buffer, 100, 0);
				memset(buffer, 0, 100);
				recv(Client, buffer, 100, 0);
				SetWindowTextA(hText, "");
				if (!strcmp(buffer, "-1"))
				{
					wchar_t buffer[100] = L"Can't connect to the server, name ";
					wcscat_s(buffer, name);
					wcscat_s(buffer, L" already in use");
					SetDlgItemText(hDlg, IDC_ERROR, buffer);
					break;
				}
				else
				{
					SetWindowText(hText, L"Connection success\r\n");
					g_ClientStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
					hThr = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)GetMessageFromServer, NULL, NULL, NULL);
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
				}
			}
			break;


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




/*LRESULT CALLBACK ChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	const int LineHeight = 16;
	std::vector<std::string>::iterator it;
	int y, k;
	static int n, length, sx, sy, cx, iVscrollPos, COUNT, MAX_WIDTH;
	static SIZE size = { 8, 16 }; //Ширина и высота символа

	switch (message)
	{
	case WM_CREATE:
		SendMessage(hWnd, WM_SIZE, 0, sy << 16 | sx);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_SIZE:
		n = chat.size();
		sx = LOWORD(lParam);
		sy = HIWORD(lParam);
		k = n - sy / size.cy;
		if (k > 0) COUNT = k; else COUNT = iVscrollPos = 0;
		SetScrollRange(hWnd, SB_VERT, 0, COUNT, FALSE);
		SetScrollPos(hWnd, SB_VERT, iVscrollPos, TRUE);

		break;
	case WM_LBUTTONDOWN:

		break;

	case WM_VSCROLL:
		switch (LOWORD(wParam))
		{
		case SB_LINEUP: iVscrollPos--; break;
		case SB_LINEDOWN: iVscrollPos++; break;
		case SB_PAGEUP: iVscrollPos -= sy / size.cy; break;
		case SB_PAGEDOWN: iVscrollPos += sy / size.cy; break;
		case SB_THUMBPOSITION: iVscrollPos = HIWORD(wParam); break;
		}
		iVscrollPos = max(0, min(iVscrollPos, COUNT));
		if (iVscrollPos != GetScrollPos(hWnd, SB_VERT))
		{
			SetScrollPos(hWnd, SB_VERT, iVscrollPos, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	case WM_PAINT:

		n = chat.size();
		sx = LOWORD(lParam);
		sy = HIWORD(lParam);
		k = n - sy / size.cy;
		if (k > 0) COUNT = k; else COUNT = iVscrollPos = 0;
		SetScrollRange(hWnd, SB_VERT, 0, COUNT, FALSE);
		SetScrollPos(hWnd, SB_VERT, iVscrollPos, TRUE);

		hdc = BeginPaint(hWnd, &ps);
		for (y = 0, it = chat.begin() + iVscrollPos; it != chat.end() && y < sy; ++it, y += size.cy)

				TextOutA(hdc, 0, y, it->data(), it->length());
		EndPaint(hWnd, &ps);
		break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}*/


void AppendText(HWND hEditWnd, LPCTSTR Text)
{
	int idx = GetWindowTextLength(hEditWnd);
	SendMessage(hEditWnd, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
	SendMessage(hEditWnd, EM_REPLACESEL, 0, (LPARAM)Text);
}

int trim(char str[])
{
	/*std::string s(str);
	int i = 0, j = s.size();
	while (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n') ++i;
	while (s[j] == ' ' || s[j] == '\t' || s[j] == '\r' || s[j] == '\n') --j;

	strcpy_s(str,924,s.substr(i, s.size() - (i + (s.size() - j))).c_str());
	return s.size();*/


	// удаляем пробелы и табы с начала строки:
	int i = 0, j;
	while ((str[i] == ' ') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == '\n'))
	{
		i++;
	}
	if (i > 0)
	{
		for (j = 0; j < strlen(str); j++)
		{
			str[j] = str[j + i];
		}
		str[j] = '\0';
	}

	// удаляем пробелы и табы с конца строки:
	i = strlen(str) - 1;
	while ((str[i] == ' ') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == '\n'))
	{
		i--;
	}
	if (i < (strlen(str) - 1))
	{
		str[i + 1] = '\0';
	}
	return i;
}