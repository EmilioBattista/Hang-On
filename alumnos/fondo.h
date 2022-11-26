#ifndef FONDO_H
#define FONDO_H

#include <stdint.h>

#define FONDO1_FILAS 2
#define FONDO1_COLUMNAS 216
#define FONDO2_FILAS 8
#define FONDO2_COLUMNAS 152

extern const uint8_t fondo1_paleta[FONDO1_FILAS][FONDO1_COLUMNAS];
extern const uint16_t fondo1_mosaico[FONDO1_FILAS][FONDO1_COLUMNAS];
extern const uint8_t fondo2_paleta[FONDO2_FILAS][FONDO2_COLUMNAS];
extern const uint16_t fondo2_mosaico[FONDO2_FILAS][FONDO2_COLUMNAS];

#endif
