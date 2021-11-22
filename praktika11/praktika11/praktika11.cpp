// praktika11.cpp : Определяет точку входа для приложения.
//
#include "framework.h"
#include "praktika11.h"
#include <iostream>
#include <time.h>
#include "windows.h"
#include "tlhelp32.h"
#include <CommCtrl.h>

#include "string.h"
#include <string>
#include "windowsx.h"
#include <mutex>
#include "Psapi.h"
#include "strsafe.h"

#define MAX_LOADSTRING 100


#define WM_ADDITEM WM_USER + 1
#define WM_ADDITEM1 WM_USER + 2
#define IDC_LB 1000
#define IDC_LBmodul 1001
#define IDC_BUT 1003
#define IDC_BUTwait 1004

HWND hButton;
HWND hButtonWait;
HWND hListBox;
HWND hListBoxModul;

TCHAR szBuffer[100] = TEXT("");
DWORD n = NULL;
TCHAR index[100] = TEXT("");

HANDLE Num = NULL;

// for mutex
#define COUNT_THREADS 1
#define ITERATIONS 100

// for mutex
DWORD dwCounter = 0;
HANDLE hMutex = NULL;

// for mutex
VOID Mutex(VOID);
DWORD WINAPI Func0(LPVOID lpParam);

void SerchPID();
void EnumerateProcs(HWND hwnd);
void EnumerateModules(HWND hWnd, DWORD PID);
void LoadProcessesToListBox(HWND hListBox);
void LoadModulesToListBox(HWND hwnd, DWORD dwProcessId);


// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

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
    LoadStringW(hInstance, IDC_PRAKTIKA11, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PRAKTIKA11));

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

    return (int) msg.wParam;
}

/*
VOID Mutex(VOID)
{
    HANDLE globalThreads[COUNT_THREADS] = { 0 };
    hMutex = CreateMutex(NULL, FALSE, L"name");
    globalThreads[0] = CreateThread(NULL, 0, Func0, NULL, NULL, 0);

    WaitForMultipleObjects(COUNT_THREADS, globalThreads, TRUE, INFINITE);

    if (globalThreads[0] != INVALID_HANDLE_VALUE) CloseHandle(globalThreads[0]);
}

DWORD WINAPI Func0(LPVOID lpParam)
{
    WaitForSingleObject(hMutex, INFINITE);
    //std::wcout << L"Func0: " << dwCounter++ << std::endl;
    //SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)"Func0 " + dwCounter++);
    ReleaseMutex(hMutex);
    ExitThread(0);
}
*/
void LoadProcessesToListBox(HWND hListBox)
{
    ListBox_ResetContent(hListBox);

    DWORD aProcessIds[1024], cbNeeded = 0;
    BOOL bRet = EnumProcesses(aProcessIds, sizeof(aProcessIds), &cbNeeded);

    if (FALSE != bRet)
    {
        TCHAR szName[MAX_PATH];
        for (DWORD i = 0, n = cbNeeded / sizeof(DWORD); i < n; ++i)
        {
            DWORD dwProcessId = aProcessIds[i], cch = 0;

            if (0 == dwProcessId) continue;

            HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
            
            if (NULL != hProcess)
            {
                cch = GetModuleBaseName(hProcess, NULL, szName, MAX_PATH);
                CloseHandle(hProcess);
            }

            if (0 == cch) StringCchCopy(szName, MAX_PATH, TEXT("Неизвестный процесс"));
            StringCchPrintf(szBuffer, _countof(szBuffer), TEXT("%s (PID:%u)"), szName, dwProcessId);

            int iItem = ListBox_AddString(hListBox, szBuffer);
            ListBox_SetItemData(hListBox, iItem, dwProcessId);
        }
    }
}

/*
void EnumerateProcs(HWND hWnd)
{
    ListBox_ResetContent(hListBox);

    HANDLE pSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    bool bIsok = false;
    PROCESSENTRY32 ProcEntry;
    ProcEntry.dwSize = sizeof(ProcEntry);
    bIsok = Process32First(pSnap, &ProcEntry);

    while (bIsok)
    {

        StringCchPrintf(szBuffer, _countof(szBuffer),
            TEXT("%s   (PID:%u)"), ProcEntry.szExeFile, ProcEntry.th32ProcessID);
        SendMessage(hWnd, WM_ADDITEM, 0, 0);
        bIsok = Process32Next(pSnap, &ProcEntry);
        int iItem = ListBox_AddString(hListBox, szBuffer);
        ListBox_SetItemData(hListBox, iItem, bIsok);  
    }
    CloseHandle(pSnap);
}
*/

void LoadModulesToListBox(HWND hwnd, DWORD dwProcessId)
{
    ListBox_ResetContent(hListBoxModul);
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
    if (NULL != hProcess)
    {
        DWORD cb = 0;
        EnumProcessModulesEx(hProcess, NULL, 0, &cb, LIST_MODULES_ALL);

        DWORD nCount = cb / sizeof(HMODULE);
        HMODULE* hModule = new HMODULE[nCount];

        cb = nCount * sizeof(HMODULE);
        BOOL bRet = EnumProcessModulesEx(hProcess, hModule, cb, &cb, LIST_MODULES_ALL);
        if (FALSE != bRet)
        {
            TCHAR szFileName[MAX_PATH];
            for (DWORD i = 0; i < nCount; ++i)
            {
                bRet = GetModuleFileNameEx(hProcess, hModule[i], szFileName, MAX_PATH);
                if (FALSE != bRet)
                    ListBox_AddString(hListBoxModul, szFileName);
            }
        }
        int iItem = ListBox_AddString(hListBoxModul, szBuffer);
        ListBox_SetCurSel(hListBoxModul, iItem);

        delete[]hModule;
        CloseHandle(hProcess);
    }
}

