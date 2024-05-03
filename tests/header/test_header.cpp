//! \file test_header.cpp
//  \brief Unit tests for reading the header properly.

#include <iostream>

#include <tr_table.hpp>

#include <testing.hpp>

using namespace TableReader;

bool TestLoad(Table* table) {
  auto result = table->ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "An error occurred while reading the table:\n"
              << result.message;
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  UnitTests tester{"Header"};
  Table table;
  tester.RunTest(&TestLoad, "Load Test", &table);

  tester.PrintSummary();

  if (tester.tests_failed.size() > 0) {
    return -1;
  }
  return 0;
}
