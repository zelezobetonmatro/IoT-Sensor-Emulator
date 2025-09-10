#Flow projektu
#TCP styl komunikacie potrebny nakolko potrebujeme mat istotu ze data prisli/odisli
#v Ccku chces mat istotu vsade inac hrozia rozne vulns/dos hrozby

#1. Senzory (sensors_src/)
Každý súbor sensor_*.c simuluje jeden senzor (teplota, vlhkosť, tlak...).
musi byt template_senzor z ktoreho budu "dedit" dalsie senzory,
template si vytvorte a bude obsahovat funkcie ktore budu
standardne komunikovat s boardom <- dohodnite sa :3
Planujte ze funkcie co budu generovat hodnoty budu lahko zamenitelne
s realnym senzorom (cize data budu chodit z nejakeho endpointu)
Kompiluje sa do modules/sensor_*.so. <- odtial si ich doska bude nacitavat

#2. Board (doska)
Načítava všetky .so senzory z modules/. (dynamic loading)
Každý senzor sa volá raz za sekundu, hodnota sa uloží do kruhového bufferu (256 sekúnd).

Poskytuje REST API:
/api/v1/sensors – zoznam senzorov + posledná hodnota
/api/v1/window?name=... – okno 256 vzoriek

#3. Processor

Volá board API, vyťahuje dáta.
Spracuje (min, max, avg, last).
Poskytuje REST API /api/v1/summary. <- odporucam peknucky json nech web guy nemusi plakat

#4. Web
http server v C, basic errory handlovat, simplish auth v C staci s hard coded credentials
Servuje HTML stránku + JS + CSS.
JavaScript každé 2s fetchne dáta z processor apis a vykreslí tabuľku.

#Rozdelenie úloh v tíme
Osoba 1 – senzory: vytvoriť viaceré senzory podľa template_sensor.c (každý vlastný .c súbor).
Osoba 2 – board: načítavanie .so senzorov (dlopen), kruhový buffer, REST API /sensors a /window.
Osoba 3 – processor: fetch dát z boardu, výpočty (avg, min, max), REST API /summary.
Osoba 4 – web: statické súbory (HTML, JS, CSS), jednoduchý HTTP server.

#Resources
https://octobot.medium.com/how-git-internally-works-1f0932067bee <= ako funguje git (OPTIONAL)
https://www.learn-c.org/ <= bez aika vsetko tu su basics ktore obsahuju syntax snippets, ktore vam stacia
https://tldp.org/HOWTO/Program-Library-HOWTO/dl-libraries.html <= jednoduche dl (dynamic loading)
https://beej.us/guide/bgnet/ <= jednoduchy network programming mozno sa nieco z toho zide
https://www.json.org/json-en.html <= presne ako funguje json
https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API/Using_Fetch <= simple fetch

#Vysvetlenie pripon súborov
.c – zdrojový kód v jazyku C (to co pises).
.h – hlavičkový súbor, deklarácie funkcií/štruktúr, ktoré môžu používať iné .c.
.so – shared object (zdieľaná knižnica, plugin). Kompiluje sa z .c cez -shared -fPIC.
Board potom vie načítať každý sensor_*.so bez rekompilácie. (.so)

#Datove typy
| Typ        | Rozsah / význam                           |
| ---------- | ----------------------------------------- |
| `int`      | celé číslo (zvyčajne 32 bit)              |
| `unsigned` | nezáporné celé číslo                      |
| `char`     | 1 bajt (znak alebo číslo 0–255)           |
| `double`   | desatinné číslo (64 bit float)            |
| `float`    | desatinné číslo (32 bit)                  |
| `uint32_t` | presne 32-bit celé číslo (z `<stdint.h>`) |
| `size_t`   | veľkosť v pamäti (unsigned int)           |
| `void*`    | generický ukazovateľ                      |
| `struct`   | zoskupenie premenných                     |


#Syntax cheatsheet
// Premenná
int x = 5;
double t = 21.5;
char msg[16] = "ahoj";

// Podmienka
if (x > 10) { ... } else { ... }

// Cykly
for (int i=0; i<10; i++) { ... }
while (x < 100) { ... }

// Funkcia
int add(int a, int b) {
    return a + b;
}

// Štruktúra
typedef struct {
    double value;
    unsigned int ts;
} sample_t;

// Formatted string
| Špecifikátor | Význam                                          | Príklad výstupu    |
| ------------ | ----------------------------------------------- | ------------------ |
| `%d` / `%i`  | celé číslo (int, signed)                        | `42`               |
| `%u`         | celé číslo (unsigned)                           | `42`               |
| `%f`         | desatinné číslo (float/double)                  | `3.141593`         |
| `%.2f`       | desatinné číslo s 2 des. miestami               | `3.14`             |
| `%c`         | znak (char)                                     | `A`                |
| `%s`         | reťazec (char\*)                                | `Hello`            |
| `%p`         | adresa (pointer)                                | `0x7ffee7deadbeef` |
| `%x`         | celé číslo v hexadecimálnej sústave (lowercase) | `2a`               |
| `%X`         | celé číslo v hexadecimálnej sústave (uppercase) | `2A`               |
| `%o`         | celé číslo v osmičkovej sústave                 | `52`               |

// Pointers

int a = 5;
int *ptr = &a;   // ptr = adresa premennej a

// &a  -> adresa a
// *ptr -> hodnota na adrese (5)

// Výpis
printf("%p\n", (void*)&a);  // adresa a
printf("%d\n", a);          // hodnota 5
printf("%p\n", (void*)ptr); // obsah pointera = adresa a
printf("%d\n", *ptr);       // dereferencovanie = 5

// Zmena hodnoty cez pointer
*ptr = 42;   // prepíše hodnotu na adrese
printf("%d\n", a); // a = 42

// Pole a pointer aritmetika
int arr[3] = {10,20,30};
int *p = arr;
printf("%d\n", *p);   // 10, ukazujes na adresu pola, ta sa zacina 0ltym elementom
printf("%d\n", *(p+1)); // 20 posun o 1 element

// napr
int a = 10; 	  // teoreticka adresa 0xdeadbeef
int ptr = a;      // ptr = 10
X-> int *ptr = a; // snazime sa ukazat na hodnotu 10 ale dostaneme segfault,
		  // pretoze 10 nie je validna adresa <= seg fault
X-> int ptr = &a; // ptr = integer(0xdeadbeef) <- na 64bit systemoch sa nemusi zmestit <= seg fault,
		  // ak sa zmesti bude tam int value adresy
		  // (proste len vezmes 0xdeadbeef a prepocitas to do integer)
int *ptr = &a     // ptr = 0xDEADBEEF (adresa 'a')
		  // *ptr = 10 (hodnota na tej adrese)


/*
Prehľad:
&  -> adresa
*  -> dereferencovanie
int *p; -> ukazovateľ na int
*/

// Priklad
#include <stdio.h>

int main(void) {
    int a = 5;
    int *ptr = &a;

    printf("Adresa a:   %p\n", (void*)&a);
    printf("Hodnota a:  %d\n", a);

    printf("ptr (adresa):  %p\n", (void*)ptr);
    printf("*ptr (hodnota): %d\n", *ptr);

    *ptr = 123;  // zmena hodnoty cez pointer
    printf("Po zmene cez *ptr, a = %d\n", a);

    return 0;
}

