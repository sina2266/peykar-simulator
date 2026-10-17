import { useState } from 'react';
import { type Schema, type PlotData, identity, versionPath, nativeSeries, download } from './api';
import { useSession } from './session';
import { useT } from './i18n';
import { NativeAdmission } from './catalogs';
import {
  Boundary,
  useResource,
  Tabs,
  Empty,
  Pager,
  Meta,
  Badge,
  JsonDetails,
  Limitations,
  Evidence,
  Plot,
  ErrorBox,
} from './ui';

type Model = Schema['CellMLModelPackage'];
type Execution = Schema['CellMLExecutionReceipt'];
type Metadata = {
  voi: { units: string };
  columns: { component: string; name: string; units: string }[];
};

export function cellmlSeries(csv: string, metadata: Metadata): PlotData {
  if (!csv.startsWith('time,')) throw new Error('Invalid CellML time column.');
  return nativeSeries('Time,' + csv.slice(5), {
    model_units: { time_unit: metadata.voi.units },
    quantities: Object.fromEntries(
      metadata.columns.map((m) => [m.component + '/' + m.name, { unit: m.units }]),
    ),
  });
}

export function CellMLCatalog() {
  const { client } = useSession();
  const t = useT();
  const [view, setView] = useState('CellML models');
  const [offset, setOffset] = useState(0);
  const [selected, setSelected] = useState<string>();
  const models = view === 'CellML models';
  const resource = useResource(
    (signal) =>
      client.request<(Model | Execution)[]>(
        `/v1/${models ? 'cellml-models' : 'cellml-executions'}?limit=20&offset=${offset}`,
        { signal },
      ),
    [client, models, offset],
  );
  return (
    <div className="stack">
      <div className="notice">
        {t(
          'CellML 2 admission preserves original equations and units. Execution requires the reviewed offline operator solver; API queue submission is not supported yet.',
        )}
      </div>
      <Tabs
        label="CellML records"
        tabs={['CellML models', 'Operator executions']}
        selected={view}
        onSelect={(v) => {
          setView(v);
          setOffset(0);
          setSelected(undefined);
        }}
      />
      <button className="compact" onClick={resource.refresh}>
        {t('Refresh')}
      </button>
      <Boundary resource={resource}>
        {(records) => {
          const current = records.find((r) => identity('spec' in r ? r.spec : r) === selected);
          return (
            <div className="split-view">
              <div className="record-list">
                {records.length ? (
                  records.map((r) => {
                    const s = 'spec' in r ? r.spec : r;
                    return (
                      <button
                        key={identity(s)}
                        className={`record ${identity(s) === selected ? 'selected' : ''}`}
                        onClick={() => setSelected(identity(s))}
                      >
                        <span className="record-top">
                          <strong>
                            <bdi>{'title' in s ? s.title : s.id}</bdi>
                          </strong>
                          <span className="version">v{s.version}</span>
                        </span>
                        <span className="mono">{s.id}</span>
                        <span>
                          {t('spec' in r ? 'Original CellML 2' : 'Retained operator result')}
                        </span>
                      </button>
                    );
                  })
                ) : (
                  <Empty />
                )}
                <Pager
                  offset={offset}
                  count={records.length}
                  setOffset={(v) => {
                    setOffset(v);
                    setSelected(undefined);
                  }}
                />
              </div>
              <div className="inspector">
                {current ? (
                  'spec' in current ? (
                    <CellMLModel model={current} />
                  ) : (
                    <CellMLExecution execution={current} />
                  )
                ) : (
                  <Empty text="Choose a CellML model or retained execution to inspect." />
                )}
              </div>
            </div>
          );
        }}
      </Boundary>
      {models && <NativeAdmission cellml onDone={resource.refresh} />}
    </div>
  );
}

