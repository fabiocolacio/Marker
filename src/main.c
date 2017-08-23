
#include "marker-app.h"
#include "marker-editor-window.h"

int
main(int   argc,
     char* argv[])
{
    MarkerApp* app = marker_app_new();
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

