# Flow projektu

## TCP štýl komunikácie
Potrebný, nakoľko potrebujeme mať istotu, že dáta prišli/odišli.  
V C-čku chceš mať istotu všade, inak hrozia rôzne **vulns/DoS hrozby**.

---

## 1. Senzory (`sensors_src/`)

- Každý súbor `sensor_*.c` simuluje jeden senzor (teplota, vlhkosť, tlak...).
- Musí byť `template_senzor`, z ktorého budú „dediť“ ďalšie senzory.  
  Obsahuje funkcie, ktoré budú **štandardne komunikovať s boardom**.
- Funkcie, ktoré generujú hodnoty, budú **ľahko zameniteľné s reálnym senzorom** (dáta z endpointu).
- Kompiluje sa do `modules/sensor_*.so` – odtiaľ si ich doska bude načítavať.

---

## 2. Board (doska)

- Načítava všetky `.so` senzory z `modules/` (dynamic loading).
- Každý senzor sa volá raz za sekundu, hodnota sa uloží do **kruhového bufferu (256 sekúnd)**.

### REST API:
- `/api/v1/sensors` – zoznam senzorov + posledná hodnota
- `/api/v1/window?name=...` – okno 256 vzoriek

---

## 3. Processor

- Volá **board API**, vyťahuje dáta.
- Spracuje (min, max, avg, last).
- Poskytuje REST API:  
  `/api/v1/summary` – odporúčam pekný JSON nech web guy nemusí plakať.

---

## 4. Web

- HTTP server v C
- Basic error handling
- Jednoduchá **auth** v C (stačia hardcoded credentials)
- Servuje **HTML stránku + JS + CSS**.
- JavaScript každé 2s fetchne dáta z **processor APIs** a vykreslí tabuľku.

---

## Rozdelenie úloh v tíme

- **Osoba 1 – senzory:** vytvoriť viacero senzorov podľa `template_sensor.c` (každý vlastný `.c` súbor).
- **Osoba 2 – board:** načítavanie `.so` senzorov (`dlopen`), kruhový buffer, REST API `/sensors` a `/window`.
- **Osoba 3 – processor:** fetch dát z boardu, výpočty (avg, min, max), REST API `/summary`.
- **Osoba 4 – web:** statické súbory (HTML, JS, CSS), jednoduchý HTTP server.

---

## Resources

- [Ako funguje git (OPTIONAL)](https://octobot.medium.com/how-git-internally-works-1f0932067bee)  
- [Learn C – basics + snippets](https://www.learn-c.org/)  
- [Dynamic loading (dlopen)](https://tldp.org/HOWTO/Program-Library-HOWTO/dl-libraries.html)  
- [Network programming](https://beej.us/guide/bgnet/)  
- [JSON špecifikácia](https://www.json.org/json-en.html)  
- [Fetch API (JS)](https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API/Using_Fetch)  

---

## Vysvetlenie prípon súborov

| Prípona | Význam                                                                 |
| ------- | ---------------------------------------------------------------------- |
| `.c`    | zdrojový kód v jazyku C                                                |
| `.h`    | hlavičkový súbor (deklarácie funkcií/štruktúr)                         |
| `.so`   | shared object (zdieľaná knižnica, plugin). Kompilácia: `-shared -fPIC` |

Board načíta každý `sensor_*.so` **bez rekompilácie**.

---

## Dátové typy

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

---

## Syntax cheatsheet

### Premenná
```
int x = 5;
double t = 21.5;
char msg[16] = "ahoj";
```

### Podmienka
```
if (x > 10) { ... } else { ... }
```

### Cykly
```
for (int i=0; i<10; i++) { ... }
while (x < 100) { ... }
```

### Funkcia
```
int add(int a, int b) {
    return a + b;
}
```

### Štruktúra
```
typedef struct {
    double value;
    unsigned int ts;
} sample_t;
```

### Formatted string
| Špecifikátor | Význam                                          | Príklad výstupu |
| ------------ | ----------------------------------------------- | --------------- |
| `%d` / `%i`  | celé číslo (int, signed)                        | `42`            |
| `%u`         | celé číslo (unsigned)                           | `42`            |
| `%f`         | desatinné číslo (float/double)                  | `3.141593`      |
| `%.2f`       | desatinné číslo s 2 des. miestami               | `3.14`          |
| `%c`         | znak (char)                                     | `A`             |
| `%s`         | reťazec (char\*)                                | `Hello`         |
| `%p`         | adresa (pointer)                                | `0x7ffee7dead`  |
| `%x`         | celé číslo v hexadecimálnej (lowercase)         | `2a`            |
| `%X`         | celé číslo v hexadecimálnej (uppercase)         | `2A`            |
| `%o`         | celé číslo v osmičkovej sústave                 | `52`            |

---

## Pointers

```
int a = 5;
int *ptr = &a;   // ptr = adresa premennej a

// &a  -> adresa a
// *ptr -> hodnota na adrese (5)
```

```
printf("%p\n", (void*)&a);  // adresa a
printf("%d\n", a);          // hodnota 5
printf("%p\n", (void*)ptr); // obsah pointera = adresa a
printf("%d\n", *ptr);       // dereferencovanie = 5
```

### Zmena hodnoty cez pointer
```
*ptr = 42;   // prepíše hodnotu na adrese
printf("%d\n", a); // a = 42
```

### Pole a pointer aritmetika
```
int arr[3] = {10,20,30};
int *p = arr;

printf("%d\n", *p);      // 10
printf("%d\n", *(p+1));  // 20
```

### Nesprávne použitie
```
int a = 10; 	
// int *ptr = a;  -> segfault, 10 nie je validná adresa
// int ptr = &a;  -> môže sa nezmestit na 64-bit systéme
```

### Správne použitie
```
int *ptr = &a;   // ptr = adresa 'a'
                 // *ptr = 10
```

---

## Kompletný príklad
```
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
```
