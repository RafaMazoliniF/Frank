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
#include <commdlg.h> // Necessário para Open/Save Dialogs
#include <stdio.h>

#define IDC_MAIN_EDIT 101
#define IDC_BTN_COMPILE 102
#define IDC_OUT_EDIT 103
#define IDC_BTN_OPEN 104
#define IDC_BTN_SAVE 105

HWND hEditIn, hEditOut;

// Função auxiliar para abrir arquivo no Windows
void DoOpenFile(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Arquivos de Texto\0*.txt\0Todos os Arquivos\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        FILE *f = fopen(ofn.lpstrFile, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            
            char *content = (char*)malloc(fsize + 1);
            if (content) {
                fread(content, 1, fsize, f);
                content[fsize] = 0;
                SetWindowText(hEditIn, content);
                free(content);
            }
            fclose(f);
        } else {
            MessageBox(hwnd, "Não foi possível abrir o arquivo.", "Erro", MB_ICONERROR);
        }
    }
}

// Função auxiliar para salvar arquivo no Windows
void DoSaveFile(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = "meu_programa.obj";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Arquivos Objeto\0*.obj\0Todos os Arquivos\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE) {
        int len = GetWindowTextLength(hEditOut);
        if (len > 0) {
            char *buf = (char*)GlobalAlloc(GPTR, len + 1);
            GetWindowText(hEditOut, buf, len + 1);
            
            FILE *f = fopen(ofn.lpstrFile, "w");
            if (f) {
                fprintf(f, "%s", buf);
                fclose(f);
                MessageBox(hwnd, "Arquivo salvo com sucesso!", "Sucesso", MB_OK);
            } else {
                MessageBox(hwnd, "Erro ao salvar o arquivo.", "Erro", MB_ICONERROR);
            }
            GlobalFree(buf);
        } else {
            MessageBox(hwnd, "Nada para salvar.", "Aviso", MB_OK);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // Área de Edição (Entrada)
            hEditIn = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, 
                10, 10, 460, 400, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEditIn, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

            // Botão ABRIR (Posicionado acima do Compilar)
            CreateWindow("BUTTON", "ABRIR", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                480, 120, 80, 40, hwnd, (HMENU)IDC_BTN_OPEN, GetModuleHandle(NULL), NULL);

            // Botão COMPILAR (Centro)
            CreateWindow("BUTTON", "COMPILAR", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                480, 180, 80, 50, hwnd, (HMENU)IDC_BTN_COMPILE, GetModuleHandle(NULL), NULL);

            // Botão SALVAR (Posicionado abaixo do Compilar)
            CreateWindow("BUTTON", "SALVAR", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                480, 250, 80, 40, hwnd, (HMENU)IDC_BTN_SAVE, GetModuleHandle(NULL), NULL);

            // Área de Saída
            hEditOut = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Aguardando...", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 
                570, 10, 300, 400, hwnd, (HMENU)IDC_OUT_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEditOut, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_BTN_OPEN:
                    DoOpenFile(hwnd);
                    break;

                case IDC_BTN_SAVE:
                    DoSaveFile(hwnd);
                    break;

                case IDC_BTN_COMPILE:
                {
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
                            MessageBox(hwnd, "Compilação concluída!", "Sucesso", MB_OK);
                        }
                    } else {
                        if(file) fclose(file);
                        SetWindowText(hEditOut, gui_error_msg);
                        MessageBox(hwnd, "Erro na compilação!", "Erro", MB_ICONERROR);
                    }
                }
                break;
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
    
    // Janela um pouco maior para acomodar o layout
    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Frank IDE (Windows)", WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 480, NULL, NULL, hInstance, NULL);
        
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
GtkWidget *window; // Global para referência nos diálogos

// --- Função para ABRIR arquivo (Upload) ---
void on_open_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Abrir Código Fonte",
                                         GTK_WINDOW(window),
                                         action,
                                         "_Cancelar",
                                         GTK_RESPONSE_CANCEL,
                                         "_Abrir",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        
        char *content = NULL;
        gsize length;
        GError *error = NULL;

        if (g_file_get_contents(filename, &content, &length, &error)) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_in));
            gtk_text_buffer_set_text(buffer, content, length);
            g_free(content);
        } else {
            fprintf(stderr, "Erro ao abrir arquivo: %s\n", error->message);
            g_error_free(error);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// --- Função para SALVAR arquivo (Gerar Saída) ---
void on_save_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Salvar Arquivo Objeto",
                                         GTK_WINDOW(window),
                                         action,
                                         "_Cancelar",
                                         GTK_RESPONSE_CANCEL,
                                         "_Salvar",
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);
    
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_current_name(chooser, "meu_programa.obj");

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_out));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        GError *error = NULL;
        if (!g_file_set_contents(filename, text, -1, &error)) {
             fprintf(stderr, "Erro ao salvar arquivo: %s\n", error->message);
             g_error_free(error);
        }
        
        g_free(text);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Função de compilar (Linux)
void on_compile_clicked(GtkWidget *widget, gpointer data) {
    GtkTextBuffer *buffer_in = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_in));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer_in, &start, &end);
    char *text = gtk_text_buffer_get_text(buffer_in, &start, &end, FALSE);
    
    FILE *tmp = fopen("temp_gui.txt", "w");
    if (tmp) { fprintf(tmp, "%s", text); fclose(tmp); }
    g_free(text);

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
        if(file) fclose(file);
        gtk_text_buffer_set_text(buffer_out, gui_error_msg, -1);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Frank IDE (Linux)");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    // Toolbar de botões
    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_START);
    gtk_box_pack_start(GTK_BOX(box), button_box, FALSE, FALSE, 5);

    GtkWidget *btn_open = gtk_button_new_with_label("Abrir Arquivo");
    g_signal_connect(btn_open, "clicked", G_CALLBACK(on_open_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(button_box), btn_open);

    GtkWidget *btn_compile = gtk_button_new_with_label("COMPILAR");
    g_signal_connect(btn_compile, "clicked", G_CALLBACK(on_compile_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(button_box), btn_compile);

    GtkWidget *btn_save = gtk_button_new_with_label("Salvar Saída");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(button_box), btn_save);

    // Entradas e Saídas
    GtkWidget *lbl_in = gtk_label_new("Código Fonte:");
    gtk_widget_set_halign(lbl_in, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), lbl_in, FALSE, FALSE, 0);

    GtkWidget *scrolled_in = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_in, -1, 250);
    text_view_in = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_in), text_view_in);
    gtk_box_pack_start(GTK_BOX(box), scrolled_in, TRUE, TRUE, 0);

    GtkWidget *lbl_out = gtk_label_new("Código Objeto (MVD):");
    gtk_widget_set_halign(lbl_out, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(box), lbl_out, FALSE, FALSE, 0);

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