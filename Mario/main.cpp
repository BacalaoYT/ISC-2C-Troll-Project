// g++ main.cpp -o mario_saltarin -lallegro -lallegro_font -lallegro_ttf -lallegro_primitives -lallegro_image

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cstring>

using namespace std;

struct Resultado {
    int saltos_arriba;
    int saltos_abajo;
};

struct Nube { float x, y, vel; };

// Ahora el nombre es un arreglo de char, ya no usamos la funcion de <vector>
struct Boton { char nombre[20]; float x, y, w, h; };

enum TipoBoton { SALIR, PAUSA, AVANZAR, RETROCEDER };

int contar_muros_mayores(const int* muros, int tam, int idx, int valor) {
    if (idx >= tam) return 0;
    int cuenta = 0;
    if (muros[idx] > valor) {
        cuenta = 1;
    } else {
        cuenta = 0;
    }
    return cuenta + contar_muros_mayores(muros, tam, idx + 1, valor);
}

Resultado procesarEscenario(const int* muros, int tam) {
    Resultado res = {0, 0};
    for (int i = 1; i < tam; ++i) {
        if (muros[i] > muros[i - 1])
            res.saltos_arriba++;
        else if (muros[i] < muros[i - 1])
            res.saltos_abajo++;
    }
    return res;
}

// Dibuja un suelo lizo color naranja
void dibujar_suelo(float ancho, float y_base, float alto = 20) {
    ALLEGRO_COLOR color_suelo = al_map_rgb(255, 140, 0);
    al_draw_filled_rectangle(0, y_base - alto, ancho, y_base, color_suelo);
}

// Esta funcion dibuja un boton con un rectangulo amarillo y texto centrado
void dibujar_boton(float x, float y, float w, float h, const char* texto, ALLEGRO_FONT* font, ALLEGRO_COLOR color) {
    al_draw_filled_rectangle(x, y, x + w, y + h, al_map_rgb(255, 255, 0));
    al_draw_rectangle(x, y, x + w, y + h, al_map_rgb(100, 100, 100), 2);
    int tx = x + w / 2;
    int ty = y + (h - al_get_font_line_height(font)) / 2;
    al_draw_text(font, color, tx, ty, ALLEGRO_ALIGN_CENTER, texto);
}

bool mouse_sobre(float mx, float my, float x, float y, float w, float h) {
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}

