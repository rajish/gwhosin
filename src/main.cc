#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <string>
#include <iostream>
#include <giomm.h>
#include <sys/stat.h>

#include "defines.h"
#include "UnWidget.h"
#include "Dbg.h"


using namespace std;

Glib::ustring& find_iconpath( Glib::ustring progname )
{
    struct stat filemode;
    static Glib::ustring iconpath = Glib::find_program_in_path( progname );

    if( iconpath != "" && iconpath.find("bin") != std::string::npos )
    {	
        iconpath.erase( iconpath.find("bin") );
        iconpath += "share/gwhosin/img/";
    }

    if( iconpath == "" )
        iconpath = "/usr/share/gwhosin/img/"; // try to prevent errors from PATH lacking the executable
		
		
    if( stat( iconpath.c_str(), &filemode) != 0 )
        iconpath = "/usr/local/share/gwhosin/img/";
		
	
    // last resort to current working directory for img (when running
    // gwhosin from the extraction directory to try it out for instance
    if( stat( iconpath.c_str(), &filemode) != 0 ) 
        iconpath = Glib::get_current_dir() + (Glib::ustring)"/img/";
	
		
    if( stat( iconpath.c_str(), &filemode) != 0 ) 
        iconpath = Glib::get_current_dir() + (Glib::ustring)"/../img/";
		
    DBG(cout << "FIND_ICONPATH: " << iconpath << std::endl);

    if( stat( iconpath.c_str(), &filemode) != 0 )
        cerr << GT("Gwhosin img could not be found! This might cause a segfault.\n");

    return iconpath;
}
	

int main(int argc, char** argv)
{
    Gio::init();
    Gtk::Main kit(argc, argv);
    Notify::init("gwhosin");

    UnWidget log_window(find_iconpath("gwhosin"));
    
    kit.run();    // don't pass window as an argument to prevent it from showing on startup
    return 0;
}
