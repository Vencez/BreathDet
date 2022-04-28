/*
 * Author: Václav Zvoníček
 * Project: BreathDet - simple software project for spotting breaths
 * Released: 2022
*/

#ifndef BREATH_PLOT_WINDOW
#define BREATH_PLOT_WINDOW

#include "../lib/base.h"
#include <memory>

class BreathPlotApp 
{
    private:
        std::unique_ptr<BreathPlotWindow> main_win;
    public:
        BreathPlotApp();
        ~BreathPlotApp() = default;
        BreathPlotWindow* instatiate_window() { return main_win.get(); }
};


#endif
