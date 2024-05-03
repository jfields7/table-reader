//! \file test_header.cpp
//  \brief Unit tests for reading the header properly.

#include <iostream>

#include <tr_table.hpp>

#include <testing.hpp>

using namespace TableReader;

bool TestLoad() {
  Table table;
  auto result = table.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "An error occurred while reading the table:\n"
              << result.message;
    return false;
  }
  return true;
}

bool TestBadFilename() {
  Table table;
  auto result = table.ReadTable("non-existent.athtab");
  if (result.error != ReadResult::BAD_FILENAME) {
    std::cout << "Did not return BAD_FILENAME after reading non-existent table.\n";
    return false;
  }
  return true;
}

bool TestMetadata() {
  Table table;
  auto result = table.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
    return false;
  }
  auto& metadata = table.GetMetadata();

  // A lambda function for checking metadata
  auto test = [&](std::string key, std::string value) {
    try {
      if (metadata.at(key).compare(value) != 0) {
        std::cout << "Key '" << key << "' has wrong value:\n"
                  << "  Expected: " << value << "\n"
                  << "  Actual: " << metadata.at(key) << "\n";
        return false;
      }
    } catch(std::out_of_range& e) {
      std::cout << "Did not properly load '" << key << "' key.\n";
      return false;
    }
    return true;
  };

  if (!test("version", "1.0")) {
    return false;
  }
  if (!test("endianness", "little")) {
    return false;
  }
  if (!test("precision", "double")) {
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  UnitTests tester{"Header"};

  tester.RunTest(&TestLoad, "Load Test");
  tester.RunTest(&TestBadFilename, "Bad Filename Test");

  tester.RunTest(&TestMetadata, "Metadata Test");

  tester.PrintSummary();

  if (tester.tests_failed.size() > 0) {
    return -1;
  }
  return 0;
}
