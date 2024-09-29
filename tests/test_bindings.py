"""This module calls vttv module to test that vttv bindings work as expected"""
import os
import subprocess
import json
import sys
import yaml
import vttv

# source dir is the directory a level above this file
source_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Read the YAML config file
with open(f"{source_dir}/tests/test_bindings_conf.yaml", 'r', encoding="utf-8") as stream:
    try:
        params = yaml.safe_load(stream)
    except yaml.YAMLError as exc:
        print(exc)

# Check main key is "visualization"
if "visualization" not in params:
    print(
        "The YAML configuration file is not valid: "+\
        "missing required paramater \"visualization\""
    )
    sys.exit(1)

# make output_visualization_dir directory parameter absolute
if "output_visualization_dir" in params["visualization"]:
    if not os.path.isabs(params["visualization"]["output_visualization_dir"]):
        params["visualization"]["output_visualization_dir"] = source_dir + \
            "/" + params["visualization"]["output_visualization_dir"]

# Serialize visualization parameters
params_serialized = yaml.dump(params["visualization"])

# Calculate n_ranks
n_ranks = params["visualization"]["x_ranks"] * \
    params["visualization"]["y_ranks"] * params["visualization"]["z_ranks"]

rank_data = []
for rank in range(n_ranks):
    # JSON data file for rank
    datafile = f'{source_dir}/data/lb_test_data/data.{rank}.json'

    # Check JSON schema validity
    IS_VALID: bool
    try:
        p = subprocess.run([
                'python',
                os.path.join(source_dir, 'scripts/json_datafile_validator.py'),
                "--file_path=" + datafile
            ], check=True, capture_output=True)
        p.check_returncode()
        IS_VALID = True
    except subprocess.CalledProcessError as e:
        IS_VALID = False
        print(e.output.decode() + f"[JSON data file invalid] {datafile}")

    # If validation failed
    if IS_VALID is False:
        sys.exit(1)

    # Read JSON data file
    with open(datafile, 'r', encoding="utf-8") as f:
        data = json.load(f)
    data_serialized = json.dumps(data)

    # Add serialized data into the rank
    rank_data.append((json.dumps(data)))

# Launch VT TV from JSON data
vttv.tvFromJson(rank_data, params_serialized, n_ranks)
