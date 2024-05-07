﻿#include "exceptions.h"
#include "scene.h"

using namespace std;
using namespace mini;
using namespace gk2;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);
	auto exitCode = EXIT_FAILURE;
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	try
	{
		Scene app(hInstance);
		exitCode = app.Run();
	}
	catch (Exception& e)
	{
		MessageBoxW(nullptr, e.getMessage().c_str(), L"Błąd", MB_OK);
		exitCode = e.getExitCode();
	}
	catch (exception& e)
	{
		string s(e.what());
		MessageBoxW(nullptr, wstring(s.begin(), s.end()).c_str(), L"Błąd", MB_OK);
	}
	catch (const char* str)
	{
		string s(str);
		MessageBoxW(nullptr, wstring(s.begin(), s.end()).c_str(), L"Błąd", MB_OK);
	}
	catch (const wchar_t* str)
	{
		MessageBoxW(nullptr, str, L"Błąd", MB_OK);
	}
	catch (...)
	{
		MessageBoxW(nullptr, L"Nieznany Błąd", L"Błąd", MB_OK);
	}
	return exitCode;
}