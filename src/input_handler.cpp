#include "../lib/input_handler.h"

void InputHandler::load(std::istream& is)
{
    /* Loads data from a file into DataTable object. */

    unsigned col_cnt = readHeader(is);
    dt.set_col(col_cnt);
    readData(is, col_cnt); 
}


unsigned InputHandler::readHeader(std::istream& is)
{
    /* 
        Reads header, column titles and counts them.
        To avoid various symbols in header, i.e. notes
        of the user, reading is implemented characater
        by characater.
    */

    unsigned col_cnt = 0;
    std::string col_name = "";
    // Ignore first row - only names useful for user 
    char c = is.get();
    while (c != '\n' && !is.fail()) c = is.get();
    c = '\0';

    /* 
        Robust implementation for reading headers 
        gives user free will to write any notes 
    */

    while (c != '\n' && !is.fail())
    {
        if (is.fail()) break;
        c = is.get();
        switch (c)
        {
            case sep_:
            case '\n':
                ++line_idx_;
                col_name = "";
                state = init;
                break;
            default:
                if (isspace(c))
                {
                    switch(state)
                    {
                        case init:
                            state = normal;
                            break;
                        case normal:
                            dt.add_col(col_name);
                            col_name = "";
                            ++col_cnt;
                            state = space;
                            break;
                        default:
                            break;
                    }
                }
                else if (!isspace(c) && state != init) 
                {
                    col_name += c;
                    state = normal;
                }
                break;
        }
    }
    return col_cnt;
}


void InputHandler::readData(std::istream& is, unsigned col)
{
    std::string s;
    std::vector<double> temp_row(col);
    
    for(;;)
    {
        unsigned i = 0;
        state = normal;

        /* 
            Necessary to read line twice for checking correct number of columns 
        */
        std::getline(is, s);
        std::stringstream ss(s); 

        /* 
          If an exception occured, setting state to err skips the
          corrupted row without terminating the reading process.
        */

        std::string error_message = "";

        std::string num;
        while(ss >> num)
        {
            if (i >= col) 
            {
                error_message = "Too many columns";
                state = err;
            }
            else
            {
                // Test if num is really a double value
                char* end = nullptr;
                double val = std::strtod(num.c_str(), &end);

                if (end == num.c_str() || *end != '\0')
                {
                    // Argument is not of double type
                    error_message = "Wrong format of entry";
                    state = err;
                }
                else
                {
                    temp_row[++i - 1] = val;
                }
            }
        }

        if (state != err && i < col && s.length() != 0)
        {
            error_message = "Too few columns";
            state = err; 
        }
        if (state != err)
        {
            if (is.fail()) return;
            dt.add_row(temp_row);
        }
        else throw input_exc{error_message, line_idx_};
        ++line_idx_;
    }
}
