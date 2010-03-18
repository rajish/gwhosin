#include <string>
#include <giomm.h>
#include "widget.h"

using namespace std;


int main(int argc, char** argv)
{
    Gio::init();
    Gtk::Main kit(argc, argv);
    Notify::init("unotify");

    UnWidget window;

    kit.run();    // don't pass window as an argument to prevent it from showing on startup
    return 0;
}
