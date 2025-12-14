#include "../../include/window/callback.h"
#include "../../include/window/utils.h"

GtkBuilder *builder = NULL;
GtkWidget *window = NULL;

GtkWidget *color = NULL;
GtkWidget *grayscale = NULL;
GtkWidget *bw = NULL;

GtkWidget *noise = NULL;

GtkWidget *rotation_check = NULL;
GtkWidget *rotation_box = NULL;
GtkWidget *rotation_spin = NULL;

GtkWidget *viewport = NULL;
GtkWidget *load_img_btn = NULL;
char *img = NULL;

GtkWidget *load_nn_btn = NULL;
char *nn_path = NULL;

GtkWidget *solve_btn = NULL;
GtkWidget *save_btn = NULL;

GtkBuilder *training_builder = NULL;
GtkWidget *training_btn = NULL;
GtkWidget *training_win = NULL;
int train_running = 0;

GtkWidget *open_dataset = NULL;
GtkWidget *select_nn_folder = NULL;
GtkWidget *start_training_btn = NULL;

char *dataset = NULL;
char *nn_folder = NULL;

GtkWidget *dataset_label = NULL;
GtkWidget *nn_label = NULL;

GtkWidget *accuracy = NULL;
GtkWidget *mse = NULL;
GtkWidget *predictions = NULL;

TrainingState *state = NULL;

void callback_init(GtkBuilder *b, GtkWidget *w)
{
    builder = b;
    window = w;

    if (!(window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"))))
        errx(EXIT_FAILURE, "Could not find the main window");

    if (!(viewport =
              GTK_WIDGET(gtk_builder_get_object(builder, "viewport_img"))))
        errx(EXIT_FAILURE, "Could not find the viewport");

    if (!(rotation_box =
              GTK_WIDGET(gtk_builder_get_object(builder, "rotation_gbox"))))
        errx(EXIT_FAILURE, "Could not find rotation box");

    if (!(rotation_spin =
              GTK_WIDGET(gtk_builder_get_object(builder, "rotation_btn"))))
        errx(EXIT_FAILURE, "Could not find rotation spin");

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

    if (!(save_btn = GTK_WIDGET(gtk_builder_get_object(builder, "save_btn"))))
        errx(EXIT_FAILURE, "Could not find save btn");

    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save), NULL);

    if (!(training_btn =
              GTK_WIDGET(gtk_builder_get_object(builder, "open_training_btn"))))
        errx(EXIT_FAILURE, "Could not find training btn");

    g_signal_connect(
        training_btn, "clicked", G_CALLBACK(on_open_training), NULL
    );

    if (!(color =
              GTK_WIDGET(gtk_builder_get_object(builder, "render_color_btn"))))
        errx(EXIT_FAILURE, "Could not find render color btn");

    if (!(grayscale = GTK_WIDGET(
              gtk_builder_get_object(builder, "render_grayscale_btn")
          )))
        errx(EXIT_FAILURE, "Could not find grayscale btn");

    if (!(bw = GTK_WIDGET(gtk_builder_get_object(builder, "render_bw_btn"))))
        errx(EXIT_FAILURE, "Could not find bw btn");

    if (!(noise = GTK_WIDGET(
              gtk_builder_get_object(builder, "image_correction_btn")
          )))
        errx(EXIT_FAILURE, "Could not find noise btn");
}

void on_autorotate_check(GtkToggleButton *toggle_button, gpointer user_data)
{
    (void)user_data;

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

        img = copy_to_temp_file_path((const char *)filename);

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

        if (nn_path) { g_free(nn_path); }

        nn_path = filename;
    }

    gtk_widget_destroy(dialog);
}

void on_save(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    if (!img) return;

    GtkWindow *parent_window = GTK_WINDOW(window);  // Fenêtre parente
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new(
        "Save File", parent_window, action, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT, NULL
    );

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    gtk_file_chooser_set_current_name(chooser, "solved_grid.pnd");

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(chooser);

        // TODO: Implémentez votre logique de sauvegarde ici
        // Ex: écrire le contenu de votre grille dans filename

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void on_solve(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    if (!img) return;

    ProcessedImages *p = process_image(
        img, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(grayscale)),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bw)),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(noise)),
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rotation_check)),
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(rotation_spin))
    );

    CharBBox **grid = NULL;
    char *char_grid = NULL;
    int height = 0;
    int width = 0;
    char **words = NULL;
    int num_words = 0;

    // Call function - it fills all the pointers
    int error = extract_crossword_data(
        p->nn_image_path,  // image path
        nn_path,           // model path
        &grid,             // pointer to receive grid
        &char_grid,        // pointer to receive char array
        &height,           // pointer to receive height
        &width,            // pointer to receive width
        &words,            // pointer to receive words array
        &num_words         // pointer to receive word count
    );

    if (error) return;

    MagickWand *wand = read_image(p->ui_image_path);

    if (!wand)
    {
        fprintf(stderr, "Error: unable to read input image.\n");
        // MagickWandTerminus();
        return;
    }

    show_result(grid, char_grid, height, width, words, wand);

    MagickBooleanType status;

    status = MagickWriteImage(wand, "/tmp/output.png");

    if (status == MagickFalse)
    {
        printf("Failed to save image result to tmp file\n");
        return;
    }

    gtk_image_set_from_file(GTK_IMAGE(viewport), "/tmp/output.png");

    free(p);
}

