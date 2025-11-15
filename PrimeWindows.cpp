#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

bool isPrime(int n) {
    if (n < 2) return false;
    for (int d = 2; d * d <= n; ++d)
        if (n % d == 0) return false;
    return true;
}

int main(int argc, char* argv[]) {
    const int NUM_PROCESSES = 10;
    const int RANGE_SIZE = 1000;
    const int MAX_NUMBER = 10000;

    // daca avem 2 argumente: start si end
    if (argc == 3) {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);

        for (int n = start; n <= end; ++n) {
            if (isPrime(n)) {
                cout << n << " ";
            }
        }
        return 0;
    }

    cout << "Numere prime intre 1 si " << MAX_NUMBER << ":\n";

    for (int i = 0; i < NUM_PROCESSES; ++i) {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        // pipe: copil scrie -> parinte citeste
        HANDLE childStd_OUT_Rd;
        HANDLE childStd_OUT_Wr;
        CreatePipe(&childStd_OUT_Rd, &childStd_OUT_Wr, &saAttr, 0);
        SetHandleInformation(childStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.hStdError = childStd_OUT_Wr;
        si.hStdOutput = childStd_OUT_Wr;
        si.dwFlags |= STARTF_USESTDHANDLES;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        int start = i * RANGE_SIZE + 1;
        int end = (i + 1) * RANGE_SIZE;
        if (end > MAX_NUMBER) end = MAX_NUMBER;

        string cmdLine = string(argv[0]) + " " +
            to_string(start) + " " +
            to_string(end);

 
        CreateProcessA(
            NULL,
            cmdLine.data(),
            NULL,
            NULL,
            TRUE,   // copilul mosteneste handle-urile
            0,
            NULL,
            NULL,
            &si,
            &pi
        );

        CloseHandle(childStd_OUT_Wr); // nu mai scriem in pipe din parinte

        // citim ce a scris copilul
        char buffer[256];
        DWORD bytesRead;
        while (true) {
            BOOL ok = ReadFile(childStd_OUT_Rd, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
            if (!ok || bytesRead == 0) break;
            buffer[bytesRead] = '\0';
            cout << buffer; // afisam direct
        }

        CloseHandle(childStd_OUT_Rd);

        // asteptam copilul sa termine
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    cout << "\n";
    return 0;
}
