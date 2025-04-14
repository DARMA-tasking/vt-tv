# General imports
import os

# Trame imports
from trame.app import get_server
from trame.ui.vuetify import SinglePageWithDrawerLayout
from trame.widgets import vtk, html, vuetify, trame
from trame_vtk.modules.vtk.serializers import configure_serializer

# VTK imports
from vtkmodules.vtkCommonDataModel import (
    vtkPolyData,
    vtkDataObject)
from vtkmodules.vtkCommonTransforms import vtkTransform
from vtkmodules.vtkIOXML import vtkXMLPolyDataReader
from vtkmodules.vtkFiltersCore import vtkGlyph2D
from vtkmodules.vtkFiltersGeneral import vtkTransformPolyDataFilter
from vtkmodules.vtkFiltersSources import vtkGlyphSource2D
from vtkmodules.vtkInteractionWidgets import vtkScalarBarWidget
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkColorTransferFunction,
    vtkPolyDataMapper,
    vtkRenderer,
    vtkRenderWindow,
    vtkRenderWindowInteractor)
from vtkmodules.vtkRenderingAnnotation import vtkScalarBarActor
# Required for rendering initialization, not necessary for
# local rendering, but doesn't hurt to include it
import vtkmodules.vtkRenderingOpenGL2  # noqa

# MatPlotLib imports
import matplotlib.pyplot as plt

# Constants
CURRENT_DIRECTORY = os.path.abspath(os.path.dirname(__file__))
N_COLORS = 256

# Configure scene encoder
configure_serializer(encode_lut=True, skip_light=True)

# Global variables
dataset_arrays = []
rank_glyph = vtkGlyphSource2D()
ranks = vtkTransformPolyDataFilter()
rank_actor = vtkActor()
rank_bar = vtkScalarBarActor()
rank_mapper = vtkPolyDataMapper()
render_window = vtkRenderWindow()
interactor = vtkRenderWindowInteractor()
interactor.SetRenderWindow(render_window)

# Create global renderer with parallel projection
renderer = vtkRenderer()
renderer.AddActor(rank_actor)
renderer.AddActor(rank_bar)
renderer.SetBackground(1.0, 1.0, 1.0)
renderer.GetActiveCamera().ParallelProjectionOn()

# Enumeration of representation types
class Representation:
    Surface = 0
    SurfaceWithEdges = 1

# Enumeration of color maps
class ColorMap:
    Default = 0
    Blue_to_Red = 1
    White_to_Black = 2

# Trame
server = get_server(client_type="vue2")
state, ctrl = server.state, server.controller
state.setdefault("active_ui", None)

def actives_change(ids):
    """ Selection change"""
    _id = ids[0]
    if _id == "1":  # Mesh
        state.active_ui = "mesh"
    else:
        state.active_ui = "nothing"


def visibility_change(event):
    """ Visibility change"""
    _id = event["id"]
    _visibility = event["visible"]

    if _id == "1":
        # Rank mesh
        rank_actor.SetVisibility(_visibility)
    ctrl.view_update()


def update_representation(actor, mode):
    """ Define supported representation types"""
    property = actor.GetProperty()
    if mode == Representation.Surface:
        property.SetRepresentationToSurface()
        property.SetPointSize(1)
        property.EdgeVisibilityOff()
    elif mode == Representation.SurfaceWithEdges:
        property.SetRepresentationToSurface()
        property.SetPointSize(1)
        property.EdgeVisibilityOn()


@state.change("mesh_representation")
def update_mesh_representation(mesh_representation, **kwargs):
    """ Representation callback"""
    update_representation(rank_actor, mesh_representation)
    ctrl.view_update()


# Color By Callbacks
def color_by_array(actor, scalar_bar, array):
    # Change QOI to which mapper maps color
    mapper = actor.GetMapper()
    qoi = array.get("text")
    mapper.SelectColorArray(qoi)
    mapper.SetScalarRange(array.get("range"))
    mapper.GetLookupTable().SetRange(array.get("range"))

    # Cleanly update scalar bar title
    scalar_bar.SetTitle(qoi.title().replace('_', ' '))

@state.change("mesh_color_array_idx")
def update_mesh_color_by_name(mesh_color_array_idx, **kwargs):
    array = dataset_arrays[mesh_color_array_idx]
    color_by_array(rank_actor, rank_bar, array)
    ctrl.view_update()


