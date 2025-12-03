#pragma once
#include <err.h>
#include <gtk/gtk.h>
#include <stdlib.h>

void callback_init(GtkBuilder *b);
void on_autorotate_check(GtkToggleButton *toggle_button, gpointer user_data);
