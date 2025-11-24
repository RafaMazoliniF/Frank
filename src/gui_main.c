// src/gui_main.c
#include "sintatic.h"
#include "includes.h"
#include "codegen.h"

// Protótipos para resetar o compilador
extern int current_line;
extern int label;
extern int addr;
extern void init_table();

// Função auxiliar para ler o arquivo objeto gerado
char* read_result_file() {
    FILE *f = fopen("result.obj", "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *content = malloc(fsize + 1);
    fread(content, fsize, 1, f);
    content[fsize] = 0;
    fclose(f);
    return content;
}

// ============================================================================
//                              IMPLEMENTAÇÃO WINDOWS
// ============================================================================
#ifdef _WIN32
#include <windows.h>

#define IDC_MAIN_EDIT 101
#define IDC_BTN_COMPILE 102
#define IDC_OUT_EDIT 103

HWND hEditIn, hEditOut;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hEditIn = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 
                10, 10, 460, 400, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEditIn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

            CreateWindow("BUTTON", "COMPILAR", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                480, 180, 80, 50, hwnd, (HMENU)IDC_BTN_COMPILE, GetModuleHandle(NULL), NULL);

            hEditOut = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Aguardando...", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 
                570, 10, 300, 400, hwnd, (HMENU)IDC_OUT_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEditOut, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_BTN_COMPILE) {
                int len = GetWindowTextLength(hEditIn);
                char *buf = (char*)GlobalAlloc(GPTR, len + 1);
                GetWindowText(hEditIn, buf, len + 1);
                FILE *tmp = fopen("temp_gui.txt", "w");
                if (tmp) { fprintf(tmp, "%s", buf); fclose(tmp); }
                GlobalFree(buf);

                gui_mode = 1;
                gui_error_msg[0] = '\0';
                current_line = 1; label = 0; addr = 1; init_table();
                remove("result.obj");

                if (setjmp(env_buffer) == 0) {
                    getFile("temp_gui.txt");
                    handle_program();
                    if(file) fclose(file);
                    
                    char *res = read_result_file();
                    if(res) {
                        SetWindowText(hEditOut, res);
                        free(res);
                        MessageBox(hwnd, "Sucesso!", "Frank", MB_OK);
                    }
                } else {
                    if(file) fclose(file);
                    SetWindowText(hEditOut, gui_error_msg);
                    MessageBox(hwnd, "Erro!", "Frank", MB_ICONERROR);
                }
            }
            break;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "FrankClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Frank IDE (Windows)", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 900, 460, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    return 0;
}

// ============================================================================
//                              IMPLEMENTAÇÃO LINUX (GTK)
// ============================================================================
#else
#include <gtk/gtk.h>

GtkWidget *text_view_in;
GtkWidget *text_view_out;

void on_compile_clicked(GtkWidget *widget, gpointer data) {
    GtkTextBuffer *buffer_in = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_in));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer_in, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer_in, &start, &end, FALSE);
    
    FILE *tmp = fopen("temp_gui.txt", "w");
    if (tmp) { fprintf(tmp, "%s", text); fclose(tmp); }
    g_free(text);

    // Reseta ambiente
    gui_mode = 1;
    gui_error_msg[0] = '\0';
    current_line = 1; label = 0; addr = 1; init_table();
    remove("result.obj");

    GtkTextBuffer *buffer_out = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_out));

    if (setjmp(env_buffer) == 0) {
        getFile("temp_gui.txt");
        handle_program();
        if(file) fclose(file);
        
        char *res = read_result_file();
        if(res) {
            gtk_text_buffer_set_text(buffer_out, res, -1);
            free(res);
        } else {
             gtk_text_buffer_set_text(buffer_out, "Erro: Arquivo objeto nao gerado.", -1);
        }
    } else {
        // Falha
        if(file) fclose(file);
        gtk_text_buffer_set_text(buffer_out, gui_error_msg, -1);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Frank IDE (Linux)");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Entrada
    GtkWidget *scrolled_in = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_in, -1, 300);
    text_view_in = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_in), text_view_in);
    gtk_box_pack_start(GTK_BOX(box), scrolled_in, TRUE, TRUE, 0);

    // Botão
    GtkWidget *btn = gtk_button_new_with_label("COMPILAR");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_compile_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 5);

    // Saída
    GtkWidget *scrolled_out = gtk_scrolled_window_new(NULL, NULL);
    text_view_out = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_out), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_out), text_view_out);
    gtk_box_pack_start(GTK_BOX(box), scrolled_out, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
#endif