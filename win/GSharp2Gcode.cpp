/* GSharp2Gcode.cpp : Defines the entry point for the application.
*
*  Copyright 2016, Night Road Software (https://github.com/nrsoft)
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are
*  met:
*
*      * Redistributions of source code must retain the above copyright
*  notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*  copyright notice, this list of conditions and the following disclaimer
*  in the documentation and/or other materials provided with the
*  distribution.
*      * Neither the name of "Night Road Software" nor the names of its
*  contributors may be used to endorse or promote products derived from
*  this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*  *********************************************************************
*
*  Stand-alone interpreter of G# macro-programming language into plain G-Code
*  (GUI version for Windows OS)
*
*  G# is a subset of LinuxCNC (https://github.com/LinuxCNC/linuxcnc)
*    and RS274/NGC (https://www.nist.gov/customcf/get_pdf.cfm?pub_id=823374)
*
*  For compatibility with the above standards please visit
*    https://github.com/nrsoft/gsharp or check "gsharp.h" header
*
*  This code was written with the help from:
*   http://www.instructables.com/id/Making-a-simple-application-using-the-Win32-API
*/
#include "stdafx.h"

#include <commdlg.h>
#include <string>
#include <sstream>
#include <fstream>

#include "GSharp2Gcode.h"
#include "..\include\gsharp.h"
#include "..\include\gsharp_extra.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

static HWND hwndInFile;
static HWND hwndOutFile;
static HWND hwndMsgStr;
static WCHAR strMsg[80];

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL                ConvertGSharp2GCode(WCHAR* strFileIn, WCHAR* strFileOut);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GS2GC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GS2GC));

    MSG msg;

    // Main message loop:
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

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GS2GC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);// CreateSolidBrush(RGB(180, 180, 180));
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GS2GC);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 450, 180, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WCHAR strFileIn[260], strFileOut[260]; // buffer for file names
	switch (message)
    {
	case WM_CREATE:
		{
			HFONT defaultFont;
			defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			HWND hwndInStr = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("STATIC"), TEXT("Input:"), WS_CHILD | WS_VISIBLE,
				15, 12, 50, 22, hWnd, NULL, GetModuleHandle(NULL), NULL);
			SendMessage(hwndInStr, WM_SETFONT, WPARAM(defaultFont), TRUE);
			HWND hwndOutStr = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("STATIC"), TEXT("Output:"), WS_CHILD | WS_VISIBLE,
				10, 42, 50, 22, hWnd, NULL, GetModuleHandle(NULL), NULL);
			SendMessage(hwndOutStr, WM_SETFONT, WPARAM(defaultFont), TRUE);
			hwndInFile = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
				70, 10, 300, 22, hWnd, (HMENU)IDC_INPUT_FILE, GetModuleHandle(NULL), NULL);
			SendMessage(hwndInFile, WM_SETFONT, WPARAM(defaultFont), TRUE);
			hwndOutFile = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
				70, 40, 300, 22, hWnd, (HMENU)IDC_OUTPUT_FILE, GetModuleHandle(NULL), NULL);
			SendMessage(hwndOutFile, WM_SETFONT, WPARAM(defaultFont), TRUE);
			HWND hwndInBtn = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("..."), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				385, 10, 30, 22, hWnd, (HMENU)IDC_SELECT_IN, GetModuleHandle(NULL), NULL);
			SendMessage(hwndInBtn, WM_SETFONT, WPARAM(defaultFont), TRUE);
			HWND hwndOutBtn = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("..."), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				385, 40, 30, 22, hWnd, (HMENU)IDC_SELECT_OUT, GetModuleHandle(NULL), NULL);
			SendMessage(hwndOutBtn, WM_SETFONT, WPARAM(defaultFont), TRUE);
			HWND hwndConvertBtn = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("Convert"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				100, 75, 120, 22, hWnd, (HMENU)IDC_CONVERT, GetModuleHandle(NULL), NULL);
			SendMessage(hwndConvertBtn, WM_SETFONT, WPARAM(defaultFont), TRUE);
			HWND hwndAboutBtn = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("About"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				280, 75, 70, 20, hWnd, (HMENU)IDC_ABOUT, GetModuleHandle(NULL), NULL);
			SendMessage(hwndAboutBtn, WM_SETFONT, WPARAM(defaultFont), TRUE);
			hwndMsgStr = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("STATIC"),
				TEXT("Select input and output files for conversion"), WS_CHILD | WS_VISIBLE,
				10, 110, 430, 20, hWnd, NULL, GetModuleHandle(NULL), NULL);
			SendMessage(hwndMsgStr, WM_SETFONT, WPARAM(defaultFont), TRUE);
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDC_SELECT_IN:
			{
				OPENFILENAME ofn;       // common dialog box structure
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = strFileIn;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(strFileIn);
				ofn.lpstrFilter = L"All\0*.*\0G-Sharp\0*.g#\0";
				ofn.nFilterIndex = 2;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				if (GetOpenFileName(&ofn) == TRUE)
					SetWindowText(hwndInFile, strFileIn);
				SetWindowText(hwndMsgStr, L"\0");
			}
				break;
			case IDC_SELECT_OUT:
			{
				OPENFILENAME ofn;
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFile = strFileOut;
				ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(strFileOut);
				ofn.lpstrFilter = L"All\0*.*\0G-Code\0*.nc;*.ngc;*.gc;*.cnc\0";
				ofn.nFilterIndex = 2;

				if (GetOpenFileName(&ofn) == TRUE)
					SetWindowText(hwndOutFile, strFileOut);
				SetWindowText(hwndMsgStr, L"\0");
			}
			break;
			case IDC_CONVERT:
				GetWindowText(hwndInFile, strFileIn, 260);
				GetWindowText(hwndOutFile, strFileOut, 260);
				ConvertGSharp2GCode(strFileIn, strFileOut);
				SetWindowText(hwndMsgStr, strMsg);
				break;
			case IDC_ABOUT:
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
            // TODO: Add any drawing code that uses hdc here...
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

