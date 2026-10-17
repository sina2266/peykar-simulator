"""Execute the vendored GEARS network's forward/backward path on a synthetic graph."""

import argparse
import hashlib
import json
from pathlib import Path

import torch
from gears.model import GEARS_Model
from torch_geometric.data import Data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("."))
    args = parser.parse_args()
    args.output.mkdir(exist_ok=True, parents=True)
    torch.set_num_threads(1)
    torch.manual_seed(7)
    graph = torch.tensor([[0, 1, 2, 3, 1, 2, 3, 0], [1, 2, 3, 0, 0, 1, 2, 3]])
    weights = torch.ones(graph.shape[1])
    settings = {
        "num_genes": 4,
        "num_perts": 4,
        "hidden_size": 8,
        "uncertainty": False,
        "num_go_gnn_layers": 1,
        "decoder_hidden_size": 4,
        "num_gene_gnn_layers": 1,
        "no_perturb": False,
        "device": "cpu",
        "G_coexpress": graph,
        "G_coexpress_weight": weights,
        "G_go": graph,
        "G_go_weight": weights,
    }
    model = GEARS_Model(settings)
    data = Data(
        x=torch.arange(8, dtype=torch.float32).reshape(-1, 1) / 10,
        batch=torch.tensor([0, 0, 0, 0, 1, 1, 1, 1]),
        pert_idx=[[0], [1]],
    )
    target = data.x.reshape(2, 4) + 0.1
    optimizer = torch.optim.Adam(model.parameters(), lr=0.001)
    losses = []
    for _ in range(3):
        optimizer.zero_grad()
        prediction = model(data)
        assert prediction.shape == (2, 4) and torch.isfinite(prediction).all()
        loss = torch.nn.functional.mse_loss(prediction, target)
        loss.backward()
        assert all(torch.isfinite(p.grad).all() for p in model.parameters() if p.grad is not None)
        optimizer.step()
        losses.append(float(loss.detach()))
    output = {
        "losses": losses,
        "last_training_prediction": prediction.detach().tolist(),
        "input_kind": "Synthetic four-gene graph with random initialization",
    }
    artifact = args.output / "forward-backward.json"
    artifact.write_text(json.dumps(output, indent=2) + "\n")
    receipt = {
        "engine": "GEARS",
        "torch": torch.__version__,
        "device": "cpu",
        "native_execution": "passed",
        "source_commit": "f374e43e197b295016d80395d7a54ddb81cc6769",
        "training_steps": 3,
        "scientific_validity": "not_established",
        "pretrained_biological_model": False,
        "limitations": [
            "Network-gradient check; published perturbation benchmark reproduction is open.",
            "Cell-type transfer and tissue coupling remain unsupported without evidence.",
        ],
        "artifacts": {artifact.name: hashlib.sha256(artifact.read_bytes()).hexdigest()},
    }
    (args.output / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt))


if __name__ == "__main__":
    main()
