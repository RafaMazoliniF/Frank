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
    if (content) {
        fread(content, fsize, 1, f);
        content[fsize] = 0;
    }
    fclose(f);
    return content;
}

// ============================================================================
//                              IMPLEMENTAÇÃO WINDOWS
// ============================================================================
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>

#define IDC_MAIN_EDIT 101
#define IDC_BTN_COMPILE 102
#define IDC_OUT_EDIT 103
#define IDC_BTN_OPEN 104
#define IDC_BTN_SAVE 105
#define IDC_LBL_IN 106
#define IDC_LBL_OUT 107

HWND hEditIn, hEditOut;
HFONT hFont; // Fonte moderna global

// Função para criar fonte Segoe UI (Padrão moderno)
HFONT CreateModernFont() {
    return CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                      CLEARTYPE_QUALITY, VARIABLE_PITCH, "Segoe UI");
}

void DoOpenFile(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = {0};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Arquivos de Texto (*.txt)\0*.txt\0Todos os Arquivos (*.*)\0*.*\0";
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
        }
    }
}

void DoSaveFile(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = "programa_compilado.obj";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Objeto Compilado (*.obj)\0*.obj\0Todos os Arquivos (*.*)\0*.*\0";
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
                MessageBox(hwnd, "Arquivo salvo com sucesso!", "Frank IDE", MB_OK | MB_ICONINFORMATION);
            }
            GlobalFree(buf);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            hFont = CreateModernFont();

            // Label Entrada
            CreateWindow("STATIC", "Código Fonte (Entrada):", 
                WS_VISIBLE | WS_CHILD, 
                20, 10, 400, 20, hwnd, (HMENU)IDC_LBL_IN, GetModuleHandle(NULL), NULL);
            SendMessage(GetDlgItem(hwnd, IDC_LBL_IN), WM_SETFONT, (WPARAM)hFont, TRUE);

            // Editor Entrada
            hEditIn = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, 
                20, 35, 420, 400, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEditIn, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Botões Centrais
            HWND btnOpen = CreateWindow("BUTTON", "Abrir Arquivo", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                460, 150, 120, 35, hwnd, (HMENU)IDC_BTN_OPEN, GetModuleHandle(NULL), NULL);
            SendMessage(btnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND btnComp = CreateWindow("BUTTON", ">> COMPILAR >>", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                460, 200, 120, 50, hwnd, (HMENU)IDC_BTN_COMPILE, GetModuleHandle(NULL), NULL);
            SendMessage(btnComp, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND btnSave = CreateWindow("BUTTON", "Salvar Saída", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
                460, 270, 120, 35, hwnd, (HMENU)IDC_BTN_SAVE, GetModuleHandle(NULL), NULL);
            SendMessage(btnSave, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Label Saída
            CreateWindow("STATIC", "Código Objeto (MVD):", 
                WS_VISIBLE | WS_CHILD, 
                600, 10, 400, 20, hwnd, (HMENU)IDC_LBL_OUT, GetModuleHandle(NULL), NULL);
            SendMessage(GetDlgItem(hwnd, IDC_LBL_OUT), WM_SETFONT, (WPARAM)hFont, TRUE);

            // Editor Saída
            hEditOut = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Aguardando compilação...", 
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_BORDER, 
                600, 35, 360, 400, hwnd, (HMENU)IDC_OUT_EDIT, GetModuleHandle(NULL), NULL);
            SendMessage(hEditOut, WM_SETFONT, (WPARAM)hFont, TRUE);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_BTN_OPEN: DoOpenFile(hwnd); break;
                case IDC_BTN_SAVE: DoSaveFile(hwnd); break;
                case IDC_BTN_COMPILE: {
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
                            MessageBox(hwnd, "Compilação realizada com sucesso!", "Sucesso", MB_OK | MB_ICONINFORMATION);
                        }
                    } else {
                        if(file) fclose(file);
                        SetWindowText(hEditOut, gui_error_msg);
                        MessageBox(hwnd, "Erro durante a compilação!", "Erro", MB_ICONERROR);
                    }
                } break;
            }
            break;
        case WM_CTLCOLORSTATIC: // Fundo transparente para labels
            SetBkMode((HDC)wParam, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        case WM_DESTROY: 
            DeleteObject(hFont);
            PostQuitMessage(0); 
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "FrankClass";
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW); // Cor padrão de janela mais clara
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    
    // Janela Centralizada
    int width = 1000;
    int height = 500;
    int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "Frank IDE - Compilador Didático", 
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, // Janela fixa para não quebrar layout absoluto
        x, y, width, height, NULL, NULL, hInstance, NULL);
        
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
GtkWidget *window;

