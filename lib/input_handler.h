#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "data_processing.h"

#ifndef _INPUT_HANDLER_
#define _INPUT_HANDLER_

class input_exc : public std::exception
{
    public:
        input_exc(const std::string& s, size_t idx) : idx_(idx) 
        { 
            s_ = "\nError at row " + std::to_string(idx_) + ": " + s + "!\n\n";
        }
        virtual const char *what() const throw() override { return s_.c_str(); }
    private:
        std::string s_;
        size_t idx_;
};

enum ReadStatus { init, normal, space, err };

class InputHandler
{
    /* 
        Class serving as an input interface for loading 
        data from a well formatted file as well as names of columns
        given in the header of file.
    */
    public:
        InputHandler() : line_idx_(0){};
        void load(std::istream& is);    
        DataTable* get_data() { return &dt; }
    private:
        unsigned readHeader(std::istream& is);
        void readData(std::istream& is, unsigned col);
        ReadStatus state = init;
        DataTable dt;
        unsigned line_idx_;
        static constexpr char sep_ = '%';
};

#endif
