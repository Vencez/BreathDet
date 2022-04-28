#include "../lib/data_processing.h"
#include <fstream>

DataTable::DataTable() noexcept
{
    data_state = ok;
    breath_points.resize(FCT::_count);
}


DataTable::DataTable(const RowType& v) noexcept
{ 
    data.resize(col_cnt_);
}


void DataTable::add_row(const RowType& v)
{
    for (unsigned i = 0; i < col_cnt_; ++i)
    {
        data[i].push_back(v[i]); 
    }
}


double DataTable::param_val(unsigned param_idx, unsigned long idx)
{
    if (param_idx >= col_cnt_ || idx >= data[0].size())
    {
        // Error value
        return -1;
    }
    return data[param_idx][idx];
}


void DataTable::set_col(unsigned col_cnt)
{
    col_cnt_ = col_cnt;
    data.resize(col_cnt_);
}


void DataTable::add_col(const std::string& c) { col_names_.push_back(c); }

const char* BreathData::breath_state_name(PlotState pst)
{
    switch(pst)
    {
        case idle:
            /*
                This case takes place only after the exhalation
                has stopped => corresponds to end of breath
            */
            return "( Breath End )";
            break;
        case BreathStart:
            return "( Inhalation )";
            break;
        case BreathEnd:
            return "( Exhalation )";
            break;
        default:
            return "( Nothing spotted )";
            break;
    }
}


void DataTable::find_breath(unsigned x_start, unsigned x_end, unsigned plot_id, unsigned k)
{
    /* 
        Finds a such that y = ax is fitted k points in a plot with parameter index plot_id. Uses Least Squares Method.
    */

    /* Trial-error guessed values for sufficient breaths detection */
    data_state = ok;
    std::ifstream config_read;

    try
    {
        config_read.open("config.txt");
    }
    catch (std::exception& e)
    {
        throw config_exc{"file cannot be opened."};
        data_state = config_open_err;
        return;
    }

    BreathLimits bl_cm;
    size_t config_line = 0;

    try
    {
        std::string down_val, up_val;

        ++config_line;
        config_read >> down_val >> up_val;
        bl_cm.BREATH_UP_LIMIT[0] = std::stod(down_val);
        bl_cm.BREATH_UP_LIMIT[1] = std::stod(up_val);

        ++config_line;
        config_read >> down_val >> up_val;
        bl_cm.PEAK_LIMIT[0] = std::stod(down_val);
        bl_cm.PEAK_LIMIT[1] = std::stod(up_val);

        ++config_line;
        config_read >> down_val >> up_val;
        bl_cm.BREATH_DOWN_LIMIT[0] = std::stod(down_val);
        bl_cm.BREATH_DOWN_LIMIT[1] = std::stod(up_val);

        config_read.close();
    }
    catch (std::exception& e)
    {
        throw config_exc{config_line};
        data_state = config_format_err;
        return;
    }

    BreathData bd_cm{k, idle, plot_id, bl_cm};
    bd_cm.precompute_tang(data, x_start);

    /* Main loop of couting beginning from kth point */
    unsigned i = x_start + k - 1;

    while ((++i - k) < x_end)
    {
        bd_cm.move_to(i);
        bd_cm.compute_tang(data, i);

        PlotState curr_pst = bd_cm.plot_state();
        switch (curr_pst)
        {
            case idle:
                bd_cm.check_breath_up(data);
                break;
            case BreathEnd:
                bd_cm.check_breath_down(data);
                break;
            case BreathStart:
                bd_cm.check_breath_peak(data);
                break;
        }
        if (curr_pst != bd_cm.plot_state()) 
        { 
            breath_points[FCT::breath_time].push_back(data[0][i - k + 1]);
            breath_points[FCT::breath_val].push_back(data[plot_id][i - k + 1]);
            breath_points[FCT::breath_type].push_back(bd_cm.plot_state());
        }
    }
}


void DataTable::export_data(const std::string& file_name)
{
    /* 
        Exports data as a table consisting of: 
        * time of detection,
        * value of detection,
        * type of detection
        into a text file named $file_name
    */ 

    std::ofstream ofs(file_name); 
    if (ofs.is_open() && breath_points.size() > 0)
    {
        for (unsigned i = 0; i < breath_points[0].size(); ++i)
        {
            // First write only numeric values into the file
            for (unsigned j = 0; j < breath_points.size() - 1; ++j)
            {
                ofs << breath_points[j][i] << ' ';
            }
            // Now append the type of breath
            ofs << BreathData::breath_state_name(static_cast<PlotState>(breath_points[FCT::breath_type][i])) << '\n';
        }
        ofs.close();
    }
    else return;
}

/* BreathData class implementation */

void BreathData::compute_tang(const Table& data, unsigned i)
{
    // Evaluates tangent parameters for new kth-tuple of points
    prev_a_ = tc_.a;

    tc_.divid = pts_count_ * tc_.x_sq_sum - tc_.x_sum * tc_.x_sum;
    tc_.a = pts_count_ * tc_.xy_sum  - tc_.x_sum * tc_.y_sum;
    tc_.b = tc_.y_sum * tc_.x_sq_sum - tc_.x_sum * tc_.xy_sum;
    tc_.a /= tc_.divid; tc_.b /= tc_.divid;

    // Update data
    tc_.x_sum += (data[0][i] - data[0][i - pts_count_]);
    tc_.y_sum += (data[plot_idx_][i] - data[plot_idx_][i - pts_count_]);
    tc_.x_sq_sum += (data[0][i] * data[0][i] - data[0][i - pts_count_] * data[0][i - pts_count_]);
    tc_.xy_sum += (data[0][i] * data[plot_idx_][i] - data[0][i - pts_count_] * data[plot_idx_][i - pts_count_]);
}


void BreathData::precompute_tang(const Table& data, unsigned x_start)
{
    tc_ = { 0, 0, 1, 0, 0, 0, 0 }; 
    for (unsigned i = x_start; i < x_start + pts_count_; ++i)
    {
        tc_.x_sum += data[0][i];
        tc_.y_sum += data[plot_idx_][i];
        tc_.x_sq_sum += data[0][i] * data[0][i];
        tc_.xy_sum += data[0][i] * data[plot_idx_][i];
    }
    prev_a_ = tc_.a;
}


using BL = BreathLimits;
void BreathData::check_breath_up(Table& dt)
{
    if (tc_.a > BREATH_LIM.BREATH_UP_LIMIT[BL::MIN_VAL])
    {
        pst_ = BreathStart;
    }
}


void BreathData::check_breath_down(Table& dt)
{
    if (tc_.a > BREATH_LIM.BREATH_DOWN_LIMIT[BL::MIN_VAL] && tc_.a < BREATH_LIM.BREATH_DOWN_LIMIT[BL::MAX_VAL])
    {
        pst_ = idle;
    }
}


void BreathData::check_breath_peak(Table& dt)
{
    if (ratio() < BREATH_LIM.PEAK_LIMIT[BL::MAX_VAL] && dt[plot_idx_][x_pos_] < dt[plot_idx_][x_pos_ - 1])
    {
        pst_ = BreathEnd;
    }
}
