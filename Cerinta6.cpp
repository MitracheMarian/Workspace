#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SHARED_MEM_NAME   L"Local\\SharedCounterExample"
#define SEMAPHORE_NAME    L"Local\\SharedCounterSemaphore"
#define MAX_COUNT         1000

typedef struct SharedData {
    LONG counter;
} SharedData;

HANDLE init_shared_memory(BOOL* pIsCreator, SharedData** ppData) {
    *pIsCreator = FALSE;
    *ppData = NULL;

    HANDLE hMap = CreateFileMappingW(
        INVALID_HANDLE_VALUE,      
        NULL,                      
        PAGE_READWRITE,            
        0,                         
        sizeof(SharedData),        
        SHARED_MEM_NAME            
    );

    if (hMap == NULL) {
        printf("CreateFileMappingW a esuat, error = %lu\n", GetLastError());
        return NULL;
    }

    DWORD lastErr = GetLastError();
    if (lastErr != ERROR_ALREADY_EXISTS) {
        *pIsCreator = TRUE; 
    }

    LPVOID pView = MapViewOfFile(
        hMap,
        FILE_MAP_ALL_ACCESS,
        0, 0,
        sizeof(SharedData)
    );
    if (!pView) {
        printf("MapViewOfFile a esuat, error = %lu\n", GetLastError());
        CloseHandle(hMap);
        return NULL;
    }

    *ppData = (SharedData*)pView;

    if (*pIsCreator) {
        (*ppData)->counter = 0;
        printf("[PID %lu] Am creat memoria partajata, counter = 0\n",
            GetCurrentProcessId());
    }
    else {
        printf("[PID %lu] Am deschis memoria partajata, counter curent = %ld\n",
            GetCurrentProcessId(), (*ppData)->counter);
    }

    return hMap;
}

HANDLE init_semaphore() {
    HANDLE hSem = CreateSemaphoreW(
        NULL,   
        1,      
        1,      
        SEMAPHORE_NAME
    );

    if (!hSem) {
        printf("CreateSemaphoreW a esuat, error = %lu\n", GetLastError());
        return NULL;
    }

    return hSem;
}

void worker_loop() {
    BOOL isCreator = FALSE;
    SharedData* pData = NULL;

    HANDLE hMap = init_shared_memory(&isCreator, &pData);
    if (!hMap) return;

    HANDLE hSem = init_semaphore();
    if (!hSem) {
        UnmapViewOfFile(pData);
        CloseHandle(hMap);
        return;
    }

    srand((unsigned int)(time(NULL) ^ GetCurrentProcessId()));

    while (1) {
        DWORD dw = WaitForSingleObject(hSem, INFINITE);
        if (dw != WAIT_OBJECT_0) {
            printf("[PID %lu] WaitForSingleObject a esuat, error = %lu\n",
                GetCurrentProcessId(), GetLastError());
            break;
        }

        LONG current = pData->counter;

        if (current >= MAX_COUNT) {
            ReleaseSemaphore(hSem, 1, NULL);
            printf("[PID %lu] Counter >= %d, ies din bucla.\n",
                GetCurrentProcessId(), MAX_COUNT);
            break;
        }

        printf("[PID %lu] Tura noua: pornesc de la %ld\n",
            GetCurrentProcessId(), current);

        while (current < MAX_COUNT) {
            int coin = (rand() % 2) + 1;
            if (coin == 2) {
                current++;
                pData->counter = current;
                printf("[PID %lu] coin = 2, scriu %ld in memorie.\n",
                    GetCurrentProcessId(), current);
            }
            else {
                printf("[PID %lu] coin = 1, ma opresc in tura asta.\n",
                    GetCurrentProcessId());
                break;
            }
        }

        if (!ReleaseSemaphore(hSem, 1, NULL)) {
            printf("[PID %lu] ReleaseSemaphore a esuat, error = %lu\n",
                GetCurrentProcessId(), GetLastError());
            break;
        }

        if (current >= MAX_COUNT) {
            printf("[PID %lu] Am ajuns la %d, termin.\n",
                GetCurrentProcessId(), MAX_COUNT);
            break;
        }

        Sleep(10 + (rand() % 40));
    }

    UnmapViewOfFile(pData);
    CloseHandle(hMap);
    CloseHandle(hSem);
}

int main(int argc, wchar_t* argv[]) {
    printf("[PID %lu] Pornesc proces.\n", GetCurrentProcessId());
    worker_loop();
    printf("[PID %lu] Termin proces.\n", GetCurrentProcessId());
    return 0;
}
