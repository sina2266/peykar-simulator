import { useState } from 'react';
import {
  type Model,
  type NativeModel,
  type Ref,
  type Schema,
  type Plan,
  type NativePlan,
  type Run,
  type RunKind,
  type CatalogItem,
  type Input,
  defaultPlan,
  identity,
  versionPath,
  ref,
} from './api';
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
  ErrorBox,
  Meta,
  JsonDetails,
  JsonForm,
  Pager,
  Limitations,
  Badge,
} from './ui';

export function Experiments({
  initial,
}: {
  initial?: { kind: 'reference' | 'native'; model: Ref };
}) {
  const [tab, setTab] = useState('Single run');
  return (
    <>
      <PageHeading
        eyebrow="Experiment Studio"
        title="Design a reproducible run"
        description="Use exact model versions and explicit budgets. Follow the result through both queues."
      />
      <Tabs
        tabs={['Single run', 'Scalar flows', 'Cohorts', 'Reference DAGs']}
        selected={tab}
        onSelect={setTab}
      />
      {tab === 'Single run' ? <SingleRun initial={initial} /> : <Definitions key={tab} tab={tab} />}
    </>
  );
}
function SingleRun({ initial }: { initial?: { kind: 'reference' | 'native'; model: Ref } }) {
  const { client } = useSession();
  const t = useT();
  const [kind, setKind] = useState<'reference' | 'native'>(initial?.kind || 'reference');
  const [selected, setSelected] = useState(initial ? identity(initial.model) : '');
  const models = useResource(
    async (signal) => {
      const rows = await client.request<(Model | NativeModel)[]>(
        `/v1/${kind === 'reference' ? 'models' : 'native-models'}?limit=200`,
        { signal },
      );
      if (
        initial?.kind === kind &&
        !rows.some((m) => identity('spec' in m ? m.spec : m) === identity(initial.model))
      )
        rows.unshift(
          await client.request<Model | NativeModel>(
            `/v1/${kind === 'reference' ? 'models' : 'native-models'}/${versionPath(initial.model)}`,
            { signal },
          ),
        );
      return rows;
    },
    [client, kind],
  );
  return (
    <div className="surface run-builder">
      <div className="section-tools">
        <h2>{t('Create run')}</h2>
        <Field label="Queue">
          <select
            value={kind}
            onChange={(e) => {
              setKind(e.target.value as 'reference' | 'native');
              setSelected('');
            }}
          >
            <option value="reference">{t('Reference')}</option>
            <option value="native">{t('Native')}</option>
          </select>
        </Field>
      </div>
      <Boundary resource={models}>
        {(rows) => {
          if (!rows.length) return <Empty text="Admit a model before creating a run." />;
          const model =
            rows.find((m) => identity('spec' in m ? m.spec : m) === selected) || rows[0];
          const spec = 'spec' in model ? model.spec : model;
          return (
            <>
              <Field label="Model">
                <select value={identity(spec)} onChange={(e) => setSelected(e.target.value)}>
                  {rows.map((m) => {
                    const s = 'spec' in m ? m.spec : m;
                    return (
                      <option key={identity(s)} value={identity(s)}>
                        {s.title} · {identity(s)}
                      </option>
                    );
                  })}
                </select>
              </Field>
              {rows.length >= 200 && (
                <div className="notice">
                  Showing the first 200 model versions. Use Model Studio pagination to select
                  another version.
                </div>
              )}
              <Meta
                items={[
                  ['Context', `${spec.context.species} · ${spec.context.biological_system}`],
                  ['License', spec.license],
                ]}
              />
              {'spec' in model ? (
                <NativeRunForm key={identity(spec)} model={model} />
              ) : (
                <ReferenceRunForm key={identity(spec)} model={model} />
              )}
            </>
          );
        }}
      </Boundary>
    </div>
  );
}
function ReferenceRunForm({ model }: { model: Model }) {
  const { client, admin, openRun } = useSession();
  const t = useT();
  const [plan, setPlan] = useState<Plan>(defaultPlan(model));
  const [estimate, setEstimate] = useState<unknown>();
  const [estimateFor, setEstimateFor] = useState('');
  const action = useAction();
  const update = (patch: Partial<Plan>) => {
    setPlan((p) => ({ ...p, ...patch }));
    setEstimate(undefined);
  };
  const configure = (field: 'duration' | 'sample_interval', value: number, unit?: string) =>
    update({ [field]: { value, unit: unit || plan[field].unit } });
  return (
    <form
      className="stack"
      onSubmit={async (e) => {
        e.preventDefault();
        const r = await action.act(plan, (key) =>
          client.request<Run>('/v1/simulations', { body: plan, key }),
        );
        if (r) openRun('simulations', r.id);
      }}
    >
      <div className="form-grid">
        <Field label="Duration">
          <input
            type="number"
            min="0.000001"
            step="any"
            required
            value={plan.duration.value}
            onChange={(e) => configure('duration', Number(e.target.value))}
          />
        </Field>
        <Field label="Time unit">
          <input
            value={plan.duration.unit}
            required
            onChange={(e) => configure('duration', plan.duration.value, e.target.value)}
          />
        </Field>
        <Field label="Sample interval">
          <input
            type="number"
            min="0.000001"
            step="any"
            required
            value={plan.sample_interval.value}
            onChange={(e) => configure('sample_interval', Number(e.target.value))}
          />
        </Field>
        <Field label="Time unit">
          <input
            value={plan.sample_interval.unit}
            required
            onChange={(e) =>
              configure('sample_interval', plan.sample_interval.value, e.target.value)
            }
          />
        </Field>
        <Field label="Seed">
          <input
            type="number"
            min="0"
            max="4294967295"
            step="1"
            value={plan.seed}
            required
            onChange={(e) => update({ seed: Number(e.target.value) })}
          />
        </Field>
        <Field label="Wall time limit (seconds)">
          <input
            type="number"
            min="0.001"
            max="3600"
            step="any"
            value={plan.budget?.wall_seconds}
            required
            onChange={(e) =>
              update({
                budget: {
                  ...plan.budget,
                  wall_seconds: Number(e.target.value),
                },
              })
            }
          />
        </Field>
      </div>
      <fieldset>
        <legend>{t('Parameters')}</legend>
        <div className="form-grid">
          {model.parameters.map((p) => (
            <Field label={`${p.name} (${p.default.unit})`} key={p.name}>
              <input
                type="number"
                step="any"
                min={p.minimum ?? undefined}
                max={p.maximum ?? undefined}
                required
                value={
                  plan.parameters?.find((v) => v.name === p.name)?.quantity.value ?? p.default.value
                }
                onChange={(e) =>
                  update({
                    parameters: [
                      ...(plan.parameters || []).filter((v) => v.name !== p.name),
                      {
                        name: p.name,
                        quantity: {
                          value: Number(e.target.value),
                          unit: p.default.unit,
                        },
                      },
                    ],
                  })
                }
              />
            </Field>
          ))}
        </div>
      </fieldset>
      <Field label="Purpose">
        <textarea
          rows={2}
          required
          maxLength={1000}
          value={plan.purpose}
          onChange={(e) => update({ purpose: e.target.value })}
        />
      </Field>
      <Limitations values={model.validity.limitations} />
      {estimate !== undefined && estimateFor === JSON.stringify(plan) && (
        <div className="notice success" role="status">
          <Meta items={Object.entries(estimate as Record<string, string | number>)} />
        </div>
      )}
      {action.error !== undefined && <ErrorBox error={action.error} />}
      <div className="actions">
        <button
          type="button"
          disabled={!admin || action.busy}
          onClick={() =>
            void action.act(plan, async () => {
              const result = await client.request('/v1/simulations/estimate', { body: plan });
              setEstimate(result);
              setEstimateFor(JSON.stringify(plan));
              return result;
            })
          }
        >
          {t('Estimate')}
        </button>
        <button className="primary" disabled={!admin || action.busy}>
          {t(action.busy ? 'Submitting…' : 'Submit run')}
        </button>
      </div>
      {!admin && <p className="muted">{t('Use an administrator token to make changes.')}</p>}
      <JsonDetails value={plan} />
    </form>
  );
}
function NativeRunForm({ model }: { model: NativeModel }) {
  const { client, admin, openRun } = useSession();
  const t = useT();
  const [index, setIndex] = useState(0);
  const [duration, setDuration] = useState(10);
  const [unit, setUnit] = useState('second');
  const [intervals, setIntervals] = useState(10);
  const [wall, setWall] = useState(180);
  const action = useAction();
  const profiles = useResource(
    async (signal) =>
      (
        await client.request<Schema['NativeExecutionProfile'][]>('/v1/native-profiles', { signal })
      ).filter((p) => p.kind === (model.spec.format === 'sbml' ? 'copasi_sbml' : 'osp_pkml')),
    [client, model],
  );
  return (
    <Boundary resource={profiles}>
      {(rows) => {
        if (!rows.length)
          return <Empty text="No reviewed queue profile is admitted for this format." />;
        const profile = rows[index] || rows[0];
        const sbml = profile.kind === 'copasi_sbml';
        const plan: NativePlan = {
          model: ref(model.spec),
          profile: ref(profile),
          duration: sbml ? { value: duration, unit } : null,
          intervals: sbml ? intervals : null,
          wall_seconds: wall,
        };
        return (
          <form
            className="stack"
            onSubmit={async (e) => {
              e.preventDefault();
              const r = await action.act(plan, (key) =>
                client.request<Run>('/v1/native-runs', {
                  body: plan,
                  key,
                }),
              );
              if (r) openRun('native-runs', r.id);
            }}
          >
            <Field label="Profile">
              <select value={index} onChange={(e) => setIndex(Number(e.target.value))}>
                {rows.map((p, i) => (
                  <option key={identity(p)} value={i}>
                    {identity(p)} · {p.kind}
                  </option>
                ))}
              </select>
            </Field>
            <div className="form-grid">
              {sbml ? (
                <>
                  <Field label="Duration">
                    <input
                      type="number"
                      step="any"
                      required
                      min="0.000001"
                      value={duration}
                      onChange={(e) => setDuration(Number(e.target.value))}
                    />
                  </Field>
                  <Field label="Time unit">
                    <input required value={unit} onChange={(e) => setUnit(e.target.value)} />
                  </Field>
                  <Field label="Intervals">
                    <input
                      type="number"
                      required
                      min="1"
                      max="10000"
                      value={intervals}
                      onChange={(e) => setIntervals(Number(e.target.value))}
                    />
                  </Field>
                </>
              ) : (
                <div className="notice">{t('Original PKML schedule')}</div>
              )}
              <Field label="Wall time limit (seconds)">
                <input
                  type="number"
                  required
                  step="any"
                  min="0.001"
                  max="600"
                  value={wall}
                  onChange={(e) => setWall(Number(e.target.value))}
                />
              </Field>
            </div>
            <div className="notice">
              {t(
                'A separate operator native agent must be running. A queued job does not prove agent availability.',
              )}
            </div>
            <Limitations values={profile.limitations} />
            {action.error !== undefined && <ErrorBox error={action.error} />}
            <button className="primary" disabled={!admin || action.busy}>
              {t(action.busy ? 'Submitting…' : 'Submit run')}
            </button>
            <JsonDetails value={plan} />
          </form>
        );
      }}
    </Boundary>
  );
}
function Definitions({ tab }: { tab: string }) {
  const { client, admin, openRun } = useSession();
  const t = useT();
  const [offset, setOffset] = useState(0);
  const action = useAction();
  const type =
    tab === 'Scalar flows' ? 'hybrid-flows' : tab === 'Cohorts' ? 'batches' : 'experiments';
  const queue: RunKind =
    type === 'hybrid-flows' ? 'hybrid-runs' : type === 'batches' ? 'batch-runs' : 'experiment-runs';
  const resource = useResource(
    async (signal) => {
      const data = await client.request<
        (CatalogItem | { spec: { id: string; version: string; title: string } })[]
      >(`/v1/${type}?limit=20&offset=${offset}`, { signal });
      return data.map((d) => ('spec' in d ? { ...ref(d.spec), title: d.spec.title } : d));
    },
    [client, type, offset],
  );
  const [details, setDetails] = useState<unknown>();
  const initial =
    type === 'batches'
      ? {
          id: 'my.cohort',
          version: '1.0.0',
          title: '',
          cases: [],
          sweeps: [],
          replicates: 1,
          base_seed: 42,
          budget: {
            max_jobs: 64,
            max_total_samples: 1000000,
            max_total_wall_seconds: 14400,
            max_in_flight: 4,
          },
          failure_policy: 'continue',
        }
      : {
          id: type === 'hybrid-flows' ? 'my.flow' : 'my.experiment',
          version: '1.0.0',
          title: '',
          nodes: [],
          bindings: [],
          limitations: ['Scientific validity not established.'],
        };
  return (
    <div className="stack">
      {type !== 'batches' && (
        <div className="notice">
          {t(
            'Sequential endpoint transfer only; no simultaneous feedback or biological bridge is implied.',
          )}
        </div>
      )}
      <Boundary resource={resource}>
        {(items) => (
          <section className="surface">
            <div className="section-tools">
              <h2>{t('Frozen definitions')}</h2>
              <button onClick={resource.refresh}>{t('Refresh')}</button>
            </div>
            {items.length ? (
              items.map((item) => (
                <div className="definition-row" key={identity(item)}>
                  <div>
                    <strong>{item.title}</strong>
                    <p className="mono">{identity(item)}</p>
                  </div>
                  <div className="actions">
                    <button
                      onClick={() =>
                        void action.act(ref(item), async () => {
                          const value = await client.request(`/v1/${type}/${versionPath(item)}`);
                          setDetails(value);
                          return value;
                        })
                      }
                    >
                      {t('Details')}
                    </button>
                    <button
                      className="primary"
                      disabled={!admin || action.busy}
                      onClick={() =>
                        void action.act({ type, ...ref(item) }, async (key) => {
                          const run = await client.request<Run>(`/v1/${queue}`, {
                            body: ref(item),
                            key,
                          });
                          openRun(queue, run.id);
                          return run;
                        })
                      }
                    >
                      {t('Run this version')}
                    </button>
                  </div>
                </div>
              ))
            ) : (
              <Empty />
            )}
            <Pager offset={offset} count={items.length} setOffset={setOffset} />
            {details !== undefined && <JsonDetails value={details} />}
          </section>
        )}
      </Boundary>
      {action.error !== undefined && <ErrorBox error={action.error} />}
      {type === 'hybrid-flows' && <FlowDesigner onDone={resource.refresh} />}
      <details className="surface editor">
        <summary>{t('Register definition')}</summary>
        <p>
          {t(
            'Registration verifies model, context, units, ownership and aggregate budgets on the server.',
          )}
        </p>
        <JsonForm
          key={type}
          label="Definition JSON"
          title="Freeze definition"
          initial={initial}
          disabled={!admin}
          clientAction={(body) => client.request(`/v1/${type}`, { body })}
          onDone={resource.refresh}
        />
      </details>
    </div>
  );
}

