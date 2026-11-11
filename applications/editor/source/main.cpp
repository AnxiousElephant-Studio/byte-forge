#include <byte-forge-editor/editor_application.h>

#include <cstdlib>

int main(const int argc, const char** argv)
{
    byteforge::editor::editor_application_t editor_application;

    if (!editor_application.initialize())
    {
        editor_application.dispose();
        return EXIT_FAILURE;
    }

    if (argc > 1)
    {
        editor_application.request_project(argv[1]);
    }

    editor_application.run();

    editor_application.dispose();

    return EXIT_SUCCESS;
}
