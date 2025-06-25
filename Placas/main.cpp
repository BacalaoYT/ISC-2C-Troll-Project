#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
using namespace std;

// Letras validas para las placas, solo consonantes para evitar palabras raras
const vector<char> letrasValidas = {
    'B','C','D','F','G','H','J','K','L','M','N',
    'P','Q','R','S','T','V','W','X','Y','Z'
};

// Esta funcion avanza las letras de la placa, por ejemplo si tienes BZZ y sumas uno, te da CBB
// Busca la letra actual, si puede avanzar la cambia, si no, la reinicia y avanza la anterior
void avanzarLetras(string& letras, int pos = 2) {
    if (pos < 0) return;
    for (size_t i = 0; i < letrasValidas.size(); ++i) {
        if (letras[pos] == letrasValidas[i]) {
            if (i + 1 < letrasValidas.size()) {
                letras[pos] = letrasValidas[i + 1];
            } else {
                letras[pos] = letrasValidas[0];
                avanzarLetras(letras, pos - 1);
            }
            break;
        }
    }
}

// Esta funcion recibe una placa como "0001 BCD" y regresa la siguiente
// Si el numero llega a 9999, se reinicia a 0000 y las letras avanzan usando la funcion de arriba
string siguientePlaca(string numeros, string letras) {
    int num = stoi(numeros);
    if (num < 9999) {
        num++;
    } else {
        num = 0;
        avanzarLetras(letras);
    }
    numeros = to_string(num);
    while (numeros.length() < 4) numeros = "0" + numeros;
    return numeros + " " + letras;
}

// Esta funcion dibuja un boton con sombra, fondo rojo y texto centrado en negro
// El boton se usa en toda la interfaz para mantener el mismo estilo visual
void drawButton(int x, int y, int w, int h, const char* text, ALLEGRO_FONT* fontSmall) {
    // Dibuja la sombra del boton
    al_draw_filled_rounded_rectangle(x + 6, y + 6, x + w + 6, y + h + 6, 20, 20, al_map_rgba(80, 80, 80, 80));
    // Dibuja el fondo rojo carmesi claro
    al_draw_filled_rounded_rectangle(x, y, x + w, y + h, 20, 20, al_map_rgb(220, 80, 80));
    // Agrega un brillo en la parte superior del boton
    al_draw_filled_rounded_rectangle(x, y, x + w, y + h / 2, 20, 20, al_map_rgba(255, 255, 255, 30));
    // Dibuja el borde del boton
    al_draw_rounded_rectangle(x, y, x + w, y + h, 20, 20, al_map_rgb(140, 30, 30), 4);
    // Calcula el ancho y alto del texto para centrarlo
    int textW = al_get_text_width(fontSmall, text);
    int textH = al_get_font_line_height(fontSmall);
    // Dibuja el texto centrado
    al_draw_text(fontSmall, al_map_rgb(0, 0, 0), x + (w - textW) / 2, y + (h - textH) / 2, 0, text);
}

