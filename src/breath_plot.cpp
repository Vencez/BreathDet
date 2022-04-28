#include "../lib/breath_plot.h"
#include <fstream>
#include <iostream>

BreathPlotApp::BreathPlotApp()
{
    main_win = std::make_unique<BreathPlotWindow>(Glib::ustring("Time"), Glib::ustring("Values"));
}

