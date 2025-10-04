#include "../../include/window/window.h"

static void print_widget_tree(GtkWidget *widget, int depth)
{
    if (!GTK_IS_WIDGET(widget)) return;

    for (int i = 0; i < depth; i++) g_print("  ");

    g_print(
        "%s (realized: %d, window: %p)\n", G_OBJECT_TYPE_NAME(widget),
        gtk_widget_get_realized(widget), (void *)gtk_widget_get_window(widget)
    );

    if (GTK_IS_CONTAINER(widget))
    {
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget));
        for (GList *l = children; l; l = l->next)
            print_widget_tree(GTK_WIDGET(l->data), depth + 1);
        g_list_free(children);
    }
}

static gboolean present_window_idle(gpointer data)
{
    gtk_window_present(GTK_WINDOW(data));
    return G_SOURCE_REMOVE;
}

void init_window(int *argc, char ***argv)
{
    GtkBuilder *builder = NULL;
    GtkWidget *window = NULL;
    GError *error = NULL;

    g_setenv("GTK_MODULES", "", TRUE);
    gtk_init(argc, argv);

    builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(
            builder, "./KeltIA/ressources/window design keltia.glade", &error
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

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_widget_realize(window);

    if (gtk_widget_get_window(window) == NULL)
    {
        g_printerr("Warning: GdkWindow is NULL after realize\n");
    }

    g_print("\n=== Widget tree diagnostic ===\n");
    print_widget_tree(window, 0);
    g_print("==============================\n\n");

    g_idle_add(present_window_idle, window);

    gtk_main();

    g_object_unref(builder);
}
