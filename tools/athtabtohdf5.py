#!/usr/bin/env python3
#
# athtabtohdf5.py
#
# Convert an AthenaK .athtab EOS table to a PyCompOSE-style HDF5 file.
#

import argparse

import h5py
from readtable import Table


def main(**kwargs):

    table = Table(kwargs["input"])

    with h5py.File(kwargs["output"], "w") as h5:

        # Scalar datasets
        for key in table.get_scalar_keys():
            h5.create_dataset(key, data=table.get_scalar(key))

        # Coordinate arrays
        for key in table.get_points_keys():
            h5.create_dataset(key, data=table.get_points(key))

        # Field datasets
        #
        # These are written in the same order as they appeared in the
        # athtab header. hdf5toathtab.py simply iterates over all datasets
        # except mn/mp/nb/yq/t and writes them back out.
        for key in table.get_field_keys():
            h5.create_dataset(key, data=table.get_field(key))


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="Convert an AthenaK .athtab EOS table to HDF5."
    )

    parser.add_argument(
        "-i",
        "--input",
        required=True,
        help="input AthenaK table (.athtab)",
    )

    parser.add_argument(
        "-o",
        "--output",
        required=True,
        help="output HDF5 file",
    )

    args = parser.parse_args()
    main(**vars(args))
