#ifndef IMAGEN_H
#define IMAGEN_H

#include "pixel.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct imagen imagen_t;

void imagen_destruir(imagen_t *imagen);

imagen_t *imagen_crear(size_t ancho, size_t alto);

void imagen_get_dimensiones(const imagen_t *im, size_t *ancho, size_t *alto);

bool imagen_set_pixel(const imagen_t *im, size_t x, size_t y, pixel_t p);

void imagen_set_alto_ancho(imagen_t *im, size_t alto, size_t ancho);

pixel_t imagen_get_pixel(const imagen_t *im, size_t x, size_t y);

void imagen_pintar(imagen_t *im, pixel_t color);

imagen_t *imagen_escalar(const imagen_t *origen, size_t ancho_destino, size_t alto_destino);

void imagen_pegar(imagen_t *destino, const imagen_t *origen, int x, int y);

void imagen_escribir_ppm(const imagen_t *im, FILE *fo, void (*pixel_a_rgb)(pixel_t, uint8_t *, uint8_t *, uint8_t *));

bool imagen_guardar_ppm(const imagen_t *im, const char *fn, void (*pixel_a_rgb)(pixel_t, uint8_t *, uint8_t *, uint8_t *));

void imagen_pegar_con_paleta(imagen_t *destino, const imagen_t *origen, int x, int y, const pixel_t paleta[]);

imagen_t *imagen_generar(size_t ancho, size_t alto, pixel_t p);

#endif
