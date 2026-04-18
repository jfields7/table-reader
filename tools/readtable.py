# readtable.py
# Read a binary file in table-reader format to a python object
import numpy as np
import argparse
import copy

class Table:
    def read_file(self, path, verbose=False, type_overrides={}):
        """
        This function reads from a binary file in the table-reader format.
        """
        if verbose: print("Reading from file at: " + str(path))
        with open(path, "rb") as f:
            _metadata = []
            while True:
                line = f.readline().decode().strip()
                _metadata.append(line)
                if "<metadataend>" in line:
                    break
            
            if verbose:
                print("Read following lines as metadata:")
                for line in _metadata:
                    print(line)
                print()

            _scalars = []
            while True:
                line = f.readline().decode().strip()
                _scalars.append(line)
                if "<scalarsend>" in line:
                    break

            if verbose:
                print("Read following lines as scalars:")
                for line in _scalars:
                    print(line)
                print()

            _points = []
            while True:
                line = f.readline().decode().strip()
                _points.append(line)
                if "<pointsend>" in line:
                    break

            if verbose:
                print("Read following lines as points:")
                for line in _points:
                    print(line)
                print()

            _fields = []
            while True:
                line = f.readline().decode().strip()
                _fields.append(line)
                if "<fieldsend>" in line:
                    break

            if verbose:
                print("Read following lines as fields:")
                for line in _fields:
                    print(line)
                print()
            
            def parselines_to_dict(lines, typef = str,  type_overrides = {}):
                dct = {}
                lst = []
                for line in lines:
                    if "=" in line:
                        key, value = line.split("=")
                        key = key.strip()
                        value = value.strip()
                        lst.append(key)
                        if verbose: 
                            print("    Found key: " + key)
                            print("    Found value: " + value)
                        if key in type_overrides.keys():
                            value = type_overrides[key](value)
                            print("    Using type: " + str(type_overrides[key]))
                        else:
                            value = typef(value)
                            print("    Using type: " + str(typef))
                        dct[key] = typef(value)
                    else:
                        if verbose: print("    Skipped line: " + line)
                return dct, lst
            
            def parselines_to_list(lines):
                lst = []
                for line in lines:
                    if line[0] == "<" and line[-1] == ">":
                        if verbose: print("    Skipped line: " + line)
                        continue
                    else:
                        value = line
                        lst.append(value)
                        if verbose: print("    Found value: " + value)
                return lst
            
            _type_overrides = copy.deepcopy(type_overrides)

            print("Extracting metadata:")
            self.metadata, self.metadata_keys = parselines_to_dict(_metadata, str,  type_overrides = _type_overrides)

            print("Extracting scalars:")
            self.scalars, self.scalars_keys = parselines_to_dict(_scalars, float,  type_overrides = _type_overrides)

            print("Extracting points:")
            self.points, self.points_keys = parselines_to_dict(_points, int,  type_overrides = _type_overrides)

            print("Extracting fields:")
            self.fields = parselines_to_list(_fields)

            endianness = self.metadata["endianness"]
            precision = self.metadata["precision"]

            # dtype
            if precision == "double":
                dtype = np.dtype(np.float64)
            else:
                dtype = np.dtype(np.float32)

            if endianness == "little":
                dtype = dtype.newbyteorder("<")
            elif endianness == "big":
                dtype = dtype.newbyteorder(">")
            else:
                dtype = dtype.newbyteorder("=")

            if verbose:
                print("Using precision: " + precision)
                print("Using endianness: " + endianness)
                print("Using dtype: " + str(dtype))

            self.data_points = {}
            npts_fields = 1
            shape_fields = []
            for idx, key in enumerate(self.points_keys):
                npoint = self.points[key]
                if verbose: print(f"Axis {idx+1:d} with name '{key:s}' has size {npoint:d}")
                npts_fields *= npoint
                shape_fields.append(npoint)
                self.data_points[key] = np.fromfile(f, dtype=dtype, count=npoint)
                if verbose: print(f"{self.data_points[key][0]:.3e} <= {key:s} <= {self.data_points[key][-1]:.3e}")
            
            self.shape = tuple(shape_fields)
            print("Field shape is: " + str(self.shape))
            self.data_fields = {}
            for key in self.fields:
                arr = np.fromfile(f, dtype=dtype, count=npts_fields)
                self.data_fields[key] = arr.reshape(self.shape)
                if verbose: print(f"{self.data_fields[key].min():.3e} <= {key:s} <= {self.data_fields[key].max():.3e}")

        return True
    
    def get_metadata_keys(self):
        return self.metadata_keys

    def get_metadata(self, key):
        return self.metadata[key]
    
    def get_scalar_keys(self):
        return self.scalars_keys
    
    def get_scalar(self, key):
        return self.scalars[key]
    
    def get_points_keys(self):
        return self.points_keys
    
    def get_points_size(self, key):
        return self.points[key]

    def get_points(self, key):
        return self.data_points[key]

    def get_field_keys(self):
        return self.fields
    
    def get_field(self, key):
        return self.data_fields[key]
    
    def __init__(self, path, verbose=False):
        success = self.read_file(path, verbose=verbose)
        if not success:
            raise RuntimeError("Could not read table at {:s} successfully.".format(path))
        

def main(**kwargs):
    fin = kwargs['input']
    table = Table(fin, verbose=kwargs["verbose"])
    return

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', help='name of input file')
    parser.add_argument('-v', '--verbose', action='store_true', help='print out information about the table after reading')
    args = parser.parse_args()
    main(**vars(args))