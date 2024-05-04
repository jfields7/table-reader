//! \file test_table.cpp
//  \brief Unit tests for loading data from the table.

#include <iostream>

#include <tr_table.hpp>

#include <testing.hpp>

using namespace TableReader;

bool TestSize() {
  Table table;
  auto result = table.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
    return false;
  }
  auto& point_info = table.GetPointInfo();
  int npoints_calc = 1;
  int size_calc = 0;
  for (auto& p : point_info) {
    npoints_calc *= p.second;
    size_calc += p.second;
  }
  int npoints = table.GetNPoints();
  if (npoints != npoints_calc) {
    std::cout << "Number of points and calculated dimensions disagree!\n"
              << "  Calculated: " << npoints_calc << "\n"
              << "  Reported: " << npoints << "\n";
    return false;
  }
  int nfields = table.GetFieldNames().size();
  size_calc += nfields*npoints_calc;
  int size = table.GetMemSize();
  if (size != size_calc) {
    std::cout << "Size of allocated memory does not calculated requirements.\n"
              << "  Calculated size: " << size_calc << "\n"
              << "  Allocated size: " << size << "\n";
    return false;
  }

  return true;
}

bool TestOffsets() {
  Table table;
  auto result = table.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
    return false;
  }
  int offset = 0;
  auto& point_info = table.GetPointInfo();
  auto& field_names = table.GetFieldNames();
  const double* data = table.GetRawData();
  for (auto &p : point_info) {
    if (table[p.first] != &data[offset]) {
      std::cout << "Point field '" << p.first << "' has the wrong memory offset!\n";
      return false;
    }
    offset += p.second;
  }
  int npoints = table.GetNPoints();
  int size = table.GetMemSize();
  for (auto& f : field_names) {
    if (table[f] != &data[offset]) {
      std::cout << "Data field '" << f << "' has the wrong memory offset!\n";
      return false;
    }
    offset += npoints;
  }

  if (offset != size) {
    std::cout << "Allocated memory size doesn't match calculated memory end!\n"
              << "  Allocated: " << size << "\n"
              << "  Calculated offset: " << offset << "\n";
  }
  return true;
}

int main(int argc, char *argv[]) {
  UnitTests tester{"Table"};

  tester.RunTest(&TestSize, "Size Test");
  tester.RunTest(&TestOffsets, "Offset Test");

  tester.PrintSummary();

  if (tester.tests_failed.size() > 0) {
    return -1;
  }
  return 0;
}
