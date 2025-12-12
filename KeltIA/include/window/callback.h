#pragma once
#include <err.h>
#include <gtk/gtk.h>
#include <stdlib.h>

void callback_init(GtkBuilder *b);
void on_autorotate_check(GtkToggleButton *toggle_button, gpointer user_data);
void on_open_image(GtkButton *btn, gpointer user_data);
void on_open_nn(GtkButton *btn, gpointer user_data);
// void on_save(GtkButton *btn, gpointer user_data);
void on_solve(GtkButton *btn, gpointer user_data);
void on_open_training(GtkButton *btn, gpointer user_data);