# Color map callbacks
def use_colormap(actor, colormap):
    # Retrieve current array range and midpoint
    rng = state.array.get("range")
    midpoint = (rng[0] + rng[1]) * .5

    # Build desired color transfer function
    ctf = vtkColorTransferFunction()
    if colormap == ColorMap.Default:
        # Default color spectrum from green to orange via yellow
        ctf.SetColorSpaceToRGB()
        ctf.SetBelowRangeColor(0.8, 0.8, .8)
        ctf.AddRGBPoint(rng[0], .431, .761, .161)
        ctf.AddRGBPoint(midpoint, .98, .992, .059)
        ctf.AddRGBPoint(rng[1], 1.0, .647, 0.0)
        ctf.SetAboveRangeColor(1.0, 0.0, 1.0)
    elif colormap == ColorMap.Blue_to_Red:
        ctf.SetColorSpaceToDiverging()
        ctf.SetBelowRangeColor(0.0, 1.0, 0.0)
        ctf.AddRGBPoint(rng[0], .231, .298, .753)
        ctf.AddRGBPoint(midpoint, .865, .865, .865)
        ctf.AddRGBPoint(rng[1], .906, .016, .109)
        ctf.SetAboveRangeColor(1.0, 0.0, 1.0)
    elif colormap == ColorMap.White_to_Black:
        ctf.SetColorSpaceToRGB()
        ctf.SetBelowRangeColor(0.0, 0.0, 1.0)
        ctf.AddRGBPoint(rng[0], 1.0, 1.0, 1.0)
        ctf.RemovePoint(midpoint)
        ctf.AddRGBPoint(rng[1], 0.0, 0.0, 0.0)
        ctf.SetAboveRangeColor(1.0, 0.0, 0.0)

    # Convert and pass to mapper as lookup table
    lut = actor.GetMapper().GetLookupTable()
    lut.SetRange(rng)
    lut.SetNumberOfTableValues(N_COLORS)
    lut.SetNanColor(1., 1., 1., 0.)
    lut.UseBelowRangeColorOn()
    lut.UseAboveRangeColorOn()
    lut.Build()
    fac = (rng[1] - rng[0]) / (N_COLORS - 1)
    x = rng[0]
    for i in range(N_COLORS):
        lut.SetTableValue(i, *ctf.GetColor(x), 1.0)
        x += fac

@state.change("mesh_colormap")
def update_mesh_colormap(mesh_colormap, **kwargs):
    use_colormap(rank_actor, mesh_colormap)
    ctrl.view_update()

@state.change("mesh_scale")
def update_mesh_scale(mesh_scale, **kwargs):
    """ Mesh scale callback"""
    rank_glyph.SetScale(mesh_scale)

    # Forcing passing of data which cannot be done earlier due to glyphing
    ranks.Update()
    ranks.GetOutput().GetCellData().ShallowCopy(state.rank_data)
    ctrl.view_update()

@state.change("mesh_opacity")
def update_mesh_opacity(mesh_opacity, **kwargs):
    """ Opacity callback"""
    rank_actor.GetProperty().SetOpacity(mesh_opacity)
    ctrl.view_update()

def standard_buttons():
    """ Define standard buttons"""
    vuetify.VCheckbox(
        v_model="$vuetify.theme.dark",
        on_icon="mdi-lightbulb-off-outline",
        off_icon="mdi-lightbulb-outline",
        classes="mx-1",
        hide_details=True,
        dense=True,
    )
    vuetify.VCheckbox(
        v_model=("viewMode", "local"),
        on_icon="mdi-lan-disconnect",
        off_icon="mdi-lan-connect",
        true_value="local",
        false_value="remote",
        classes="mx-1",
        hide_details=True,
        dense=True,
    )
    with vuetify.VBtn(icon=True, click="$refs.view.resetCamera()"):
        vuetify.VIcon("mdi-crop-free")


def pipeline_widget():
    trame.GitTree(
        sources=(
            "pipeline",
            [
                {"id": "1", "parent": "0", "visible": 1, "name": "Rank Mesh"},
            ],
        ),
        actives_change=(actives_change, "[$event]"),
        visibility_change=(visibility_change, "[$event]"),
    )


def ui_card(title, ui_name):
    with vuetify.VCard(v_show=f"active_ui == '{ui_name}'"):
        vuetify.VCardTitle(
            title,
            classes="grey lighten-1 py-1 grey--text text--darken-3",
            style="user-select: none; cursor: pointer",
            hide_details=True,
            dense=True)
        content = vuetify.VCardText(classes="py-2")
    return content


def mesh_card():
    with ui_card(title="Mesh", ui_name="mesh"):
        vuetify.VSelect(
            # Representation
            v_model=("mesh_representation", Representation.Surface),
            items=(
                "representations",
                [{"text": "Surface", "value": 0},
                 {"text": "SurfaceWithEdges", "value": 1}]),
            label="Representation",
            hide_details=True,
            dense=True,
            outlined=True,
            classes="pt-1")
        with vuetify.VRow(classes="pt-2", dense=True):
            with vuetify.VCol(cols="6"):
                vuetify.VSelect(
                    # Color By
                    label="Color by",
                    v_model=("mesh_color_array_idx", 0),
                    items=("array_list", dataset_arrays),
                    hide_details=True,
                    dense=True,
                    outlined=True,
                    classes="pt-1")
            with vuetify.VCol(cols="6"):
                vuetify.VSelect(
                    # Color Map
                    label="Colormap",
                    v_model=("mesh_colormap", ColorMap.Default),
                    items=(
                        "colormaps",
                        [{"text": "Default", "value": 0},
                         {"text": "Blue to Red", "value": 1},
                         {"text": "White to Black", "value": 2}]),
                    hide_details=True,
                    dense=True,
                    outlined=True,
                    classes="pt-1")
        vuetify.VSlider(
            # Scale
            v_model=("mesh_scale", 0.95),
            min=0,
            max=1,
            step=0.05,
            label="Scale",
            classes="mt-1",
            hide_details=True,
            dense=True)
        vuetify.VSlider(
            # Opacity
            v_model=("mesh_opacity", 1.0),
            min=0,
            max=1,
            step=0.05,
            label="Opacity",
            classes="mt-1",
            hide_details=True,
            dense=True)