/*
void EnumerateModules(HWND hWnd, DWORD PID)
{
    ListBox_ResetContent(hListBoxModul);
    HANDLE pMdlSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PID);
    BOOL bIsok = false;
    MODULEENTRY32 MdlEntry;
    MdlEntry.dwSize = sizeof(MODULEENTRY32);
    bIsok = Module32First(pMdlSnap, &MdlEntry);

    while (bIsok)
    {
        StringCchPrintf(szBuffer, _countof(szBuffer), L"%s  ", MdlEntry.szModule);
        SendMessage(hListBoxModul, WM_ADDITEM1, 0, 0);
        bIsok = Module32Next(pMdlSnap, &MdlEntry);
        int iItem = ListBox_AddString(hListBoxModul, szBuffer);
        ListBox_SetCurSel(hListBoxModul, iItem);
    }

    CloseHandle(pMdlSnap);
}
*/
/*
void SerchPID()
{
    TCHAR b = sizeof(szBuffer);
    for (int i = 0; i < b; i++)
    {
        if (szBuffer[i] == ':')
        {  
            int h = 0;
            for (int j = i + 1; j < b; j++)
            {
                if (szBuffer[j] != ')')
                {
                    if (szBuffer[j] == '\0')
                    {
                        continue;
                    }
                    else
                    {
                        index[h] = szBuffer[j];
                        h++;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        else if (szBuffer[i] == '\0') break;
    }
    n = (DWORD)index;
}
*/
/*
void ProcID()
{
    int iOldBuff = 0, iNewBuff = 0;
    char newBuff[100];
    bool flag = true, flag1 = true, flag2 = false;

    while (iOldBuff < _countof(szBuffer)) 
    {

        switch (szBuffer[iOldBuff]) 
        {
            case ' ':
                flag2 = true;
                break;
        }

        if (flag2 == true)
        {
            switch (szBuffer[iOldBuff]) {
            case '0':
                flag = false;
                break;
            case '1':
                flag = false;
                break;
            case '2':
                flag = false;
                break;
            case '3':
                flag = false;
                break;
            case '4':
                flag = false;
                break;
            case '5':
                flag = false;
                break;
            case '6':
                flag = false;
                break;
            case '7':
                flag = false;
                break;
            case '8':
                flag = false;
                break;
            case '9':
                flag = false;
                break;
            default:
                if (flag == false)
                {
                    flag1 = false;
                    break;
                }
                else
                {
                    iOldBuff++;
                    continue;
                }
            }
        }

        if (flag1 == true)
        {
            if (flag2 == true)
            {
                newBuff[iNewBuff] = szBuffer[iOldBuff];
                iNewBuff++;
            }
            iOldBuff++;
        }
        else
        {
            break;
        }
        n = atoi(newBuff);
    }
}
*/
//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PRAKTIKA11));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PRAKTIKA11);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

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
    HWND SP = GetDlgItem(hWnd, IDC_LB);
    int itm = ListBox_GetCurSel(SP);

    switch (message)
    {
    case WM_CREATE:
    {
        hListBox = CreateWindowEx(0, TEXT("ListBox"), 
            NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD, 20, 10, 300, 550, hWnd, (HMENU)IDC_LB, hInst, NULL);
        LoadProcessesToListBox(hListBox);

        hListBoxModul = CreateWindowEx(0, WC_LISTBOX, TEXT("ListBoxModul"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_STANDARD, 360, 10, 300, 550, hWnd, (HMENU)IDC_LBmodul, hInst, NULL);

        hButton = CreateWindowEx(0, WC_BUTTON, TEXT("delete process"),
            WS_CHILD | WS_VISIBLE | WS_BORDER, 680, 10, 300, 30, hWnd, (HMENU)IDC_BUT, hInst, NULL);

        hButtonWait = CreateWindowEx(0, WC_BUTTON, TEXT("wait for process 5 sec"),
            WS_CHILD | WS_VISIBLE | WS_BORDER | BS_NOTIFY, 680, 50, 300, 30, hWnd, (HMENU)IDC_BUTwait, hInst, NULL);
    }

    case WM_COMMAND:
        {
            int wmEvent = HIWORD(wParam);
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDC_BUTwait:
            {
                DWORD num = SendMessage(hListBox, LB_GETCURSEL, 0, wParam);
                n = ListBox_GetItemData(hListBox, num);               
                DWORD tm = 1200;
                HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, NULL, n);
                if (proc != NULL) {
                    WaitForSingleObject(proc, tm);
                    CloseHandle(proc);
                }
                MessageBox(hWnd, TEXT("Ожидание процесса закончилось"), TEXT("Информация"), MB_OK | MB_ICONINFORMATION);
            }
            break;
            case IDC_BUT:
            {
                DWORD num = SendMessage(hListBox, LB_GETCURSEL, 0, wParam);
                n = ListBox_GetItemData(hListBox, num);
                if (TerminateProcess(OpenProcess(PROCESS_TERMINATE, NULL, n), -1))
                {
                    MessageBox(hWnd, TEXT("Завершение процесса"), TEXT("Информация"), MB_OK | MB_ICONINFORMATION);
                    ListBox_DeleteString(SP, itm);
                }
            }
            break;
            case IDC_LB:
            {
                if (wmEvent == LBN_DBLCLK)
                {
                    SendMessage(hListBoxModul, LB_RESETCONTENT, 0, 0);
                    DWORD num = SendMessage(hListBox, LB_GETCURSEL, 0, wParam);
                    n = ListBox_GetItemData(hListBox, num);
                    LoadModulesToListBox(hListBoxModul, n);
                }
            }
            break;
            case IDC_LBmodul:
            {

            }
            break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
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
