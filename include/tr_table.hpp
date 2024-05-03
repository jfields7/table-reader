#ifndef TR_TABLE_HPP_
#define TR_TABLE_HPP_
//! \file tr_table.hpp
//! \brief Declares Table class

#include <string>
#include <map>
#include <vector>
#include <fstream>

namespace TableReader {

struct ReadResult {
  enum ErrorCode {
    SUCCESS,
    BAD_FILENAME,
    BAD_HEADER
  };
  ErrorCode error;
  std::string message;
};

class Table {
 public:
  Table();
  ~Table();

  ReadResult ReadTable(const std::string fname);

  inline const std::map<std::string, std::string> GetMetadata() {
    return metadata;
  }

  inline const std::map<std::string, int> GetPointInfo() {
    return point_info;
  }

  inline const std::map<std::string, double> GetScalars() {
    return scalars;
  }

  inline int GetNDimensions() {
    return ndim;
  }

  inline double * operator[](std::string& field) {
    return fields[field];
  }

  inline bool IsInitialize() {
    return initialized;
  }

 private:

  ReadResult ExtractBlock(std::ifstream& file, const std::string name,
                          std::vector<std::string>& lines);

  bool SplitToken(const std::string& in, std::string& key, std::string& value);

  void TrimWhiteSpace(std::string& str);

  std::map<std::string, std::string> metadata;
  std::map<std::string, int> point_info;
  std::map<std::string, double> scalars;
  std::map<std::string, double*> fields;
  double * data;
  int ndim;
  bool initialized;
};

} // namespace TableReader

#endif