type DraftNode = Input<Schema['HybridFlowNode']>;
type DraftBinding = Input<Schema['HybridFlowBinding']>;
function FlowDesigner({ onDone }: { onDone: () => void }) {
  const { client, admin } = useSession();
  const t = useT();
  const [nodes, setNodes] = useState<DraftNode[]>([]);
  const [bindings, setBindings] = useState<DraftBinding[]>([]);
  const [name, setName] = useState('my.scalar-flow');
  const [version, setVersion] = useState('1.0.0');
  const [title, setTitle] = useState('');
  const [done, setDone] = useState<unknown>();
  const action = useAction();
  const resources = useResource(
    (s) =>
      Promise.all([
        client.request<Model[]>('/v1/models?limit=200', { signal: s }),
        client.request<NativeModel[]>('/v1/native-models?limit=200', {
          signal: s,
        }),
        client.request<Schema['NativeInterfaceSpec'][]>('/v1/native-interfaces', { signal: s }),
        client.request<Schema['NativeExecutionProfile'][]>('/v1/native-profiles', { signal: s }),
      ]),
    [client],
  );
  return (
    <details className="surface editor">
      <summary>{t('Design flow')}</summary>
      <p>
        {t(
          'Registration verifies model, context, units, ownership and aggregate budgets on the server.',
        )}
      </p>
      <Boundary resource={resources}>
        {([models, nativeModels, interfaces, profiles]) => {
          const options = [
            ...models.map((m) => ({
              key: `reference:${identity(m)}`,
              label: `${m.title} · ${identity(m)}`,
              plan: defaultPlan(m) as Plan | NativePlan,
              native_interface: null as Ref | null,
            })),
            ...nativeModels.flatMap((m) =>
              profiles
                .filter((p) => p.kind === (m.spec.format === 'sbml' ? 'copasi_sbml' : 'osp_pkml'))
                .flatMap((p) =>
                  interfaces
                    .filter((i) => identity(i.model) === identity(m.spec))
                    .map((i) => ({
                      key: `${identity(m.spec)}:${identity(p)}:${identity(i)}`,
                      label: `${m.spec.title} · ${identity(i)}`,
                      plan: {
                        model: ref(m.spec),
                        profile: ref(p),
                        duration:
                          p.kind === 'copasi_sbml'
                            ? {
                                value: 10,
                                unit: 'second',
                              }
                            : null,
                        intervals: p.kind === 'copasi_sbml' ? 10 : null,
                        wall_seconds: 180,
                      } as NativePlan,
                      native_interface: ref(i),
                    })),
                ),
            ),
          ];
          const keyFor = (n: DraftNode) =>
            options.find(
              (o) =>
                identity(o.plan.model) === identity(n.plan.model) &&
                JSON.stringify(o.native_interface) === JSON.stringify(n.native_interface),
            )?.key || '';
          const ports = (id: string, direction: string) => {
            const n = nodes.find((n) => n.id === id);
            if (!n) return [];
            return (
              (n.native_interface
                ? interfaces
                    .find((i) => identity(i) === identity(n.native_interface!))
                    ?.ports.map((p) => p.port)
                : models.find((m) => identity(m) === identity(n.plan.model))?.ports
              )?.filter((p) => p.direction === direction) || []
            );
          };
          const replace = (index: number, patch: Partial<DraftNode>) => {
            setNodes((ns) => ns.map((n, i) => (i === index ? { ...n, ...patch } : n)));
            setDone(undefined);
          };
          const updateBinding = (index: number, patch: Partial<DraftBinding>) => {
            setBindings((bs) => bs.map((b, i) => (i === index ? { ...b, ...patch } : b)));
            setDone(undefined);
          };
          const spec = {
            id: name,
            version,
            title,
            nodes,
            bindings,
            limitations: [
              'Sequential scalar endpoint transfer. Biological validity not established.',
            ],
          };
          return (
            <form
              className="stack"
              onSubmit={async (e) => {
                e.preventDefault();
                const result = await action.act(spec, () =>
                  client.request('/v1/hybrid-flows', {
                    body: spec,
                  }),
                );
                if (result) {
                  setDone(result);
                  onDone();
                }
              }}
            >
              <div className="form-grid">
                <Field label="Definition ID">
                  <input
                    required
                    pattern="[a-z][a-z0-9_.-]{0,79}"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                  />
                </Field>
                <Field label="Version">
                  <input
                    required
                    pattern="\d+\.\d+\.\d+"
                    value={version}
                    onChange={(e) => setVersion(e.target.value)}
                  />
                </Field>
                <Field label="Title">
                  <input
                    required
                    maxLength={500}
                    value={title}
                    onChange={(e) => setTitle(e.target.value)}
                  />
                </Field>
              </div>
              <div className="flow-diagram" aria-label="Flow nodes">
                {nodes.map((n, i) => (
                  <span key={i}>
                    <Badge value={n.native_interface ? 'Native' : 'Reference'} />
                    <strong>{n.id}</strong>
                    <code>{identity(n.plan.model)}</code>
                  </span>
                ))}
              </div>
              {nodes.map((n, index) => (
                <fieldset key={index}>
                  <legend>
                    {t('Node ID')} {index + 1}
                  </legend>
                  <div className="form-grid">
                    <Field label="Node ID">
                      <input
                        required
                        value={n.id}
                        pattern="[a-z][a-z0-9_.-]{0,79}"
                        onChange={(e) => {
                          const old = n.id;
                          replace(index, {
                            id: e.target.value,
                          });
                          setBindings((bs) =>
                            bs.map((b) => ({
                              ...b,
                              source_node: b.source_node === old ? e.target.value : b.source_node,
                              target_node: b.target_node === old ? e.target.value : b.target_node,
                            })),
                          );
                        }}
                      />
                    </Field>
                    <Field label="Model">
                      <select
                        value={keyFor(n)}
                        onChange={(e) => {
                          const o = options.find((o) => o.key === e.target.value)!;
                          replace(index, {
                            plan: o.plan,
                            native_interface: o.native_interface,
                          });
                          setBindings((bs) =>
                            bs.filter((b) => b.source_node !== n.id && b.target_node !== n.id),
                          );
                        }}
                      >
                        {options.map((o) => (
                          <option key={o.key} value={o.key}>
                            {o.label}
                          </option>
                        ))}
                      </select>
                    </Field>
                    {n.plan.duration && (
                      <Field label="Duration">
                        <input
                          type="number"
                          min="0.000001"
                          step="any"
                          required
                          value={n.plan.duration.value}
                          onChange={(e) =>
                            replace(index, {
                              plan: {
                                ...n.plan,
                                duration: {
                                  ...n.plan.duration!,
                                  value: Number(e.target.value),
                                },
                              },
                            })
                          }
                        />
                      </Field>
                    )}
                  </div>
                  <button
                    type="button"
                    onClick={() => {
                      setNodes((ns) => ns.filter((_, i) => i !== index));
                      setBindings((bs) =>
                        bs.filter((b) => b.source_node !== n.id && b.target_node !== n.id),
                      );
                      setDone(undefined);
                    }}
                  >
                    {t('Remove')}
                  </button>
                </fieldset>
              ))}
              <button
                type="button"
                disabled={!options.length || nodes.length >= 16 || !admin}
                onClick={() => {
                  const o = options[0];
                  let ordinal = nodes.length + 1;
                  while (nodes.some((n) => n.id === `node${ordinal}`)) ordinal++;
                  setNodes((ns) => [
                    ...ns,
                    {
                      id: `node${ordinal}`,
                      plan: o.plan,
                      native_interface: o.native_interface,
                    },
                  ]);
                  setDone(undefined);
                }}
              >
                {t('Add node')}
              </button>
              {bindings.map((b, i) => (
                <fieldset key={i}>
                  <legend>
                    {t('Add binding')} {i + 1}
                  </legend>
                  <div className="form-grid">
                    <Field label="From node">
                      <select
                        value={b.source_node}
                        onChange={(e) =>
                          updateBinding(i, {
                            source_node: e.target.value,
                            source_port: '',
                          })
                        }
                      >
                        {nodes.map((n) => (
                          <option key={n.id}>{n.id}</option>
                        ))}
                      </select>
                    </Field>
                    <Field label="Output port">
                      <select
                        required
                        value={b.source_port}
                        onChange={(e) =>
                          updateBinding(i, {
                            source_port: e.target.value,
                          })
                        }
                      >
                        <option value="">—</option>
                        {ports(b.source_node, 'output').map((p) => (
                          <option key={p.name} value={p.name}>
                            {p.name} · {p.unit} · {p.semantic_id}
                          </option>
                        ))}
                      </select>
                    </Field>
                    <Field label="To node">
                      <select
                        value={b.target_node}
                        onChange={(e) =>
                          updateBinding(i, {
                            target_node: e.target.value,
                            target_port: '',
                            target_parameter: null,
                          })
                        }
                      >
                        {nodes.map((n) => (
                          <option key={n.id}>{n.id}</option>
                        ))}
                      </select>
                    </Field>
                    <Field label="Input port">
                      <select
                        required
                        value={b.target_port}
                        onChange={(e) =>
                          updateBinding(i, {
                            target_port: e.target.value,
                          })
                        }
                      >
                        <option value="">—</option>
                        {ports(b.target_node, 'input').map((p) => (
                          <option key={p.name} value={p.name}>
                            {p.name} · {p.unit} · {p.semantic_id}
                          </option>
                        ))}
                      </select>
                    </Field>
                    {!nodes.find((n) => n.id === b.target_node)?.native_interface && (
                      <Field label="Target parameter">
                        <select
                          required
                          value={b.target_parameter || ''}
                          onChange={(e) =>
                            updateBinding(i, {
                              target_parameter: e.target.value,
                            })
                          }
                        >
                          <option value="">—</option>
                          {models
                            .find(
                              (m) =>
                                identity(m) ===
                                identity(nodes.find((n) => n.id === b.target_node)!.plan.model),
                            )
                            ?.parameters.map((p) => (
                              <option key={p.name}>{p.name}</option>
                            ))}
                        </select>
                      </Field>
                    )}
                    <Field label="Transfer evidence">
                      <textarea
                        required
                        value={b.evidence.join('\n')}
                        onChange={(e) =>
                          updateBinding(i, {
                            evidence: e.target.value.split('\n'),
                          })
                        }
                      />
                    </Field>
                  </div>
                  <button
                    type="button"
                    onClick={() => {
                      setBindings((bs) => bs.filter((_, j) => j !== i));
                      setDone(undefined);
                    }}
                  >
                    {t('Remove')}
                  </button>
                </fieldset>
              ))}
              <button
                type="button"
                disabled={nodes.length < 2 || bindings.length >= 32 || !admin}
                onClick={() => {
                  setBindings((bs) => [
                    ...bs,
                    {
                      source_node: nodes[0].id,
                      source_port: '',
                      target_node: nodes[1].id,
                      target_port: '',
                      target_parameter: null,
                      evidence: [''],
                    },
                  ]);
                  setDone(undefined);
                }}
              >
                {t('Add binding')}
              </button>
              {action.error !== undefined && <ErrorBox error={action.error} />}
              <button className="primary" disabled={!admin || !nodes.length || action.busy}>
                {t(action.busy ? 'Working…' : 'Freeze flow')}
              </button>
              {done !== undefined && (
                <div className="notice success" role="status">
                  <JsonDetails value={done} />
                </div>
              )}
              <JsonDetails label="Definition JSON" value={spec} />
            </form>
          );
        }}
      </Boundary>
    </details>
  );
}
