import numpy as np

def raw_to_vtk(raw_filename, vtk_filename, field_name, dims, spacing, origin, dtype=np.uint16):
    """
    Convert a binary .raw file into an ASCII VTK structured points file.

    Parameters:
        raw_filename: str  -> path to the input .raw file
        vtk_filename: str  -> path to output .vtk file
        field_name: str   ->  name displayed when rendering this file
        dims: tuple(int,int,int)  -> (nx, ny, nz)
        spacing: tuple(float,float,float)
        origin: tuple(float,float,float)
        dtype: numpy dtype (default: np.float64)
    """
    nx, ny, nz = dims
    num_points = nx * ny * nz

    data = np.fromfile(raw_filename, dtype=dtype)
    if data.size != num_points:
        raise ValueError(
            f"Data size mismatch: expected {num_points} elements, got {data.size}"
        )

    with open(vtk_filename, "w") as vtk:
        vtk.write("# vtk DataFile Version 5.1\n")
        vtk.write("vtk output\n")
        vtk.write("ASCII\n")
        vtk.write("DATASET STRUCTURED_POINTS\n")
        vtk.write(f"DIMENSIONS {nx} {ny} {nz}\n")
        vtk.write(f"SPACING {spacing[0]} {spacing[1]} {spacing[2]}\n")
        vtk.write(f"ORIGIN {origin[0]} {origin[1]} {origin[2]}\n")
        vtk.write(f"POINT_DATA {num_points}\n")
        vtk.write("FIELD FieldData 1\n")
        vtk.write(f"{field_name} 1 {num_points} double\n")

        for i in range(0, num_points, 9):
            chunk = data[i:i+9]
            vtk.write(" ".join(map(str, chunk)) + "\n")

    print(f"Wrote VTK file: {vtk_filename}")

if __name__ == "__main__":
    raw_to_vtk(
        raw_filename="redseasmall/beechnut_1024x1024x1546_uint16.raw",
        vtk_filename="redseasmall/beechnut.vtk",
        field_name="BEECHNUT",
        dims=(1024, 1024, 1546),
        spacing=(0.04, 0.04, 0.04),
        origin=(41.9871, 10.0271, -20.0),
        dtype=np.uint16 
    )
