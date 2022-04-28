#include<iostream>
#include "../lib/base.h"
#include "../lib/breath_plot.h"

int main(int argc, char **argv) 
{
    Glib::set_application_name("BreathDet 1.0");
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "");

    BreathPlotApp main_win;
    auto win = main_win.instatiate_window();
    return app->run(*win);
    
}
