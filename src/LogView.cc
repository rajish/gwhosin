#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <iomanip>
#include "LogView.h"
#include "Dbg.h"

using namespace std;
using Glib::ustring;


LogView::LogView() :
    hide_ignored(true)
{
    // First things first
    ignored_users.insert("LOGIN");

    tree_model = Gtk::ListStore::create(columns);
    tree_model_filter = Gtk::TreeModelFilter::create(tree_model, Gtk::TreeModel::Path());
    tree_model_filter->set_modify_func( columns_display, sigc::mem_fun(*this, &LogView::on_filter_modify) );
    
    set_model(tree_model_filter);

    append_column("Time", columns_display.time_text);
    append_column("In/Out", columns_display.type_text);
    append_column("Login", columns_display.login_text);
    append_column("Host", columns_display.host_text);
    append_column("Line", columns_display.tty_text);
    append_column("IP", columns_display.ip_text);
    append_column_numeric("PID", columns_display.pid_text, "%d");

    for(guint i = 0; i < 7; i++)
    {
        Gtk::TreeView::Column* pColumn = get_column(i);
        if(pColumn)
        {
            pColumn->set_reorderable();
            pColumn->set_sort_column(i);
        }
    }

    signal_button_press_event().connect(sigc::mem_fun(*this, &LogView::on_button_press_event), false);

}

bool LogView::on_button_press_event(GdkEventButton* event)
{
    TRACEFN;
    bool return_value = false;

#ifdef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
    //Call base class, to allow normal handling,
    //such as allowing the row to be selected by the right-click:
    return_value = Gtk::TreeView::on_button_press_event(event);
#endif
    return return_value;
}

template <class T> void cell2val(const T& cell, Glib::ValueBase& val)
{
    Glib::Value< T > valString;
    valString.init( Glib::Value< T >::value_type() );

    valString.set(cell);
    val = valString;
}

void LogView::on_filter_modify(const Gtk::TreeModel::iterator& iter, Glib::ValueBase& value, int column)
{
    //iter is an iterator to the row in the filter model.
    //column is the column number in the filter model.
    //value should be set to the value of that column in this row, to be
    //displayed.

    //Look in the child model, to calculate the model to show in the filter model:
    Gtk::TreeModel::iterator iter_child = tree_model_filter->convert_iter_to_child_iter(iter);
    Gtk::TreeModel::Row row_child = *iter_child;

    switch(column)
    {
    case 0: // timestamp
        cell2val<Glib::ustring>(row_child[columns.timestamp], value);
        break;
    case 1: // type
        switch(row_child[columns.type])
        {
        case LOGIN_PROCESS:
        case USER_PROCESS:
            cell2val<Glib::ustring>("IN", value);
            break;
        case DEAD_PROCESS:
            cell2val<Glib::ustring>("OUT", value);
            break;
        default:
            cell2val<Glib::ustring>("OTHER", value);
            break;
        }
        break;
    case 2: // Login name
        cell2val<Glib::ustring>(row_child[columns.login], value);
        break;
    case 3: // host
        cell2val<Glib::ustring>(row_child[columns.host], value);
        break;
    case 4: // line
        cell2val<Glib::ustring>(row_child[columns.tty], value);
        break;
    case 5: // IP
        cell2val<Glib::ustring>(row_child[columns.IP], value);
        break;
    case 6: // PID
        cell2val<int>(row_child[columns.PID], value);
        break;
    default:
        break;
    }
}

void LogView::ignore_user(const std::string& login)
{
    ignored_users.insert(login); 
}

bool LogView::is_ignored_user(const std::string login) const
{
    return (ignored_users.find(login) != ignored_users.end());
}

void LogView::add_log_line(const UtEntry& entry)
{
    if(!hide_ignored || !is_ignored_user(entry.get_user()))
    {       
        Gtk::TreeModel::Row row = *(tree_model->append());
        row[columns.login]     = entry.get_user();
        row[columns.timestamp] = entry.get_time_str(false);
        row[columns.tty]       = entry.get_line();
        row[columns.host]      = entry.get_IP_str();
        row[columns.IP]        = entry.get_IP_str();
        row[columns.PID]       = entry.get_pid();
        row[columns.type]      = entry.get_type();
    }
}

