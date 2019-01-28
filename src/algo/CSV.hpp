#pragma once

#include <condition_variable>
#include <cstdio>
#include <deque>
#include <fstream>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace boostander {
namespace algo {

using namespace std;

typedef std::vector<std::string> row;

class CSV {
public:
  // If counts of rows and columns !=0, this fills vector of rows "data";
  // each of rows has cols_count empty strings
  CSV(char delim = ',', unsigned int rows_n = 0, unsigned int cols_n = 0);

  // This loads .csv file and fills vector of rows "data";
  // counts of rows and columns determines from file data.
  // The delimiter is always ",".
  void loadFromFile(const std::string& filename);

  // This saves in .csv file vector of rows "this->data".
  // This vector must be already filled.
  // The delimiter is always ",".
  void saveToFile(const std::string& filename);

  // This gets count of rows.
  unsigned int getRowsCount() { return rowsCount_; }

  // This gets count of columns.
  unsigned int getColsCount() { return colsCount_; }

  // This gets row's field ( typedef std::vector<std::string> row; )
  // with row index row_n and field(column) index col_n.
  std::string getAt(unsigned int row_n, unsigned int col_n);

  // This gets reference to row's field ( typedef std::vector<std::string> row; )
  // with row index row_n and field(column) index col_n.
  std::string& getAtRef(unsigned int row_n, unsigned int col_n);

  // This gets row ( typedef std::vector<std::string> row; )
  // in data ( std::vector<row> data; ) with row index row_n.
  row getRow(unsigned int row_n) { return getRowRef(row_n); }

  void loadFromMemory(const std::string& rowsDeque_);

  std::string setAt(unsigned int row_n, unsigned int col_n, const std::string& rowsDeque_);

  void appendRow(const std::vector<std::string>& strVec);

  void setRowNum(unsigned int num);

  void setColNum(unsigned int num);

private:
  // This gets reference to row ( typedef std::vector<std::string> row; )
  // in data ( std::vector<row> data; ) with row index row_n.
  row& getRowRef(unsigned int row_n);

  // This loads data from file to readed_data
  void loadFile(const std::string& filename, std::deque<std::string>& readed_data);

  // This parse readed_data to data
  void parseData(std::deque<std::string>& readed_data);

private:
  unsigned int rowsCount_{0};

  unsigned int colsCount_{0};

  std::deque<row> rowsDeque_;

  char delimiter_{','};

  static const int MAX_FIELD_SIZE{1024};

  static const int READ_BLOCK_SIZE{2048};

  char unparsedBuffer_[READ_BLOCK_SIZE];

  int unparsedBufferSize_{0};
};

} // namespace algo
} // namespace boostander
