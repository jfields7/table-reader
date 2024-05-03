# hdf5toathtab.py
# Convert a PyCompOSE HDF5 eos file to an AthenaK table (.athtab).
import argparse
import struct
import sys

import h5py as h5
import numpy as np

def main(**kwargs):
  
  endianness = '='
  endianstring = sys.byteorder
  if kwargs['endianness'] == 'big':
    endianness = '>'
    endianstring = 'big'
  elif kwargs['endianness'] == 'little':
    endianness = '<'
    endianstring = 'little'
  elif kwargs['endianness'] != 'native':
    raise RuntimeError(f'Unknown endianness {endianness}.')

  precision = 'single'
  fptype = np.float32
  fspec = 'f'
  if kwargs['double']:
    precision = 'double'
    fptype = np.float64
    fspec = 'd'

  fout = kwargs['output']
  fin = kwargs['input']

  eos = h5.File(fin)

  # Load in the thermodynamic quantities
  mn = np.array(eos['mn'][()], dtype=fptype)
  n = np.array(eos['nb'][:], dtype=fptype)
  Q1 = np.array(eos['Q1'][:,0,0], dtype=fptype)
  Q7 = np.array(eos['Q7'][:,0,0], dtype=fptype)
  cs2 = np.array(eos['cs2'][:,0,0], dtype=fptype)

  nn = len(n)

  # Prepare the header
  header = f"<metadatabegin>\n" \
           f"version=1.0\n" \
           f"endianness={endianstring}\n" \
           f"precision={precision}\n" \
           f"<metadataend>\n" \
           f"<scalarsbegin>\n" \
           f"mn={mn}\n" \
           f"<scalarsend>\n" \
           f"<pointsbegin>\n" \
           f"nn={len(n)}\n" \
           f"<pointsend>\n" \
           f"<fieldsbegin>\n" \
           f"Q1\n" \
           f"Q7\n" \
           f"cs2\n" \
           f"<fieldsend>\n"

  print(header)

  # Open the output file to write the header.
  output = open(fout, 'w')
  output.write(header)
  output.close()

  # Now open the file in binary and write the data
  output = open(fout, 'ab')
  output.write(struct.pack(f'{endianness}{nn}{fspec}',*n))
  output.write(struct.pack(f'{endianness}{nn}{fspec}',*Q1))
  output.write(struct.pack(f'{endianness}{nn}{fspec}',*Q7))
  output.write(struct.pack(f'{endianness}{nn}{fspec}',*cs2))
  output.close()


if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('-i', '--input', help='name of PyCompOSE input file')
  parser.add_argument('-o', '--output', help='name of athtab output file')
  parser.add_argument('-e', '--endianness', help='force specific endianness',
                      default='native')
  parser.add_argument('-d', '--double', action='store_true',
                      help='use double precision output')
  args = parser.parse_args()
  main(**vars(args))
