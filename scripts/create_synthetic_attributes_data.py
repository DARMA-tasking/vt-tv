import json
import brotli
import os

def get_attributes_dict(id):
    attributes = {
        "doubleAttribute": float(id) * 3.14,
        "elementIDAttribute": id + 30000000000,
        "intAttribute": id,
        "stringAttribute": f"id is {id}"
    }
    return attributes

project_dir = os.path.dirname(os.path.dirname(__file__))
test_data_dir = os.path.join(project_dir, "tests", "data", "lb_test_data")

output_dir = os.path.join(project_dir, "tests", "data", "synthetic_attributes")
os.makedirs(output_dir, exist_ok=True)

# Loop through all ranks
num_ranks = 4
for rank_id in range(num_ranks):
    json_file = os.path.join(test_data_dir, f"data.{rank_id}.json")
    with open(json_file) as f:
        json_data = json.load(f)

    # Add rank attributes
    rank_attributes = get_attributes_dict(rank_id)
    json_data["metadata"]["attributes"] = rank_attributes

    # Then object attributes
    for phase in json_data["phases"]:
        for task in phase["tasks"]:
            task_id = task["entity"]["id"]
            attributes = get_attributes_dict(task_id)
            task["attributes"] = attributes

    # Write out json
    output_file = os.path.join(output_dir, f"data.{rank_id}.json")
    with open(output_file, "w") as out_json:
        json.dump(json_data, out_json)

    # Write out the compressed json
    compressed_output_file = os.path.join(output_dir, f"data.{rank_id}.json.br")
    with open(compressed_output_file, "wb") as out_br:
        compressed_data = brotli.compress(json.dumps(json_data).encode('utf-8'))
        out_br.write(compressed_data)
