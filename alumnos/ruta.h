#ifndef RUTA_H
#define RUTA_H

#include <stdbool.h>
#include <stddef.h>

struct figura_en_ruta {
    int figura;     // Se deja libertad para definir esto como un enum donde corresponda
    size_t paleta;
    int y;
    bool reflejar;
};

struct ruta {
    float radio_curva;
    size_t indice_figura;
};

extern const struct figura_en_ruta figuras_en_ruta[];
extern const struct ruta ruta[4200 + 70];

#endif