def get_mesh(filename):
    """ Read mesg from filename"""
    # Read Data
    reader = vtkXMLPolyDataReader()
    reader.SetFileName(os.path.join(CURRENT_DIRECTORY, filename))
    reader.Update()
    return reader.GetOutput()

def create_rendering_pipeline():
    # Initialize render window and interactor
    render_window.AddRenderer(renderer)

    # Extract rank data information
    default_id = 0
    rank_mesh = get_mesh("../data/synthetic-dataset-blocks_rank_mesh_0.vtp")
    state.rank_data = rank_mesh.GetPointData()
    for i in range(state.rank_data.GetNumberOfArrays()):
        array = state.rank_data.GetArray(i)
        array_range = array.GetRange()
        if (qoi := array.GetName()) == "load":
            default_id = i
        dataset_arrays.append({
            "text": qoi,
            "value": i,
            "range": list(array_range)})
    state.array = dataset_arrays[default_id]

    # Create square glyphs at ranks
    rank_glyph.SetGlyphTypeToSquare()
    rank_glyph.FilledOn()
    rank_glyph.CrossOff()
    rank_glypher = vtkGlyph2D()
    rank_glypher.SetSourceConnection(rank_glyph.GetOutputPort())
    rank_glypher.SetInputData(rank_mesh)
    rank_glypher.SetScaleModeToDataScalingOff()

    # Lower glyphs slightly for visibility and pass point data
    z_lower = vtkTransform()
    z_lower.Translate(0.0, 0.0, -0.01)
    ranks.SetTransform(z_lower)
    ranks.SetInputConnection(rank_glypher.GetOutputPort())

    # Initialize mapper for rank glyphs
    rank_mapper.SetInputConnection(ranks.GetOutputPort())
    rank_mapper.SelectColorArray(state.array.get("text"))
    rank_mapper.SetScalarModeToUseCellFieldData()
    rank_mapper.SetScalarVisibility(True)
    rank_mapper.SetUseLookupTableScalarRange(True)

    # Set up rank actor
    rank_actor.SetMapper(rank_mapper)
    rank_actor.GetProperty().SetRepresentationToSurface()
    rank_actor.GetProperty().SetPointSize(1)
    rank_actor.GetProperty().EdgeVisibilityOff()

    # Scalar bar actor
    rank_bar.SetLookupTable(rank_mapper.GetLookupTable())
    rank_bar.SetTitle(state.array.get("text"))
    rank_bar.SetNumberOfLabels(6)
    rank_bar.DrawTickLabelsOn()
    rank_bar.UnconstrainedFontSizeOn()
    rank_bar.SetHeight(0.08)
    rank_bar.SetWidth(0.42)
    rank_bar.SetBarRatio(0.3)
    rank_bar.DrawTickLabelsOn()
    rank_bar.GetTitleTextProperty().SetColor(0.0, 0.0, 0.0)
    rank_bar.GetLabelTextProperty().SetColor(0.0, 0.0, 0.0)
    rank_bar_widget = vtkScalarBarWidget()
    rank_bar_widget.SetInteractor(interactor)
    rank_bar_widget.SetScalarBarActor(rank_bar)
    rank_bar_widget.On()

    # Initialize renderer
    renderer.ResetCamera()


if __name__ == "__main__":
    # Create rendering pipeline
    create_rendering_pipeline()

    # Launch GUI
    with SinglePageWithDrawerLayout(server) as layout:
        # Layout title
        layout.title.set_text("vt-tv-web")

        # Layout toolbar
        with layout.toolbar:
            # toolbar components
            vuetify.VSpacer()
            vuetify.VDivider(vertical=True, classes="mx-2")
            standard_buttons()

        # Layout drawer
        with layout.drawer as drawer:
            # drawer components
            drawer.width = 325
            pipeline_widget()
            vuetify.VDivider(classes="mb-2")
            mesh_card()

        # Layout footer
        layout.footer.hide()

        with layout.content:
            # Content components
            with vuetify.VContainer(
                fluid=True,
                classes="pa-0 fill-height",
            ):
                view = vtk.VtkRemoteLocalView(
                    render_window, interactor, namespace="view", mode="local", interactive_ratio=1)
                ctrl.view_update = view.update
                ctrl.view_reset_camera = view.reset_camera

    # Start server
    server.start()
