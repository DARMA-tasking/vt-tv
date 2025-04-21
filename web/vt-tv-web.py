# General imports
import os
import re
from tkinter import filedialog, Tk

# Trame imports
from trame.app import get_server
from trame.ui.vuetify import SinglePageWithDrawerLayout
from trame.ui.html import DivLayout
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
import vtkmodules.vtkRenderingOpenGL2

# MatPlotLib imports
import matplotlib.pyplot as plt

# Constants
BASE_DIR = os.path.join(
    os.path.abspath(os.path.dirname(__file__)), "../data")
N_COLORS = 256

# Configure scene encoder
configure_serializer(encode_lut=True, skip_light=True)

# Global variables
_ui = {
    "view": None}
_pipeline = {
    "rank_glyph": vtkGlyphSource2D(),
    "ranks": vtkTransformPolyDataFilter(),
    "rank_actor": vtkActor(),
    "rank_bar": vtkScalarBarActor(),
    #"rank_bar_widget": vtkScalarBarWidget(),
    "object_actor": vtkActor(),
    "renderer": vtkRenderer(),
    "render_window": vtkRenderWindow(),
    "interactor": vtkRenderWindowInteractor()}
_data = {
    # vtkPointData instances
    "rank_mesh": None,
    "object_mesh": None}

# Tkinter
root = Tk()
root.withdraw()
root.lift()
root.attributes("-topmost", True)
root.wm_attributes("-topmost", True)

# Trame
server = get_server(client_type="vue2")
state, ctrl = server.state, server.controller

# Initialize state
state.data_dir = os.path.join(
    os.path.abspath(os.path.dirname(__file__)), "../data")
state.vtp_files = []
state.rank_file = None
state.rank_arrays = []
state.rank_qoi = None
state.rank_scale = 0.95
state.object_file = None
state.object_arrays = []
state.object_qoi = None
state.object_scale = 1.0
class ColorMap:
    Default = 0
    Blue_to_Red = 1
    White_to_Black = 2
state.rank_colormap = ColorMap.Default
state.object_colormap = ColorMap.Blue_to_Red
class Representation:
    Surface = 0
    SurfaceWithEdges = 1
state.rank_representation = Representation.Surface
state.object_representation = Representation.SurfaceWithEdges

# GUI state variable
state.setdefault("active_ui", None)

def actives_change(ids):
    """ Selection change"""
    _id = ids[0]
    if _id == "1":
        state.active_ui = "ranks"
    elif _id == "2":
        state.active_ui = "objects"
    else:
        state.active_ui = "nothing"

def visibility_change(event):
    """ Visibility change"""
    _id = event["id"]
    _visibility = event["visible"]

    if _id == "1":
        _pipeline["rank_actor"].SetVisibility(_visibility)
    elif _id == "2":
        _pipeline["object_actor"].SetVisibility(_visibility)
    ctrl.view_update()

@ctrl.set("open_directory")
def open_directory():
    state.data_dir = filedialog.askdirectory(
        title="Select Directory",
        initialdir=state.data_dir)

@state.change("data_dir")
def find_vtp_files(**kwargs):
    """ Find all VTP files in current data folder"""
    if not os.path.exists(state.data_dir):
        state.vtp_files = []
        return
    state.vtp_files = sorted([
        f for f in os.listdir(state.data_dir) if f.lower().endswith(".vtp")])

@state.change("rank_file")
def on_rank_file_selected(**kwargs):
    # Punt if no rank file is provided
    if not state.rank_file:
        return

    # Retrieve rank mesh and data
    _data["rank_mesh"] = get_mesh(os.path.join(state.data_dir, state.rank_file))
    pd = _data["rank_mesh"].GetPointData()

    # Recreate dataset arrays de novo to ensure detection by Trame
    state.rank_arrays = [
        {"text": pd.GetArray(i).GetName(),
         "value": i,
         "range": list(pd.GetArray(i).GetRange())}
        for i in range(pd.GetNumberOfArrays())
        if pd.GetArray(i) and pd.GetArray(i).GetName()]

    # Invoke pipeline with data obtained from file
    update_rendering_pipeline()
    state.rank_color_array_idx = 0 if state.rank_arrays else None
    ctrl.view_update()

