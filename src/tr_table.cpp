//! \file tr_table.cpp
//! \brief Implementation of Table class
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include <tr_table.hpp>

using namespace TableReader;

Table::Table() : ndim(0), initialized(false) {
}

Table::~Table() {
  if (initialized) {
    delete[] data;
  }
}

ReadResult Table::ReadTable(const std::string fname) {
  ReadResult result;

  std::ifstream file;
  try {
    file.open(fname.c_str(), std::ifstream::in);
  } catch (std::ifstream::failure& e) {
    result.error = ReadResult::BAD_FILENAME;
    std::stringstream ss;
    ss << "Could not read '" << fname << "'\n"
       << "open() returned the following error:\n"
       << e.what();
    result.message = ss.str();
    return result;
  }

  // Something bizarre happened while reading the file.
  if (!file.is_open()) {
    result.error = ReadResult::BAD_FILENAME;
    std::stringstream ss;
    ss << "No exception occurred, but ReadTable() failed to open '" << fname << "'\n";
    result.message = ss.str();
    return result;
  }

  // HEADER PARSING

  // Read in the metadata
  std::vector<std::string> block_lines;
  result = ExtractBlock(file, "metadata", block_lines);
  if (result.error != ReadResult::SUCCESS) {
    file.close();
    return result;
  }
  for (auto line : block_lines) {
    std::string key, value;
    bool success = SplitToken(line, key, value);
    if (!success) {
      result.error = ReadResult::BAD_HEADER;
      std::stringstream ss;
      ss << "'" << line << "' is not a valid metadata line.\n";
      result.message = ss.str();
      return result;
    }
    else {
      metadata[key] = value;
    }
  }
  block_lines.clear();

  // Read in the scalars
  result = ExtractBlock(file, "scalars", block_lines);
  if (result.error != ReadResult::SUCCESS) {
    file.close();
    return result;
  }
  for (auto line : block_lines) {
    std::string key, value;
    bool success = SplitToken(line, key, value);
    if (!success) {
      result.error = ReadResult::BAD_HEADER;
      std::stringstream ss;
      ss << "'" << line << "' is not a valid scalars line.\n";
      result.message = ss.str();
      return result;
    }
    else {
      scalars[key] = std::stod(value);
    }
  }
  block_lines.clear();

  //size_t header_size = file.tellg();

  result.error = ReadResult::SUCCESS;

  return result;
}

ReadResult Table::ExtractBlock(std::ifstream& file, const std::string name,
                               std::vector<std::string>& lines) {
  ReadResult result;
  // Read the first block
  std::stringstream ss;
  ss << "<" << name << "begin" << ">";
  char buffer[256];
  file.getline(buffer, 256);

  std::string line = std::string(buffer);
  if (line.compare(ss.str()) != 0) {
    ss.str("");
    result.error = ReadResult::BAD_HEADER;
    ss << "Header is either missing '" << name << "' or is in the wrong order.\n";
    result.message = ss.str();
    return result;
  }

  ss.str("");
  ss << "<" << name << "end" << ">";
  while(!file.eof()) {
    file.getline(buffer, 256);
    // Check if we're at the end of this block.
    line = std::string(buffer);
    if (line[0] == '<') {
      if (line.compare(ss.str()) != 0) {
        ss.str("");
        result.error = ReadResult::BAD_HEADER;
        ss << "Unexpected new block before reaching end of '" << name << "' in header.\n";
        result.message = ss.str();
        return result;
      }
      else {
        break;
      }
    }
    else {
      lines.push_back(line);
    }
  }

  if (file.eof()) {
    // We reached the end of the file, but we shouldn't have.
    ss.str("");
    result.error = ReadResult::BAD_HEADER;
    ss << "Unexpected end of file while reading '" << name << "' in header.\n";
    result.message = ss.str();
    return result;
  }

  result.error = ReadResult::SUCCESS;
  return result;
}

bool Table::SplitToken(const std::string& in, std::string& key, std::string& value) {
  size_t pos = in.find('=');
  // The equals sign does not exist or is in the wrong location.
  if (pos == std::string::npos) {
    return false;
  } else if (in.back() == '=' || in.front() == '=') {
    return false;
  }

  key = in.substr(0, pos);
  value = in.substr(pos+1, in.size());

  TrimWhiteSpace(key);
  TrimWhiteSpace(value);

  return true;
}

void Table::TrimWhiteSpace(std::string& str) {
  while(str.front() == ' ' || str.front() == '\t') {
    str.erase(0,1);
  }
  while(str.back() == ' ' || str.front() == '\t') {
    str.pop_back();
  }
}
