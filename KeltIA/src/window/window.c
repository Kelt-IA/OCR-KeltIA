#include "../../include/window/window.h"
#include "../../include/window/callback.h"

extern GResource *keltia_get_resource(void);

void init_window(int *argc, char ***argv)
{
    GtkBuilder *builder = NULL;
    GtkWidget *window = NULL;
    GError *error = NULL;

    g_resources_register(keltia_get_resource());

    // g_setenv("GTK_MODULES", "", TRUE);
    gtk_init(argc, argv);

    builder = gtk_builder_new();

    if (!gtk_builder_add_from_resource(
            builder, "/org/keltia/ressources/window_design_keltia.glade", &error
        ))
    {
        g_printerr(
            "Error loading file: %s\n", error ? error->message : "unknown"
        );
        if (error) g_error_free(error);
        if (builder) g_object_unref(builder);
        return;
    }

    window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    if (!GTK_IS_WIDGET(window))
    {
        g_printerr("Builder object 'main_window' not found or not a widget\n");
        g_object_unref(builder);
        return;
    }

    if (!GTK_IS_WINDOW(window))
    {
        g_printerr(
            "'main_window' is not a GtkWindow (type: %s)\n",
            G_OBJECT_TYPE_NAME(window)
        );
        g_object_unref(builder);
        return;
    }

    // Destroy stop the program when main window is destroyed
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

<<<<<<< HEAD
    callback_init(builder, window);

=======
    callback_init(builder);
>>>>>>> 5aaa9fb (Rotation CB untested)

    gtk_widget_show_all(window);
    gtk_widget_realize(window);

    if (gtk_widget_get_window(window) == NULL)
    {
        g_printerr("Warning: GdkWindow is NULL after realize\n");
    }

    gtk_main();

    g_object_unref(builder);
}
