#pragma once
#include <err.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

<<<<<<< HEAD
#include "../../include/detect_zones/detect_char.h"
#include "../../include/detect_zones/detect_zones.h"
#include "../../include/image/removenoise.h"
#include "../../include/image/treatment.h"
#include "../../include/nn/callback_for_ui.h"
#include "../../include/nn/network.h"
#include "../../include/nn/network_io.h"
=======
#include "../../include/image/treatment.h"
>>>>>>> 30b78d7 (Linked image processing)

void callback_init(GtkBuilder *b, GtkWidget *w);
void on_autorotate_check(GtkToggleButton *toggle_button, gpointer user_data);
void on_open_image(GtkButton *btn, gpointer user_data);
void on_open_nn(GtkButton *btn, gpointer user_data);
void on_save(GtkButton *btn, gpointer user_data);
void on_solve(GtkButton *btn, gpointer user_data);
void on_open_training(GtkButton *btn, gpointer user_data);
gboolean on_training_window_delete_event(
    GtkWidget *widget,
    GdkEvent *event,
    gpointer user_data
);
void on_open_dataset(GtkButton *btn, gpointer user_data);
void on_select_nn_folder(GtkButton *btn, gpointer user_data);
void on_start_training(GtkButton *btn, gpointer user_data);
<<<<<<< HEAD
void update_metrics(EvaluationMetrics *metrics);
void on_stop_training();
=======
>>>>>>> 30b78d7 (Linked image processing)
