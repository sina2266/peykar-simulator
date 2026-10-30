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

- Model 1.1: [`model-1.1/`](model-1.1/README.md), on generic eye model version 2
  (`peykar.model_learning.eye_recovery_population2`) with a declared wear-off of one
  dexamethasone implant and forecast intervals from the pooled study noise. Tested
  once on tranche 2 (reserved before triage; 63 records): H1–H4 passed on 92
  group-mean series (coverage 84.4%/89.6%, next observation 7.91 vs persistence
  8.81 pp); H5 failed, because model 1.0.0 with the same training forecast slightly
  better (7.79 pp) while covering only 66.0%/77.2% ([Persian results](model-1.1/results.fa.md)).

Tranches 1 and 2 are now spent for testing (the dataset and tranche-2 READMEs are
pinned by protocols 1.0.0 and 1.1.0 and stay unchanged). A later version must reserve
one of the unread tranches 3–4 (155 articles) before triage and be tested on it.

## Model 1.2 (development only, 2026-10-29)

Leave-one-study-out development on the 128 studies of tranches 1 and 2 tested a
prior-treatment level, an OCT-device level and a regimen level against 1.1.0. None
gained 0.10 pp (the regimen level, declared from an out-of-sample bias by regimen,
gained 0.012 pp and left those biases almost unchanged), so **no 1.2 protocol was
frozen**. Tranche 3 (67 extracted records) stays sealed and unread for a later version;
see the [development log](model-1.2/development-log.md).
## Model 1.3 (forecast stacking, 2026-10-30)
[`model-1.3/`](model-1.3/README.md): model 1.1.0's fit with forecast stacking
(`eye_stacking` 1.0.0: fixed weights 0.742/0.233/0.025 on the model, persistence and
the pooled-curve increment, and 0.839/0.161 on the model and the pooled training curve
for initial states, fitted on the 128 training studies' leave-one-study-out forecasts).
Tested once on tranche 3 (67 records): H2–H4 passed on 96 group-mean series (next
observation 5.70 vs persistence 6.43 pp; landmark 7.72 vs 10.41 for the pooled-curve
increment; initial state 11.62 vs the pooled training curve 15.44); H1 failed (80%
coverage 0.911, above its window) and H5 failed (initial state 11.62 vs 11.61 for model
1.1.0 from the same fit; next observation and landmark were better, 5.70 vs 6.08 and
7.72 vs 7.98) ([Persian results](model-1.3/results.fa.md)). Model 1.1.0 stays the
component's model; on tranche 3 its own 80% coverage was 0.903, just above the window.
Tranche 3 is now consumed; tranche 4 (35 articles) stays untriaged and unread.

## Model 1.4 (history-only stacking, scaled intervals, 2026-10-30)

[`model-1.4/`](model-1.4/README.md): model 1.1.0's fit with stacking 2.0.0
(`eye_stacking2`): only the forecasts from follow-up are stacked (fixed weights
0.729/0.212/0.058 on the model, persistence and the pooled-curve increment), initial
states stay model 1.1.0's own, and each stacked interval is model 1.1.0's scaled
around the stacked point (0.79 for the next observation, 1.00 for the landmark),
fitted on the 195 training studies of tranches 1–3. Tested once on tranche 4 (17
records, 24 group-mean series): H1 passed (coverage 0.793/0.839), H2 passed (next
observation 5.84 vs persistence 7.00 pp) and H4 passed (initial state 10.98 vs 15.14);
H3 failed (landmark 10.58 vs the pooled-curve increment 9.80) and H5 failed (landmark
10.58 vs model 1.1.0's 9.99 from the same fit) ([Persian results](model-1.4/results.fa.md)).
On tranche 4 thickness kept changing after the first post-treatment visit, so the
persistence weight hurt the long-range forecasts. Model 1.1.0 stays the component's
model; its own H3 also fails on tranche 4 (9.99 vs 9.80). Tranche 4 is consumed and no
unread article of this dataset remains.