int main() {
    // Inicializa Allegro y los complementos necesarios
    al_init();
    al_install_keyboard();
    al_install_mouse();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();

    // Obtiene el tamaño del monitor y configura la pantalla en modo fullscreen
    ALLEGRO_MONITOR_INFO monitorInfo;
    al_get_monitor_info(0, &monitorInfo);
    int screenW = monitorInfo.x2 - monitorInfo.x1;
    int screenH = monitorInfo.y2 - monitorInfo.y1;
    al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    ALLEGRO_DISPLAY* display = al_create_display(screenW, screenH);

    // Crea los recursos principales
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_FONT* font = al_load_ttf_font("arial.ttf", 48, 0);
    ALLEGRO_FONT* fontSmall = al_load_ttf_font("arial.ttf", 28, 0);
    if (!font || !fontSmall) {
        cerr << "No se pudo cargar la fuente.\n";
        return 1;
    }

    // Registra los eventos de ventana, teclado, mouse y timer
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    // Define los estados principales de la aplicacion: menu, manual y resultados
    enum Estado { MENU, MANUAL, RESULTADOS };
    Estado estado = MENU;

    // Variables principales para manejar el codigo
    vector<string> placasEntrada, placasSalida;
    string input = "", resultado = "";
    bool running = true, redraw = true;
    int cursorTimer = 0;
    float fade = 0;
    size_t index = 0;

    // Crea las nubes animadas, cada nube tiene su posicion, tamaño y velocidad
    const int numNubes = 7;
    float nubeX[numNubes], nubeY[numNubes], nubeW[numNubes], nubeH[numNubes], nubeVel[numNubes];
    for (int i = 0; i < numNubes; ++i) {
        nubeX[i] = rand() % screenW;
        nubeY[i] = 50 + rand() % (screenH / 3);
        nubeW[i] = 180 + rand() % 120;
        nubeH[i] = 60 + rand() % 30;
        nubeVel[i] = 0.3f + 0.5f * (rand() % 100) / 100.0f;
    }

    al_start_timer(timer);

    // Define el tamaño y posicion de los cuadros y botones principales
    int boxW = 900, boxH = 420;
    int boxX = (screenW - boxW) / 2;
    int boxY = (screenH - boxH) / 2;

    int btnW = 350, btnH = 90;
    int btn1X = boxX + (boxW - btnW) / 2, btn1Y = boxY + 60;
    int btn2X = btn1X, btn2Y = btn1Y + btnH + 40;

    int btnExitW = 200, btnExitH = 70;
    int btnExitX = boxX + boxW - btnExitW - 40, btnExitY = boxY + boxH - btnExitH - 30;
    int btnBackX = boxX + 40, btnBackY = btnExitY;

    int navBtnW = 120, navBtnH = 80;
    int btnPrevX = boxX + 60, btnPrevY = boxY + boxH + 40;
    int btnNextX = boxX + boxW - navBtnW - 60, btnNextY = btnPrevY;

    // Boton ENTER para cuando estas en modo manual
    int btnEnterW = 240, btnEnterH = 80;
    int btnEnterX = boxX + (boxW - btnEnterW) / 2;
    int btnEnterY = boxY + boxH - 180;

    // Loop principal, maneja todo el flujo del codigo
    while (running) {
        ALLEGRO_EVENT ev;
        while (al_get_next_event(queue, &ev)) {
            if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                running = false;

            // Maneja el teclado segun el estado actual
            if (ev.type == ALLEGRO_EVENT_KEY_CHAR) {
                if (estado == MENU) {
                    if (ev.keyboard.unichar == '1') {
                        estado = MANUAL;
                        input.clear();
                        resultado.clear();
                    } else if (ev.keyboard.unichar == '2') {
                        // Carga el archivo de placas y calcula las siguientes
                        ifstream fin("entrada.txt");
                        ofstream fout("salida.txt");
                        string linea;
                        placasEntrada.clear();
                        placasSalida.clear();
                        while (getline(fin, linea)) {
                            if (linea == "9999 ZZZ") break;
                            placasEntrada.push_back(linea);
                            string num = linea.substr(0, 4);
                            string let = linea.substr(5, 3);
                            string res = siguientePlaca(num, let);
                            placasSalida.push_back(res);
                            fout << res << endl;
                        }
                        fin.close();
                        fout.close();
                        estado = RESULTADOS;
                        index = 0;
                        fade = 0;
                    }
                }
                else if (estado == MANUAL) {
                    // borra el ultimo caracter
                    if (ev.keyboard.unichar == 8 && !input.empty()) {
                        input.pop_back();
                    // Si se presiona enter y hay una placa valida, calcula la siguiente
                    } else if (ev.keyboard.unichar == 13) {
                        if (input.size() >= 8) {
                            string num = input.substr(0, 4);
                            string let = input.substr(5, 3);
                            resultado = siguientePlaca(num, let);
                            ofstream fout("salida.txt");
                            fout << resultado << endl;
                            fout.close();
                        }
                    // Si se escribe una letra o numero, lo agrega al input
                    } else if ((isalnum(ev.keyboard.unichar) || ev.keyboard.unichar == ' ') && input.size() < 10) {
                        input += toupper(ev.keyboard.unichar);
                    }
                }
                redraw = true;
            }

            // Maneja el mouse segun el estado actual y la posicion del click
            if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                int mx = ev.mouse.x, my = ev.mouse.y;
                if (estado == MENU) {
                    // Boton para ir a modo manual
                    if (mx >= btn1X && mx <= btn1X + btnW && my >= btn1Y && my <= btn1Y + btnH) {
                        estado = MANUAL;
                        input.clear();
                        resultado.clear();
                        redraw = true;
                    }
                    // Boton para cargar archivo y mostrar resultados
                    if (mx >= btn2X && mx <= btn2X + btnW && my >= btn2Y && my <= btn2Y + btnH) {
                        ifstream fin("entrada.txt");
                        ofstream fout("salida.txt");
                        string linea;
                        placasEntrada.clear();
                        placasSalida.clear();
                        while (getline(fin, linea)) {
                            if (linea == "9999 ZZZ") break;
                            placasEntrada.push_back(linea);
                            string num = linea.substr(0, 4);
                            string let = linea.substr(5, 3);
                            string res = siguientePlaca(num, let);
                            placasSalida.push_back(res);
                            fout << res << endl;
                        }
                        fin.close();
                        fout.close();
                        estado = RESULTADOS;
                        index = 0;
                        fade = 0;
                        redraw = true;
                    }
                    // Boton para salir del programa
                    if (mx >= btnExitX && mx <= btnExitX + btnExitW && my >= btnExitY && my <= btnExitY + btnExitH) {
                        running = false;
                    }
                }
                if (estado == MANUAL) {
                    // Boton para volver al menu
                    if (mx >= btnBackX && mx <= btnBackX + btnExitW && my >= btnBackY && my <= btnBackY + btnExitH) {
                        estado = MENU;
                        redraw = true;
                    }
                    // Boton para salir del programa
                    if (mx >= btnExitX && mx <= btnExitX + btnExitW && my >= btnExitY && my <= btnExitY + btnExitH) {
                        running = false;
                    }
                }
                if (estado == RESULTADOS) {
                    // Boton para volver al menu
                    if (mx >= btnBackX && mx <= btnBackX + btnExitW && my >= btnBackY && my <= btnBackY + btnExitH) {
                        estado = MENU;
                        redraw = true;
                    }
                    // Boton para salir del programa
                    if (mx >= btnExitX && mx <= btnExitX + btnExitW && my >= btnExitY && my <= btnExitY + btnExitH) {
                        running = false;
                    }
                    // Boton para ir al resultado anterior
                    if (mx >= btnPrevX && mx <= btnPrevX + navBtnW && my >= btnPrevY && my <= btnPrevY + navBtnH && index > 0) {
                        index--;
                        fade = 0;
                        redraw = true;
                    }
                    // Boton para ir al siguiente resultado
                    if (mx >= btnNextX && mx <= btnNextX + navBtnW && my >= btnNextY && my <= btnNextY + navBtnH && index + 1 < placasSalida.size()) {
                        index++;
                        fade = 0;
                        redraw = true;
                    }
                }
            }

            // El timer se encarga de animar las nubes, los arboles y el cursor parpadeante
            if (ev.type == ALLEGRO_EVENT_TIMER) {
                for (int i = 0; i < numNubes; ++i) {
                    nubeX[i] += nubeVel[i];
                    if (nubeX[i] > screenW + 50) nubeX[i] = -nubeW[i];
                }
                if (fade < 255.0) fade += 3.5;
                cursorTimer++;
                redraw = true;
            }
        }

        // Dibuja todo en pantalla segun el estado actual
        if (redraw) {
            // Dibuja el fondo azul claro y las nubes animadas
            al_clear_to_color(al_map_rgb(180, 240, 255));
            for (int i = 0; i < numNubes; ++i) {
                al_draw_filled_rounded_rectangle(
                    nubeX[i], nubeY[i], nubeX[i] + nubeW[i], nubeY[i] + nubeH[i],
                    40, 40, al_map_rgb(255, 255, 255)
                );
            }

            // Dibuja una fila de arboles en la parte de abajo que se mueven
            static float arbolOffset = 0;
            arbolOffset += 0.7f;
            if (arbolOffset > 180) arbolOffset = 0;
            int baseY = screenH - 140;
            for (int i = -1; i < screenW / 90 + 2; ++i) {
                float x = i * 90 - arbolOffset;
                al_draw_filled_rectangle(x + 32, baseY + 70, x + 58, baseY + 140, al_map_rgb(110, 70, 30));
                al_draw_filled_circle(x + 45, baseY + 70, 48, al_map_rgb(60, 180, 60));
                al_draw_filled_circle(x + 25, baseY + 95, 28, al_map_rgb(60, 160, 60));
                al_draw_filled_circle(x + 65, baseY + 95, 28, al_map_rgb(60, 160, 60));
            }

            // Dibuja el cuadro principal con sombra y degradado
            al_draw_filled_rectangle(boxX + 10, boxY + 10, boxX + boxW + 10, boxY + boxH + 10, al_map_rgba(80, 80, 80, 80));
            for (int i = 0; i < boxH; ++i) {
                float t = i / (float)boxH;
                ALLEGRO_COLOR c = al_map_rgb(255 - 20 * t, 245 - 30 * t, 230 - 40 * t);
                al_draw_filled_rectangle(boxX, boxY + i, boxX + boxW, boxY + i + 1, c);
            }
            al_draw_rectangle(boxX, boxY, boxX + boxW, boxY + boxH, al_map_rgb(160, 130, 100), 6);

            // Segun el estado, dibuja los botones y textos correspondientes
            if (estado == MENU) {
                drawButton(btn1X, btn1Y, btnW, btnH, "Ingresar matricula", fontSmall);
                drawButton(btn2X, btn2Y, btnW, btnH, "Cargar archivo", fontSmall);
                drawButton(btnExitX, btnExitY, btnExitW, btnExitH, "SALIR", fontSmall);
            }
            else if (estado == MANUAL) {
                // Dibuja el input de la placa y el cursor parpadeante
                string texto = "Ingresa matricula: " + input + ((cursorTimer / 30) % 2 == 0 ? "_" : "");
                al_draw_text(font, al_map_rgb(0, 0, 0), boxX + 60, boxY + 80, 0, texto.c_str());

                drawButton(btnEnterX, btnEnterY, btnEnterW, btnEnterH, "ENTER", fontSmall);
                drawButton(btnBackX, btnBackY, btnExitW, btnExitH, "VOLVER", fontSmall);
                drawButton(btnExitX, btnExitY, btnExitW, btnExitH, "SALIR", fontSmall);

                // Si ya hay resultado, lo muestra en un recuadro abajo
                if (!resultado.empty()) {
                    int resY = boxY + boxH + 40;
                    al_draw_filled_rectangle(boxX + 10, resY + 10, boxX + boxW + 10, resY + 110, al_map_rgba(80, 80, 80, 80));
                    for (int i = 0; i < 100; ++i) {
                        float t = i / 100.0f;
                        ALLEGRO_COLOR c = al_map_rgb(255 - 20 * t, 255 - 20 * t, 220 - 40 * t);
                        al_draw_filled_rectangle(boxX, resY + i, boxX + boxW, resY + i + 1, c);
                    }
                    al_draw_rectangle(boxX, resY, boxX + boxW, resY + 100, al_map_rgb(100, 80, 60), 4);
                    al_draw_text(font, al_map_rgb(40, 40, 40), boxX + 60, resY + 30, 0, ("Siguiente: " + resultado).c_str());
                }
            }
            else if (estado == RESULTADOS) {
                // Muestra la placa de entrada y la siguiente, y los botones para navegar
                al_draw_text(font, al_map_rgb(0, 0, 0), boxX + 80, boxY + 80, 0,
                             ("Entrada: " + placasEntrada[index]).c_str());
                al_draw_text(font, al_map_rgb(0, 0, 0), boxX + 80, boxY + 180, 0,
                             ("Siguiente: " + placasSalida[index]).c_str());

                drawButton(btnPrevX, btnPrevY, navBtnW, navBtnH, "<", fontSmall);
                drawButton(btnNextX, btnNextY, navBtnW, navBtnH, ">", fontSmall);
                drawButton(btnBackX, btnBackY, btnExitW, btnExitH, "VOLVER", fontSmall);
                drawButton(btnExitX, btnExitY, btnExitW, btnExitH, "SALIR", fontSmall);

                al_draw_text(fontSmall, al_map_rgb(30, 30, 30), boxX + 80, btnPrevY + navBtnH + 40, 0,
                             "Presiona los botones para avanzar o retroceder");
            }

            al_flip_display();
            redraw = false;
        }
    }

    // Libera todos los recursos de Allegro antes de salir
    al_destroy_font(font);
    al_destroy_font(fontSmall);
    al_destroy_display(display);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    return 0;
}