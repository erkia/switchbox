#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <windows.h>
#include <winreg.h>
#include <errno.h>

#include "pwrusb_internal.h"

#define SZ_FTDIBUS          "SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS"
#define SZ_PORT_NAME        "PortName"


static CHAR *getLastErrorText (CHAR *pBuf, ULONG bufSize)
{
    DWORD retSize;
    LPTSTR pTemp = NULL;

    if (bufSize < 16) {
        if (bufSize > 0) {
            pBuf[0] = '\0';
        }
        return pBuf;
    }

    retSize = FormatMessage (
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        NULL,
        GetLastError (),
        LANG_NEUTRAL,
        (LPTSTR)&pTemp,
        0,
        NULL
    );

    if (!retSize || pTemp == NULL) {
        pBuf[0] = '\0';
    } else {
        pTemp[strlen(pTemp)-2] = '\0'; //remove cr and newline character
        sprintf (pBuf,"%s (0x%x)", pTemp, (int)GetLastError());
        LocalFree ((HLOCAL)pTemp);
    }

    return pBuf;
}


static int EnumerateKey (HKEY hParentKey, const char *pszKeyName, char *buf, size_t buflen)
{
    char pszName[MAX_PATH] = "";
    DWORD dwNameLen = MAX_PATH;
    DWORD dwResult;
    DWORD dwKeyCount;
    DWORD dwValueCount;
    DWORD i;
    int gotIt = 0;
    HKEY hKey;
    DWORD lpType;
    char lpData[32];
    DWORD lpcbData;

    dwResult = RegOpenKeyEx (hParentKey, pszKeyName, 0, KEY_READ, &hKey);

    if (dwResult != ERROR_SUCCESS) {
        return 0;
    }

    // Get the number of keys and values for this parent
    dwResult = RegQueryInfoKey (
        hKey,
        NULL, NULL,
        NULL,
        &dwKeyCount,
        NULL,
        NULL,
        &dwValueCount,
        NULL, NULL, NULL, NULL
    );

    if (dwResult != ERROR_SUCCESS) {
        return 0;
    }

    // Enumerate values and search for PortName
    for (i = 0; i < dwValueCount; i++) {

        lpcbData = sizeof (lpData);
        dwResult = RegEnumValue (hKey, i, pszName, &dwNameLen, NULL, &lpType, (BYTE *)lpData, &lpcbData);

        if (dwResult == ERROR_SUCCESS) {

            if (strcmpi (pszName, SZ_PORT_NAME) == 0) {
                gotIt = strlen (lpData);
                if (gotIt < buflen) {
                    strcpy (buf, lpData);
                } else {
                    gotIt = -1;
                }
            }

        }

        pszName[0] = '\0';
        dwNameLen = MAX_PATH;

        if (gotIt != 0) {
            break;
        }

    }

    // Enumerate keys
    for (i = 0; i < dwKeyCount && gotIt == 0; i++) {

        dwResult = RegEnumKeyEx (hKey, i, pszName, &dwNameLen, NULL, NULL, NULL, NULL);

        if (dwResult == ERROR_SUCCESS) {
            gotIt = EnumerateKey (hKey, pszName, buf, buflen);
        }

        pszName[0] = '\0';
        dwNameLen = MAX_PATH;

    }

    RegCloseKey (hKey);

    return gotIt;
}


