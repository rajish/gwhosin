#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <exception>
#include "UnWidget.h"

using namespace std;
using Glib::ustring;


const string wtmp_fname = WTMP_FILE;
const string utmp_fname = UTMP_FILE;



UnWidget::UnWidget(const Glib::ustring& iconpath) : notification("", ""), wtmp_in(wtmp_fname.c_str(),  ios::in | ios::binary)
{
    set_title("Logged users");
    set_border_width(5);
    set_default_size(600, 400);

    // System tray icon
    pixbuf[0] = Gdk::Pixbuf::create_from_file(iconpath + "green-light.png");
    pixbuf[1] = Gdk::Pixbuf::create_from_file(iconpath + "red-light.png");
    status_icon = Gtk::StatusIcon::create(pixbuf[0]);
    status_icon->set_tooltip("Login sentinel");
    status_icon->signal_popup_menu().connect(sigc::mem_fun(*this, &UnWidget::on_popup_menu));
    status_icon->signal_activate().connect(sigc::mem_fun(*this, &UnWidget::on_icon_activate));
    
    // popup menu
    action_group = Gtk::ActionGroup::create();
    action_group->add(Gtk::Action::create("PopupExit", "Exit"), sigc::mem_fun(*this, &UnWidget::on_menu_exit));

    ui_manager = Gtk::UIManager::create();
    ui_manager->insert_action_group(action_group);
    add_accel_group(ui_manager->get_accel_group());
    
    ustring ui_info =
        "<ui>"
        "  <popup name='PopupMenu'>"
        "    <menuitem action='PopupExit' />"
        "  </popup>"
        "</ui>";
    try 
    {
        ui_manager->add_ui_from_string(ui_info);
    }
    catch (const Glib::Error& ex)
    {
        cout << "building menu failed: " << ex.what();
    }
    popup_menu = dynamic_cast<Gtk::Menu*>(ui_manager->get_widget("/PopupMenu"));
    if(!popup_menu)
        cout << "warning: menu not found" << endl;

    // notification popup
    notification.attach_to_widget(*this);

    // logged-in users tree
    add(vbox);

    scrolled_window.add(tree_view);
    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    vbox.pack_start(scrolled_window);



    // Switch it on
    show_all_children();
    init_logins();
    update_icon();
}

void UnWidget::on_close(const Glib::ustring& action)
{
    
}

void UnWidget::on_wtmp_changed(const Glib::RefPtr<Gio::File>& file,
                     const Glib::RefPtr<Gio::File>& other_file,
                     Gio::FileMonitorEvent event)
{
    utmp utmp_entry;
    if (event == Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT)
    {
        while(wtmp_in.read((char*)&utmp_entry, sizeof utmp_entry))
        {
            check_entry(utmp_entry);
        }
        wtmp_in.clear();                           // clear eof and other bad flags
        dump_lists();
        update_icon();
    }    
}

void UnWidget::on_popup_menu(guint button, guint32 activate_time)
{
    cout << "UnWidget::on_popup_menu(" << button << ", " << activate_time << ")" << endl;
    popup_menu->popup(button, activate_time);
}

void UnWidget::on_icon_activate()
{
    cout << "UnWidget::on_icon_activate()" << endl;
    set_visible(!get_visible());
}

void UnWidget::on_menu_exit()
{
    cout << "UnWidget::on_menu_exit()" << endl;
    // TODO: There should be more elegant way to close the app
    exit(0);
}

void UnWidget::update_icon()
{
    int cnt_users = (int) users_map.size();
    for (UsersMap::iterator it = users_map.begin(); it != users_map.end(); it++)
    {
        if (tree_view.is_ignored_user((*it).first))
        {
            cnt_users--;
        }
    }
    stringstream cnt_str;
    cnt_str << "Users logged in: " << cnt_users;
    status_icon->set_tooltip(cnt_str.str());
    if(cnt_users > 0)
    {
        // int w = pixbuf[1]->get_width();
        // int h = pixbuf[1]->get_height();
        // Glib::RefPtr<Gdk::Drawable> drawable = Gdk::Drawable::create();
        // Glib::RefPtr<Gdk::Pixmap> pixmap = Gdk::Pixmap::create( (Glib::RefPtr<Gdk::Drawable>) 0, w, h);
        // pixmap->draw_pixbuf(pixbuf[1], 0, 0, 0, 0, -1, -1, Gdk::RGB_DITHER_NONE, 0, 0);

        // Glib::RefPtr<Pango::Layout> pl = create_pango_layout(cnt_str.str());
        // Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(pixmap);
        // drawable->draw_layout(gc, 0, 0, pl);
        
        // Glib::RefPtr<Gdk::Pixbuf> icon = Gdk::Pixbuf::create(Glib::RefPtr<Gdk::Drawable>::cast_static(pixmap), 0, 0, w, h);
        // status_icon->set(icon);
        status_icon->set(pixbuf[1]);
    }
    else
    {
        status_icon->set(pixbuf[0]);
    }
}

