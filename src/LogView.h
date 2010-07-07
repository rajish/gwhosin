// -*- C++ -*-

#ifndef _LOGVIEW_H_
#define _LOGVIEW_H_

#include <gtkmm.h>
#include "UtEntry.h"

class LogView : public Gtk::TreeView
{
public:
    LogView();
    void set_hide_ignored(bool hide = true) { hide_ignored = hide; }
    bool get_hide_ignored() const { return hide_ignored; }
    void add_log_line(const UtEntry& entry);

protected:

    virtual bool on_button_press_event(GdkEventButton *ev);

    void on_filter_modify(const Gtk::TreeModel::iterator& iter, Glib::ValueBase& value, int column);

    //These are the types of the columns in the child model:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
            { 
                add(timestamp); 
                add(type);
                add(login);
                add(host); 
                add(tty); 
                add(IP); 
                add(PID); 
            }

        Gtk::TreeModelColumn<Glib::ustring> login;
        Gtk::TreeModelColumn<Glib::ustring> timestamp;
        Gtk::TreeModelColumn<Glib::ustring> host;
        Gtk::TreeModelColumn<Glib::ustring> tty;
        Gtk::TreeModelColumn<Glib::ustring> IP;
        Gtk::TreeModelColumn<pid_t> PID;
        Gtk::TreeModelColumn<short int> type;
    };

    ModelColumns columns;

    //These are the types of the model that will be shown.
    //The values will be generated dynamically, based on values in the child model.
    //
    //Remember, if you just want to hide some columns, then you should just not add them to the view,
    //and if you just want to change how some columns are displayed then use set_cell_data_func().
    class ModelColumnsDisplay : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumnsDisplay()
            { add(time_text); add(type_text); add(login_text); add(host_text); add(tty_text); add(ip_text); add(pid_text); }

        Gtk::TreeModelColumn<Glib::ustring> time_text;
        Gtk::TreeModelColumn<Glib::ustring> type_text;
        Gtk::TreeModelColumn<Glib::ustring> login_text;
        Gtk::TreeModelColumn<Glib::ustring> host_text;
        Gtk::TreeModelColumn<Glib::ustring> tty_text;
        Gtk::TreeModelColumn<Glib::ustring> ip_text;
        Gtk::TreeModelColumn<pid_t> pid_text;
    };

    ModelColumnsDisplay columns_display;

    Glib::RefPtr<Gtk::ListStore> tree_model;
    Glib::RefPtr<Gtk::TreeModelFilter> tree_model_filter;

    bool hide_ignored;
};


#endif // _LOGVIEW_H_
