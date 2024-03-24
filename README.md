# NazunaRansom
NazunaRansom, es un ransomware inspirado en mi waifu (Nazuna)

### Caracteristicas

- Usa cifrado AES-128 en modo CBC
- Cambia el fondo de pantalla (descarga la imagen de internet)
- Cifra los archivos de un directorio y sus subdirectorios
- Implementa una persistencia basica basada en el registro para ejectuar el ransomware cuando se vuelva a prender la computadora
- Crea un hilo el cual tiene un temporizador de 5 minutos antes de que llame a una funcion para que se eliminen los archivos cifrados (el temporizador se reinicia al cerrar sesion o apagar la computadora)
- Si se abre el administrador de tareas los archivos se eliminan inmediatamente
- Se implementa CheckRemoteDebuggerPresent como tecnica anti depuracion
- Se implementa una exclusion mutua (mutex) para que si la computadora ya fue infectada, no vuelve a crear otro proceso
- La key para cifrar los archivos esta intencionalmente creada para que sea la misma secuencia del 0 al 15

### Funciones de cifrado

**CryptAcquireContext**

```c
if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0)) {
    if (!CryptAcquireContext(&hProv, NULL, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET)) {
        return FALSE;
    }
}
```
En el primer if trata de obtener el identificador del contenedor CSP, ```hProv``` funciona como un puntero que almacena el identificador CSP, si no se puede obtener el identifcador se vuelva a llamar a ```CryptAcquireContext``` con ```CRYPT_NEWKEYSET``` para crear un contenedor de claves en CSP

**CryptGenKey**

```c
CryptGenKey(hProv, CALG_AES_128, CRYPT_EXPORTABLE, &hkey);
```
Genera una key AES-128 de acuerdo al identificador del CSP y la key la almacena en ```hkey```

**CryptEncrypt**

```c
CryptEncrypt(hkey, 0, TRUE, 0, buf, &bytes_leer, BUF_LEN);
```
Cifra los datos de acuerdo al CSP, ```buf``` es un puntero al buffer datos que contiene los datos que se van a cifrar, ```bytes_leer``` es la cantidad de bytes que se van a cifrar, que son los bytes del archivo original, este buffer cifrado se sobreescribira por el original con unas funciones más adelante

### Funcion recorrer_dir

Las funciones ```FindFirstFile``` y ```FindNextFile``` se usan para recorrer el directorios y subdirectorios desde la ruta almacenada en ```ruta_encriptar```, el segundo if verifica si ```ruta_data``` es un directorio mediante el AND y ```FILE_ATTRIBUTE_DIRECTORY```, el tercer if usando ```strcmp``` se compara ```ruta_data.cFileName``` con ```.``` y ```..``` para ver si ```ruta_data``` se refiere al directorio actual(.) o al directorio padre(..), si es diferente de 0(que no corresponde a ninguno de los dos) y se cumple la condicion anterior, se crea una variable llamada ```subruta_recorrer``` la cual representa la ruta del subdirectorio que se va a recorrer y la agrega a ```subdirectorios```.

El penultimo if representa las extensiones y hace un ```OR``` entre todas las extensiones, si encuentra una extension entrara al ultimo if el cual verifica si ```cifrar_archivos``` regresa ```true```, y en caso de que si, cambia la extension del archivo a .Nazuna y lo renombra con esa extension

### Funcion eliminar_archivos_cifrados

Tiene un funcionamiento similar a recorrer_dir, solo que se verifica si la extension es igual a .Nazuna (```extension == ".Nazuna"```), y en caso de que si sea, se elimina el archivo

### Funcion persistencia

Es la que se encarga de que el ransomware de inicie al prender la compu e iniciar sesion, simplemente usa ```RegOpenKeyEx``` para abrir el registro en la ruta almacenada en ```lpSubKey``` y despues usa ```RegSetValueEx``` para establecer una entrada el registro, donde ```lpValue``` es la ruta del ejecutable, y ```lpValueName``` es el nombre del ejecutable.

### Funcion eliminar_archivos_routine

Es la funcion que se ejecutara al crear un hilo con ```CreateThread```, el parametro ```lpParam``` indica la ruta del directorio que se va a recorrer para eliminar los archivos, despues se agrega un pequeño temporizador de 5 minutos para "detener" la ejecucion del hilo 5 minutos antes de llamar a ```eliminar_archivos_cifrados```

### Detectar al administrador de tareas

Dentro del main cuando se hace uso de ```CreateThread```, si el identificador del hilo es diferente de NULL (hThread != NULL) entra en un bucle infinito donde mediante un if y la funcion ```FindWindowA``` esta constantemente buscando por la ventana con el nombre ```TaskManagerWindow``` y cuando la encuentre manda a llamar a ```eliminar_archivos_cifrados```

### Modificaciones

Dentro de la funcion ```recorrer_dir``` puedes especificar las extensiones de los archivos que quieres que se cifren:

```c
if (extension == ".txt" || extension == ".cpp" || extension == ".pdf" || extension == ".docx" || extension == ".xlsx")
```

En la funcion ```main``` puedes especificar otro directorio que se va a cifrar

```c
string ruta = filesystem::path(string(getenv("USERPROFILE")) + "\\Desktop").string();
```
Solo modifica ```Desktop``` por otra, ej: ```Documents```

En la funcion routine ```eliminar_archivos_routine``` le puedes indicar el tiempo en minutos que deseas que espere antes de borrar los archivos

```c
this_thread::sleep_for(chrono::minutes(1)); <-- El numero indica los minutos
```

Puedes cambiar el valor del mutex por otro que no este en uso

```c
#define MUTEX "TeAmoNazuna" --> #define MUTEX "otroValor"
```

En la funcion ```persistencia``` se ejecuta el ransomware desde la ruta Downloads con el nombre ```NazunaRansom```, puedes cambiar la ruta indicandolo en:

```c
string ruta_descarga = filesystem::path(string(getenv("USERPROFILE")) + "\\Downloads").string();
```

Y puedes cambiar el nombre del ejecutable en:

```c
LPCSTR lpValueName = "NazunaRansom";
```

### Compilado
Yo lo compile desde GNU/Linux con el comando

```bash
x86_64-w64-mingw32-g++ -o NazunaRansom NazunaRansom.cpp -lbcrypt -static -static-libgcc -static-libstdc++ -lurlmon
```

![](https://github.com/ic4rta/NazunaRansom/blob/main/Nazuna_Ransom_Fondo.jpg)
