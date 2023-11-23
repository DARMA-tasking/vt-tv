import json
import yaml
import vttv

import sys
import os

# source dir is the directory a level above this file
source_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

with open(f'{source_dir}/tests/unit/lb_test_data/data.0.json', 'r') as f:
  data = json.load(f)

with open(f'{source_dir}/config/conf.yaml', 'r') as stream:
  try:
    params = yaml.safe_load(stream)
  except yaml.YAMLError as exc:
    print(exc)


data_serialized = json.dumps(data)
params_serialized = yaml.dump(params)

vttv.tv_from_json(data_serialized, params_serialized)
