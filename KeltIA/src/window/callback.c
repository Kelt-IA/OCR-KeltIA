#include "../../include/window/callback.h"
#include "../../include/window/utils.h"

GtkBuilder *builder = NULL;
GtkWidget *window = NULL;

GtkWidget *rotation_check = NULL;
GtkWidget *rotation_box = NULL;

GtkWidget *viewport = NULL;
GtkWidget *load_img_btn = NULL;
char *img = NULL;

GtkWidget *load_nn_btn = NULL;
char *nn = NULL;

GtkWidget *solve_btn = NULL;

void callback_init(GtkBuilder *b, GtkWidget *w)
{
    builder = b;
    window = w;

    if (!(window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"))))
        errx(EXIT_FAILURE, "Could not find the main window");

    if (!(rotation_box =
              GTK_WIDGET(gtk_builder_get_object(builder, "rotation_gbox"))))
        errx(EXIT_FAILURE, "Could not find rotation box");

    if (!(rotation_check =
              GTK_WIDGET(gtk_builder_get_object(builder, "auto_rotation_btn"))))
        errx(EXIT_FAILURE, "Could not find rotation checkbox");

    g_signal_connect(
        rotation_check, "clicked", G_CALLBACK(on_autorotate_check), NULL
    );

    if (!(viewport =
              GTK_WIDGET(gtk_builder_get_object(builder, "viewport_img"))))
        errx(EXIT_FAILURE, "Could not find viewport");

    if (!(load_img_btn =
              GTK_WIDGET(gtk_builder_get_object(builder, "load_image_btn"))))
        errx(EXIT_FAILURE, "Could not find load img btn");

    g_signal_connect(load_img_btn, "clicked", G_CALLBACK(on_open_image), NULL);

    if (!(load_nn_btn =
              GTK_WIDGET(gtk_builder_get_object(builder, "load_model_btn"))))
        errx(EXIT_FAILURE, "Could not find load model btn");

    g_signal_connect(load_nn_btn, "clicked", G_CALLBACK(on_open_nn), NULL);

    if (!(solve_btn = GTK_WIDGET(gtk_builder_get_object(builder, "solve_btn"))))
        errx(EXIT_FAILURE, "Could not find solve btn");

    g_signal_connect(solve_btn, "clicked", G_CALLBACK(on_solve), NULL);
}

void on_autorotate_check(GtkToggleButton *toggle_button, gpointer user_data)
{
    (void)user_data;
<<<<<<< HEAD

=======
>>>>>>> 9231657 (Quick fix)
    gboolean active = gtk_toggle_button_get_active(toggle_button);

    if (active) { gtk_widget_set_sensitive(rotation_box, FALSE); }
    else
    {
        gtk_widget_set_sensitive(rotation_box, TRUE);
    }
}

void on_open_image(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    GtkWidget *parent = window;
    GtkWidget *dialog;
    gint res;

    dialog = gtk_file_chooser_dialog_new(
        "Open image", GTK_WINDOW(parent), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL
    );

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        GtkFileFilter *filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Image files");

        gtk_file_filter_add_pattern(filter, "*.png");
        gtk_file_filter_add_pattern(filter, "*.jpg");
        gtk_file_filter_add_pattern(filter, "*.jpeg");
        gtk_file_filter_add_pattern(filter, "*.bmp");

        gtk_file_chooser_add_filter(chooser, filter);
        gtk_file_chooser_set_filter(chooser, filter);

        filename = gtk_file_chooser_get_filename(chooser);

        if (!is_supported_by_magickwand(filename))
        {
            g_free(filename);
            return;
        }

        if (img)
        {
            remove(img);
            free(img);
        }

        img = copy_to_temp_file_path((const char *)img);

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void on_open_nn(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    GtkWidget *parent = window;
    GtkWidget *dialog;
    gint res;

    dialog = gtk_file_chooser_dialog_new(
        "Open neural network", GTK_WINDOW(parent), GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, NULL
    );

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        GtkFileFilter *filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Neural network files");

        gtk_file_filter_add_pattern(filter, "*.nn");

        gtk_file_chooser_add_filter(chooser, filter);
        gtk_file_chooser_set_filter(chooser, filter);

        filename = gtk_file_chooser_get_filename(chooser);

        if (nn) { g_free(nn); }

        nn = filename;
    }

    gtk_widget_destroy(dialog);
}

void on_save() {}

void on_solve(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;
}

void on_open_training(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;
}
