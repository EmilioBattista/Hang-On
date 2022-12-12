#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "imagen.h"

typedef uint8_t secuencia_t;



pixel_t rom_bit_a_pixel(uint16_t lin, size_t i){
    if(((lin << i * 4) & 0xf) == 0xf){
        return 0;
    }
    return (((lin << i * 4) & 0xf) >> 12);
}

bool es_d_f(uint16_t dato){
    if((dato & 0x000f) == 0x000f){
        return true;
    }
    return false;
}

imagen_t *rom_a_figura(uint16_t rom[], size_t alto, size_t ancho, size_t inicio){
    imagen_t *im = imagen_crear(ancho, alto);
    if(im==NULL){
        return NULL;        
    }
    size_t contador_rom = inicio;
    for(size_t j = 0; j < imagen_get_alto(im); j++){
        for(size_t i = 0; i < imagen_get_ancho(im) && (j < imagen_get_alto(im)); i += 4){
            if(!(es_d_f(rom[contador_rom]))){
                for(size_t k = 0; k < 3; k++){
                    if(!imagen_set_pixel(im, i + k, j, rom_bit_a_pixel(rom[contador_rom], k))){
                        return NULL;
                    }
                }
                i = -4;
                j++;
                contador_rom++;
            }
            else{
                for(size_t k = 0; k < 4; k++){
                    if(!imagen_set_pixel(im, i + k, j, rom_bit_a_pixel(rom[contador_rom], k))){
                        return NULL;
                    } 
                }
                contador_rom++;
            }
        }
    }
    return im;
}

imagen_t *generar_mosaico(imagen_t *teselas[], const pixel_t paleta[][8], size_t filas, size_t columnas, const uint16_t mosaico_teselas[filas][columnas], const uint8_t mosaico_paletas[filas][columnas]){

    imagen_t *im = imagen_generar(columnas * 8, filas * 8, 0);
    if(im == NULL) return NULL;

    for(size_t j = 0; j < filas; j++){
        for(size_t i = 0; i < columnas; i++){
            imagen_pegar_con_paleta(im, teselas[mosaico_teselas[j][i]], i * 8, j * 8, paleta[mosaico_paletas[j][i]]);
        }
    }
    return im;
}


bool leer_teselas(imagen_t* teselas[]) {
    FILE* fR = fopen(ARCHIVO_ROM_R, "rb");
    if (fR == NULL) {
        return false;
    }
    FILE* fG = fopen(ARCHIVO_ROM_G, "rb");
    if (fG == NULL) {
        return false;
    }
    FILE* fB = fopen(ARCHIVO_ROM_B, "rb");
    if (fB == NULL) {
        return false;
    }

    secuencia_t sec_r[8];
    secuencia_t sec_g[8];
    secuencia_t sec_b[8];


    for (size_t k = 0;k < CANTIDAD_TESELAS;k++) {

        if (fread(sec_r, sizeof(secuencia_t), 8, fR) != 8) {
            return false;
        }
        if (fread(sec_g, sizeof(secuencia_t), 8, fG) != 8) {
            return false;
        }
        if (fread(sec_b, sizeof(secuencia_t), 8, fB) != 8) {
            return false;
        }

        for (size_t j = 0; j < 8; j++) {
            for (size_t i = 0; i < 8; i++) {
                if (!imagen_set_pixel(teselas[k], i, j, pixel3_crear((sec_r[j] << i) & 0x80, (sec_g[j] << i) & 0x80, (sec_b[j] << i) & 0x80))) {
                    printf("4");
                    return false;
                }
            }
        }
    }

    fclose(fR);
    fclose(fG);
    fclose(fB);
    return true;
}


// v(d)
// vertical: posición vertical sobre la ruta
// distancia: distancia hacia el objeto
// el casting a int funciona como la función piso
int vertical(int distancia) {
    return (96 - 96 * exp(-0.11 * (distancia)));
}

// d(v)
// inversa de v(d)
// vertical: posición vertical sobre la ruta
// distancia: distancia hacia el objeto
float distancia(int vertical) {
    return -(1 / 0.11) * log((96 - vertical) / 96);
}

