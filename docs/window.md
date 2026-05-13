# Window / GUI Module

Source: `KeltIA/src/window/`  
Headers: `KeltIA/include/window/`

The window module is the GTK3 graphical interface. It ties together every other module — image preprocessing, zone detection, CNN inference, and the solver — behind a point-and-click UI.

The UI definition is compiled into the binary as a GResource using `keltia.gresource.xml`.

---

## Table of Contents

- [window — Initialisation](#window--initialisation)
- [callback — UI Event Handlers](#callback--ui-event-handlers)
- [utils — Widget Helpers](#utils--widget-helpers)
- [Application Entry Point](#application-entry-point)

---

## window — Initialisation

Header: `include/window/window.h`  
Source: `src/window/window.c`

```c
// Initialise GTK, load the UI definition from the embedded GResource,
// connect all signal handlers, and enter the GTK main loop.
// argc and argv are forwarded from main().
void init_window(int *argc, char **argv[]);
```

This is the only function you need to call to start the application. It blocks until the window is closed.

---

## callback — UI Event Handlers

Header: `include/window/callback.h`  
Source: `src/window/callback.c`

All GTK signal handlers are defined here and connected to widgets by `init_window`.

```c
// Called once after the builder has loaded the UI; sets up widget references.
void callback_init(GtkBuilder *b, GtkWidget *w);

// Toggle the auto-rotate pre-processing option.
void on_autorotate_check(GtkToggleButton *toggle_button, gpointer user_data);

// Open a file-chooser dialog to load a crossword image.
void on_open_image(GtkButton *btn, gpointer user_data);

// Open a file-chooser dialog to load a .nn model file.
void on_open_nn(GtkButton *btn, gpointer user_data);

// Save the currently displayed result image to disk.
void on_save(GtkButton *btn, gpointer user_data);

// Run the full OCR + solver pipeline on the loaded image and model.
// Calls process_crossword_image(), then show_result(), and updates the display.
void on_solve(GtkButton *btn, gpointer user_data);

// Open the training panel window.
void on_open_training(GtkButton *btn, gpointer user_data);

// Handle the training window's close button (signals training to stop if running).
gboolean on_training_window_delete_event(GtkWidget *widget, GdkEvent *event,
    gpointer user_data);

// Open a file-chooser dialog to select the training dataset folder.
void on_open_dataset(GtkButton *btn, gpointer user_data);

// Open a folder-chooser dialog to select where trained models are saved.
void on_select_nn_folder(GtkButton *btn, gpointer user_data);

// Start background training via start_training() with the configured options.
void on_start_training(GtkButton *btn, gpointer user_data);

// Receive per-epoch EvaluationMetrics from the training thread and update
// the accuracy/MSE labels in the training panel (called on the GTK main thread).
void update_metrics(EvaluationMetrics *metrics);

// Called by the training thread when it finishes or is stopped.
void on_stop_training();
```

### Training panel workflow

1. User clicks **Open Training** → `on_open_training` opens the training panel.
2. User selects a dataset folder → `on_open_dataset`.
3. User selects a save folder → `on_select_nn_folder`.
4. User clicks **Start Training** → `on_start_training` calls `start_training()`, which spawns a background thread.
5. After each epoch, the thread invokes `update_metrics` via a GTK idle callback to refresh the UI.
6. User clicks **Stop** or closes the panel → `on_stop_training` or `on_training_window_delete_event` signals the thread.

---

## utils — Widget Helpers

Header: `include/window/utils.h`  
Source: `src/window/utils.c`

Internal helpers used by callback.c. Not part of the public API but documented here for contributors.

Functions in this file typically handle things like updating the image display widget, resetting UI state between operations, and building GTK dialog configurations.

---

## Application Entry Point

Source: `src/keltia.c`

```c
int main(int argc, char *argv[]) {
    init_window(&argc, &argv);
    return EXIT_SUCCESS;
}
```

The entry point simply delegates to `init_window`. All application logic lives inside the window and callback modules.
