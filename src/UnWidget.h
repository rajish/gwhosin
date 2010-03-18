// -*- C++ -*-

#ifndef _UNWIDGET_H_
#define _UNWIDGET_H_

#include <gtkmm.h>
#include <libnotifymm.h>
#include <giomm/fileinputstream.h>

#include <fstream>
#include <map>
#include <list>
#include <set>

#include "UtEntry.h"
#include "LogView.h"



class UnWidget: public Gtk::Window
{
public:
    UnWidget();

protected:
    void on_close(const Glib::ustring& action);
    void on_wtmp_changed(const Glib::RefPtr<Gio::File>& file,
                              const Glib::RefPtr<Gio::File>& other_file,
                              Gio::FileMonitorEvent event);
    void on_popup_menu(guint button, guint32 activate_time);
    void on_icon_activate();
    void on_menu_exit();
    void on_filter_modify(const Gtk::TreeModel::iterator& iter, Glib::ValueBase& value, int column);
    bool on_tree_button_press_event(GdkEventButton* event);

    void update_icon();
    void dump_lists() const;
    void notify(bool log_in, const UtEntry& entry);
    void add_log_line(const UtEntry& entry);
    void check_entry(struct utmp& utmp_entry);
    void init_logins();

    /** Keeps pairs of 'line'->'user name' associations */
    typedef std::map<std::string, std::string> LineEntries;
    typedef std::pair<std::string, std::string> LineEntry;
    LineEntries line_entries;

    /** Keeps pairs of 'user name'->'utmp entry' associations */
    typedef std::set<UtEntry> UserEntries;
    typedef std::map<std::string, UserEntries> UsersMap;
    typedef std::pair<std::string, UserEntries> User;
    UsersMap users_map;

    
    std::ifstream wtmp_in;
    Glib::RefPtr<Gio::File> wtmp;
    Glib::RefPtr<Gio::FileMonitor> monitor;

    Notify::Notification notification;
    Glib::RefPtr<Gtk::StatusIcon> status_icon;

    // menu
    Glib::RefPtr<Gtk::UIManager> ui_manager;
    Glib::RefPtr<Gtk::ActionGroup> action_group;
    Gtk::Menu* popup_menu;

    Glib::RefPtr< Gdk::Pixbuf > pixbuf[2];

    // tree view


    Gtk::VBox vbox;

    Gtk::ScrolledWindow scrolled_window;
    LogView tree_view;
};


#endif  // _UNWIDGET_H_
