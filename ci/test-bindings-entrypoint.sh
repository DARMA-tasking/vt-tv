Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &
. /opt/conda/etc/profile.d/conda.sh && conda activate deves && pip install /opt/src/vt-tv && pip install PyYAML
