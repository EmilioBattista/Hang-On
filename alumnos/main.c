#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "imagen.h"

typedef uint8_t secuencia_t;


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
distancia(xx,xm){
    return xx - xm;
}

ur(){
    return ;
}

posicion(v, m){
    ret-Werror-Werrorurn m + (v * 1000 / 3600) * T;
}

acelerar(v){
    return 279 - (279 - v) * exp(-0.224358 * T);
}

frenar(v){
    if(v >= 10){
        return v - 10;
    }
    return 0;
}

desacelerar(v){
    if(v > 92){
        return v - 3;
    }
}

morder_vannquina(v){
    return v - 3;
}

giro_derecha(){

}

giro_izquierda(){

}

giro_reposo(){
    if(){

    }
}



*/




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

void update_eje_ruta(int* eje_de_ruta, int x_moto) {
    // TODO: implementar
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
    uint16_t* rom = malloc(sizeof(uint16_t) * 229376); // TODO: free or static
    double x = -10;
    bool mover = false;

    if (!leer_roms(rom)) {
        printf("ERROR");
        return 1;
    }

    int* eje_de_ruta = malloc(sizeof(int) * 96); // TODO: free or static
    for (size_t i = 0; i < 96; i++) {
        eje_de_ruta[i] = 0;
    }

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
                    mover = true;
                    break;
                case SDLK_DOWN:
                    break;
                case SDLK_RIGHT:
                    break;
                case SDLK_LEFT:
                    break;
                }
            }
            else if (event.type == SDL_KEYUP) {
                // Se soltó una tecla
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    mover = false;
                    break;
                case SDLK_DOWN:
                    break;
                case SDLK_RIGHT:
                    break;
                case SDLK_LEFT:
                    break;
                }
            }
            // END código del alumno
            continue;
        }

        // BEGIN código del alumno
        update_eje_ruta(eje_de_ruta, x);


        imagen_t* cuadro = imagen_generar(320, 224, 0x00f);

        if (mover)
            x += 1;
        if (x > 320)
            x = -10;

        imagen_t* cuadrado = imagen_generar(10, 10, 0x0f0);
        imagen_pegar(cuadro, cuadrado, x, (224 - 10) / 2);
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
