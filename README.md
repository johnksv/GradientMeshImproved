# A Gradient Mesh Tool for Non-Rectangular Gradient Meshes
This is a research project at HiOA (Oslo and Akershus University College of Applied Sciences).

## Our work has been accepted to [SIGGRAPH 2017](http://s2017.siggraph.org/)
- ACM Digital Library: [doi 10.1145/3102163.3102172 ](https://doi.org/10.1145/3102163.3102172)
- [Paper](https://svergja.no/wp-content/uploads/2017/07/GradientMeshTool.pdf)
- [Poster](https://svergja.no/wp-content/uploads/2017/07/Poster.pdf)
- [Video](https://www.youtube.com/watch?v=N3q8WE9iYZ4)
- [Informal user study](https://svergja.no/wp-content/uploads/2017/07/Informal-user-study.pdf)

## Executable
[Windows](https://github.com/johnksv/GradientMeshImproved/releases)

## Implementation details
This is a C++ project using Qt 5.6 Widgets.
The project is coded with the QtCreator IDE (project file included).
The code is developed and tested with the Visual Studio 2015 compiler.

Required libraries:
* Qt 5.6
* OpenMesh 6.3 ([http://www.openmesh.org](http://www.openmesh.org/))

## Getting Started
- Clone this repo
- Modify line 9 in GradientMeshImproved.pro to point to your OpenMesh installation
- Build and run

## How to
All tools are point-and-click.

### Adding first face
With line tool (default):
Add control points by left clicking on the canvas.
Close the face by selecting the first control point.

With rectange tool:
Place the top-left control point, then place the bottom-right control point.

### Adding faces
Select an existing control point and place new ones on the canvas. The face is closed by clicking on an existing
control point. A non-manifold mesh can be achieved if the initial control point closes the face (or by later deleting faces).

### Choosing colour
We offer up to 3 colours one can quick pick from. To select the colours, click on the colour, or use their associated hotkeys (`1` for primary, `2` for secondary, `3` for tertiary).

When a colour is choosen, the colour dialog will be displayed.
Tick the `Lock colour changes`checkbox (or press `4`) if you don't want to display the colour dialog every time you change colour.

### Change colour of existing control point
Select desired control point and select the new colour from the colour menu.

Or:

Right-click on the control point and select `Choose color`.

### Deleting faces
Rigt click on the desired face, and choose "Delete face".

### Move and select
Multiple control points can be selected by holding down `ctrl`.
To drag and select control points, hold down `shift`.
One can also add contorl points selected by draging by holding down `shift+ctrl` .

### Edge split tool
Click on the desired location along the edge to insert a new control point.

### Collapse edge tool
_Removes_ a control point by collapsing into an adjacent control point.
NB: Constraints for the new edge is not preserved.

### Saving all layers
When saving all layers, a suffix will be added to the file name.

### Importing
Multiple layers can be imported as once.
The order of the layers will then be in the order they were imported.

NB: It is necessary to `Render mesh` after importing.

### Handy shortcuts
Shortcut   |   Action
---   |   ---
` 1 `   |   Select primary color
` 2 `   |   Select secondary color
` 3 `   |   Select tertiary color
` 4 `   |   Lock color change
` esc `   |   Cancel current drawing
` delete `   |   Delete everything on layer
` ctrl+q `   |   Move and Select
` ctrl+w `   |   Rectangle Tool
` ctrl+e `   |   Line tool
` ctrl+r `   |   Edge split tool
` ctrl+r `   |   Collapse Edge
` ctrl+z `   |   Undo
` ctrl+x `   |   Show/hide face overlay
` ctrl+c `   |   Show/hide constraint handlers
` ctrl+v `   |   Turn on/off auto rendering (can be expensive for big meshes)
` ctrl+b `   |   Toggle Render only in split window

### Shortcomings
Undo is only partially supported, and not for every action that is possible.
User interface in general
Deletion of faces: the click must be done inside without concerns to the edges (because the edges are rendered at runtime, the underlying  mesh has no knowledge of it).
