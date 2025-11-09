# MVF
Multivariate field visualizer (MVF) is a tool that helps user visualize multivariate fields (fields with more than or equal to a single component. Ex: Scalar, Vector, Tensor field etc).


![Bonsai tree model](assets/result.png "Distance field generated from a 1d trait (isosurface) on a bonsai tree model")


## Field visualization
The tool currently supports loading Visualization toolkit (VTK) files. The user can then select an appropriate number of components from the field and then visualize it. This visualization could take the form of a slice for a scalar field or an arrow glyph for vector fields (these options can be configured) etc.

## Attribute visualization
The real power of the tool arises from the definition of an attribute space. The user can define an arbitrary dimensional attribute space (could be > 3D) and then select *traits* within this space. The corresponding distance field is then calculated which the user can view as a *direct volume rendered* field or as an isosurface. These isosurfaces correspond to *feature level sets*. For more info on these concepts, please refer [Feature level sets: Generalizing Isosurfaces to multivariate data](https://ieeexplore.ieee.org/document/8453863).

### Build
The tool could be built on windows/linux platforms. <br>
```bash
./configure
make [debug]
```

The [debug] flag is optional.<br>
For windows, this command can be run with msys/cygwin/mingw installed. <br>
On all platforms, the following dependencies are required. (configure script should ideally warn you otherwise..)
* GNU coreutils + bash shell
* g++ (Should support c++23 std and \<ranges\> functionality)
* gtkmm >= 4.0
* libepoxy

Once build is complete, the binary *mvf* should be available in project root
```bash
./mvf
```