#include "../../include/window/callback.h"

GtkBuilder *builder = NULL;
GtkWidget *rotation_check = NULL;
GtkWidget *rotation_box = NULL;

void callback_init(GtkBuilder *b)
{
    builder = b;
    if (!(rotation_box =
              GTK_WIDGET(gtk_builder_get_object(builder, "rotation_gbox"))))
        errx(EXIT_FAILURE, "Could not find rotation box");

    if (!(rotation_check =
              GTK_WIDGET(gtk_builder_get_object(builder, "auto_rotation_btn"))))
        errx(EXIT_FAILURE, "Could not find rotation checkbox");

    g_signal_connect(
        rotation_check, "clicked", G_CALLBACK(on_autorotate_check), NULL
    );
}

void on_autorotate_check(GtkToggleButton *toggle_button, gpointer user_data)
{
    (void)user_data;

    gboolean active = gtk_toggle_button_get_active(toggle_button);

    if (active) { gtk_widget_set_sensitive(rotation_box, FALSE); }
    else { gtk_widget_set_sensitive(rotation_box, TRUE); }
}

void on_open_image() {}

void on_open_nn() {}

void on_save() {}

void on_solve() {}

void on_open_training(GtkToggleButton *toggle_button, gpointer user_data)
{
    (void)toggle_button;
    (void)user_data;
}
