import { useState } from 'react';
import {
  type Schema,
  type Model,
  type NativeModel,
  type CatalogItem,
  versionPath,
  identity,
  ref,
} from './api';
import { useSession } from './session';
import { useT } from './i18n';
import {
  useResource,
  Boundary,
  PageHeading,
  Tabs,
  Empty,
  Field,
  Badge,
  Meta,
  JsonDetails,
  Evidence,
  Limitations,
  Pager,
  Plot,
  ErrorBox,
  JsonForm,
} from './ui';

export function Models() {
  const { client } = useSession();
  const t = useT();
  const [tab, setTab] = useState('Reference models');
  const [offset, setOffset] = useState(0);
  const [search, setSearch] = useState('');
  const [selected, setSelected] = useState<string>();
  const resource = useResource(
    async (signal) => {
      if (tab === 'Reference models')
        return await client.request<Model[]>(`/v1/models?limit=20&offset=${offset}`, { signal });
      if (tab === 'Native models')
        return await client.request<NativeModel[]>(`/v1/native-models?limit=20&offset=${offset}`, {
          signal,
        });
      return await Promise.all([
        client.request<Schema['LibrarySourceReceipt'][]>('/v1/libraries', { signal }),
        client.request<Schema['NativeRuntimeEvidence'][]>('/v1/runtimes', { signal }),
        client.request<Schema['NativeExecutionProfile'][]>('/v1/native-profiles', { signal }),
      ]);
    },
    [client, tab, offset],
  );
  return (
    <>
      <PageHeading
        eyebrow="Model Studio"
        title="Models"
        description="Use exact model versions and explicit budgets. Follow the result through both queues."
      />
      <Tabs
        tabs={['Reference models', 'Native models', 'Sources & runtimes']}
        selected={tab}
        onSelect={(v) => {
          setTab(v);
          setOffset(0);
          setSelected(undefined);
          setSearch('');
        }}
      />
      <Boundary resource={resource}>
        {(data) =>
          tab === 'Sources & runtimes' ? (
            <SourceCatalog
              data={
                data as [
                  Schema['LibrarySourceReceipt'][],
                  Schema['NativeRuntimeEvidence'][],
                  Schema['NativeExecutionProfile'][],
                ]
              }
            />
          ) : (
            (() => {
              const models = data as (Model | NativeModel)[];
              const shown = models.filter((m) => {
                const s = 'spec' in m ? m.spec : m;
                return `${s.id} ${s.title}`.toLowerCase().includes(search.toLowerCase());
              });
              const current = models.find((m) => identity('spec' in m ? m.spec : m) === selected);
              return (
                <>
                  <div className="section-tools">
                    <Field label="Search this page">
                      <input
                        type="search"
                        value={search}
                        onChange={(e) => setSearch(e.target.value)}
                      />
                    </Field>
                    <button onClick={resource.refresh}>{t('Refresh')}</button>
                  </div>
                  <div className="split-view">
                    <div className="record-list">
                      {shown.length ? (
                        shown.map((m) => {
                          const s = 'spec' in m ? m.spec : m;
                          return (
                            <button
                              className={`record ${identity(s) === selected ? 'selected' : ''}`}
                              key={identity(s)}
                              onClick={() => setSelected(identity(s))}
                            >
                              <span className="record-top">
                                <strong>
                                  <bdi>{s.title}</bdi>
                                </strong>
                                <span className="version">v{s.version}</span>
                              </span>
                              <span className="mono">{s.id}</span>
                              <span>
                                {'format' in s ? s.format.toUpperCase() : (s as Model).engine} ·{' '}
                                <bdi>{s.context.species}</bdi>
                              </span>
                            </button>
                          );
                        })
                      ) : (
                        <Empty text={search ? 'No matching records' : undefined} />
                      )}
                      <Pager
                        offset={offset}
                        count={models.length}
                        setOffset={(v) => {
                          setOffset(v);
                          setSelected(undefined);
                        }}
                      />
                    </div>
                    <div className="inspector">
                      {current ? (
                        <ModelDetail model={current} />
                      ) : (
                        <Empty text="Choose a model to inspect its exact version, parameters and ports." />
                      )}
                    </div>
                  </div>
                </>
              );
            })()
          )
        }
      </Boundary>
      {tab === 'Native models' && <NativeAdmission onDone={resource.refresh} />}
    </>
  );
}
function ModelDetail({ model }: { model: Model | NativeModel }) {
  const t = useT();
  const { createRun } = useSession();
  const s = 'spec' in model ? model.spec : model;
  const m = 'validity' in model ? model : undefined;
  return (
    <>
      <div className="inspector-heading">
        <p className="eyebrow">
          {t('Version')} {s.version}
        </p>
        <h2>
          <bdi>{s.title}</bdi>
        </h2>
        <Badge value={m?.validity.status || 'not_established'} />
      </div>
      <Meta
        items={[
          ['Identity', <code>{identity(s)}</code>],
          [
            'Context',
            <bdi>
              {s.context.species} · {s.context.biological_system}
            </bdi>,
          ],
          ['License', <bdi>{s.license}</bdi>],
          ['Source', <Evidence values={[s.source]} />],
        ]}
      />
      {m && (
        <>
          <h3>{t('Capabilities')}</h3>
          <div className="chips">
            {m.capabilities.map((c) => (
              <span key={c}>{c}</span>
            ))}
          </div>
          <h3>{t('Parameters')}</h3>
          <div className="table-scroll">
            <table>
              <thead>
                <tr>
                  <th>{t('Name')}</th>
                  <th>{t('Value')}</th>
                  <th>{t('Unit')}</th>
                </tr>
              </thead>
              <tbody>
                {m.parameters.map((p) => (
                  <tr key={p.name}>
                    <td>{p.name}</td>
                    <td>{p.default.value}</td>
                    <td>{p.default.unit}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <h3>{t('Ports')}</h3>
          {m.ports.length ? (
            <div className="ports">
              {m.ports.map((p) => (
                <div key={p.name}>
                  <Badge value={p.direction} />
                  <strong>{p.name}</strong>
                  <code>{p.unit}</code>
                  <span>
                    {p.semantic_id} · {p.scale} · {p.shape} · {p.ownership}
                  </span>
                </div>
              ))}
            </div>
          ) : (
            <p className="muted">No composition ports declared.</p>
          )}
        </>
      )}
      {'inspection' in model && (
        <JsonDetails label="Original model units" value={model.inspection} />
      )}
      <Limitations
        values={m?.validity.limitations || ('spec' in model ? model.spec.limitations : [])}
      />
      <button className="primary" onClick={() => createRun(m ? 'reference' : 'native', ref(s))}>
        {t('Create run')} <span aria-hidden="true">→</span>
      </button>
      <JsonDetails value={model} />
    </>
  );
}
function SourceCatalog({
  data: [sources, runtimes, profiles],
}: {
  data: [
    Schema['LibrarySourceReceipt'][],
    Schema['NativeRuntimeEvidence'][],
    Schema['NativeExecutionProfile'][],
  ];
}) {
  const t = useT();
  return (
    <div className="stack">
      <div className="metric-strip">
        <div>
          <strong>{sources.length}</strong>
          <span>Admitted source receipts</span>
        </div>
        <div>
          <strong>{runtimes.length}</strong>
          <span>Historical native probes</span>
        </div>
        <div>
          <strong>{profiles.length}</strong>
          <span>Queue profiles</span>
        </div>
      </div>
      <div className="notice">
        Source inspection, runtime evidence, queue support and scientific validity are separate
        records. Historical probes do not report current agent availability.
      </div>
      <h2>Queue profiles</h2>
      {profiles.map((p) => (
        <article className="surface" key={identity(p)}>
          <h3>{identity(p)}</h3>
          <Meta
            items={[
              ['Profile', p.kind],
              ['Runtime', identity(p.runtime)],
              ['Driver SHA-256', <code>{p.driver_digest}</code>],
            ]}
          />
          <Limitations values={p.limitations} />
        </article>
      ))}
      <h2>Historical runtime evidence</h2>
      {runtimes.map((r) => (
        <details className="surface" key={identity(r)}>
          <summary>{identity(r)}</summary>
          <JsonDetails value={r} />
        </details>
      ))}
      <h2>{t('Sources & runtimes')}</h2>
      {sources.map((s) => (
        <details className="surface" key={identity(s.spec)}>
          <summary>
            {s.spec.title} · {s.spec.version} · {s.tracked_files.toLocaleString()} files
          </summary>
          <Meta
            items={[
              ['License', s.spec.license],
              ['Commit', <code>{s.spec.commit}</code>],
              ['Source', <Evidence values={[s.spec.repository]} />],
            ]}
          />
          <Limitations values={s.spec.limitations} />
          <JsonDetails value={s} />
        </details>
      ))}
    </div>
  );
}
function NativeAdmission({ onDone }: { onDone: () => void }) {
  const { client, admin } = useSession();
  const t = useT();
  const [xml, setXml] = useState('');
  const [fileError, setFileError] = useState<unknown>();
  const initial = {
    id: 'my.native-model',
    version: '1.0.0',
    title: 'My admitted model',
    format: 'sbml',
    context: {
      species: 'synthetic',
      biological_system: 'reference verification',
      conditions: [],
    },
    license: '',
    source: '',
    limitations: ['Scientific validity not established.'],
  };
  return (
    <details className="surface editor">
      <summary>{t('Admit a model')}</summary>
      <p>
        {t('No execution occurs during admission. Original bytes and attribution are retained.')}
      </p>
      <Field label="Declarative XML file">
        <input
          type="file"
          accept=".xml,.sbml,.pkml"
          disabled={!admin}
          onChange={async (e) => {
            setXml('');
            setFileError(undefined);
            const f = e.target.files?.[0];
            if (!f) return;
            if (f.size > 1400000) {
              setFileError(
                new Error(
                  'This file exceeds the safe HTTP import size. Use offline operator admission for larger PKML files.',
                ),
              );
              return;
            }
            try {
              setXml(
                new TextDecoder('utf-8', {
                  fatal: true,
                  ignoreBOM: true,
                }).decode(await f.arrayBuffer()),
              );
            } catch (e) {
              setFileError(e);
            }
          }}
        />
      </Field>
      {fileError !== undefined && <ErrorBox error={fileError} />}
      <JsonForm
        label="Model metadata"
        title="Inspect & admit"
        initial={initial}
        disabled={!admin || !xml}
        clientAction={(spec) =>
          client.request('/v1/native-models', {
            body: { spec, xml_text: xml },
          })
        }
        onDone={onDone}
      />
    </details>
  );
}

export function Biology() {
  const { client, admin } = useSession();
  const t = useT();
  const [graphIndex, setGraphIndex] = useState(0);
  const [entity, setEntity] = useState('');
  const [depth, setDepth] = useState(1);
  const [query, setQuery] = useState<{
    graph: Schema['BiologicalGraphSpec'];
    entity: string;
    depth: number;
  }>();
  const graphs = useResource(
    (s) => client.request<Schema['BiologicalGraphSpec'][]>('/v1/biology/graphs', { signal: s }),
    [client],
  );
  const neighborhood = useResource(
    async (s) =>
      query
        ? await client.request<{
            entities: Schema['BiologicalEntity'][];
            relationships: Schema['BiologicalRelationship'][];
            truncated: boolean;
          }>(
            `/v1/biology/graphs/${versionPath(query.graph)}/neighborhood?entity_id=${encodeURIComponent(query.entity)}&depth=${query.depth}&max_nodes=40`,
            { signal: s },
          )
        : null,
    [client, query],
  );
  return (
    <>
      <PageHeading
        eyebrow="Biological Explorer"
        title="Versioned biological context"
        description="Inspect entities, relations and their evidence."
      />
      <div className="notice">{t('Evidence relations are not executable biological bridges.')}</div>
      <Boundary resource={graphs}>
        {(items) =>
          items.length ? (
            (() => {
              const graph = items[graphIndex] || items[0];
              const entities = neighborhood.data?.entities || graph.entities.slice(0, 40);
              const relations =
                neighborhood.data?.relationships ||
                graph.relationships.filter(
                  (r) =>
                    entities.some((e) => e.id === r.source) &&
                    entities.some((e) => e.id === r.target),
                );
              const selected = graph.entities.find((e) => e.id === entity) || graph.entities[0];
              return (
                <>
                  <div className="form-grid biology-controls">
                    <Field label="Graph">
                      <select
                        value={graphIndex}
                        onChange={(e) => {
                          setGraphIndex(Number(e.target.value));
                          setEntity('');
                          setQuery(undefined);
                        }}
                      >
                        {items.map((g, i) => (
                          <option value={i} key={identity(g)}>
                            {g.title} · {g.version}
                          </option>
                        ))}
                      </select>
                    </Field>
                    <Field label="Entity">
                      <select value={selected.id} onChange={(e) => setEntity(e.target.value)}>
                        {graph.entities.map((e) => (
                          <option value={e.id} key={e.id}>
                            {e.label}
                          </option>
                        ))}
                      </select>
                    </Field>
                    <Field label="Depth">
                      <select value={depth} onChange={(e) => setDepth(Number(e.target.value))}>
                        {[1, 2, 3, 4].map((d) => (
                          <option key={d}>{d}</option>
                        ))}
                      </select>
                    </Field>
                    <button
                      disabled={neighborhood.busy && !!query}
                      onClick={() =>
                        setQuery({
                          graph,
                          entity: selected.id,
                          depth,
                        })
                      }
                    >
                      {t('Load neighborhood')}
                    </button>
                  </div>
                  {neighborhood.error !== undefined && (
                    <ErrorBox error={neighborhood.error} retry={neighborhood.refresh} />
                  )}
                  <div className="graph-surface">
                    <div className="section-tools">
                      <span className="eyebrow">
                        {graph.context.species} / {graph.context.biological_system}
                      </span>
                      <span>
                        {entities.length} {t('Entities')} · {relations.length} {t('Relationships')}
                      </span>
                    </div>
                    <EntityGraph
                      entities={entities}
                      relationships={relations}
                      selected={selected.id}
                      onSelect={setEntity}
                    />
                  </div>
                  {(neighborhood.data?.truncated || (!query && graph.entities.length > 40)) && (
                    <div className="notice">{t('Neighborhood truncated at its node limit.')}</div>
                  )}
                  <section className="surface">
                    <h2>{selected.label}</h2>
                    <Meta
                      items={[
                        ['Identity', selected.id],
                        ['Kind', selected.kind],
                        ['Ontology', selected.ontology_id],
                        ['Species', selected.species],
                      ]}
                    />
                    {relations
                      .filter((r) => r.source === selected.id || r.target === selected.id)
                      .map((r, i) => (
                        <article className="relation" key={i}>
                          <strong>
                            {r.source} → {r.target}
                          </strong>
                          <p>
                            {r.relation} · {r.context.biological_system}
                          </p>
                          <Evidence values={r.evidence} />
                        </article>
                      ))}
                    <JsonDetails value={graph} />
                  </section>
                </>
              );
            })()
          ) : (
            <Empty />
          )
        }
      </Boundary>
      <details className="surface editor">
        <summary>{t('Register graph')}</summary>
        <JsonForm
          label="Graph specification"
          title="Register graph"
          initial={{
            id: 'my.graph',
            version: '1.0.0',
            title: '',
            context: {
              species: '',
              biological_system: '',
              conditions: [],
            },
            entities: [],
            relationships: [],
          }}
          disabled={!admin}
          clientAction={(body) => client.request('/v1/biology/graphs', { body })}
          onDone={graphs.refresh}
        />
      </details>
    </>
  );
}
function EntityGraph({
  entities,
  relationships,
  selected,
  onSelect,
}: {
  entities: Schema['BiologicalEntity'][];
  relationships: Schema['BiologicalRelationship'][];
  selected: string;
  onSelect: (id: string) => void;
}) {
  const t = useT();
  const columns = Math.min(4, entities.length);
  const height = Math.max(180, Math.ceil(entities.length / columns) * 105);
  const point = (id: string) => {
    const i = entities.findIndex((e) => e.id === id);
    return {
      x: (i % columns) * (840 / columns) + 420 / columns,
      y: 55 + Math.floor(i / columns) * 105,
    };
  };
  return (
    <>
      <div className="graph-scroll" aria-hidden="true">
        <svg className="entity-graph" viewBox={`0 0 840 ${height}`}>
          {relationships.map((r, i) => {
            const a = point(r.source),
              b = point(r.target);
            return <line key={i} x1={a.x} y1={a.y} x2={b.x} y2={b.y} />;
          })}
          {entities.map((e) => {
            const p = point(e.id);
            return (
              <g key={e.id}>
                <rect
                  className={e.id === selected ? 'active' : ''}
                  x={p.x - 92}
                  y={p.y - 26}
                  width="184"
                  height="52"
                  rx="5"
                />
                <text x={p.x} y={p.y - 1} textAnchor="middle">
                  {e.label.slice(0, 24)}
                </text>
                <text className="kind-label" x={p.x} y={p.y + 16} textAnchor="middle">
                  {e.kind}
                </text>
              </g>
            );
          })}
        </svg>
      </div>
      <div className="entity-buttons" aria-label={t('Entities')}>
        {entities.map((e) => (
          <button aria-pressed={e.id === selected} key={e.id} onClick={() => onSelect(e.id)}>
            {e.label}
          </button>
        ))}
      </div>
    </>
  );
}

export function Datasets() {
  const { client, admin } = useSession();
  const t = useT();
  const [offset, setOffset] = useState(0);
  const [selected, setSelected] = useState<CatalogItem>();
  const [raw, setRaw] = useState('');
  const [fileError, setFileError] = useState<unknown>();
  const catalog = useResource(
    (s) =>
      client.request<CatalogItem[]>(`/v1/workspace/catalog/dataset?limit=20&offset=${offset}`, {
        signal: s,
      }),
    [client, offset],
  );
  const initial = {
    id: 'my.observations',
    version: '1.0.0',
    title: '',
    origin: 'experimental',
    role: 'validation',
    context: { species: '', biological_system: '', conditions: [] },
    license: '',
    source: '',
    independence_group: '',
    endpoint: 'concentration',
    time_column: 'time',
    value_column: 'value',
    time_unit: 'second',
    value_unit: 'mole/liter',
  };
  return (
    <>
      <PageHeading
        eyebrow="Dataset Studio"
        title="Trace every observation"
        description="Inspect origins, units, declared roles and transformation lineage."
      />
      <Boundary resource={catalog}>
        {(items) => (
          <div className="split-view">
            <div className="record-list">
              {items.length ? (
                items.map((item) => (
                  <button
                    className={`record ${selected && identity(selected) === identity(item) ? 'selected' : ''}`}
                    key={identity(item)}
                    onClick={() => setSelected(item)}
                  >
                    <strong>{item.title}</strong>
                    <span className="mono">{identity(item)}</span>
                  </button>
                ))
              ) : (
                <Empty />
              )}
              <Pager
                offset={offset}
                count={items.length}
                setOffset={(v) => {
                  setOffset(v);
                  setSelected(undefined);
                }}
              />
            </div>
            <div className="inspector">
              {selected ? (
                <DatasetDetail key={identity(selected)} item={selected} />
              ) : (
                <Empty text="Choose an observation to inspect its source, units and lineage." />
              )}
            </div>
          </div>
        )}
      </Boundary>
      <details className="surface editor">
        <summary>{t('Import CSV')}</summary>
        <Field label="CSV file">
          <input
            type="file"
            accept=".csv,text/csv"
            disabled={!admin}
            onChange={async (e) => {
              setRaw('');
              setFileError(undefined);
              const f = e.target.files?.[0];
              if (!f) return;
              if (f.size > 1400000) {
                setFileError(
                  new Error('CSV exceeds the HTTP import limit. Use offline operator import.'),
                );
                return;
              }
              try {
                setRaw(
                  new TextDecoder('utf-8', {
                    fatal: true,
                    ignoreBOM: true,
                  }).decode(await f.arrayBuffer()),
                );
              } catch (e) {
                setFileError(e);
              }
            }}
          />
        </Field>
        {fileError !== undefined && <ErrorBox error={fileError} />}
        <JsonForm
          label="Import specification"
          title="Import observations"
          initial={initial}
          disabled={!admin || !raw}
          clientAction={(spec) =>
            client.request('/v1/datasets/import-csv', {
              body: { spec, csv_text: raw },
            })
          }
          onDone={catalog.refresh}
        />
      </details>
    </>
  );
}
function DatasetDetail({ item }: { item: CatalogItem }) {
  const { client } = useSession();
  const t = useT();
  const resource = useResource(
    (s) =>
      client.request<Schema['DatasetRegistration']>(`/v1/datasets/${versionPath(item)}`, {
        signal: s,
      }),
    [client, item],
  );
  const lineage = useResource(
    async (s) => {
      try {
        return await client.request<unknown>(`/v1/datasets/${versionPath(item)}/lineage`, {
          signal: s,
        });
      } catch (e) {
        if (e instanceof Error && 'status' in e && e.status === 404) return null;
        throw e;
      }
    },
    [client, item],
  );
  return (
    <Boundary resource={resource}>
      {({ spec, observation }) => (
        <>
          <p className="eyebrow">
            {spec.origin} / {spec.role}
          </p>
          <h2>{spec.title}</h2>
          <Meta
            items={[
              ['Identity', identity(spec)],
              ['Context', `${spec.context.species} · ${spec.context.biological_system}`],
              ['License', spec.license],
              ['Independence group', spec.independence_group],
              ['Source', <Evidence values={[spec.source]} />],
              ['Artifact SHA-256', <code>{spec.artifact_digest}</code>],
            ]}
          />
          <Plot
            data={{
              time: observation.time,
              outputs: [observation.values],
            }}
          />
          <Boundary resource={lineage}>
            {(value) =>
              value === null ? (
                <div className="notice">
                  <strong>{t('No CSV lineage')}</strong>
                  <p>
                    {t(
                      'This dataset has no generic CSV lineage. Its original source and artifact identity remain available.',
                    )}
                  </p>
                </div>
              ) : (
                <JsonDetails label="Lineage" value={value} />
              )
            }
          </Boundary>
          <JsonDetails label="Observation" value={{ spec, observation }} />
        </>
      )}
    </Boundary>
  );
}
