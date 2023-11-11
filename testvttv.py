import json
import yaml
import vttv

with open('/Users/pierrepebay/Develop/vt-tv/tests/unit/lb_test_data/data.0.json') as f:
  data = json.load(f)


with open("/Users/pierrepebay/Develop/vt-tv/config/conf.yaml", "r") as stream:
  try:
    params = yaml.safe_load(stream)
  except yaml.YAMLError as exc:
    print(exc)


data_serialized = json.dumps(data)
params_serialized = yaml.dump(params)

vttv.tv_from_json(data_serialized, params_serialized)