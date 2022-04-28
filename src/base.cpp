#include "../lib/base.h"
#include <fstream>
#include <iostream>

BreathPlotWindow::BreathPlotWindow(
    Glib::ustring x_title, Glib::ustring y_title,
    Glib::ustring plot_title, Gdk::RGBA color) :
    plot(x_title, y_title, plot_title),
    canvas(plot),
    open_btn("Open file"),
    plot_btn("Spot breaths"),
    export_btn("Export...")
{
    set_title("Breath-Plot version 1.0");
    set_default_size(WindowWidth, WindowHeight);
    set_border_width(10);
    resize();

    display_canvas();
    display_buttons();

    grid_default_layout();
    grid.show_all();
}


void BreathPlotWindow::show_error(const std::string& err_msg)
{
    err_dialog.reset(new Gtk::MessageDialog(*this, "Error Message", false, Gtk::MESSAGE_ERROR));
    err_dialog->set_secondary_text(err_msg);
    err_dialog->set_decorated(true);
    err_dialog->signal_response().connect(
    sigc::hide(sigc::mem_fun(*err_dialog, &Gtk::Widget::hide)));
    err_dialog->show();
    
    // Act as if nothing opened
    plot_btn.set_sensitive(false);
    filename = "";
}

void BreathPlotWindow::render_window()
{
    plot_it();
    plot.hide_legend();

    // Block of widgets for customizing plot design
    unsigned par_count = data->parameters_count();
    for (unsigned i = 0; i < par_count - 1; i++)
    {
        // Skip time data
        display_label(i, data->col_names()[i + 1]);  
        display_pallete(i);
        display_line_style_select(i);
        display_width_spin(i);
        display_switch(i);
    }
    plot_btn.set_label("Spot breaths");
    plot_btn.set_sensitive(true);   
    export_btn.set_sensitive(true);   

    grid_layout(par_count - 1);
    grid.show_all();
}


void BreathPlotWindow::resize()
{
    Gdk::Geometry geometry;
    geometry.min_aspect = geometry.max_aspect = double(WindowWidth)/double(WindowHeight);
    set_geometry_hints(*this, geometry, Gdk::HINT_ASPECT);
}


void BreathPlotWindow::on_plot_btn_clicked()
{
    unsigned plot_count = data->parameters_count(); 
    if (plots.size() < plot_count && data->config_good())
    {
        // User hasn't clicked the button yet
        mark_breaths();
        export_btn.set_sensitive(true);
        plot_btn.set_label("Clean spots");
    }
    else
    {
        // User wishes to remove spots of breaths
        plots.pop_back();
        plot.remove_data(data->parameters_count() - 1);
        plot_btn.set_label("Spot breaths");
        export_btn.set_sensitive(false);
    }
}


