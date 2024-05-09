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
  mp = np.array(eos['mp'][()], dtype=fptype)

  n = np.array(eos['nb'][:], dtype=fptype)
  yq = np.array(eos['yq'][:], dtype=fptype)
  t = np.array(eos['t'][:], dtype=fptype)

  exclude = ["mn", "mp", "nb", "yq", "t"]

  fields = []
  
  for key in eos.keys():
    if key in exclude:
      continue
    else:
      fields.append([key, np.array(eos[key], dtype=fptype)])

  #Q1 = np.array(eos['Q1'][:,0,0], dtype=fptype)
  #Q7 = np.array(eos['Q7'][:,0,0], dtype=fptype)
  #cs2 = np.array(eos['cs2'][:,0,0], dtype=fptype)

  nn = len(n)
  ny = len(yq)
  nt = len(t)
  size = nn*ny*nt

  # Prepare the header
  header = f"<metadatabegin>\n" \
           f"version=1.0\n" \
           f"endianness={endianstring}\n" \
           f"precision={precision}\n" \
           f"<metadataend>\n" \
           f"<scalarsbegin>\n" \
           f"mn={mn}\n" \
           f"mp={mp}\n" \
           f"<scalarsend>\n" \
           f"<pointsbegin>\n" \
           f"nb={len(n)}\n" \
           f"yq={len(yq)}\n" \
           f"t={len(t)}\n" \
           f"<pointsend>\n" \
           f"<fieldsbegin>\n"
  for field in fields:
    header = header + f"{field[0]}\n"
  header = header + f"<fieldsend>\n"

  print(header)

  # Open the output file to write the header.
  output = open(fout, 'w')
  output.write(header)
  output.close()

  # Now open the file in binary and write the data
  output = open(fout, 'ab')
  output.write(struct.pack(f'{endianness}{nn}{fspec}',*n))
  output.write(struct.pack(f'{endianness}{ny}{fspec}',*yq))
  output.write(struct.pack(f'{endianness}{nt}{fspec}',*t))
  for field in fields:
    output.write(struct.pack(f'{endianness}{size}{fspec}', *field[1].flatten()))
  #output.write(struct.pack(f'{endianness}{nn}{fspec}',*Q1))
  #output.write(struct.pack(f'{endianness}{nn}{fspec}',*Q7))
  #output.write(struct.pack(f'{endianness}{nn}{fspec}',*cs2))
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