/*
h(v)
- altura: altura del objeto sobre la ruta, tomando en cuenta la perspectiva
- vertical: posición vertical sobre la ruta
- h0: altura del objeto sin perspectiva

- Docs: Ahora bien, los objetos tienen una determinada altura que está referida a lo que ocupan cuando . Cabe preguntarse cómo se escalan cuando están a una determinada distancia [...] El ancho debe escalarse proporcionalmente al escalamiento en la altura. Si con esta cuenta el ancho diera menor a 3 pixeles se debe forzar en 3.
*/

size_t altura(int vertical, int h0) {
    return h0 * ((96 - vertical) / 96) + ((5 * vertical) / 96); // TODO: retornar 3 si el resultado es menor a 3
}


/*
ul(v)
- desplazamiento_lateral_moto (ym)
- Docs: Teniendo la moto una posición con respecto al centro de la ruta ym, el desplazamiento lateral de la ruta se puede calcular como:
*/
float desplazamiento_lateral(int vertical, int desplazamiento_lateral_moto) {
    return -desplazamiento_lateral_moto * ((96 - vertical) / 96);
}

// TODO: implementar
// esta función deberá depender de algún otro dato externo sobre la ruta, una especie de "mapa"
float radio_curvatura(int u) {
    return 0;
}
/*
uc(v)

Doc: Como la ruta tiene transiciones, el cómputo del desplazamiento de curva tiene que hacerse acumulativo con los radios de los diferentes tramos que se observan. Para cada valor de puede saberse a qué tramo representa dado que se conoce la posición xm de la moto y puede obtenerse la posición de ese tramo como xm + d(v).
*/
float desplazamiento_por_curva(int vertical) {
    if (vertical == 0) {
        return 0;
    }
    return desplazamiento_por_curva(vertical - 1) + radio_curvatura(vertical) * exp(0.105 * vertical - 8.6);
}

/*
ur(v) = ul(v) + uc(v)
Doc: El desplazamiento del centro de la ruta va a ser la suma , la idea es computar este vector de 96 posiciones una vez por cada instante dado que el mismo no sólo sirve para dibujar la ruta sino, como ahora veremos, también sirve para posicionar el resto de los objetos en la pantalla.
*/
float desplazamiento_de_ruta(int vertical, int desplazamiento_lateral_moto) {
    return desplazamiento_lateral(vertical, desplazamiento_lateral_moto) + desplazamiento_por_curva(vertical);
}


/*
Doc: Volviendo a los objetos, para posicionar una figura en la pantalla tenemos su posición en v, su altura h y nos falta conocer su posición u. Esta posición será computada como:
*/
int u(int v, int yx, int desplazamiento_lateral_moto) {
    return yx * (96 - v) / 96 + (yx * v / 5000) + desplazamiento_de_ruta(v, desplazamiento_lateral_moto);
}


/*
Las figuras se encuentran contenidas en las ROMs que van del 6819 al 6830 y luego del 6845 al 6846.

Estas ROMs representan un bloque contínuo de memoria de tipo uint16_t donde como cada ROM tiene 32KB y hay 7 pares de ROMs serán entonces 229376 valores de 16 bits.

Las ROMs están interlineadas, esto quiere decir, que el primer byte de la ROM 6819.rom se corresponde al byte bajo del primer valor, mientras que el primer byte de la ROM 6820.rom se corresponde al byte alto. La ROM 6819 aporta todos los bytes bajos de los primeros 32768 valores mientras que la ROM 6820 aporta todos los bits altos de estos mismos valores. Así para cada ROM, por ejemplo, el primer byte de la ROM 6821.rom aporta el byte bajo del valor 32769.

Estas ROMs deben ser cargadas en memoria en un único vector uint16_t rom[229376] para poder ser accedidas para extraer las respectivas figuras allí contenidas.

*/
bool leer_roms(uint16_t* rom) {
    size_t byte = 0;
    char ARCHIVO_BAJO[20];
    char ARCHIVO_ALTO[20];
    FILE* falto, * fbajo;


    for (size_t romIndex = 6819; romIndex <= 6831; romIndex = romIndex + 2) {
        if (romIndex == 6831) { // Utilizo el 6831 para manejar el caso del 6845
            romIndex = 6845;
        }

        // TODO: chequear retornos para error 
        sprintf(ARCHIVO_BAJO, "../roms/%zu.rom", romIndex);
        sprintf(ARCHIVO_ALTO, "../roms/%zu.rom", romIndex + 1);


        fbajo = fopen(ARCHIVO_BAJO, "rb");
        if (fbajo == NULL) {
            return false;
        }

        falto = fopen(ARCHIVO_ALTO, "rb");
        if (falto == NULL) {
            return false;
        }
        uint8_t talto;
        uint8_t tbajo;

        for (size_t i = 0; i < 32768; i++) {
            if (fread(&talto, sizeof(uint8_t), 1, falto) != 1) {
                return false;
            }
            if (fread(&tbajo, sizeof(uint8_t), 1, fbajo) != 1) {
                return false;
            }
            rom[byte] = (talto);
            rom[byte] = (rom[byte] < 8) || tbajo;
            byte++;
        }
    }
    return true;
}

