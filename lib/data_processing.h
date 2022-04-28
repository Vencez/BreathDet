#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <limits>

#ifndef DATA_PROCESSING
#define DATA_PROCESSING

/* 
    After parsing data from a file, these need to be stored in containers. Not all data need to be stored at the same timedue to time cost.

    DataTable:
                 1) covers data processing with data in their true form, not strings,
                 2) makes it possible to further connect stored data with plotting mechanism.
*/

class config_exc: public std::exception
{
    public:
        config_exc(const std::string && s)
        {
            s_ = "Config file error: " + std::move(s);
        }

        config_exc(size_t line) : line_(line) 
        { 
            s_ = "\nConfig file config.txt error: format issue on line: " + std::to_string(line_) + "\n";
        }
        virtual const char *what() const throw() override { return s_.c_str(); }
    private:
        std::string s_;
        size_t line_;
};


using RowType = std::vector<double>;
using Table = std::vector<std::vector<double> >;
enum PlotState { idle, BreathStart, BreathEnd }; 

class DataTable
{
    /* 
        Stores data in a table, which is mantained as a table of columns. The data then in turn be used in methods for detecting breaths points or exporting them into an output file.
    */
    enum DataState { ok, config_open_err, config_format_err };
    public:
        DataTable() noexcept;
        DataTable(const RowType& v) noexcept;

        void add_row(const RowType& v);
        void set_col(unsigned col_cnt);
        void add_col(const std::string& c);
        const RowType& get_col(unsigned i) const { return data[i]; }
        const std::vector<std::string> col_names() const { return col_names_; }
        const Table& retrieve_breath_data() const { return breath_points; }
        bool config_good() const { return (data_state == ok); }

        double param_val(unsigned param_idx, unsigned long idx);
        unsigned size() const { return data[0].size(); }
        unsigned parameters_count() const { return data.size(); }
        void export_data(const std::string& file_name);
        void find_breath(unsigned x_start, unsigned x_end, unsigned plot_id, unsigned k = 4);

    private:
        Table data; // Table represented as a vector of COLUMNS
        std::vector<std::string> col_names_;
        Table breath_points;
        unsigned col_cnt_;
        DataState data_state;
};


/* Breath Data help structures */

enum FileColumnType 
{
   breath_time,
   breath_val,
   breath_type,
   _count
};

using FCT = FileColumnType;

struct BreathLimits
{
    enum { MIN_VAL = 0, MAX_VAL = 1 };
    double BREATH_UP_LIMIT[2];
    double PEAK_LIMIT[2];
    double BREATH_DOWN_LIMIT[2];
};


struct TangentComputation
{
    double a; double b; double divid;
    double x_sum; double y_sum;
    double x_sq_sum;
    double xy_sum;
};


class BreathData
{
    public:
        BreathData(unsigned pts_count, PlotState pst, unsigned plot_idx, BreathLimits bl) :  BREATH_LIM(bl), pts_count_(pts_count), plot_idx_(plot_idx), pst_(idle) 
        {}
        void move_to(unsigned x_pos) { x_pos_ = x_pos; }
        void precompute_tang(const Table& data, unsigned x_start);
        void compute_tang(const Table& data, unsigned i);
        void check_breath_up(Table& dt);
        void check_breath_down(Table& dt);
        void check_breath_peak(Table& dt);

        double ratio()
        {
            if (prev_a_ != 0) return tc_.a / prev_a_;
            else return std::numeric_limits<double>::max();
        }

        static const char* breath_state_name(PlotState pst);
        PlotState plot_state() { return pst_; }
        BreathLimits BREATH_LIM;
    private:
        double prev_a_;
        double pts_count_;
        unsigned x_pos_;
        unsigned plot_idx_;
        PlotState pst_;
        TangentComputation tc_;
};

#endif
