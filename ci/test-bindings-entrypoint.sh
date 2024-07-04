# X11 display
Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &

# Prepare conda environment
. /opt/conda/etc/profile.d/conda.sh && conda activate deves
pip install PyYAML
pip install /opt/src/vt-tv