function CellMLModel({ model }: { model: Model }) {
  const t = useT();
  const { client } = useSession();
  const [error, setError] = useState<unknown>();
  const [busy, setBusy] = useState(false);
  const spec = model.spec;
  return (
    <div className="stack">
      <h2>
        <bdi>{spec.title}</bdi>
      </h2>
      <Badge value={model.scientific_validity} />
      <Meta
        items={[
          ['Identity', <code>{identity(spec)}</code>],
          [
            'Context',
            <bdi>
              {spec.context.species} · {spec.context.biological_system}
            </bdi>,
          ],
          ['License', <bdi>{spec.license}</bdi>],
          ['Source', <Evidence values={[spec.source]} />],
          ['Artifact SHA-256', <code>{model.content_digest}</code>],
          ['Inspection scope', model.inspection.scope],
        ]}
      />
      <h3>{t('Variables and original units')}</h3>
      <div
        className="table-scroll"
        tabIndex={0}
        role="region"
        aria-label={t('Variables and original units')}
      >
        <table>
          <thead>
            <tr>
              <th>{t('Component')}</th>
              <th>{t('Name')}</th>
              <th>{t('Unit')}</th>
              <th>{t('Initial value')}</th>
            </tr>
          </thead>
          <tbody>
            {model.inspection.variables.map((v) => (
              <tr key={v.component + '/' + v.name}>
                <td>{v.component}</td>
                <td>{v.name}</td>
                <td>{v.units}</td>
                <td>{v.initial_value ?? '—'}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
      <Limitations values={[...spec.limitations, ...model.inspection.warnings]} />
      <button
        type="button"
        disabled={busy}
        onClick={async () => {
          setBusy(true);
          setError(undefined);
          try {
            const text = await client.request<ArrayBuffer>(
              '/v1/cellml-models/' + versionPath(spec) + '/content',
              { bytes: true },
            );
            download('model.cellml', text, 'application/octet-stream');
          } catch (e) {
            setError(e);
          } finally {
            setBusy(false);
          }
        }}
      >
        {t(busy ? 'Loading…' : 'Download original model')}
      </button>
      {error !== undefined && <ErrorBox error={error} />}
      <JsonDetails value={model} />
    </div>
  );
}

function CellMLExecution({ execution }: { execution: Execution }) {
  const t = useT();
  const { client } = useSession();
  const base = '/v1/cellml-executions/' + versionPath(execution);
  const data = useResource(
    async (signal) => {
      const [csv, metadata] = await Promise.all([
        client.request<string>(base + '/artifacts/series.csv', { text: true, signal }),
        client.request<Metadata>(base + '/artifacts/result.json', { signal }),
      ]);
      return cellmlSeries(csv, metadata);
    },
    [client, base],
  );
  const [error, setError] = useState<unknown>();
  const [busy, setBusy] = useState(false);
  return (
    <div className="stack">
      <h2>
        <bdi>{identity(execution)}</bdi>
      </h2>
      <Badge value={execution.scientific_validity} />
      <Meta
        items={[
          ['Model', <code>{identity(execution.model)}</code>],
          ['Runtime', <code>{identity(execution.runtime)}</code>],
          ['Driver SHA-256', <code>{execution.driver_digest}</code>],
          ['Method', execution.controls.method],
          ['Unit', execution.controls.time_unit],
        ]}
      />
      <Boundary resource={data}>{(plot) => <Plot data={plot} />}</Boundary>
      <h3>{t('Original artifacts')}</h3>
      {Object.entries(execution.artifacts).map(([name, digest]) => (
        <div className="artifact-row" key={name}>
          <button
            disabled={busy}
            onClick={async () => {
              setBusy(true);
              setError(undefined);
              try {
                const raw = await client.request<ArrayBuffer>(
                  base + '/artifacts/' + encodeURIComponent(name),
                  { bytes: true },
                );
                download(name, raw, 'application/octet-stream');
              } catch (e) {
                setError(e);
              } finally {
                setBusy(false);
              }
            }}
          >
            {name}
          </button>
          <code>{digest}</code>
        </div>
      ))}
      {error !== undefined && <ErrorBox error={error} />}
      <Limitations values={execution.limitations} />
      <JsonDetails value={execution} />
    </div>
  );
}
