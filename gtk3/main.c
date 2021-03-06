/***************************************************************************
 *   Copyright (C) 2010~2012 by CSSlayer                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <gtk/gtk.h>
#include <langinfo.h>
#include <libintl.h>
#include <locale.h>
#include "config.h"
#include "main_window.h"
#include "config_widget.h"
#include "fcitx-config-gtk3-resources.h"
#include <fcitx-utils/utils.h>

static void
fcitx_config_app_activate (GApplication *application)
{
    GList* list = gtk_application_get_windows (GTK_APPLICATION(application));
    if (list)
    {
        gtk_window_present (GTK_WINDOW (list->data));
    }
    else {
        GtkWidget *window;
        window = fcitx_main_window_new();
        gtk_application_add_window(GTK_APPLICATION(application), GTK_WINDOW(window));
        gtk_window_set_position( GTK_WINDOW(window), GTK_WIN_POS_CENTER_ALWAYS );
        gtk_widget_show_all (GTK_WIDGET (window));
    }
}

typedef GtkApplication FcitxConfigApp;
typedef GtkApplicationClass FcitxConfigAppClass;

G_DEFINE_TYPE (FcitxConfigApp, fcitx_config_app, GTK_TYPE_APPLICATION)

static void
fcitx_config_app_finalize (GObject *object)
{
    G_OBJECT_CLASS (fcitx_config_app_parent_class)->finalize (object);
}

static void
fcitx_config_app_init (FcitxConfigApp *app)
{
    fcitx_config_gtk3_register_resource();
    g_resources_register (fcitx_config_gtk3_get_resource ());
}


int fcitx_config_app_handle_command_line (GApplication              *application,
                                          GApplicationCommandLine   *command_line,
                                          gpointer                   user_data
                                         )
{
    int argc;
    gchar** argv = g_application_command_line_get_arguments(command_line, &argc);
    g_application_activate(G_APPLICATION (application));
    GList* list = gtk_application_get_windows (GTK_APPLICATION(application));
    static GtkWidget* dialog = NULL;
    if (list) {
        FcitxMainWindow* mainWindow = FCITX_MAIN_WINDOW (list->data);
        FcitxAddon* addon = NULL;
        if (argc >= 2 && argv[1])
            addon = find_addon_by_name(mainWindow->addons, argv[1]);
        if (argc == 2 && addon) {
            if (dialog) {
                //There are one window opened already. Close it first
                gtk_widget_destroy(dialog);
            }
            gtk_widget_hide(GTK_WIDGET(mainWindow));

            FcitxConfigFileDesc* cfdesc = NULL;
            if (addon) {
                    gchar* config_desc_name = g_strdup_printf("%s.desc", addon->name);
                    cfdesc = get_config_desc(config_desc_name);
                    g_free(config_desc_name);
            }

            if (0 == fcitx_im_config_thirdpart(cfdesc)) {
                    dialog = fcitx_config_dialog_new(addon, NULL);
                    if (dialog) {
                            gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ALWAYS);
                            gtk_widget_show_all(GTK_WIDGET(dialog));
                    }
            }
        }
        if (argc == 3 && addon && strcmp(argv[2],"exit") == 0) {
            fcitx_utils_kill_fcitx_config_gtk3(addon);
            exit(9);
        }
    }

    g_strfreev(argv);
    return 0;
}

static void
fcitx_config_app_class_init (FcitxConfigAppClass *klass)
{
    G_OBJECT_CLASS (klass)->finalize= fcitx_config_app_finalize;

    G_APPLICATION_CLASS (klass)->activate = fcitx_config_app_activate;
}

FcitxConfigApp *
fcitx_config_app_new (void)
{
#if !GLIB_CHECK_VERSION(2, 35, 1)
    g_type_init();
#endif

    FcitxConfigApp* app = g_object_new (fcitx_config_app_get_type (),
                         "application-id", "org.fcitx.FcitxConfigGtk3",
                         "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
                         NULL);
    g_signal_connect(app, "command-line", (GCallback)fcitx_config_app_handle_command_line, NULL);
    return app;
}

int
main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain("fcitx-configtool", LOCALEDIR);
    bind_textdomain_codeset("fcitx-configtool", "UTF-8");
    bindtextdomain("fcitx", LOCALEDIR);
    bind_textdomain_codeset("fcitx", "UTF-8");
    textdomain("fcitx-configtool");
    FcitxLogSetLevel(FCITX_NONE);

    fcitx_utils_launch_tool("fcitx-autostart", NULL);

    GtkApplication* app = fcitx_config_app_new();

    int status = 0;
    if (app) {
        GError* error = NULL;
        if (!g_application_register(G_APPLICATION(app), NULL, &error)) {
            g_warning("Cannot register %s", error->message);
            g_error_free(error);
            error = NULL;
        }
        if (g_application_get_is_registered(G_APPLICATION(app))) {
            if (g_application_get_is_remote(G_APPLICATION(app))) {
                g_message("fcitx-config-gtk3 is running.");
            }
        }
    }

    status = g_application_run (G_APPLICATION (app), argc, argv);

    g_object_unref (app);

    return status;
}


