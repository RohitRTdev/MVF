import vtk

def convert_vti_to_vtk(input_path, output_path):
    reader = vtk.vtkXMLImageDataReader()
    reader.SetFileName(input_path)
    reader.Update()
    image_data = reader.GetOutput()

    # StructuredPoints corresponds to ImageData in legacy VTK 
    writer = vtk.vtkStructuredPointsWriter()  
    writer.SetFileName(output_path)
    writer.SetInputData(image_data)

    writer.SetFileTypeToASCII()

    writer.Write()


output_vtk = "dataset/cylinder.vtk"
convert_vti_to_vtk(input_path="dataset/cylinder2d.vti", output_path=output_vtk)
print("Conversion complete:", output_vtk)