void update_eje_ruta(int* eje_de_ruta, int desplazamiento_lateral_moto) {
    for (size_t i = 0; i < 96; i++) {
        eje_de_ruta[i] = desplazamiento_de_ruta(i, desplazamiento_lateral_moto);
    }
}

void update_estado_moto(float* posicion_x_m, float* posicion_y_m, float* velocidad_actual_km_h, int* giro_actual, bool input_aceletando, bool input_frenando, bool* input_giro_derecha, bool* input_giro_izquierda) {
    /*
    La posición se actualizará en cada instante según la velocidad y el paso temporal. Tomar en cuenta que la velocidad se mide en km/h mientras que al posición se mide en metros, por lo que hay que convertir las unidades.
    */
    *posicion_x_m = *posicion_x_m + (*velocidad_actual_km_h * 1000 / 3600) * DELTA_TIME;

    /*
    Si está presionado el acelerador o si la moto está a menos de 80 km/h entonces se acelera
    */
    if (input_aceletando || *velocidad_actual_km_h < 80) {
        *velocidad_actual_km_h = 279 - (279 - *velocidad_actual_km_h) * exp(-0.224358 * DELTA_TIME);
    }

    /*
    Por cada segundo que el freno esté presionado se le restan 300 km/h a la velocidad.

    (Podemos asumir que no se presionan acelerador y freno a la vez, en el juego original el freno le gana al acelerador.)
    */
    if (input_frenando) {
        *velocidad_actual_km_h = *velocidad_actual_km_h - 300 * DELTA_TIME;
    }

    /*
    Si no se está presionando ni el acelerador ni el freno por cada segundo transcurrido se le restan 90 km/h a la velocidad.
    */
    if (!input_aceletando && !input_frenando) {
        *velocidad_actual_km_h = *velocidad_actual_km_h - 90 * DELTA_TIME;
    }

    /*
    Morder la banquina:
    Si la posición my es superior a 215 metros (en módulo) y además la velocidad es superior a 92 km/h entonces en ese instante se le restan 3 km/h a la velocidad
    */
    if (fabs(*posicion_y_m) > 215 && *velocidad_actual_km_h > 92) {
        *velocidad_actual_km_h = *velocidad_actual_km_h - 3;
    }

    if (*velocidad_actual_km_h < 0) {
        *velocidad_actual_km_h = 0;
    }

    /*
    Giro:

        Hay tres intensidades de giro para cada lado y una posición neutral.

    Giro a la derecha:

        Si está presionado el giro a la derecha y todavía no se alcanzó la tercera intensidad de giro a la derecha, se incrementa el giro en esa dirección.

    Giro a la izquierda:

        Análogo al giro a la derecha
    */
    if (*input_giro_derecha && *giro_actual < 3) {
        *giro_actual = *giro_actual + 1;
    }

    if (*input_giro_izquierda && *giro_actual > -3) {
        *giro_actual = *giro_actual - 1;
    }

    /*
    Reposo:

    Si no está presionado ningún giro pero todavía hay intensidad de giro en alguna dirección entonces se bajará la intensidad una unidad.
    */

    if (!*input_giro_derecha && !*input_giro_izquierda && *giro_actual != 0) {
        if (*giro_actual > 0) {
            *giro_actual = *giro_actual - 1;
        }
        else {
            *giro_actual = *giro_actual + 1;
        }
    }

    /*
    Posición my:

    La posición se incrementa en 3, 9 o 15 metros según la intensidad del giro, en la dirección que corresponda.
    */
    if (*giro_actual == 1) {
        *posicion_y_m = *posicion_y_m + 3;
    }
    else if (*giro_actual == 2) {
        *posicion_y_m = *posicion_y_m + 9;
    }
    else if (*giro_actual == 3) {
        *posicion_y_m = *posicion_y_m + 15;
    }
    else if (*giro_actual == -1) {
        *posicion_y_m = *posicion_y_m - 3;
    }
    else if (*giro_actual == -2) {
        *posicion_y_m = *posicion_y_m - 9;
    }
    else if (*giro_actual == -3) {
        *posicion_y_m = *posicion_y_m - 15;
    }

    /*
    Irse al pasto:

    Si my es mayor a 435 en módulo, entonces se fuerza en 435 con el signo que corresponda.
    */
    if (*posicion_y_m > 435) {
        *posicion_y_m = 435;
    }
    else if (*posicion_y_m < -435) {
        *posicion_y_m = -435;
    }

    /*
    TODO:
    Giro de la ruta:

    Siendo

la distancia avanzada en el paso de tiempo actual, la posición sobre la ruta se actualizará según la cantidad:

Siendo

    el radio de la curva en esa posición.

Puntaje:

    El puntaje se computa como

si la moto circula a menos de 117 km/h y como si la velocidad

    es superior.

    En caso de estar mordiendo la banquina no se suma ningún punto.

Ganar:

    Se gana si se llega al final de la ruta en 4200 metros.

Perder:

    Se pierde si se acaban los 75 segundos y no se llegó a la llegada.

Choques:

    Si la posición de la moto está entre el lado izquierdo y el derecho de una figura (no nos olvidemos de que la y de las figuras está en el centro) y la figura está a menos de

metros de distancia de la moto se produce un choque. Cuando se choca la moto vuelve a

con velocidad 0 y el juego se queda detenido durante 5 segundos.

(Esta distancia en metros se toma siendo la velocidad máxima de 279 km/h, por ejemplo a una tasa de 30 cuadros por segundo esta cuenta da 3 metros siendo que la moto recorre como máximo 2,58 metros por cuadro.)
    */


    * input_giro_derecha = false;
    *input_giro_izquierda = false;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;

    SDL_CreateWindowAndRenderer(VENTANA_ANCHO, VENTANA_ALTO, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Hang-On");

    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB444, SDL_TEXTUREACCESS_STATIC, VENTANA_ANCHO, VENTANA_ALTO);
    uint16_t canvas[VENTANA_ALTO * VENTANA_ANCHO];

    int dormir = 0;

    // BEGIN código del alumno

    imagen_t *teselas[CANTIDAD_TESELAS];

    for(size_t i = 0; i < CANTIDAD_TESELAS; i++)
        teselas[i] = imagen_generar(ANCHO_TESELA, ALTO_TESELA, 0);

    if(! leer_teselas(teselas)) {
        fprintf(stderr, "No se pudieron leer las teselas\n");

        for(size_t i = 0; i < CANTIDAD_TESELAS; i++)
            imagen_destruir(teselas[i]);

        return 1;
    }

    imagen_t *fondo1 = generar_mosaico(teselas, paleta_3, FONDO1_FILAS, FONDO1_COLUMNAS, fondo1_mosaico, fondo1_paleta);
    imagen_t *fondo2 = generar_mosaico(teselas, paleta_3, FONDO2_FILAS, FONDO2_COLUMNAS, fondo2_mosaico, fondo2_paleta);

    for(size_t i = 0; i < CANTIDAD_TESELAS; i++)
        imagen_destruir(teselas[i]);

    imagen_t *pasto = imagen_generar(1, 96, pixel12_crear(0, 13, 9));
    pixel_t colores_pasto[10] = {0x089, 0x099, 0x099, 0x0a9, 0x0a9, 0x0a9, 0x0b9, 0x0b9, 0x0c9, 0x0c9};
    for(size_t i = 0; i < 10; i++)
        imagen_set_pixel(pasto, 0, i, colores_pasto[i]);

    imagen_t *pasto_estirado = imagen_escalar(pasto, 320, 96);
    imagen_destruir(pasto);





    uint16_t* rom = malloc(sizeof(uint16_t) * 229376); // TODO: free or static

    if (!leer_roms(rom)) {
        printf("ERROR");
        return 1;
    }

    imagen_t *moto1 = rom_a_figura(rom, ALTO_MOTO_1, ANCHO_MOTO_1, INICIO_MOTO_1);

    int* eje_de_ruta = malloc(sizeof(int) * 96); // TODO: free or static
    for (size_t i = 0; i < 96; i++) {
        eje_de_ruta[i] = 0;
    }

    float velocidad_actual_km_h = 0;
    int giro_actual = 0; // 0 = reposo, derecha positivo, izquierda negativo. Va desde -3 a 3
    float posicion_x_m = 0;
    float posicion_y_m = 0;

    bool input_aceletando = false;
    bool input_frenando = false;
    bool input_giro_derecha = false;
    bool input_giro_izquierda = false;

    // END código del alumno

    unsigned int ticks = SDL_GetTicks();
    while (1) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                break;
            // BEGIN código del alumno
            if (event.type == SDL_KEYDOWN) {
                // Se apretó una tecla
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    input_aceletando = true;
                    break;
                case SDLK_DOWN:
                    input_frenando = true;
                    break;
                case SDLK_RIGHT:
                    input_giro_derecha = true;
                    break;
                case SDLK_LEFT:
                    input_giro_izquierda = true;
                    break;
                }
            }
            else if (event.type == SDL_KEYUP) {
                // Se soltó una tecla
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    input_aceletando = false;
                    break;
                case SDLK_DOWN:
                    input_frenando = false;
                    break;
                case SDLK_RIGHT:
                    input_giro_derecha = false;
                    break;
                case SDLK_LEFT:
                    input_giro_izquierda = false;
                    break;
                }
            }
            // END código del alumno
            continue;
        }

        // BEGIN código del alumno
        update_eje_ruta(eje_de_ruta, posicion_x_m);
        update_estado_moto(&posicion_x_m, &posicion_y_m, &velocidad_actual_km_h, &giro_actual, input_aceletando, input_frenando, &input_giro_derecha, &input_giro_izquierda);

        // imprimimos el estado
        printf("posicion_x_m: %f, posicion_y_m: %f, velocidad_actual_km_h: %f, giro_actual: %d\n", posicion_x_m, posicion_y_m, velocidad_actual_km_h, giro_actual);

        imagen_t* cuadro = imagen_generar(320, 224, 0x00f);

        imagen_t* cuadrado = imagen_generar(10, 10, 0x0f0);
        imagen_pegar(cuadro, cuadrado, posicion_y_m, (224 - 10) / 2);
        imagen_pegar(cuadro, fondo2, -posicion_y_m * 3/4, 64);
        imagen_pegar(cuadro, fondo1, -posicion_y_m, 112);
        imagen_pegar(cuadro, pasto_estirado, 0, 128);
        imagen_pegar_con_paleta(cuadro, moto1, posicion_y_m, (224 - 10) / 2, paleta_4[2]);
        
        // TODO: @EmilioBattista agregar la imagen de la moto
        //imagen_t *fondo1 = generar_mosaico(rom, paleta_3, FONDO1_FILAS, FONDO1_COLUMNAS, fondo1_mosaico, fondo1_paleta);
        //imagen_pegar(cuadro, fondo1, 0, 0);
        //32768
        imagen_destruir(cuadrado);

        // Procedemos a dibujar a pantalla completa:
        imagen_t* cuadro_escalado = imagen_escalar(cuadro, VENTANA_ANCHO, VENTANA_ALTO);
        // Hay que implementar esta función que dibuja de forma eficiente:
        //imagen_a_textura(cuadro_escalado, canvas);
        // Como todavía no la tenemos lo hacemos de forma ineficiente con primitivas:
        for (size_t f = 0; f < imagen_get_alto(cuadro_escalado); f++)
            for (size_t c = 0; c < imagen_get_ancho(cuadro_escalado); c++)
                canvas[f * imagen_get_ancho(cuadro_escalado) + c] = imagen_get_pixel(cuadro_escalado, c, f);
        // Implementar imagen_a_textura() cuanto antes!

        imagen_destruir(cuadro_escalado);
        imagen_destruir(cuadro);

        // END código del alumno

        SDL_UpdateTexture(texture, NULL, canvas, VENTANA_ANCHO * sizeof(uint16_t));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        ticks = SDL_GetTicks() - ticks;
        if (dormir) {
            SDL_Delay(dormir);
            dormir = 0;
        }
        else if (ticks < 1000 / JUEGO_FPS)
            SDL_Delay(1000 / JUEGO_FPS - ticks);
        else
            printf("Perdiendo cuadros\n");
        ticks = SDL_GetTicks();
    }

    // BEGIN código del alumno
    free(rom);
    // No tengo nada que destruir.
    // END código del alumno

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return 0;
}
