#include "imagen.h"


struct imagen {
    pixel_t **pixel;
    size_t ancho, alto;
};

size_t imagen_get_ancho(const imagen_t *im){
    return im->ancho;
}

size_t imagen_get_alto(const imagen_t *im){

    return im->alto;
}

pixel_t imagen_get_pixel(const imagen_t *im, size_t x, size_t y){

    return im->pixel[y][x];
}

bool imagen_set_pixel(const imagen_t *im, size_t x, size_t y, pixel_t p){
    if( x >= im->ancho || x < 0 || y >= im->alto || y < 0){
        return false;
    }
    im->pixel[y][x] = p;
    return true;
}

void imagen_set_alto_ancho(imagen_t *im, size_t alto, size_t ancho){
    im->alto = alto;
    im->ancho = ancho;
}

void imagen_escribir_ppm(const imagen_t *im, FILE *fo, void (*pixel_a_rgb)(pixel_t, uint8_t *, uint8_t *, uint8_t *)){
    fprintf(fo,"%s \n%zu %zu\n%d\n","P3",im->ancho,im->alto,255);
    uint8_t r;
    uint8_t g;
    uint8_t b;
    for (size_t i = 0; i < im->alto; i++){
        for (size_t j = 0; j < im->ancho; j++){
            pixel_a_rgb(im->pixel[i][j], &r, &g, &b);
            fprintf(fo,"%d %d %d\n",r, g, b);
        }
    }
}

bool imagen_guardar_ppm(const imagen_t *im, const char *fn, void (*pixel_a_rgb)(pixel_t, uint8_t *, uint8_t *, uint8_t *)){
    FILE *f = fopen(fn, "wb");
    if(f==NULL){
        return false;
    }
    imagen_escribir_ppm(im, f, pixel_a_rgb);
    fclose(f);
    return true;
}

void imagen_destruir(imagen_t *imagen){
    for(size_t i=0; i < imagen->alto; i++){
        free(imagen->pixel[i]);
    }
    free(imagen->pixel);
    free(imagen);
}


imagen_t *imagen_crear(size_t ancho, size_t alto){

    imagen_t *p= malloc(sizeof(imagen_t));
    if(p==NULL){
        return NULL;        
    }

    p->pixel=malloc(alto * sizeof(pixel_t*));

    if(p->pixel==NULL){
        free(p);
        return NULL;
    }


    for(size_t i=0;i < alto;i++){
        p->pixel[i]= malloc(ancho * sizeof(pixel_t));
        if(p->pixel[i]== NULL){
            p->alto=i;
            imagen_destruir(p);
            return NULL;
        }
    }
    
    p->alto = alto;
    p->ancho = ancho;

    return p;
}

imagen_t *imagen_escalar(const imagen_t *origen, size_t ancho_destino, size_t alto_destino){
    imagen_t *destino = imagen_crear(ancho_destino, alto_destino);
    if(destino==NULL){
        return NULL;
    }
    float ey=(origen->alto/ (float)alto_destino);
    float ex=(origen->ancho/ (float)ancho_destino);
    for (size_t i = 0; i < alto_destino; i++){
        for (size_t j = 0; j < ancho_destino; j++){
            destino->pixel[i][j] = origen->pixel[(int)(i*ey)][(int)(j*(ex))];
        }
    }
    return destino;
}


void imagen_pegar(imagen_t *destino, const imagen_t *origen, int x, int y){
    for(int f = y >= 0 ? 0 : -y; f < origen->alto && f + y < destino->alto; f++){
    for(int c = x >= 0 ? 0 : -x; c < origen->ancho && c + x < destino->ancho; c++){
            if(origen->pixel[f][c] && f+y >= 0 && c+x >= 0  && f+y < destino->alto && c+x < destino->ancho){
                destino->pixel[f+y][c+x]=origen->pixel[f][c];
            }
        }
    }

}


void imagen_pegar_con_paleta(imagen_t *destino, const imagen_t *origen, int x, int y, const pixel_t paleta[]){
    for(int f = y >= 0 ? 0 : -y; f < origen->alto && f + y < destino->alto; f++){
    for(int c = x >= 0 ? 0 : -x; c < origen->ancho && c + x < destino->ancho; c++){
            if(origen->pixel[f][c] && f+y >= 0 && c+x >= 0  && f+y < destino->alto && c+x < destino->ancho){
                destino->pixel[f+y][c+x]=paleta[origen->pixel[f][c]];
            } 
        }
    }
}

imagen_t *imagen_generar(size_t ancho, size_t alto, pixel_t p){
    imagen_t *im = imagen_crear(ancho, alto);
    if(im==NULL){
        return NULL;        
    }
    for(size_t j = 0; j < im->alto; j++){
        for(size_t i = 0; i < im->ancho; i++){
        im->pixel[j][i] = p;
        }
    }
    return im;
}