void on_open_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Abrir Código Fonte", GTK_WINDOW(window), action,
                                         "_Cancelar", GTK_RESPONSE_CANCEL,
                                         "_Abrir", GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        char *content = NULL;
        gsize length;
        if (g_file_get_contents(filename, &content, &length, NULL)) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_in));
            gtk_text_buffer_set_text(buffer, content, length);
            g_free(content);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void on_save_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new("Salvar Arquivo Objeto", GTK_WINDOW(window), action,
                                         "_Cancelar", GTK_RESPONSE_CANCEL,
                                         "_Salvar", GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "programa.obj");

    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_out));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        char *text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        g_file_set_contents(filename, text, -1, NULL);
        g_free(text);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

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
             gtk_text_buffer_set_text(buffer_out, "Erro: Arquivo objeto não gerado.", -1);
        }
    } else {
        if(file) fclose(file);
        gtk_text_buffer_set_text(buffer_out, gui_error_msg, -1);
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Frank IDE (Linux Edition)");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10); // Margem externa
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // Espaçamento vertical de 10px
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // --- Toolbar com Botões (Ícones) ---
    GtkWidget *toolbar = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(toolbar), GTK_BUTTONBOX_START);
    gtk_box_set_spacing(GTK_BOX(toolbar), 5);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    GtkWidget *btn_open = gtk_button_new_from_icon_name("document-open", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(btn_open, "Abrir Arquivo");
    g_signal_connect(btn_open, "clicked", G_CALLBACK(on_open_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(toolbar), btn_open);

    GtkWidget *btn_save = gtk_button_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(btn_save, "Salvar Saída");
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(toolbar), btn_save);

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(toolbar), sep);

    GtkWidget *btn_compile = gtk_button_new_with_label("COMPILAR");
    GtkWidget *icon_play = gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn_compile), icon_play);
    g_signal_connect(btn_compile, "clicked", G_CALLBACK(on_compile_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(toolbar), btn_compile);

    // --- Painel Principal Dividido (Paned) ---
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);

    // Lado Esquerdo (Entrada)
    GtkWidget *frame_in = gtk_frame_new(NULL);
    GtkWidget *lbl_in = gtk_label_new("<b>Código Fonte</b>");
    gtk_label_set_use_markup(GTK_LABEL(lbl_in), TRUE);
    gtk_frame_set_label_widget(GTK_FRAME(frame_in), lbl_in);
    
    GtkWidget *scroll_in = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll_in), GTK_SHADOW_ETCHED_IN);
    text_view_in = gtk_text_view_new();
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view_in), 5); // Margem interna texto
    gtk_container_add(GTK_CONTAINER(scroll_in), text_view_in);
    gtk_container_add(GTK_CONTAINER(frame_in), scroll_in);
    
    gtk_paned_pack1(GTK_PANED(paned), frame_in, TRUE, FALSE);

    // Lado Direito (Saída)
    GtkWidget *frame_out = gtk_frame_new(NULL);
    GtkWidget *lbl_out = gtk_label_new("<b>Código Objeto (MVD)</b>");
    gtk_label_set_use_markup(GTK_LABEL(lbl_out), TRUE);
    gtk_frame_set_label_widget(GTK_FRAME(frame_out), lbl_out);

    GtkWidget *scroll_out = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll_out), GTK_SHADOW_ETCHED_IN);
    text_view_out = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view_out), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view_out), 5);
    
    // Estilo levemente cinza para saída
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "textview { background-color: #f0f0f0; }", -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(text_view_out);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_container_add(GTK_CONTAINER(scroll_out), text_view_out);
    gtk_container_add(GTK_CONTAINER(frame_out), scroll_out);

    gtk_paned_pack2(GTK_PANED(paned), frame_out, TRUE, FALSE);
    gtk_paned_set_position(GTK_PANED(paned), 450); // Posição inicial do divisor

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
#endif