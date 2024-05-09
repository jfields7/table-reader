//! \file test_table.cpp
//  \brief Unit tests for loading data from the table.

#include <iostream>

#include <hdf5.h>
#include <hdf5_hl.h>

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
  size_t npoints_calc = 1;
  size_t size_calc = 0;
  for (auto& p : point_info) {
    npoints_calc *= p.second;
    size_calc += p.second;
  }
  size_t npoints = table.GetNPoints();
  if (npoints != npoints_calc) {
    std::cout << "Number of points and calculated dimensions disagree!\n"
              << "  Calculated: " << npoints_calc << "\n"
              << "  Reported: " << npoints << "\n";
    return false;
  }
  size_t nfields = table.GetFieldNames().size();
  size_calc += nfields*npoints_calc;
  size_t size = table.GetMemSize();
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
  size_t offset = 0;
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
  size_t npoints = table.GetNPoints();
  size_t size = table.GetMemSize();
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

bool TestTable() {
  // Load the .athtab table
  Table table;
  auto result = table.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
    return false;
  }

  // Load the HDF5 table
  herr_t ierr;
  hsize_t snb;
  hid_t file_id = H5Fopen("../data/SFHo_T0.1_beta.h5", H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file_id < 0) {
    std::cout << "Could not complete test because of HDF5 loading failure.\n";
    return false;
  }

  // Get dataset sizes
  size_t nn = table.GetPointInfo()[0].second;
  ierr = H5LTget_dataset_info(file_id, "nb", &snb, NULL, NULL);
  size_t nsnb = snb;
  if (nn != nsnb) {
    std::cout << "The table dimensions don't match!\n"
              << "  Expected: " << nsnb << "\n"
              << "  Actual: " << nn << "\n";
    return false;
  }

  // Check that the points match
  double *scratch = new double[nsnb];
  for (auto& field : table.GetFieldNames()) {
    double *quantity = table[field];
    ierr = H5LTread_dataset_double(file_id, field.c_str(), scratch);
    if (ierr < 0) {
      std::cout << "There was an issue reading " << field << " from the HDF5 table!\n";
      for (size_t i = 0; i < nn; i++) {
        if (quantity[i] != scratch[i]) {
          std::cout << "The tables don't match!\n"
                    << "  Expected: " << field << "[" << i << "] = " << scratch[i] << "\n"
                    << "  Actual: " << field << "[" << i << "] = " << quantity[i] << "\n";
          return false;
        }
      }
    }
  }

  // Cleanup
  delete[] scratch;
  H5Fclose(file_id);

  return true;
}

bool TestBigEndian() {
  // Load the little endian table
  Table little;
  auto result = little.ReadTable("../data/SFHo_T0.1_beta.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
  }

  // Load the big endian table
  Table big;
  result = big.ReadTable("../data/SFHo_T0.1_beta.be.athtab");
  if (result.error != ReadResult::SUCCESS) {
    std::cout << "Could not complete test because of loading failure.\n";
    return false;
  }

  // Get dataset sizes
  size_t nn = big.GetPointInfo()[0].second;
  size_t nn_l = little.GetPointInfo()[0].second;
  if (nn != nn_l) {
    std::cout << "The table dimensions don't match!\n"
              << "  Expected: " << nn_l << "\n"
              << "  Actual: " << nn << "\n";
    return false;
  }

  // Check that the points match
  for (auto& field : little.GetFieldNames()) {
    if (!big.HasField(field)) {
      std::cout << "Missing field '" << field << "'!\n";
      return false;
    }
    double *q_big = big[field];
    double *q_little = little[field];
    for (size_t i = 0; i < nn; i++) {
      if (q_big[i] != q_little[i]) {
        std::cout << "The tables don't match!\n"
                  << "  Expected: " << field << "[" << i << "] = " << q_little[i] << "\n"
                  << "  Actual: " << field << "[" << i << "] = " << q_big[i] << "\n";
        return false;
      }
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  UnitTests tester{"Table"};

  tester.RunTest(&TestSize, "Size Test");
  tester.RunTest(&TestOffsets, "Offset Test");
  tester.RunTest(&TestTable, "Table Validation");
  tester.RunTest(&TestBigEndian, "Big Endian Test");

  tester.PrintSummary();

  if (tester.tests_failed.size() > 0) {
    return -1;
  }
  return 0;
}