void BreathPlotWindow::on_export_btn_clicked()
{
    auto dialog = std::make_unique<Gtk::FileChooserDialog>(*this, "Open file", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog->set_do_overwrite_confirmation(true);
    dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog->add_button("_Save", Gtk::RESPONSE_OK);

    if (dialog->run() == Gtk::RESPONSE_OK)
    {
        data->export_data(dialog->get_file()->get_path());
    }
}


void BreathPlotWindow::on_open_btn_clicked()
{

    auto dialog = std::make_unique<Gtk::FileChooserDialog>(*this, "Open file", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog->set_do_overwrite_confirmation(true);
    dialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog->add_button("_Open", Gtk::RESPONSE_OK);

    if (dialog->run() == Gtk::RESPONSE_OK)
    {
        // Clean window for new rendering
        if (is_file_open())
        {
            remove_plot_data();
            clear_design_grid();
        }
    
        filename = dialog->get_filename();
        std::ifstream ifs(filename);
        input_handler = std::make_unique<InputHandler>();

        try
        {
            input_handler->load(ifs);
        }
        catch (input_exc &e)
        {
            show_error(e.what());
            return;
        }
        data = input_handler->get_data(); 
        ifs.close();
        render_window();
    }

}


void BreathPlotWindow::display_label(unsigned i, const std::string& label_title)
{
    label.push_back(Gtk::Label(label_title));
    label[i].set_hexpand(true);
    label[i].set_halign(Gtk::ALIGN_END);
}


void BreathPlotWindow::display_pallete(unsigned i)
{
    color_combo.push_back(Gtk::ColorButton());
    color_combo[i].set_hexpand(false);
    color_combo[i].set_rgba(plots[i]->get_color());
    color_combo[i].set_use_alpha(true);
    color_combo[i].signal_color_set().connect([this, i](){plots[i]->set_color(color_combo[i].get_rgba());});
}


void BreathPlotWindow::display_line_style_select(unsigned i)
{
    linestyle_combo.push_back(Gtk::ComboBoxText());
    linestyle_combo[i].append("continuous");
    linestyle_combo[i].append("long dash short gap");
    linestyle_combo[i].append("long dash short gap short dash short gap");
    linestyle_combo[i].set_hexpand(false);
    linestyle_combo[i].set_active(plots[i]->get_line_style()-1);
    linestyle_combo[i].signal_changed().connect([this, i](){plots[i]->set_line_style(static_cast<Gtk::PLplot::LineStyle>(linestyle_combo[i].get_active_row_number()+1));});
}


void BreathPlotWindow::display_width_spin(unsigned i)
{
    linewidth_adj.push_back(Gtk::Adjustment::create(1.0, 0.1, 10.0, 0.1, 1.0, 0.0));
    linewidth_spin.push_back(Gtk::SpinButton(linewidth_adj[i], 0.1, 1.0));
    linewidth_spin[i].set_wrap(true);
    linewidth_spin[i].set_snap_to_ticks(true);
    linewidth_spin[i].set_numeric(true);
    linewidth_spin[i].set_halign(Gtk::ALIGN_START);
    linewidth_spin[i].set_hexpand(true);
    linewidth_spin[i].set_value(plots[i]->get_line_width());
    linewidth_spin[i].signal_value_changed().connect([this, i](){plots[i]->set_line_width(linewidth_spin[i].get_value());});
}


void BreathPlotWindow::display_switch(unsigned i)
{
    plot_switch.push_back(Gtk::Switch());
    plot_switch[i].set_active(plots[i]->is_showing());
    plot_switch[i].set_hexpand(false);
    plot_switch[i].property_active().signal_changed().connect([this, i](){
        if (plot_switch[i].get_active())
        {
            canvas.get_plot(0)->get_data(i)->show();
            color_combo[i].set_sensitive(true);
            linestyle_combo[i].set_sensitive(true);
            linewidth_spin[i].set_sensitive(true);
        }
        else 
        {
            plot.get_data(i)->hide();
            color_combo[i].set_sensitive(false);
            linestyle_combo[i].set_sensitive(false);
            linewidth_spin[i].set_sensitive(false);
        }
    });
}


void BreathPlotWindow::grid_layout(unsigned par_cnt)
{
    
    // Lay out individual widgets
    for (unsigned i = 0;  i < par_cnt; ++i)
    {
        design_grid->attach(label[i], 0, i);
        design_grid->attach(plot_switch[i], 1, i);
        design_grid->attach(color_combo[i], 2, i);
        design_grid->attach(linestyle_combo[i], 3, i);
        design_grid->attach(linewidth_spin[i], 4, i);
    }
}


void BreathPlotWindow::clear_design_grid()
{
    if (filename.length() == 0) return; // No file has been cpen

    unsigned par_cnt = data->parameters_count() - 1;
    for (unsigned i = 0;  i < par_cnt; ++i)
    {
        design_grid->remove(label[i]);
        design_grid->remove(plot_switch[i]);
        design_grid->remove(color_combo[i]);
        design_grid->remove(linestyle_combo[i]);
        design_grid->remove(linewidth_spin[i]);
    }
}


void BreathPlotWindow::grid_default_layout()
{
    // Setting basic properties of grid layour
    grid.set_column_homogeneous(false);
    grid.set_column_spacing(5);
    grid.set_row_spacing(5);

    grid.attach(*design_grid, 0, 0, 1, 1);
    grid.attach(canvas, 0, 1, 5, 2);
    grid.attach(*button_grid, 0, 3, 5, 1);

    add(grid);
    grid.show_all();
}


void BreathPlotWindow::display_canvas()
{
    canvas.set_hexpand(true);
    canvas.set_vexpand(true);
}


void BreathPlotWindow::display_buttons()
{
    button_grid = Gtk::manage(new Gtk::Grid());
    button_grid->set_column_homogeneous(true);
    button_grid->set_column_spacing(5);
    button_grid->set_row_spacing(5);
    open_btn.set_hexpand(false);
    open_btn.set_vexpand(false);
    plot_btn.set_hexpand(false);
    plot_btn.set_vexpand(false);
    plot_btn.set_sensitive(false); // Export only after breath spotted
    export_btn.set_hexpand(false);
    export_btn.set_vexpand(false);
    export_btn.set_sensitive(false); // Export only after breath spotted

    plot_btn.signal_clicked().connect(sigc::mem_fun(*this, &BreathPlotWindow::on_plot_btn_clicked));
    export_btn.signal_clicked().connect(sigc::mem_fun(*this, &BreathPlotWindow::on_export_btn_clicked));
    open_btn.signal_clicked().connect(sigc::mem_fun(*this, &BreathPlotWindow::on_open_btn_clicked));

    button_grid->attach(open_btn, 0, 0, 1, 1);
    button_grid->attach(plot_btn, 1, 0, 1, 1);
    button_grid->attach(export_btn, 2, 0, 1, 1);

    design_grid = Gtk::manage(new Gtk::Grid());
    design_grid->set_column_spacing(5);
    design_grid->set_row_spacing(5);
    
}


void BreathPlotWindow::plot_it()
{
    /* 
        Displays all plots in a single canvas. Randomly chooses
        colors for each plot 
    */

    srand (time(NULL));
    unsigned par_cnt = data->parameters_count();
    std::vector<Gdk::RGBA> colors(par_cnt);
    for (unsigned i = 0; i < par_cnt; ++i)
    {
        colors[i].set_rgba((float)rand() / RAND_MAX, 
                           (float)rand() / RAND_MAX,
                           (float)rand() / RAND_MAX);
    }
        
    for (unsigned par = 1; par < par_cnt; ++par)
    {
        plot.add_data(*Gtk::manage(new Gtk::PLplot::PlotData2D(data->get_col(0), data->get_col(par), colors[par], Gtk::PLplot::LineStyle::LONG_DASH_LONG_GAP, 5.0)));
        plot_data.emplace_back(plot.get_data(par - 1));
        plots.emplace_back(dynamic_cast<Gtk::PLplot::PlotData2D *>(plot.get_data(par - 1)));
    }

}


void BreathPlotWindow::remove_plot_data()
{
    if (filename.length() == 0) return;
    for (unsigned i = 0; i < plot_data.size(); i++)
    {
        plot.remove_data(*plot_data[i]);
    }
    /*
     In case user clicked to spot breath points, these should been
     removed as well 
    */
    unsigned plot_count = data->parameters_count(); 
    if (plots.size() == plot_count)
    {
        plot.remove_data(0);
    }

    plot_data.clear();
    plots.clear();
}

void BreathPlotWindow::mark_breaths()
{
    try
    {
        const unsigned inspected_plot_idx = 3;
        data->find_breath(0, data->size(), inspected_plot_idx, 4);
    }
    catch (config_exc& e)
    {   
        show_error(e.what());
        return;
    }
    auto bd = data->retrieve_breath_data();

    // In case breaths haven't been spotted, append the spots to plot
    plot.add_data(*Gtk::manage(new Gtk::PLplot::PlotData2D(bd[0], bd[1], Gdk::RGBA("black"), Gtk::PLplot::LineStyle::NONE, 5.0)));
    auto breath_data = dynamic_cast<Gtk::PLplot::PlotData2D *>(plot.get_data(data->parameters_count() - 1));
    breath_data->set_symbol("O");
    plots.push_back(std::move(breath_data));
}
