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

  if (metadata.size() > 3) {
    std::cout << "Metadata contains too many keys!\n";
    return false;
  }

  return true;
}

bool TestScalars() {
  Table table;
  auto result = table.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
    return false;
  }
  auto& scalars = table.GetScalars();

  try {
    if (scalars.at("mn") != 939.56535) {
      std::cout << "Scalar 'mn' has the wrong value.\n"
                << "  Expected: 939.56535\n"
                << "  Actual: " << scalars.at("mn") << "\n";
      return false;
    }
  } catch (std::out_of_range& e) {
    std::cout << "Did not properly load scalar 'mn'\n";
    return false;
  }

  return true;
}

bool TestPoints() {
  Table table;
  auto result = table.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
    return false;
  }
  auto& point_info = table.GetPointInfo();
  if (point_info.size() > 1) {
    std::cout << "Header contains too many points!\n";
    return false;
  }
  if (point_info[0].first.compare("nn") != 0) {
    std::cout << "Did not properly load point 'nn'\n";
    return false;
  }
  if (point_info[0].second != 305) {
    std::cout << "Point 'nn' has the wrong value:\n"
              << "  Expected: 305\n"
              << "  Actual: " << point_info[0].second << "\n";
    return false;
  }
  
  return true;
}

int main(int argc, char *argv[]) {
  UnitTests tester{"Header"};

  tester.RunTest(&TestLoad, "Load Test");
  tester.RunTest(&TestBadFilename, "Bad Filename Test");

  tester.RunTest(&TestMetadata, "Metadata Test");
  tester.RunTest(&TestScalars, "Scalar Test");
  tester.RunTest(&TestPoints, "Points Test");

  tester.PrintSummary();

  if (tester.tests_failed.size() > 0) {
    return -1;
  }
  return 0;
}