int pwrusb_search (const char *search_serial, char *buf, size_t buflen)
{
    char pszName[MAX_PATH] = "";
    char pszSearch[32];
    char *pszSerial;
    DWORD dwNameLen = MAX_PATH;
    HKEY hStartKey = HKEY_LOCAL_MACHINE;
    HKEY hKey;
    DWORD dwSubKeys;
    DWORD dwResult;
    DWORD i;
    int res = -1;

    if (strlen (search_serial) == 0) {
        return -1;
    }

    // Serial number to search for at the end of the key (VID_xxxx+PID_xxxx+xxxxxxxxA)
    strncpy (pszSearch, search_serial, sizeof (pszSearch));
    pszSearch[sizeof (pszSearch) - 2] = '\0';
    if (strcmp (pszSearch, search_serial)) {
        return -1;
    }

    pszSearch[strlen (pszSearch)] = 'A';
    pszSearch[strlen (pszSearch) + 1] = '\0';

    // printf ("%s\n", pszSearch);

    // http://www.ftdichip.com/Support/Knowledgebase/index.html?wherecanifindport.htm

    // Open FTDIBUS key
    dwResult = RegOpenKeyEx (
        hStartKey,
        SZ_FTDIBUS,
        0L,
        KEY_READ,
        &hKey
    );

    if (dwResult != ERROR_SUCCESS) {
        return -1;
    }

    // Get the number of keys under FTDIBUS
    dwResult = RegQueryInfoKey (
        hKey,
        NULL, NULL,
        NULL,
        &dwSubKeys,
        NULL,
        NULL,
        NULL,
        NULL, NULL, NULL, NULL
    );

    // Enumerate the keys under FTDIBUS
    for (i = 0; i < dwSubKeys; i++) {

        dwResult = RegEnumKeyEx (hKey, i, pszName, &dwNameLen, NULL ,NULL, NULL, NULL);

        if (dwResult == ERROR_SUCCESS) {
            if (!strnicmp (pszName, "VID_0403+", 9)) {
                pszSerial = strrchr (pszName, '+');
                if (pszSerial != NULL) {
                    pszSerial++;
//					printf ("pszSerial: %s\n", pszSerial);
                    if (!strcmpi (pszSerial, pszSearch)) {
                        res = EnumerateKey (hKey, pszName, buf, buflen);
                    }
                }
            }
        }

        pszName[0] = '\0';
        dwNameLen = MAX_PATH;

    }

    // Close FTDIBUS key
    RegCloseKey (hKey);

    return res;
}


int pwrusb_init (pwrusb_ctx *ctx)
{
    ctx->fd = INVALID_HANDLE_VALUE;
    return 0;
}


int pwrusb_open (pwrusb_ctx *ctx, const char *device)
{
    char filename[32];
    int res;
    res = snprintf (filename, sizeof (filename), "\\\\.\\%s", device);
    if (res < 0 || res >= sizeof (filename)) {
        return -1;
    }

    // https://123a321.wordpress.com/2010/02/01/serial-port-with-mingw/

    HANDLE hSerial = CreateFile (
        filename,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        0,
        0
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        char buf[32];
        getLastErrorText (buf, sizeof (buf));
        printf ("CreateFile() failed: %s\n", buf);
        return -1;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof (dcbSerialParams);
    if (!GetCommState (hSerial, &dcbSerialParams)) {
        printf ("GetCommState() failed\n");
        CloseHandle (hSerial);
        return -1;
    }

    dcbSerialParams.BaudRate = 115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    if (!SetCommState (hSerial, &dcbSerialParams)) {
        printf ("SetCommState() failed\n");
        CloseHandle (hSerial);
        return -1;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts (hSerial, &timeouts)) {
        printf ("SetCommTimeouts() failed\n");
        CloseHandle (hSerial);
        return -1;
    }

    ctx->fd = hSerial;

    return 0;
}




int pwrusb_get_state (pwrusb_ctx *ctx, int *state)
{
    uint8_t cmd = 0x80;
    DWORD dwResult = 0;

    *state = 0;

    if (!WriteFile (ctx->fd, &cmd, 1, &dwResult, NULL)) {
        return -1;
    }
    
    if (!ReadFile (ctx->fd, (uint8_t *)state, 1, &dwResult, NULL)) {
        return -1;
    }

    return 0;
}


int pwrusb_set_state (pwrusb_ctx *ctx, int *state)
{
    uint8_t cmd = 0xC0;
    DWORD dwResult = 0;

    if (!WriteFile (ctx->fd, &cmd, 1, &dwResult, NULL)) {
        return -1;
    }

    if (!WriteFile (ctx->fd, (uint8_t *)state, 1, &dwResult, NULL)) {
        return -1;
    }

    return 0;
}


int pwrusb_close (pwrusb_ctx *ctx)
{
    if (ctx->fd != INVALID_HANDLE_VALUE) {
        return CloseHandle (ctx->fd);
    } else {
        return -1;
    }
}

