#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "defines.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <exception>
#include "UnWidget.h"
#include "Dbg.h"

#include <cairomm/context.h>
#include <cairomm/surface.h>

using namespace std;
using Glib::ustring;


const string wtmp_fname = WTMP_FILE;
const string utmp_fname = UTMP_FILE;



UnWidget::UnWidget(const Glib::ustring& iconpath) :
        notification("", ""), wtmp_in(wtmp_fname.c_str(),  ios::in | ios::binary),
        non_ignored_user_cnt(0)
{
    set_title("Logged users");
    set_border_width(5);
    set_default_size(600, 400);
    this->iconpath = iconpath;
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
        cerr << "building menu failed: " << ex.what();
    }
    popup_menu = dynamic_cast<Gtk::Menu*>(ui_manager->get_widget("/PopupMenu"));
    if(!popup_menu)
        cerr << "warning: menu not found" << endl;

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
    TRACEFN;
    popup_menu->popup(button, activate_time);
}

void UnWidget::on_icon_activate()
{
    TRACEFN;
    set_visible(!get_visible());
}

void UnWidget::on_menu_exit()
{
    TRACEFN;
    // TODO: There should be more elegant way to close the app
    exit(0);
}

void UnWidget::update_icon()
{
    int cnt_users = (int) users_map.size();
    for (UsersMap::iterator it = users_map.begin(); it != users_map.end(); it++)
    {
        if (UtEntry::is_ignored_user((*it).first))
        {
            cnt_users--;
        }
    }
    stringstream cnt_str;
    cnt_str << "Users logged in: " << cnt_users;
    status_icon->set_tooltip(cnt_str.str());
    if(cnt_users > 0 && cnt_users != non_ignored_user_cnt)
    {
        non_ignored_user_cnt = cnt_users;
        Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create_from_png(iconpath + "red-light.png");
        Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
        Cairo::RefPtr<Cairo::ToyFontFace> font =
            Cairo::ToyFontFace::create("FreeSans",
                                       Cairo::FONT_SLANT_NORMAL,
                                       Cairo::FONT_WEIGHT_BOLD);

        int w = surface->get_width();
        int h = surface->get_height();
        cnt_str.str("");
        cnt_str << cnt_users;
        cr->set_font_face(font);
        cr->set_font_size(1.3 * h / 2);

        Cairo::TextExtents te;
        cr->get_text_extents(cnt_str.str(), te);

        cr->set_source_rgba(0, 0, 0, 1.0);
        cr->move_to((double)w/2 - te.x_bearing - te.width/2,
                    (double)h/2 - te.y_bearing - te.height/2);
        cr->show_text(cnt_str.str());

        cr->set_source_rgb(1.0, 1.0, 0);
//        cr->rel_move_to(-1.0, -1.0); // cairomm bug?
        cr->move_to((double)w/2 - te.x_bearing - te.width/2 - 1,
                    (double)h/2 - te.y_bearing - te.height/2 - 1);
        cr->show_text(cnt_str.str());

        Glib::RefPtr<Gdk::Pixbuf> icon = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, w, h);
        int stride = surface->get_stride();
        unsigned char *src = surface->get_data();
        unsigned char *dst = icon->get_pixels();
        for(int i = 0; i < h; ++i)
        {
            for(int j = 0; j < w; ++j)
            {
                if(src[3])
                {
                    // non-zero alpha
                    dst[0] = src[2] * 255 / src[3];
                    dst[1] = src[1] * 255 / src[3];
                    dst[2] = src[0] * 255 / src[3];
                }
                else
                    dst[0] = dst[1] = dst[2] = 0;
                dst[3] = src[3];
                src += 4;
                dst += 4;
            }
            src += stride - w * 4;
            dst += stride - w * 4;
        }
        status_icon->set(icon);
    }
    else if(cnt_users <= 0)
    {
        non_ignored_user_cnt = cnt_users;
        status_icon->set(pixbuf[0]);
    }
}

void UnWidget::dump_lists() const
{
    DBG(
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
        );
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
        cerr << err.what() << endl;
    }
}

void UnWidget::check_entry(struct utmp& utmp_entry)
{
    UtEntry ut_entry(utmp_entry);
    pair<UsersMap::iterator, bool> ret;
    string lname = ut_entry.get_line();
    string uname;
    if(ut_entry.is_ignored_user())
    {
        DBG(cout << "ignored user action: " << ut_entry.to_string() << endl);
        return;
    }
    
    if (ut_entry.is_login_process() || ut_entry.is_user_process())
    {
        uname = ut_entry.get_user();
        ret = users_map.insert(User(uname, UserEntries()));
        pair<UserEntries::iterator, bool> sret = ret.first->second.insert(ut_entry);    // it means: ret.iterator->UserEntries...
        DBG( cout << (ret.second == true ? "new user '" : "append user '") << uname << "': " 
            << (sret.second ? ut_entry.to_string() : lname + " already in set") << endl );
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
                DBG(cout << "deleting: " << entry.to_string() << endl);
                notify(false, ut_entry);
                tree_view.add_log_line(ut_entry);
                uset.erase(it);
                line_entries.erase(line_entries.find(lname));
                return;
            }
        }        
    }
    DBG(cout << "not processed: " << ut_entry.to_string() << endl);
}

void UnWidget::init_logins()
{
    // check currently logged in users
    ifstream utmp(utmp_fname.c_str(), ios::in | ios::binary);
    if (!utmp.is_open())
    {
        cerr << "could not open " << utmp_fname << endl;
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