@state.change("object_file")
def on_object_file_selected(**kwargs):
    # Punt if no object file is provided
    if not state.object_file:
        return

    # Retrieve object mesh and data
    object_mesh = get_mesh(os.path.join(state.data_dir, state.object_file))
    pd = object_mesh.GetPointData()

    # Recreate dataset arrays de novo to ensure detection by Trame
    state.object_arrays = [
        {"text": pd.GetArray(i).GetName(),
         "value": i,
         "range": list(pd.GetArray(i).GetRange())}
        for i in range(pd.GetNumberOfArrays())
        if pd.GetArray(i) and pd.GetArray(i).GetName()]

    # Invoke pipeline with data obtained from file
    update_rendering_pipeline()
    state.object_color_array_idx = 0 if state.object_arrays else None
    ctrl.view_update()

def update_representation(actor, representation):
    """ Define supported representation types"""
    property = actor.GetProperty()
    if representation == Representation.Surface:
        property.SetRepresentationToSurface()
        property.SetPointSize(1)
        property.EdgeVisibilityOff()
    elif representation == Representation.SurfaceWithEdges:
        property.SetRepresentationToSurface()
        property.SetPointSize(1)
        property.EdgeVisibilityOn()

@state.change("rank_representation")
def update_rank_representation(rank_representation, **kwargs):
    """ Representation callback"""
    if not state.rank_file:
        return
    update_representation(_pipeline["rank_actor"], rank_representation)
    ctrl.view_update()

def color_by_array(actor, array):
    # Change QOI to which mapper maps color
    qoi = array.get("text")
    mapper = actor.GetMapper()
    mapper.SelectColorArray(qoi)
    mapper.SetScalarRange(array.get("range"))
    mapper.GetLookupTable().SetRange(array.get("range"))

    # Cleanly update scalar bar title
    _pipeline["rank_bar"] = vtkScalarBarActor()
    _pipeline["rank_bar"].SetTitle(qoi.title().replace('_', ' '))

@state.change("rank_color_array_idx")
def update_rank_color_by_name(rank_color_array_idx, **kwargs):
    if not state.rank_file:
        return
    array = state.rank_arrays[rank_color_array_idx]
    color_by_array(_pipeline["rank_actor"],array)
    ctrl.view_update()

# Color map callbacks
def apply_colormap(colormap, mapper):
    # Retrieve current array range and midpoint
    rng = state.rank_qoi.get("range") if state.rank_qoi else [0., 1.]
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
    lut = mapper.GetLookupTable()
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

@state.change("rank_colormap")
def update_rank_colormap(rank_colormap, **kwargs):
    """ Rank rank colormap callback"""
    if not(mapper := _pipeline["rank_actor"].GetMapper()):
        return
    apply_colormap(state.rank_colormap, mapper)
    ctrl.view_update()


@state.change("rank_scale")
def update_rank_scale(rank_scale, **kwargs):
    """ Rank mesh scale callback"""
    if not state.rank_file:
        return
    _pipeline["rank_glyph"].SetScale(rank_scale)

    # Forcing passing of data which cannot be done earlier due to glyphing
    _pipeline["ranks"].Update()
    _pipeline["ranks"].GetOutput().GetCellData().ShallowCopy(
        _data["rank_mesh"].GetPointData())
    ctrl.view_update()

@state.change("rank_opacity")
def update_rank_opacity(rank_opacity, **kwargs):
    """ Opacity callback"""
    if not state.rank_file:
        return
    _pipeline["rank_actor"].GetProperty().SetOpacity(rank_opacity)
    ctrl.view_update()