void on_open_training(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    if (training_win)
    {
        gtk_widget_show_all(training_win);
        gtk_window_present(GTK_WINDOW(training_win));
        return;
    }

    GError *error = NULL;

    training_builder = gtk_builder_new();

    if (!gtk_builder_add_from_resource(
            training_builder, "/org/keltia/ressources/training_window.glade",
            &error
        ))
    {
        g_printerr(
            "Error loading file: %s\n", error ? error->message : "unknown"
        );
        if (error) g_error_free(error);
        if (training_builder) g_object_unref(training_builder);
        return;
    }

    training_win =
        GTK_WIDGET(gtk_builder_get_object(training_builder, "training_window"));

    gtk_window_set_title(GTK_WINDOW(training_win), "KeltIA - NN Training Mode");

    g_signal_connect(
        training_win, "delete-event",
        G_CALLBACK(on_training_window_delete_event), NULL
    );

    if (!(open_dataset = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "dataset_btn")
          )))
        errx(EXIT_FAILURE, "Could not find dataset btn");

    if (!(dataset_label = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "dataset_folder_label")
          )))
        errx(EXIT_FAILURE, "Could not find dataset label");

    g_signal_connect(
        open_dataset, "clicked", G_CALLBACK(on_open_dataset), NULL
    );

    if (!(select_nn_folder = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "nn_folder_btn")
          )))
        errx(EXIT_FAILURE, "Could not find nn folder btn");

    if (!(nn_label = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "output_folder")
          )))
        errx(EXIT_FAILURE, "Could not find nn folder label");

    g_signal_connect(
        select_nn_folder, "clicked", G_CALLBACK(on_select_nn_folder), NULL
    );

    if (!(start_training_btn = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "training_btn")
          )))
        errx(EXIT_FAILURE, "Could not find start training btn");

    g_signal_connect(
        start_training_btn, "clicked", G_CALLBACK(on_start_training), NULL
    );

    if (!(accuracy = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "accuracy_label")
          )))
        errx(EXIT_FAILURE, "Could not find accuracy");

    if (!(mse = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "mse_label")
          )))
        errx(EXIT_FAILURE, "Could not find mse");

    if (!(predictions = GTK_WIDGET(
              gtk_builder_get_object(training_builder, "prediction_label")
          )))
        errx(EXIT_FAILURE, "Could not find predictions");

    gtk_widget_show_all(training_win);
}

gboolean on_training_window_delete_event(
    GtkWidget *widget,
    GdkEvent *event,
    gpointer user_data
)
{
    (void)user_data;
    (void)widget;
    (void)event;

    gtk_widget_hide(training_win);

    train_running = 0;
    return TRUE;
}

void on_open_dataset(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    GtkWidget *parent = window;
    GtkWidget *dialog;
    gint res;

    dialog = gtk_file_chooser_dialog_new(
        "Open dataset folder", GTK_WINDOW(parent),
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT, NULL
    );

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *folder;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        folder = gtk_file_chooser_get_filename(chooser);

        if (dataset) { g_free(dataset); }

        dataset = folder;

        gtk_label_set_text(GTK_LABEL(dataset_label), dataset);

        gtk_window_present(GTK_WINDOW(training_win));
    }

    gtk_widget_destroy(dialog);
}

void on_select_nn_folder(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    GtkWidget *parent = window;
    GtkWidget *dialog;
    gint res;

    dialog = gtk_file_chooser_dialog_new(
        "Select nn folder", GTK_WINDOW(parent),
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT, NULL
    );

    gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(dialog), TRUE);

    res = gtk_dialog_run(GTK_DIALOG(dialog));

    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *folder;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

        folder = gtk_file_chooser_get_filename(chooser);

        if (nn_folder) { g_free(nn_folder); }

        nn_folder = folder;

        gtk_label_set_text(GTK_LABEL(nn_label), nn_folder);

        gtk_window_present(GTK_WINDOW(training_win));
    }

    gtk_widget_destroy(dialog);
}

void on_start_training(GtkButton *btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;

    if (state && state->is_training)
    {
        stop_training(state);
        return;
    }

    if (!dataset || !nn_folder) return;

    gtk_button_set_label(GTK_BUTTON(start_training_btn), "Stop training");

    TrainingConfig config = {};

    config.model_path = nn_path;
    config.dataset_folder = dataset;
    config.save_folder = nn_folder;
    config.max_epochs = 100;
    config.save_interval = 20;
    config.callback = update_metrics;

    int val = start_training(&config, &state);

    if (val) state = NULL;
}

void update_metrics(EvaluationMetrics *metrics)
{
    char acc[10] = {0};
    char mse_text[100] = {0};
    char pred[100] = {0};

    sprintf(acc, "%.2f %%", metrics->accuracy * 100.0);
    sprintf(mse_text, "%.4f", metrics->mse);
    sprintf(pred, "%d", metrics->correct_predictions);

    gtk_label_set_text(GTK_LABEL(accuracy), acc);
    gtk_label_set_text(GTK_LABEL(mse), mse_text);
    gtk_label_set_text(GTK_LABEL(predictions), pred);
}

void on_stop_training()
{
    gtk_label_set_text(GTK_LABEL(accuracy), "0 %");
    gtk_label_set_text(GTK_LABEL(mse), "0");
    gtk_label_set_text(GTK_LABEL(predictions), "0");

    gtk_button_set_label(GTK_BUTTON(start_training_btn), "Start training");

    state = NULL;
}
