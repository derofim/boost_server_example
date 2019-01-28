#include "algo/CSV.hpp" // IWYU pragma: associated
#include "log/Logger.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace boostander {
namespace algo {

// If counts of rows and columns !=0, this fills vector of rows "data";
// each of rows has cols_count empty strings
CSV::CSV(char delim, unsigned int rows_count, unsigned int cols_count) {
  this->delimiter_ = delim;
  this->rowsCount_ = rows_count;
  this->colsCount_ = cols_count;
  row rw(cols_count, "");
  rowsDeque_.resize(rows_count, rw);
}

// This loads .csv file and fills vector of rows "data";
// counts of rows and columns determines from file data.
void CSV::loadFromFile(const std::string& filename) {
  // Load file data
  std::deque<std::string> readed_data;
  loadFile(filename, readed_data);
  // Parse readed data
  parseData(readed_data);
}

void CSV::loadFromMemory(const std::string& data) {
  // Load file data
  std::deque<std::string> readed_data;

  for (unsigned i = 0; i < data.length(); i += READ_BLOCK_SIZE) {
    readed_data.push_back(data.substr(i, READ_BLOCK_SIZE));
  }

  // Parse readed data
  parseData(readed_data);
}

void CSV::parseData(std::deque<std::string>& readed_data) {
  char cur_field[MAX_FIELD_SIZE];
  int cur_field_size = 0;
  int cur_col = 0;
  char c;

  rowsDeque_.push_back(row());
  row* rw = new row(rowsCount_);
  for (auto str : readed_data) {
    int sz = str.size();
    const char* cdata = str.data();
    for (int i = 0; i < sz; ++i) {
      c = cdata[i];
      if (c != delimiter_ && (int)c != 10 && (int)c != 13) {
        cur_field[cur_field_size] = c;
        cur_field_size++;
      } else {
        // new field
        if (cur_field_size > 0) {
          rw->emplace_back(cur_field, cur_field_size);
          cur_field_size = 0;
        }
        // new row
        if (c != delimiter_) {
          if (rw->size() > 0) {
            rowsDeque_.emplace_back(row());
            rowsDeque_[cur_col] = std::move(*rw);
            cur_col++;
            delete rw;
            rw = new row(rowsCount_);
          }
        }
      }
    }
  }
  delete rw;
  rowsDeque_.pop_back();
  rowsCount_ = rowsDeque_.size();
  if (rowsCount_ > 0)
    colsCount_ = rowsDeque_[0].size();
}

void CSV::appendRow(const std::vector<std::string>& strVec) {
  if (strVec.size() != colsCount_) {
    LOG(WARNING) << "CSV::appendRow: strVec.size() != cols_count";
    return;
  }
  rowsDeque_.push_back(strVec);
  rowsCount_ = rowsDeque_.size();
}

void CSV::setRowNum(unsigned int num) { rowsCount_ = num; }

void CSV::setColNum(unsigned int num) { colsCount_ = num; }

// This loads data from file to rd
void CSV::loadFile(const std::string& filename, std::deque<std::string>& readed_data) {
  char buffer[READ_BLOCK_SIZE];
  fstream csvFile(filename, ios::in | ios::binary);
  if (!csvFile.is_open()) {
    LOG(WARNING) << "Can`t open " << filename;
  }
  csvFile.seekg(0, ios::beg); // to end of the file
  auto fileSize = csvFile.tellg();
  csvFile.seekg(0, ios::beg); // to start of the file
  // Read and write by "BLOCK_SIZE" bytes
  int blocks_count = fileSize / READ_BLOCK_SIZE;
  for (int i = 0; i <= blocks_count; ++i) {
    csvFile.read(buffer, READ_BLOCK_SIZE);
    if (!csvFile)
      LOG(WARNING) << "error: only " << csvFile.gcount() << " could be read from " << filename;
    readed_data.push_back(std::string((char*)buffer, READ_BLOCK_SIZE));
  }
  csvFile.close();
}

// This saves in .csv file vector of rows "this->data".
// This vector must be already filled.
void CSV::saveToFile(const std::string& filename) {
  if (this->colsCount_ == 0) // if count of columns equals 0, generate error
  {
    cout << "Wrong csv format: columns=0" << endl;
    throw;
  }
  if (this->rowsCount_ == 0) // if count of rows equals 0, generate error
  {
    cout << "Wrong csv format: rows=0" << endl;
    throw;
  }
  // Try to open file
  ofstream out(filename.c_str());
  if (!out) {
    cout << "Can't create or open file \"" << filename << '\"' << endl;
    throw;
  }
  // write "data" in file
  for (unsigned int i = 0; i < this->rowsCount_; ++i) {
    row& rw = rowsDeque_[i];                              // create reference to part of data
    unsigned int delimiters_count = this->colsCount_ - 1; // count of delimiters
    // write row in file
    for (unsigned int j = 0; j < this->colsCount_; ++j) {
      out << rw[j];
      if (j < delimiters_count)
        out << delimiter_;
    }
    // if row is not last, write endline
    if (i < this->rowsCount_ - 1)
      out << endl;
  }
  out.close(); // close file
}

// This gets reference to row ( typedef std::vector<std::string> row; )
// in data ( std::vector<row> data; ) with row index row_n.
row& CSV::getRowRef(unsigned int row_n) {
  if (row_n >= this->rowsCount_) // if row index is wrong, generate error
  {
    cout << "Wrong row index: " << row_n << ", but max index is: " << this->rowsCount_ - 1 << endl;
    throw;
  }
  return rowsDeque_[row_n];
}

// This gets reference to row's field ( typedef std::vector<std::string> row; )
// with row index row_n and field(column) index col_n.
std::string& CSV::getAtRef(unsigned int row_n, unsigned int col_n) {
  row& rw = getRowRef(row_n);
  if (col_n >= this->colsCount_) // if field index is wrong, generate error
  {
    cout << "Wrong field index: " << col_n << ", but max index is: " << this->colsCount_ - 1
         << endl;
    throw;
  }
  return rw[col_n];
}

// This gets row's field ( typedef std::vector<std::string> row; )
// with row index row_n and field(column) index col_n.
std::string CSV::getAt(unsigned int row_n, unsigned int col_n) { return getAtRef(row_n, col_n); }

// This gets row's field ( typedef std::vector<std::string> row; )
// with row index row_n and field(column) index col_n.
std::string CSV::setAt(unsigned int row_n, unsigned int col_n, const std::string& data) {
  return getAtRef(row_n, col_n) = data;
}

} // namespace algo
} // namespace boostander
