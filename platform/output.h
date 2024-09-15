/* The MIT License

   Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdio.h>

#define APP_LOG(level, fmt, ...) fprintf(level, "[%s:%d]\t" fmt "\n", __FUNCTION__, __LINE__,  ##__VA_ARGS__)

#define APP_DEBUG(fmt, ...) APP_LOG(stdout, "[D] " fmt, ##__VA_ARGS__)
#define APP_NOTICE(fmt, ...) APP_LOG(stdout, "[I] " fmt, ##__VA_ARGS__)
#define APP_WARNING(fmt, ...) APP_LOG(stderr, "[W] " fmt, ##__VA_ARGS__)
#define APP_FATAL(fmt, ...) APP_LOG(stderr, "[E] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

namespace tblfmt {

class endl{};

// TablePrinter
//
// Print a pretty table into your output of choice.
//
// Usage:
//   TablePrinter tp(&std::cout);
//   tp.AddColumn("Name", 25);
//   tp.AddColumn("Age", 3);
//   tp.AddColumn("Position", 30);
//
//   tp.PrintHeader();
//   tp << "Dat Chu" << 25 << "Research Assistant";
//   tp << "John Doe" << 26 << "Professional Anonymity";
//   tp << "Jane Doe" << tp.SkipToNextLine();
//   tp << "Tom Doe" << 7 << "Student";
//   tp.PrintFooter();

class TablePrinter {
   void PrintHorizontalLine();

  template<typename T> void OutputDecimalNumber(T input);

  std::ostream *out_stream_;
  std::vector<std::string> column_headers_;
  std::vector<int> column_widths_;
  std::string separator_;

  int i_; // index of current row
  int j_; // index of current column

  int table_width_;

public:
  int get_num_columns() const;
  int get_table_width() const;
  void set_separator(const std::string & separator);

  void AddColumn(const std::string & header_name, int column_width);
  void PrintHeader();
  void PrintFooter();

  TablePrinter& operator<<(endl input) {
    while (j_ != 0) {
      *this << "";
    }
    return *this;
  }

  // Can we merge these?
  TablePrinter& operator<<(float input);
  TablePrinter& operator<<(double input);

  template<typename T> TablePrinter& operator<<(T input) {
    if (j_ == 0) {
      *out_stream_ << "|";
    }

    // Leave 3 extra space: One for negative sign, one for zero, one for decimal
    *out_stream_ << std::setw(column_widths_.at(j_)) << input;

    if (j_ == get_num_columns()-1) {
      *out_stream_ << "|\n";
      i_ = i_ + 1;
      j_ = 0;
    } else {
      *out_stream_ << separator_;
      j_ = j_ + 1;
    }

    return *this;
  }

  TablePrinter(std::ostream * output, const std::string &separator = "|");
  ~TablePrinter();
};

template<typename T> void TablePrinter::OutputDecimalNumber(T input) {
  // If we cannot handle this number, indicate so
  if (input < 10*(column_widths_.at(j_)-1) || input > 10*column_widths_.at(j_)) {
    std::stringstream string_out;
    string_out << std::setiosflags(std::ios::fixed)
               << std::setprecision(column_widths_.at(j_))
               << std::setw(column_widths_.at(j_))
               << input;

    std::string string_rep_of_number = string_out.str();

    string_rep_of_number[column_widths_.at(j_)-1] = '*';
    std::string string_to_print = string_rep_of_number.substr(0, column_widths_.at(j_));
    *out_stream_ << string_to_print;

  } else {

    // determine what precision we need
    int precision = column_widths_.at(j_) - 1; // leave room for the decimal point
    if (input < 0) {
      --precision; // leave room for the minus sign
    }

    // leave room for digits before the decimal?
    if (input < -1 || input > 1){
      int num_digits_before_decimal = 1 + (int)log10(std::abs(input));
      precision -= num_digits_before_decimal;
    } else {
      precision --; // e.g. 0.12345 or -0.1234
    }

    if (precision < 0) {
      precision = 0; // don't go negative with precision
    }

    *out_stream_ << std::setiosflags(std::ios::fixed)
                 << std::setprecision(precision)
                 << std::setw(column_widths_.at(j_))
                 << input;
  }

  if (j_ == get_num_columns()-1) {
    *out_stream_ << "|\n";
    i_ = i_ + 1;
    j_ = 0;
  } else {
    *out_stream_ << separator_;
    j_ = j_ + 1;
  }
}

} // namspace tblfmt

#endif // __cplusplus

#endif // OUTPUT_H