def left_buttons():
    """ Define left-side buttons"""
    with vuetify.VBtn(icon=True, click=ctrl.open_directory):
        vuetify.VIcon("mdi-folder-open")
    with vuetify.VBtn(icon=True, click="$refs.view.resetCamera()"):
        vuetify.VIcon("mdi-crop-free")

def right_buttons():
    """ Define right-side buttons"""
    vuetify.VCheckbox(
        v_model="$vuetify.theme.dark",
        on_icon="mdi-lightbulb-off-outline",
        off_icon="mdi-lightbulb-outline",
        classes="mx-1",
        hide_details=True,
        dense=True)
    vuetify.VCheckbox(
        v_model=("viewMode", "local"),
        on_icon="mdi-lan-disconnect",
        off_icon="mdi-lan-connect",
        true_value="local",
        false_value="remote",
        classes="mx-1",
        hide_details=True,
        dense=True)

def pipeline_widget():
    trame.GitTree(
        sources=(
            "pipeline", [
                {"id": "1", "parent": "0", "visible": 1, "name": "Ranks"},
                {"id": "2", "parent": "0", "visible": 1, "name": "Objects"}]),
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

def rank_card():
    with ui_card(title="Ranks", ui_name="ranks"):
        find_vtp_files()
        vuetify.VSelect(
            label="Mesh File",
            items=("vtp_files",),
            v_model=("rank_file", None),
            outlined=True,
            dense=True)
        vuetify.VSelect(
            # Representation
            v_model=("rank_representation", Representation.Surface),
            items=(
                "representations",
                [{"text": "Surface", "value": Representation.Surface},
                 {"text": "SurfaceWithEdges", "value": Representation.SurfaceWithEdges}]),
            label="Mesh Representation",
            hide_details=True,
            dense=True,
            outlined=True,
            classes="pt-1")
        with vuetify.VRow(classes="pt-2", dense=True):
            with vuetify.VCol(cols="6"):
                vuetify.VSelect(
                    # Color By
                    label="Color by",
                    v_model=("rank_color_array_idx", 0),
                    items=("rank_arrays", []),
                    hide_details=True,
                    dense=True,
                    outlined=True,
                    classes="pt-1")
            with vuetify.VCol(cols="6"):
                vuetify.VSelect(
                    # Color Map
                    label="Colormap",
                    v_model=("rank_colormap", ColorMap.Default),
                    items=(
                        "colormaps",
                        [{"text": "Default", "value": ColorMap.Default},
                         {"text": "Blue to Red", "value": ColorMap.Blue_to_Red},
                         {"text": "White to Black", "value": ColorMap.White_to_Black}]),
                    hide_details=True,
                    dense=True,
                    outlined=True,
                    classes="pt-1")
        vuetify.VSlider(
            # Scale
            v_model=("rank_scale", state.rank_scale),
            min=0,
            max=1,
            step=0.05,
            label="Scale",
            classes="mt-1",
            hide_details=True,
            dense=True)
        vuetify.VSlider(
            # Opacity
            v_model=("rank_opacity", 1.0),
            min=0,
            max=1,
            step=0.05,
            label="Opacity",
            classes="mt-1",
            hide_details=True,
            dense=True)

def object_card():
    with ui_card(title="Objects", ui_name="objects"):
        find_vtp_files()
        vuetify.VSelect(
            label="Mesh File",
            items=("vtp_files",),
            v_model=("object_file", None),
            outlined=True,
            dense=True)
        vuetify.VSelect(
            # Representation
            v_model=("object_representation", Representation.Surface),
            items=(
                "representations",
                [{"text": "Surface", "value": Representation.Surface},
                 {"text": "SurfaceWithEdges", "value": Representation.SurfaceWithEdges}]),
            label="Mesh Representation",
            hide_details=True,
            dense=True,
            outlined=True,
            classes="pt-1")
        with vuetify.VRow(classes="pt-2", dense=True):
            with vuetify.VCol(cols="6"):
                vuetify.VSelect(
                    # Color By
                    label="Color by",
                    v_model=("object_color_array_idx", 0),
                    items=("object_arrays", []),
                    hide_details=True,
                    dense=True,
                    outlined=True,
                    classes="pt-1")
            with vuetify.VCol(cols="6"):
                vuetify.VSelect(
                    # Color Map
                    label="Colormap",
                    v_model=("object_colormap", ColorMap.Default),
                    items=(
                        "colormaps",
                        [{"text": "Default", "value": ColorMap.Default},
                         {"text": "Blue to Red", "value": ColorMap.Blue_to_Red},
                         {"text": "White to Black", "value": ColorMap.White_to_Black}]),
                    hide_details=True,
                    dense=True,
                    outlined=True,
                    classes="pt-1")
        vuetify.VSlider(
            # Scale
            v_model=("object_scale", state.object_scale),
            min=0,
            max=1,
            step=0.05,
            label="Scale",
            classes="mt-1",
            hide_details=True,
            dense=True)
        vuetify.VSlider(
            # Opacity
            v_model=("object_opacity", 1.0),
            min=0,
            max=1,
            step=0.05,
            label="Opacity",
            classes="mt-1",
            hide_details=True,
            dense=True)

def find_file_stems():
    """Retrieve list of unique file stems from data directory"""
    # Return empty list if directory does not exist
    try:
        # Retrieve all files
        files = os.listdir(BASE_DIR)
    except FileNotFoundError:
        return []

    # Pattern to be matched as regular expression
    pattern = re.compile(r"^(.*?)(_\d+)?\.vtp$", re.IGNORECASE)

    # Iterate over files and keep those matching pattern
    stems = set()
    for filename in files:
        match = pattern.match(filename)
        if match:
            stems.add(match.group(1))

    # Return sorted list of matching stems
    return sorted(list(stems))

def get_mesh(filename):
    """ Read mesg from filename"""
    # Read Data
    reader = vtkXMLPolyDataReader()
    reader.SetFileName(os.path.join(BASE_DIR, filename))
    reader.Update()
    return reader.GetOutput()

def initialize_rendering_pipeline():
    """ Initialize VTK pipeline settings once and for all"""

    # Rank glyph settings
    _pipeline["rank_glyph"].SetGlyphTypeToSquare()
    _pipeline["rank_glyph"].FilledOn()
    _pipeline["rank_glyph"].CrossOff()
    _pipeline["rank_glyph"].SetScale(state.rank_scale)

    # Rank actor settings
    _pipeline["rank_actor"].GetProperty().SetRepresentationToSurface()
    _pipeline["rank_actor"].GetProperty().SetPointSize(1)
    _pipeline["rank_actor"].GetProperty().EdgeVisibilityOff()

    _pipeline["rank_bar"].SetOrientationToHorizontal()
    _pipeline["rank_bar"].GetPositionCoordinate().SetValue(0.5, 0.9, 0.0)
    _pipeline["rank_bar"].SetNumberOfLabels(6)
    _pipeline["rank_bar"].DrawTickLabelsOn()
    _pipeline["rank_bar"].UnconstrainedFontSizeOn()
    _pipeline["rank_bar"].SetHeight(0.15)
    _pipeline["rank_bar"].SetBarRatio(0.3)
    _pipeline["rank_bar"].DrawTickLabelsOn()
    _pipeline["rank_bar"].GetTitleTextProperty().SetColor(0.0, 0.0, 0.0)
    _pipeline["rank_bar"].GetLabelTextProperty().SetColor(0.0, 0.0, 0.0)

    # Renderer settings
    _pipeline["renderer"].AddActor(_pipeline["rank_actor"])
    _pipeline["renderer"].SetBackground(1.0, 1.0, 1.0)
    _pipeline["renderer"].GetActiveCamera().ParallelProjectionOn()

    # Render window and interactor
    _pipeline["render_window"].AddRenderer(_pipeline["renderer"])
    _pipeline["interactor"].SetRenderWindow(_pipeline["render_window"])


def update_rendering_pipeline():
    # Extract rank data information
    state.rank_qoi = state.rank_arrays[0]

    # Create square glyphs at ranks
    rank_glypher = vtkGlyph2D()
    rank_glypher.SetSourceConnection(_pipeline["rank_glyph"].GetOutputPort())
    rank_glypher.SetInputData(_data["rank_mesh"])
    rank_glypher.SetScaleModeToDataScalingOff()

    # Lower glyphs slightly for visibility and pass point data
    z_lower = vtkTransform()
    z_lower.Translate(0.0, 0.0, -0.01)
    _pipeline["ranks"].SetTransform(z_lower)
    _pipeline["ranks"].SetInputConnection(rank_glypher.GetOutputPort())
    _pipeline["ranks"].Update()
    # Forcing passing of data which cannot be done earlier due to glyphing
    _pipeline["ranks"].GetOutput().GetCellData().ShallowCopy(
        _data["rank_mesh"].GetPointData())

    # Create mapper for rank glyphs
    rank_mapper = vtkPolyDataMapper()
    rank_mapper.SetInputConnection(_pipeline["ranks"].GetOutputPort())
    rank_mapper.SelectColorArray(state.rank_qoi.get("text"))
    rank_mapper.SetScalarRange(state.rank_qoi.get("range"))
    rank_mapper.SetScalarModeToUseCellFieldData()
    rank_mapper.SetScalarVisibility(True)
    rank_mapper.SetUseLookupTableScalarRange(True)
    _pipeline["rank_actor"].SetMapper(rank_mapper)

    # Set up rank scalar bar actor
    _pipeline["rank_bar"].SetLookupTable(rank_mapper.GetLookupTable())
    _pipeline["rank_bar"].SetTitle(state.rank_qoi.get("text"))
    apply_colormap(state.rank_colormap, _pipeline["rank_actor"].GetMapper())
    _pipeline["renderer"].AddActor(_pipeline["rank_bar"])

    # Rank bar widget
    #_pipeline["rank_bar_widget"].SetInteractor(_pipeline["interactor"])
    #_pipeline["rank_bar_widget"].SetScalarBarActor(_pipeline["rank_bar"])
    #_pipeline["rank_bar_widget"].On()

    # Initialize renderer
    _pipeline["renderer"].ResetCamera()

if __name__ == "__main__":
    # Initialize VTK pipeline settings
    initialize_rendering_pipeline()
    
    # Launch GUI
    with SinglePageWithDrawerLayout(server) as layout:
        # Layout title
        layout.title.set_text("vt-tv-web")

        # Create toolbar layout
        with layout.toolbar:
            # Set up toolbar components
            vuetify.VDivider(vertical=True, classes="mx-4")
            left_buttons()
            vuetify.VDivider(vertical=True, classes="mx-4")
            vuetify.VSpacer()
            vuetify.VDivider(vertical=True, classes="mx-4")
            right_buttons()

        # Create drawer layout
        with layout.drawer as drawer:
            # InserInsert drawer components
            drawer.width = 320
            pipeline_widget()
            vuetify.VDivider(classes="mb-2")

            # Insert rank mesh card
            rank_card()

            # Insert objects mesh card
            object_card()

        # Create footer layout
        layout.footer.hide()
        with layout.content:
            # Content components
            with vuetify.VContainer(
                fluid=True,
                classes="pa-0 fill-height",
            ):
                _ui["view"] = vtk.VtkRemoteLocalView(
                    _pipeline["render_window"], _pipeline["interactor"],
                    namespace="view", mode="local", interactive_ratio=1)
                ctrl.view_update = _ui["view"].update
                ctrl.view_reset_camera = _ui["view"].reset_camera

    # Start server
    server.start()
