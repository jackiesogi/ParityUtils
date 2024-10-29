#include <gtk-3.0/gtk/gtk.h>
#include "backup_gui.h"
#include "backup.h"  // Access `backup_options` struct and other utilities
#include "parity.h"

static GtkWidget *file_chooser_button;
static GtkWidget *algorithm_combo_box;

// 儲存 backup_options 以便在觸發 Process 按鈕時使用
static struct backup_options *options;

void on_process_button_clicked(GtkWidget *widget, gpointer data)
{
    // 取得選擇的檔案
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(file_chooser_button);
    GSList *files = gtk_file_chooser_get_filenames(chooser);
    int file_count = g_slist_length(files);

    // 儲存檔案路徑至 options->input
    new_inputs(options, file_count);
    int i = 0;
    for (GSList *iter = files; iter != NULL; iter = iter->next, ++i)
    {
        options->input[i] = g_strdup((char *)iter->data);
    }
    g_slist_free(files);

    // 取得選擇的演算法
    const char *algorithm = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(algorithm_combo_box));
    options->algorithm = g_strdup(algorithm);

    // 執行主要備份邏輯
    backup_internal(options);
    
    // 清理
    g_print("Backup completed!\n");
}

void on_add_files_button_clicked(GtkWidget *widget, gpointer data)
{
    // 彈出檔案選擇對話框
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Select Files", NULL, 
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL);

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        GSList *files = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        for (GSList *iter = files; iter != NULL; iter = iter->next)
        {
            g_print("Selected file: %s\n", (char *)iter->data);
        }
        g_slist_free(files);
    }
    gtk_widget_destroy(dialog);
}

void activate(GtkApplication *app, gpointer user_data)
{
    // 創建主視窗
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Backup Utility");
    gtk_window_set_default_size(GTK_WINDOW(window), 380, 180);

    // 創建主要的 Box 佈局（使用垂直方向）
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // 10 像素的間距
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // 新增檔案選擇標籤和按鈕
    GtkWidget *file_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // 水平盒子
    GtkWidget *file_label = gtk_label_new("Select Files:");
    gtk_box_pack_start(GTK_BOX(file_hbox), file_label, FALSE, FALSE, 0);

    file_chooser_button = gtk_file_chooser_button_new("Choose Files", GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser_button), TRUE); // 支援多選
    gtk_box_pack_start(GTK_BOX(file_hbox), file_chooser_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), file_hbox, FALSE, FALSE, 0);

    // 新增 Parity Algorithm 標籤和下拉式選單
    GtkWidget *algorithm_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *algorithm_label = gtk_label_new("Parity Algorithm:");
    gtk_box_pack_start(GTK_BOX(algorithm_hbox), algorithm_label, FALSE, FALSE, 0);
    
    algorithm_combo_box = gtk_combo_box_text_new();
    for (struct parity_map *map = parity_map_table; map->name != NULL; ++map)
    {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(algorithm_combo_box), map->name);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo_box), 0); 
    gtk_box_pack_start(GTK_BOX(algorithm_hbox), algorithm_combo_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), algorithm_hbox, FALSE, FALSE, 0);

    // 新增 Output File 標籤和輸入欄位
    GtkWidget *output_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *output_label = gtk_label_new("Output File Name:");
    gtk_box_pack_start(GTK_BOX(output_hbox), output_label, FALSE, FALSE, 0);
    
    GtkWidget *output_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(output_hbox), output_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), output_hbox, FALSE, FALSE, 0);

    // 新增 Process 按鈕
    GtkWidget *process_button = gtk_button_new_with_label("Process");
    g_signal_connect(process_button, "clicked", G_CALLBACK(on_process_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), process_button, FALSE, FALSE, 0);

    // 置中 Box 內容
    gtk_widget_set_halign(file_chooser_button, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(algorithm_combo_box, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(process_button, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(output_entry, GTK_ALIGN_CENTER);

    // 使元件自適應大小
    gtk_widget_set_size_request(file_chooser_button, 250, -1);
    gtk_widget_set_size_request(algorithm_combo_box, 250, -1);
    gtk_widget_set_size_request(process_button, 100, -1);
    gtk_widget_set_size_request(output_entry, 250, -1);

    gtk_widget_show_all(window);
}

bool run_gui (struct backup_options *opts)
{
    options = opts;  // 儲存 backup_options
    GtkApplication *app = gtk_application_new("org.example.backup", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}