// Message handler for about box.
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

// Main interpreter routine
using namespace std;
BOOL ConvertGSharp2GCode(WCHAR* strFileIn, WCHAR* strFileOut)
{
	gsharp::Interpreter r;

	if(strFileIn[0] == L'\0' || strFileOut[0] == L'\0') {
		swprintf(strMsg, 80, L"Please select both input and output files");
		return FALSE;
	}

	// open input file
	char strFileName[260];
	WideCharToMultiByte(CP_UTF8, 0, strFileIn, -1, strFileName, 260, NULL, NULL);
	string filename(strFileName);
	ifstream file_in(filename, ifstream::in);
	if(!file_in.good()){
		swprintf(strMsg, 80, L"Cannot open file: %s", strFileIn);
		return FALSE;
	}

	// read file into string stream
	stringstream ss;
	ss << file_in.rdbuf();

	// Load program
	try{
		r.Load(ss.str());
	}
	catch(exception& e){
		swprintf(strMsg, 80, L"File parsing error: %hs", e.what());
		return FALSE;
	}

	// create output file
	WideCharToMultiByte(CP_UTF8, 0, strFileOut, -1, strFileName, 260, NULL, NULL);
	filename = strFileName;
	ofstream file_out(filename, ifstream::out);
	if(!file_out.good()){
		swprintf(strMsg, 80, L"Cannot create file: %s", strFileOut);
		return FALSE;
	}

	// run interpreter
	string str;
	gsharp::ExtraInfo extra;
	try {
		while(r.Step(str, extra)){
			// record next G-Code line
			if(!str.empty())
				file_out << str << endl;
		}
	}
	catch(exception& e){
		swprintf(strMsg, 80, L"Interpreter error: %hs", e.what());
		return FALSE;
	}

	swprintf(strMsg, 80, L"Success");
	return TRUE;
}
