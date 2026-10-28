# Central retinal thickness after treatment — fifth eye component

How the central thickness of the retina (micrometres, by OCT) changes after macular
edema, neovascular AMD and related conditions are treated, as a fraction of each
series' pretreatment value. It is the usual anatomical outcome of anti-VEGF
injections, steroid implants and injections, laser, photodynamic therapy and
vitrectomy, and it reflects fluid in and under the retina.

- Dataset: [`experiments/data/retina-cst-2026-10-28`](../../data/retina-cst-2026-10-28/README.md)
  (3,128 screened open-access articles, 395 passed a masked-table rule, tranche 1 of
  120 triaged blind, 78 admitted, split 54/24 before any value was read; 46
  development records with 98 series and 19 custodian-extracted sealed records with
  82 series; tranches 2–4 with 275 articles stay unread).
- Model 1.0: [`model-1.0/`](model-1.0/README.md), the generic eye recovery
  population model (`peykar.model_learning.eye_recovery_population`, the generic form
  of IOP model 1.1) with persistent deviations from the law in forecasting. Its
  one-shot sealed test passed all five declared hypotheses on 40 group-mean series
  (next observation 5.75 vs persistence 6.34 pp; [Persian results](model-1.0/results.fa.md)).

The tranche-1 development and sealed sets are now spent for testing (the dataset
README is pinned by protocol 1.0.0 and stays unchanged). A later version must
reserve one of the unread tranches 2–4 before triage and be tested on it.
