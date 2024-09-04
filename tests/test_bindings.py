import json
import yaml
import vttv

import sys
import os

# source dir is the directory a level above this file
source_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

with open(f'{source_dir}/tests/test_bindings_conf.yaml', 'r') as stream:
  try:
    params = yaml.safe_load(stream)
  except yaml.YAMLError as exc:
    print(exc)

# make output_visualization_dir directory parameter absolute
params["visualization"]["output_visualization_dir"] = os.path.abspath(params["visualization"]["output_visualization_dir"])

params_serialized = yaml.dump(params["visualization"])

n_ranks = params["visualization"]["x_ranks"] * params["visualization"]["y_ranks"] * params["visualization"]["z_ranks"]
rank_data = []

for rank in range(n_ranks):
  with open(f'{source_dir}/data/lb_test_data/data.{rank}.json', 'r') as f:
    data = json.load(f)

  data_serialized = json.dumps(data)

  rank_data.append((data_serialized))

vttv.tvFromJson(rank_data, params_serialized, n_ranks)