int main() {
    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_init_image_addon();

    int ancho = 800, alto = 600;
    ALLEGRO_DISPLAY* display = al_create_display(ancho, alto);
    al_set_window_title(display, "Mario Saltarin");
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_FONT* font = al_load_ttf_font("C:\\Windows\\Fonts\\arial.ttf", 20, 0);
    if (!font) return -1;

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    ALLEGRO_COLOR color_muro = al_map_rgb(255, 0, 0);
    ALLEGRO_COLOR color_texto = al_map_rgb(0, 0, 0);

    ALLEGRO_BITMAP* img_muro = al_load_bitmap("muro.png");
    ALLEGRO_BITMAP* img_mario = al_load_bitmap("mario.png");
    if (!img_mario) return -1;

    int casos;
    FILE* archivo_entrada = fopen("entrada.txt", "r");
    if (!archivo_entrada) return -1;
    fscanf(archivo_entrada, "%d", &casos);

    int** escenarios = new int*[casos];
    int* num_muros = new int[casos];
    Resultado* resultados = new Resultado[casos];

    // Archivo de salida con nombre unico dependiendo de la fecha y hora actual
    char nombre_salida[64];
    time_t now = time(0);
    tm* ltm = localtime(&now);
    sprintf(nombre_salida, "salida_%d%02d%02d_%02d%02d%02d.txt",
        1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
        ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    FILE* archivo_salida = fopen(nombre_salida, "w");

    for (int c = 0; c < casos; ++c) {
        int n;
        fscanf(archivo_entrada, "%d", &n);
        num_muros[c] = n;
        escenarios[c] = new int[n];
        for (int i = 0; i < n; ++i) fscanf(archivo_entrada, "%d", &escenarios[c][i]);
        resultados[c] = procesarEscenario(escenarios[c], n);
        fprintf(archivo_salida, "%d %d\n", resultados[c].saltos_arriba, resultados[c].saltos_abajo);
    }
    fclose(archivo_entrada);
    fclose(archivo_salida);

    // El vector de nubes ahora es un arreglo de estructuras
    Nube nubes[4] = {
        {100, 100, 0.5f},
        {300, 150, 0.3f},
        {600, 80, 0.4f},
        {400, 200, 0.2f}
    };

    Boton botones[4] = {
        {"Salir",      10,  10, 70, 28},
        {"Pausa",      90,  10, 80, 28},
        {"Avanzar",    180, 10, 90, 28},
        {"Retroceder", 280, 10, 110, 28}
    };

    int caso_actual = 0, posicion_muro = 0;
    float mario_x = 50 + 25, mario_y = alto - escenarios[0][0] * 20 - 35;
    float destino_x = mario_x, destino_y = mario_y;
    bool en_salto = false, salir = false, mario_pausa = false;
    ALLEGRO_EVENT ev;

    al_start_timer(timer);

    while (!salir) {
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            salir = true;

        if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            float mx = ev.mouse.x, my = ev.mouse.y;
            for (int i = 0; i < 4; ++i) {
                if (mouse_sobre(mx, my, botones[i].x, botones[i].y, botones[i].w, botones[i].h)) {
                    if (i == SALIR) salir = true;
                    if (i == PAUSA) mario_pausa = !mario_pausa;
                    if (i == AVANZAR) {
                        if (posicion_muro < num_muros[caso_actual] - 1) {
                            posicion_muro++;
                            mario_x = 50 + posicion_muro * 80 + 25;
                            mario_y = alto - escenarios[caso_actual][posicion_muro] * 20 - 35;
                        }
                    }
                    if (i == RETROCEDER) {
                        if (posicion_muro > 0) {
                            posicion_muro--;
                            mario_x = 50 + posicion_muro * 80 + 25;
                            mario_y = alto - escenarios[caso_actual][posicion_muro] * 20 - 35;
                        } else if (caso_actual > 0) {
                            caso_actual--;
                            posicion_muro = num_muros[caso_actual] - 1;
                            mario_x = 50 + posicion_muro * 80 + 25;
                            mario_y = alto - escenarios[caso_actual][posicion_muro] * 20 - 35;
                        }
                    }
                }
            }
        }

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            al_clear_to_color(al_map_rgb(80, 180, 255));

            for (int i = 0; i < 4; ++i) { //Hice la correccion pedida, cambie el for de rango a un for tradicional visto en el curso
                Nube& nube = nubes[i];
                al_draw_filled_circle(nube.x, nube.y, 18, al_map_rgb(255,255,255));
                al_draw_filled_circle(nube.x+20, nube.y+5, 15, al_map_rgb(255,255,255));
                al_draw_filled_circle(nube.x-18, nube.y+8, 13, al_map_rgb(255,255,255));
                al_draw_filled_circle(nube.x+10, nube.y-10, 10, al_map_rgb(255,255,255));
                al_draw_circle(nube.x, nube.y, 18, al_map_rgb(220,220,220), 2);
                al_draw_circle(nube.x+20, nube.y+5, 15, al_map_rgb(220,220,220), 2);
                al_draw_circle(nube.x-18, nube.y+8, 13, al_map_rgb(220,220,220), 2);
                al_draw_circle(nube.x+10, nube.y-10, 10, al_map_rgb(220,220,220), 2);
                nube.x += nube.vel;
                if (nube.x > ancho+50) nube.x = -40;
            }

            dibujar_suelo(ancho, alto);

            int* muros = escenarios[caso_actual];
            int muros_tam = num_muros[caso_actual];
            for (int i = 0; i < muros_tam; ++i) {
                float muro_x = 50 + i * 80;
                float muro_y = alto - muros[i] * 20;
                if (img_muro) {
                    al_draw_scaled_bitmap(img_muro, 0, 0, al_get_bitmap_width(img_muro), al_get_bitmap_height(img_muro),
                        muro_x, muro_y, 50, muros[i] * 20, 0);
                } else {
                    al_draw_filled_rectangle(muro_x, muro_y, muro_x + 50, alto, color_muro);
                }
            }

            al_draw_scaled_bitmap(img_mario, 0, 0, al_get_bitmap_width(img_mario), al_get_bitmap_height(img_mario),
                mario_x - 15, mario_y - 5, 30, 30, 0);

            char texto[128];
            sprintf(texto, "Caso %d | Arriba: %d Abajo: %d | Muros altos: %d",
                caso_actual + 1,
                resultados[caso_actual].saltos_arriba,
                resultados[caso_actual].saltos_abajo,
                contar_muros_mayores(muros, muros_tam, 0, 3));
            al_draw_text(font, color_texto, ancho / 2, 50, ALLEGRO_ALIGN_CENTER, texto);

            for (int i = 0; i < 4; ++i) {
                dibujar_boton(botones[i].x, botones[i].y, botones[i].w, botones[i].h, botones[i].nombre, font, al_map_rgb(0,0,0));
            }

            al_flip_display();

            if (!mario_pausa && !en_salto && posicion_muro < muros_tam - 1) {
                destino_x = 50 + (posicion_muro + 1) * 80 + 25;
                destino_y = alto - muros[posicion_muro + 1] * 20 - 35;
                en_salto = true;
            }
            if (!mario_pausa && en_salto) {
                float dx = (destino_x - mario_x) * 0.1f;
                float dy = (destino_y - mario_y) * 0.1f;
                mario_x += dx;
                mario_y += dy;
                if (fabs(destino_x - mario_x) < 1 && fabs(destino_y - mario_y) < 1) {
                    mario_x = destino_x;
                    mario_y = destino_y;
                    en_salto = false;
                    posicion_muro++;
                }
            }
            if (posicion_muro >= muros_tam - 1) {
                caso_actual++;
                if (caso_actual >= casos) salir = true;
                else {
                    posicion_muro = 0;
                    mario_x = 50 + 25;
                    mario_y = alto - escenarios[caso_actual][0] * 20 - 35;
                }
            }
        }
    }

    for (int i = 0; i < casos; ++i) delete[] escenarios[i];
    delete[] escenarios;
    delete[] resultados;
    delete[] num_muros;
    al_destroy_font(font);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    if (img_muro) al_destroy_bitmap(img_muro);
    if (img_mario) al_destroy_bitmap(img_mario);

    return 0;
}

