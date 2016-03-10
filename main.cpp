#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <memory>

void FileTimeElapsed(const FILETIME& start, const FILETIME& end, FILETIME* elapsed);
void PrintFileTimeAsDate(wchar_t* prefix, const FILETIME& filetime);
void PrintFileTimeAsElapsed(wchar_t* prefix, const FILETIME& filetime);

int wmain(int argc, wchar_t* argv[])
{
    wchar_t* cmdline = GetCommandLineW();

    cmdline += wcslen(argv[0]);
    while (iswspace(*cmdline)) ++cmdline;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    if (!CreateProcessW(nullptr, cmdline, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        wprintf(L"CreateProcess failed (%d).\n", GetLastError());
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    FILETIME create;
    FILETIME exit;
    FILETIME elapsed;
    FILETIME kernel;
    FILETIME user;

    if (!GetProcessTimes(pi.hProcess, &create, &exit, &kernel, &user))
    {
        wprintf(L"GetProcessTimes failed (%d).\n", GetLastError());
        return 1;
    }

    FileTimeElapsed(create, exit, &elapsed);

    PrintFileTimeAsDate(L"Creation time:  ", create);
    PrintFileTimeAsDate(L"Exit time:      ", exit);
    PrintFileTimeAsElapsed(L"Elapsed:        ", elapsed);
    PrintFileTimeAsElapsed(L"Kernel elapsed: ", kernel);
    PrintFileTimeAsElapsed(L"User elapsed:   ", user);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

void FileTimeElapsed(const FILETIME& start, const FILETIME& end, FILETIME* elapsed)
{
    ULARGE_INTEGER uli;
    int64_t left, right, result;
    
    uli.LowPart = start.dwLowDateTime;
    uli.HighPart = start.dwHighDateTime;
    right = uli.QuadPart;
    
    uli.LowPart = end.dwLowDateTime;
    uli.HighPart = end.dwHighDateTime;
    left = uli.QuadPart;

    result = left - right;

    uli.QuadPart = result;
    elapsed->dwLowDateTime = uli.LowPart;
    elapsed->dwHighDateTime = uli.HighPart;
}

void PrintFileTimeAsDate(wchar_t* prefix, const FILETIME& filetime)
{
    wprintf(L"%s", prefix);

    FILETIME localtime;
    SYSTEMTIME systemtime;

    if (!FileTimeToLocalFileTime(&filetime, &localtime))
    {
        wprintf(L"FileTimeToLocalFileTime failed (%d).\n", GetLastError());
        return;
    }

    if (!FileTimeToSystemTime(&localtime, &systemtime))
    {
        wprintf(L"FileTimeToSystemTime failed (%d).\n", GetLastError());
        return;
    }

    int size = GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &systemtime, nullptr, nullptr, 0);
    if (size == 0)
    {
        wprintf(L"GetTimeFormatEx length check failed (%d).\n", GetLastError());
        return;
    }

    std::unique_ptr<wchar_t []> buffer = std::make_unique<wchar_t []>(size);

    if (!GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &systemtime, nullptr, buffer.get(), size))
    {
        wprintf(L"GetTimeFormatEx failed (%d).\n", GetLastError());
        return;
    }

    wprintf(L"%s", buffer.get());

    size = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &systemtime, nullptr, nullptr, 0, nullptr);
    if (size == 0)
    {
        wprintf(L"GetDateFormatEx length check failed (%d).\n", GetLastError());
        return;
    }

    buffer = std::make_unique<wchar_t []>(size);

    if (!GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &systemtime, nullptr, buffer.get(), size, nullptr))
    {
        wprintf(L"GetDateFormatEx failed (%d).\n", GetLastError());
        return;
    }

    wprintf(L" %s\n", buffer.get());
}

void PrintFileTimeAsElapsed(wchar_t* prefix, const FILETIME& filetime)
{
    wprintf(L"%s", prefix);

    ULARGE_INTEGER uli;
    int64_t time;

    uli.LowPart = filetime.dwLowDateTime;
    uli.HighPart = filetime.dwHighDateTime;
    time = uli.QuadPart;

    int64_t ms   = time / 10000 % 1000;
    int64_t sec  = time / 10000000 % 60;
    int64_t min  = time / 10000000 / 60 % 60;
    int64_t hr   = time / 10000000 / 60 / 60 % 24;
    int64_t days = time / 10000000 / 60 / 60 / 24;

    if (days > 0)
        wprintf(L"%I64d day%s ", days, days == 1 ? L"" : L"s");

    if (hr > 0)
        wprintf(L"%02I64d:", hr);

    if (hr > 0 || min > 0)
        wprintf(L"%02I64d:", min);

    if (hr > 0 || min > 0)
        wprintf(L"%02I64d.%03I64d\n", sec, ms);
    else
        wprintf(L"%I64d.%03I64d\n", sec, ms);
}
