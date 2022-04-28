#ifndef BREATH_PLOT_H 
#define BREATH_PLOT_H 

#include "gtkmm-plplot.h"
#include <gtkmm/application.h>
#include <glibmm/miscutils.h>
#include <glib.h>
#include <gtkmm/window.h>
#include <gtkmm/switch.h>
#include <gtkmm/label.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/grid.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/printsettings.h>
#include <gtkmm/pagesetup.h>
#include <gtkmm/printoperation.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>

#include <string>
#include <memory>
#include <stdlib.h>     
#include <time.h>

#include "../lib/data_processing.h"
#include "../lib/input_handler.h"


class BreathPlotWindow: public Gtk::Window {
    
    public:
        BreathPlotWindow(Glib::ustring x_title = "Time", Glib::ustring y_title = "Y-axis",
        Glib::ustring plot_title = "", Gdk::RGBA color = Gdk::RGBA("red"));
        virtual ~BreathPlotWindow() {}
        constexpr static const char* FilterTypes[] = {"*.", "*.txt", ".asc"};
        constexpr static unsigned WindowWidth = 800;
        constexpr static unsigned WindowHeight = 600;

    protected:
        void on_plot_btn_clicked(); 
        void on_export_btn_clicked();
        void on_open_btn_clicked();
        void remove_plot_data();
        void clear_design_grid();
        void plot_it();
        void mark_breaths();

    private:
        std::unique_ptr<InputHandler> input_handler;
        std::unique_ptr<Gtk::MessageDialog> err_dialog;
        DataTable *data;
        std::string filename = "";

        std::vector<Gtk::PLplot::PlotData *> plot_data;
        std::vector<Gtk::PLplot::PlotData2D *> plots;
        Gtk::PLplot::Plot2D plot;
        Gtk::PLplot::Canvas canvas;
        Gtk::Grid grid;
        Gtk::Grid *button_grid;
        Gtk::Grid *design_grid;

        std::vector<Gtk::Label> label;
        std::vector<Gtk::ColorButton> color_combo;
        std::vector<Gtk::ComboBoxText> linestyle_combo;
        std::vector<Gtk::Switch> plot_switch;
        std::vector<Gtk::SpinButton> linewidth_spin;
        std::vector<Glib::RefPtr<Gtk::Adjustment> > linewidth_adj;
        Gtk::Button open_btn;
        Gtk::Button plot_btn;
        Gtk::Button export_btn;

        bool is_file_open() { return filename.length() != 0; }
        void show_error(const std::string& s);
        
        // Methods for displaying widgets
        void render_window();
        void resize();
        void display_label(unsigned i, const std::string& label_title);
        void display_pallete(unsigned i);
        void display_line_style_select(unsigned i);
        void display_width_spin(unsigned i);
        void display_switch(unsigned i);  
        void display_canvas();
        void display_buttons();
        void grid_layout(unsigned par_cnt);
        void grid_default_layout();
};

#endif 