void UnWidget::dump_lists() const
{
    UsersMap::const_iterator it;
    for (it = users_map.begin(); it != users_map.end(); it++)
    {
        cout << it->first << " ==> " << it->second.size() << endl;
        for (UserEntries::iterator it1 = it->second.begin(); it1 != it->second.end(); it1++)
        {
            cout << "\t" << it1->to_string() << endl;
        }
    }
    for (LineEntries::const_iterator it2 = line_entries.begin(); it2 != line_entries.end(); it2++)
    {
        cout << it2->first << " => " << it2->second << endl;
    }
    cout << "================================================================================" << endl;
}

void UnWidget::notify(bool log_in, const UtEntry& entry)
{
    ustring summary("User ");
    summary += entry.get_user();
    if(log_in)
        summary += " logged in";
    else
        summary += " logged out";
    notification.update(summary, entry.to_string(), "");
    notification.set_timeout(3000);
    notification.set_urgency(Notify::URGENCY_CRITICAL);
    try 
    {
        if(!notification.show()) 
        {
            cerr << "failed to show notification" << endl;
        }
        // else
        //   cerr << "notification shown" << endl;
    }
    catch (Glib::Error& err)
    {
        cout << err.what() << endl;
    }
}

void UnWidget::check_entry(struct utmp& utmp_entry)
{
    UtEntry ut_entry(utmp_entry);
    pair<UsersMap::iterator, bool> ret;
    string lname = ut_entry.get_line();
    string uname;
    
    if (ut_entry.is_login_process() || ut_entry.is_user_process())
    {
        uname = ut_entry.get_user();
        ret = users_map.insert(User(uname, UserEntries()));
        pair<UserEntries::iterator, bool> sret = ret.first->second.insert(ut_entry);    // it means: ret.iterator->UserEntries...
        cout << (ret.second == true ? "new user '" : "append user '") << uname << "': " 
             << (sret.second ? ut_entry.to_string() : lname + " already in set") << endl;
        line_entries.insert(LineEntry(lname, uname));
        tree_view.add_log_line(ut_entry);
        notify(true, ut_entry);
        return;
    }
    
    if (ut_entry.is_dead_process())
    {
        LineEntries::iterator lit = line_entries.find(lname);
        if(lit != line_entries.end())
        {
            uname = lit->second;
            UserEntries& uset = users_map.find(uname)->second;
            UserEntries::iterator it = uset.find(ut_entry);
            if (it != uset.end())
            {
                const UtEntry& entry = *it;
                cout << "deleting: " << entry.to_string() << endl;
                notify(false, ut_entry);
                tree_view.add_log_line(ut_entry);
                uset.erase(it);
                line_entries.erase(line_entries.find(lname));
                return;
            }
        }        
    }
    cout << "not processed: " << ut_entry.to_string() << endl;
}

void UnWidget::init_logins()
{
    // check currently logged in users
    ifstream utmp(utmp_fname.c_str(), ios::in | ios::binary);
    if (!utmp.is_open())
    {
        cout << "could not open " << utmp_fname << endl;
        return;
    }
    struct utmp utmp_entry;
    while(utmp.read((char*)&utmp_entry, sizeof utmp_entry))
    {
        check_entry(utmp_entry);
    }

    // Open wtmp file and go to end - we look only for additions
    if(!wtmp_in.is_open())
        throw exception();
    wtmp_in.seekg(0, ios::end);

    // Monitor file changes - should rely on kernel events, but who knows...
    wtmp = Gio::File::create_for_path(wtmp_fname);
    monitor = wtmp->monitor_file();
    monitor->signal_changed().connect(sigc::mem_fun(*this, &UnWidget::on_wtmp_changed));


}

