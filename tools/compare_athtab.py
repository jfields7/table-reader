#!/usr/bin/env python3

import argparse
import sys

import numpy as np

from readtable import Table


def compare_array(name, a, b):
    a = np.asarray(a)
    b = np.asarray(b)

    if a.shape != b.shape:
        print(f"{name}: shape differs: {a.shape} != {b.shape}")
        return False

    if not np.array_equal(a, b):
        diff = np.abs(a.astype(np.float64) - b.astype(np.float64))

        print(f"{name}: values differ")
        print(f"  number of different entries: {np.count_nonzero(a != b)}")
        print(f"  max absolute difference: {diff.max():.16e}")

        return False

    return True


def compare(file1, file2):

    t1 = Table(file1)
    t2 = Table(file2)

    ok = True

    # Metadata
    if t1.metadata != t2.metadata:
        print("Metadata differ:")
        print("  file1:", t1.metadata)
        print("  file2:", t2.metadata)
        ok = False

    # Scalars
    if set(t1.get_scalar_keys()) != set(t2.get_scalar_keys()):
        print("Scalar keys differ")
        ok = False

    for key in set(t1.get_scalar_keys()).intersection(t2.get_scalar_keys()):
        if t1.get_scalar(key) != t2.get_scalar(key):
            print(
                f"Scalar {key} differs: "
                f"{t1.get_scalar(key)} != {t2.get_scalar(key)}"
            )
            ok = False

    # Coordinates
    if set(t1.get_points_keys()) != set(t2.get_points_keys()):
        print("Point keys differ")
        print("  only in file1:", set(t1.get_points_keys()) - set(t2.get_points_keys()))
        print("  only in file2:", set(t2.get_points_keys()) - set(t1.get_points_keys()))
        ok = False

    for key in set(t1.get_points_keys()).intersection(t2.get_points_keys()):
        ok &= compare_array(
            f"points {key}",
            t1.get_points(key),
            t2.get_points(key),
        )

    # Fields
    fields1 = set(t1.get_field_keys())
    fields2 = set(t2.get_field_keys())

    if fields1 != fields2:
        print("Field names differ")
        print("  only in file1:", fields1 - fields2)
        print("  only in file2:", fields2 - fields1)
        ok = False

    for key in fields1.intersection(fields2):
        ok &= compare_array(
            f"field {key}",
            t1.get_field(key),
            t2.get_field(key),
        )

    if ok:
        print("Files are identical.")
    else:
        print("Files differ.")
        sys.exit(1)


def main():

    parser = argparse.ArgumentParser(
        description="Strictly compare two AthenaK .athtab files."
    )

    parser.add_argument("file1")
    parser.add_argument("file2")

    args = parser.parse_args()

    compare(args.file1, args.file2)


if __name__ == "__main__":
    main()
