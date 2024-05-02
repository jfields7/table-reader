# TableReader
A simple table reader to be implemented into AthenaK.

## Table Specification
Every table file is constructed of an ASCII header and a binary blob. A typical header
looks like the following:
```
<metadatabegin>
version=[version #]
endianness=[little or big]
precision=[single or double]
<metadataend>
<scalarsbegin>
mb=[baryon mass]
<scalarsend>
<pointsbegin>
nn=[# of points in n]
nYe=[# of points in Ye]
nT=[# of points in T]
<pointsend>
<fieldsbegin>
P // Pressure
s // entropy per baryon
mub // baryon chemical potential
muq // charge chemical potential
mul // lepton chemical potential
e // energy density
cs // sound speed
[other fields]
<fieldsend>
```
Note that the file specification does not require all these point labels or fields to be
present, nor does it prohibit more from being included. In principle this means that the
same format can be used for a 1D table for a zero-temperature nuclear EOS in weak
equilibrium, a 3D microphysical table (as suggested by the header above), a table of
temperature-dependent chemical reaction rates, etc.

The binary blob for this header would be structured as follows:
```
<n0><n1><n2>...<nn-1>
<Ye0><Ye1><Ye2>...<nYe-1>
<T0><T1><T2>...<nT-1>
<P(n0,Ye0,T0)><P(n0,Ye0,T1)>...<P(n0,Ye0,nT-1)>
<P(n0,Ye1,T0)><P(n0,Ye1,T0)>...<P(n0,Ye1,nT-1)>
...
<P(n0,Yen-1,T0)><P(n0,Yen-1,T1)>...<P(n0,nYe-1,nT-1>
<P(n1,Ye0,T0)><P(n1,Ye0,T1>...<P(n1,Ye0,nT-1)>
...
...
<P(nn-1,Yen-1,T0><P(nn-1,Yen-1,T1)>...<P(nn-1,nYe-1,nT-1)>
<repeat for other fields>
```
Both table points and table fields in the binary blob will always be specified in the
order they're written in the header, and the fields will be indexed assuming this same
point ordering.
