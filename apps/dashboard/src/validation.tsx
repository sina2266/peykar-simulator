import { useState } from 'react';
import { type CatalogItem, type Run, type Schema, ref, identity, versionPath } from './api';
import { useSession } from './session';
import { useT } from './i18n';
import {
  useResource,
  useAction,
  Boundary,
  PageHeading,
  Tabs,
  Field,
  Empty,
  Badge,
  Meta,
  JsonDetails,
  Evidence,
  Limitations,
  ErrorBox,
  JsonForm,
  Pager,
} from './ui';

export function Validation() {
  const { client, admin } = useSession();
  const t = useT();
  const [tab, setTab] = useState('Evaluation sets');
  const [evaluationRevision, setEvaluationRevision] = useState(0);
  const [offset, setOffset] = useState(0);
  const [selected, setSelected] = useState<CatalogItem>();
  const resource = useResource(
    async (signal) =>
      tab === 'Knowledge gaps'
        ? await client.request<Schema['KnowledgeGap'][]>(
            `/v1/knowledge-gaps?limit=20&offset=${offset}`,
            { signal },
          )
        : await client.request<CatalogItem[]>(
            `/v1/workspace/catalog/${tab === 'Reports' ? 'validation_record' : 'evaluation_set'}?limit=20&offset=${offset}`,
            { signal },
          ),
    [client, tab, offset],
  );
  return (
    <>
      <PageHeading
        eyebrow="Validation Center"
        title="Evidence before conclusions"
        description="Compare fixed endpoints and inspect declared knowledge gaps."
      />
      <div className="notice">
        {t('Declared splits do not establish independent evaluator authority.')}
      </div>
      <Tabs
        tabs={['Evaluation sets', 'Reports', 'Knowledge gaps']}
        selected={tab}
        onSelect={(v) => {
          setTab(v);
          setOffset(0);
          setSelected(undefined);
        }}
      />
      <Boundary resource={resource}>
        {(rows) => (
          <div className="split-view">
            <div className="record-list">
              {rows.length ? (
                rows.map((item) => (
                  <button
                    key={item.id}
                    className={`record ${selected?.id === item.id ? 'selected' : ''}`}
                    onClick={() =>
                      setSelected({
                        id: item.id,
                        version: 'version' in item ? item.version : '1.0.0',
                        title: 'title' in item ? item.title : item.id,
                        digest: '',
                      })
                    }
                  >
                    <strong>{'title' in item ? item.title : item.id}</strong>
                    <span className="mono">{item.id}</span>
                    {'severity' in item && <Badge value={item.severity} />}
                  </button>
                ))
              ) : (
                <Empty />
              )}
              <Pager
                offset={offset}
                count={rows.length}
                setOffset={(v) => {
                  setOffset(v);
                  setSelected(undefined);
                }}
              />
            </div>
            <div className="inspector">
              {selected ? (
                <ValidationDetail key={`${tab}:${identity(selected)}`} tab={tab} item={selected} />
              ) : (
                <Empty text="Choose a frozen set, report or declared gap to inspect its evidence." />
              )}
            </div>
          </div>
        )}
      </Boundary>
      {tab === 'Evaluation sets' && (
        <>
          <details className="surface editor">
            <summary>{t('Evaluate run')}</summary>
            <Evaluate
              key={evaluationRevision}
              onDone={() => {
                resource.refresh();
              }}
            />
          </details>
          <details className="surface editor">
            <summary>{t('Freeze evaluation set')}</summary>
            <JsonForm
              title="Freeze evaluation set"
              label="Evaluation specification"
              disabled={!admin}
              initial={{
                id: 'my.evaluation',
                version: '1.0.0',
                context: {
                  species: '',
                  biological_system: '',
                  conditions: [],
                },
                endpoint: 'concentration',
                calibration: [],
                evaluation: [],
                tolerance: { value: 0, unit: 'mole/liter' },
              }}
              clientAction={(body) => client.request('/v1/validation/sets', { body })}
              onDone={() => {
                resource.refresh();
                setEvaluationRevision((v) => v + 1);
              }}
            />
          </details>
        </>
      )}
    </>
  );
}
function ValidationDetail({ tab, item }: { tab: string; item: CatalogItem }) {
  const { client, openRun } = useSession();
  const t = useT();
  const data = useResource(
    (s) =>
      client.request<
        Schema['FrozenEvaluationSet'] | Schema['ValidationRecord'] | Schema['KnowledgeGap']
      >(
        tab === 'Reports'
          ? `/v1/validation/reports/${encodeURIComponent(item.id)}`
          : tab === 'Knowledge gaps'
            ? `/v1/knowledge-gaps/${encodeURIComponent(item.id)}`
            : `/v1/validation/sets/${versionPath(item)}`,
        { signal: s },
      ),
    [client, tab, item],
  );
  return (
    <Boundary resource={data}>
      {(value) => (
        <>
          <h2 className="break-anywhere">{item.title}</h2>
          {'report' in value ? (
            <>
              <Badge value={value.report.passed ? 'Passed' : 'Outside tolerance'} />
              <Meta
                items={[
                  ['Evidence class', value.report.evidence_class],
                  ['Endpoint', value.report.endpoint],
                  ['RMSE', `${value.report.rmse} ${value.report.unit}`],
                  ['Maximum error', `${value.report.max_absolute_error} ${value.report.unit}`],
                  ['Tolerance', `${value.report.tolerance} ${value.report.unit}`],
                  ['Samples', value.report.n],
                  ['Model', identity(value.report.model)],
                  ['Dataset', identity(value.report.dataset)],
                  ['Manifest SHA-256', <code>{value.manifest_digest}</code>],
                  ['Result SHA-256', <code>{value.result_digest}</code>],
                ]}
              />
              <Limitations values={value.report.limitations} />
              <button onClick={() => openRun('simulations', value.run_id)}>{t('Open run')}</button>
            </>
          ) : 'spec' in value ? (
            <>
              <Meta
                items={[
                  [
                    'Context',
                    `${value.spec.context.species} · ${value.spec.context.biological_system}`,
                  ],
                  ['Endpoint', value.spec.endpoint],
                  ['Tolerance', `${value.spec.tolerance.value} ${value.spec.tolerance.unit}`],
                ]}
              />
              <h3>Frozen evaluation datasets</h3>
              {value.evaluation.map((d) => (
                <div className="result-node" key={identity(d.reference)}>
                  <strong>{identity(d.reference)}</strong>
                  <p>{d.independence_group}</p>
                  <code>{d.artifact_digest}</code>
                </div>
              ))}
              <h3>Calibration datasets</h3>
              {value.calibration.map((d) => (
                <p key={identity(d.reference)}>
                  {identity(d.reference)} · {d.independence_group}
                </p>
              ))}
            </>
          ) : (
            <>
              <Meta
                items={Object.entries(value)
                  .filter(([k, v]) => typeof v === 'string' && k !== 'schema_version')
                  .map(([k, v]) => [k, String(v)])}
              />
              {'evidence' in value && <Evidence values={value.evidence as string[]} />}
            </>
          )}
          <JsonDetails value={value} />
        </>
      )}
    </Boundary>
  );
}
function Evaluate({ onDone }: { onDone: () => void }) {
  const { client, admin } = useSession();
  const t = useT();
  const [runId, setRunId] = useState('');
  const [setId, setSetId] = useState('');
  const [result, setResult] = useState<Schema['ValidationRecord'][]>();
  const action = useAction();
  const data = useResource(
    (s) =>
      Promise.all([
        client.request<Run[]>('/v1/simulations?limit=200', {
          signal: s,
        }),
        client.request<CatalogItem[]>('/v1/workspace/catalog/evaluation_set?limit=50', {
          signal: s,
        }),
      ]),
    [client],
  );
  return (
    <Boundary resource={data}>
      {([allRuns, sets]) => {
        const runs = allRuns.filter((r) => r.status === 'succeeded');
        if (!runs.length || !sets.length)
          return (
            <Empty text="A completed reference run and a frozen evaluation set are required." />
          );
        const selected = sets.find((s) => identity(s) === setId) || sets[0];
        const run = runId || runs[0].id;
        return (
          <form
            className="stack"
            onSubmit={async (e) => {
              e.preventDefault();
              const body = {
                run_id: run,
                evaluation_set: ref(selected),
              };
              const results = await action.act(body, () =>
                client.request<Schema['ValidationRecord'][]>('/v1/validation/evaluations', {
                  body,
                }),
              );
              if (results) {
                setResult(results);
                onDone();
              }
            }}
          >
            <Field label="Completed reference run">
              <select
                value={run}
                onChange={(e) => {
                  setRunId(e.target.value);
                  setResult(undefined);
                }}
              >
                {runs.map((r) => (
                  <option key={r.id}>{r.id}</option>
                ))}
              </select>
            </Field>
            <Field label="Frozen evaluation set">
              <select
                value={identity(selected)}
                onChange={(e) => {
                  setSetId(e.target.value);
                  setResult(undefined);
                }}
              >
                {sets.map((s) => (
                  <option key={identity(s)} value={identity(s)}>
                    {identity(s)}
                  </option>
                ))}
              </select>
            </Field>
            {action.error !== undefined && <ErrorBox error={action.error} />}
            <button className="primary" disabled={!admin || action.busy}>
              {t(action.busy ? 'Working…' : 'Run evaluation')}
            </button>
            {result &&
              result.map((r) => (
                <article className="result-node" role="status" key={r.id}>
                  <Badge value={r.report.passed ? 'Passed' : 'Outside tolerance'} />
                  <p>
                    {r.report.endpoint} · RMSE {r.report.rmse} {r.report.unit}
                  </p>
                  <Limitations values={r.report.limitations} />
                  <JsonDetails value={r} />
                </article>
              ))}
          </form>
        );
      }}
    </Boundary>
  );
}
