import { useState, type FormEvent } from 'react';
import { type Schema, identity, versionPath, ref, download } from './api';
import { useSession } from './session';
import { useT } from './i18n';
import {
  Boundary,
  useResource,
  Field,
  Empty,
  Pager,
  Meta,
  Evidence,
  Limitations,
  JsonDetails,
  ErrorBox,
  Tabs,
} from './ui';

type Network = Schema['BiologicalProcessNetwork'];
type Term = Schema['StoichiometricTerm'];
type ModelChoice = {
  kind: 'reference' | 'native' | 'cellml';
  id: string;
  version: string;
  title: string;
};
const base = '/v1/biology/process-networks';
const fraction = (v: Term) =>
  v.denominator === 1 ? String(v.numerator) : `${v.numerator}/${v.denominator}`;

export function ProcessExplorer() {
  const { client } = useSession();
  const t = useT();
  const [offset, setOffset] = useState(0);
  const [selected, setSelected] = useState('');
  const networks = useResource(
    (signal) => client.request<Network[]>(`${base}?limit=20&offset=${offset}`, { signal }),
    [client, offset],
  );
  return (
    <div className="stack">
      <div className="notice">
        {t(
          'Process declarations preserve molecular states and participants. They do not authorize a solver or establish biological validity.',
        )}
      </div>
      <Boundary resource={networks}>
        {(items) =>
          items.length ? (
            <>
              <Field label="Process network">
                <select
                  value={selected || identity(items[0])}
                  onChange={(e) => setSelected(e.target.value)}
                >
                  {items.map((n) => (
                    <option key={identity(n)} value={identity(n)}>
                      {n.title} · {n.version}
                    </option>
                  ))}
                </select>
              </Field>
              <Pager
                offset={offset}
                count={items.length}
                setOffset={(v) => {
                  setOffset(v);
                  setSelected('');
                }}
              />
              {(() => {
                const network = items.find((n) => identity(n) === selected) || items[0];
                return <NetworkDetail key={identity(network)} network={network} />;
              })()}
            </>
          ) : (
            <Empty text="No process networks have been admitted. Use the documented offline source bundle workflow." />
          )
        }
      </Boundary>
    </div>
  );
}

function NetworkDetail({ network }: { network: Network }) {
  const t = useT();
  const { client } = useSession();
  const [tab, setTab] = useState('Reactions');
  const [stateId, setStateId] = useState(network.states[0].id);
  const [error, setError] = useState<unknown>();
  const [busySource, setBusySource] = useState('');
  const states = new Map(network.states.map((s) => [s.id, s]));
  const chooseState = (id: string) => {
    setStateId(id);
    setTab('Molecular states');
  };
  function participants(terms: Term[]) {
    return (
      <div className="process-participants">
        {terms.map((v) => (
          <button key={v.state_id} className="secondary" onClick={() => chooseState(v.state_id)}>
            <bdi>
              {fraction(v)} × {states.get(v.state_id)?.label || v.state_id}
            </bdi>
          </button>
        ))}
      </div>
    );
  }
  return (
    <div className="stack">
      <section className="surface stack">
        <h2>
          <bdi>{network.title}</bdi>
        </h2>
        <Meta
          items={[
            ['Identity', <code>{identity(network)}</code>],
            ['Species', network.context.species],
            ['Biological system', network.context.biological_system],
            ['Molecular states', network.states.length],
            ['Reactions', network.reactions.length],
            ['Approval', t(network.approval)],
          ]}
        />
        <Limitations values={network.limitations} />
      </section>
      <Tabs
        label="Process details"
        tabs={[
          'Reactions',
          'Molecular states',
          'Stoichiometry',
          'Process ownership',
          'State instances',
          'Source files',
        ]}
        selected={tab}
        onSelect={setTab}
      />
      {tab === 'Reactions' &&
        network.reactions.map((r) => (
          <article className="surface stack" key={r.id}>
            <h2>
              <bdi>{r.label}</bdi>
            </h2>
            <Meta
              items={[
                ['Source identity', <code>{r.external_id}</code>],
                ['Source class', r.source_class],
                ['Inferred by source', t(r.inferred ? 'Yes' : 'No')],
                [
                  'Reversibility',
                  r.reversible === null ? t('Unknown') : t(r.reversible ? 'Yes' : 'No'),
                ],
              ]}
            />
            <div className="process-equation">
              <section>
                <h3>{t('Inputs')}</h3>
                {participants(r.inputs)}
              </section>
              <span className="process-arrow" aria-hidden="true">
                →
              </span>
              <section>
                <h3>{t('Outputs')}</h3>
                {participants(r.outputs)}
              </section>
            </div>
            {r.modifiers.length > 0 && (
              <section>
                <h3>{t('Catalysts and regulators')}</h3>
                {r.modifiers.map((m, i) => (
                  <p key={i}>
                    <button className="link-button" onClick={() => chooseState(m.state_id)}>
                      <bdi>{states.get(m.state_id)?.label}</bdi>
                    </button>{' '}
                    · {t(m.role)} · <code>{m.activity}</code>
                  </p>
                ))}
              </section>
            )}
            {r.unexpanded_relations.length > 0 && (
              <details>
                <summary>{t('Unexpanded source relations')}</summary>
                <ul>
                  {r.unexpanded_relations.map((v, i) => (
                    <li key={i}>
                      <bdi>
                        {v.predicate} · {v.target} · {v.label}
                      </bdi>
                    </li>
                  ))}
                </ul>
              </details>
            )}
            <Evidence values={r.evidence} />
          </article>
        ))}
      {tab === 'Molecular states' && (
        <section className="surface stack">
          <Field label="Molecular state">
            <select value={stateId} onChange={(e) => setStateId(e.target.value)}>
              {network.states.map((s) => (
                <option key={s.id} value={s.id}>
                  {s.label}
                </option>
              ))}
            </select>
          </Field>
          {(() => {
            const s = states.get(stateId)!;
            return (
              <>
                <h2>
                  <bdi>{s.label}</bdi>
                </h2>
                <Meta
                  items={[
                    ['Source identity', <code>{s.external_id}</code>],
                    ['Kind', t(s.kind)],
                    ['Molecular identity', <bdi>{s.molecular_identity || '—'}</bdi>],
                    ['Species', s.species || t('Not species-specific')],
                    [
                      'Locations',
                      s.locations.map((l) => (
                        <p key={l.id}>
                          <bdi>
                            {l.label} · {l.id}
                          </bdi>
                        </p>
                      )),
                    ],
                  ]}
                />
                {s.attributes.length > 0 && (
                  <section>
                    <h3>{t('Original state attributes')}</h3>
                    <ul>
                      {s.attributes.map((v, i) => (
                        <li key={i}>
                          <bdi>
                            {v.name} · {v.value}
                          </bdi>
                        </li>
                      ))}
                    </ul>
                  </section>
                )}
                {s.components.length > 0 && (
                  <section>
                    <h3>{t('Complex components')}</h3>
                    {participants(s.components)}
                  </section>
                )}
                {s.alternatives.length > 0 && (
                  <section className="stack">
                    <h3>{t('Alternative members')}</h3>
                    <p className="muted">
                      {t(
                        'Members are alternatives; they are not simultaneous stoichiometric components.',
                      )}
                    </p>
                    <div className="process-participants">
                      {s.alternatives.map((id) => (
                        <button className="secondary" key={id} onClick={() => chooseState(id)}>
                          <bdi>{states.get(id)?.label}</bdi>
                        </button>
                      ))}
                    </div>
                  </section>
                )}
              </>
            );
          })()}
        </section>
      )}
      {tab === 'Stoichiometry' && <Stoichiometry network={network} />}
      {tab === 'Process ownership' && <Ownership network={network} />}
      {tab === 'State instances' && <Frames network={network} />}
      {tab === 'Source files' && (
        <section className="surface stack">
          <h2>{t('Source files')}</h2>
          {error !== undefined && <ErrorBox error={error} />}
          <ul className="process-sources">
            {network.sources.map((s) => (
              <li key={s.id} className="stack">
                <strong>
                  <bdi>{s.revision}</bdi>
                </strong>
                <span>{s.license}</span>
                <code>{s.raw_digest}</code>
                <Evidence values={s.attribution} />
                <button
                  className="secondary"
                  disabled={Boolean(busySource)}
                  onClick={async () => {
                    setBusySource(s.id);
                    setError(undefined);
                    try {
                      const bytes = await client.request<Uint8Array>(
                        `${base}/${versionPath(network)}/sources/${s.id}`,
                        { bytes: true },
                      );
                      download(`${s.id}.json`, bytes, 'application/octet-stream');
                    } catch (err) {
                      setError(err);
                    } finally {
                      setBusySource('');
                    }
                  }}
                >
                  {t(busySource === s.id ? 'Working…' : 'Download original source')}
                </button>
              </li>
            ))}
          </ul>
        </section>
      )}
      <JsonDetails value={network} label="Full process declaration" />
    </div>
  );
}

function Stoichiometry({ network }: { network: Network }) {
  const { client } = useSession();
  const t = useT();
  const resource = useResource(
    (signal) =>
      client.request<{
        state_ids: string[];
        reaction_ids: string[];
        net_coefficients: string[][];
        interpretation: string;
      }>(`${base}/${versionPath(network)}/stoichiometry`, { signal }),
    [client, network],
  );
  return (
    <Boundary resource={resource}>
      {(matrix) => (
        <section className="surface stack">
          <h2>{t('Exact stoichiometry')}</h2>
          <p>
            {t(
              'Coefficients are outputs minus inputs. Catalysts and regulators remain separate; this matrix does not supply rates or prove elemental balance.',
            )}
          </p>
          <div className="table-scroll" role="region" aria-label={t('Stoichiometry')} tabIndex={0}>
            <table>
              <caption className="sr-only">{t('Exact stoichiometry')}</caption>
              <thead>
                <tr>
                  <th scope="col">{t('Molecular state')}</th>
                  {matrix.reaction_ids.map((id) => (
                    <th scope="col" key={id}>
                      <bdi>{network.reactions.find((r) => r.id === id)?.label}</bdi>
                    </th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {matrix.state_ids.map((id, i) => (
                  <tr key={id}>
                    <th scope="row">
                      <bdi>{network.states.find((s) => s.id === id)?.label}</bdi>
                    </th>
                    {matrix.net_coefficients[i].map((value, j) => (
                      <td key={j}>
                        <bdi>{value}</bdi>
                      </td>
                    ))}
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <button className="secondary" onClick={() => download('stoichiometry.json', matrix)}>
            {t('Save JSON')}
          </button>
        </section>
      )}
    </Boundary>
  );
}

function Ownership({ network }: { network: Network }) {
  const { client, admin } = useSession();
  const t = useT();
  const [offset, setOffset] = useState(0);
  const [revision, setRevision] = useState(0);
  const [error, setError] = useState<unknown>();
  const [busy, setBusy] = useState(false);
  const [saved, setSaved] = useState(false);
  const [owners, setOwners] = useState<Record<string, string>>({});
  const [reasons, setReasons] = useState<Record<string, string>>({});
  const models = useResource(
    async (signal) => {
      const [reference, native, cellml] = await Promise.all([
        client.request<Schema['ModelPackage'][]>('/v1/models?limit=50', { signal }),
        client.request<Schema['NativeModelPackage'][]>('/v1/native-models?limit=50', { signal }),
        client.request<Schema['CellMLModelPackage'][]>('/v1/cellml-models?limit=50', { signal }),
      ]);
      const same = (c: Schema['ScientificContext']) =>
        c.species === network.context.species &&
        c.biological_system === network.context.biological_system &&
        JSON.stringify(c.conditions) === JSON.stringify(network.context.conditions);
      return [
        ...reference
          .filter((m) => same(m.context))
          .map((m) => ({ ...ref(m), title: m.title, kind: 'reference' as const })),
        ...native
          .filter((m) => same(m.spec.context))
          .map((m) => ({ ...ref(m.spec), title: m.spec.title, kind: 'native' as const })),
        ...cellml
          .filter((m) => same(m.spec.context))
          .map((m) => ({ ...ref(m.spec), title: m.spec.title, kind: 'cellml' as const })),
      ] as ModelChoice[];
    },
    [client, network],
  );
  const records = useResource(
    (signal) =>
      client.request<Schema['ProcessOwnershipPlan'][]>(
        `/v1/biology/process-ownership?limit=20&offset=${offset}`,
        { signal },
      ),
    [client, offset, revision],
  );
  return (
    <div className="stack">
      <p className="notice">
        {t(
          'Each process needs one declared model owner or an explicit exclusion. Ownership is unreviewed and does not authorize execution.',
        )}
      </p>
      <Boundary resource={models}>
        {(choices) => (
          <details className="surface">
            <summary>{t('Declare process ownership')}</summary>
            <form
              className="stack"
              onSubmit={async (e) => {
                e.preventDefault();
                setBusy(true);
                setSaved(false);
                setError(undefined);
                const form = new FormData(e.currentTarget);
                const assignments = choices.flatMap((model, index) => {
                  const reaction_ids = network.reactions
                    .filter((r) => owners[r.id] === `${model.kind}:${identity(model)}`)
                    .map((r) => r.id);
                  return reaction_ids.length
                    ? [
                        {
                          owner_id: `owner.${index}`,
                          model_kind: model.kind,
                          model: ref(model),
                          reaction_ids,
                          rationale: reaction_ids.map((id) => reasons[id]).join('\n'),
                          evidence: [String(form.get('evidence'))],
                        },
                      ]
                    : [];
                });
                const spec = {
                  id: String(form.get('id')),
                  version: String(form.get('version')),
                  network: ref(network),
                  context: network.context,
                  assignments,
                  exclusions: network.reactions
                    .filter((r) => owners[r.id] === 'exclude')
                    .map((r) => ({ reaction_id: r.id, reason: reasons[r.id] })),
                };
                try {
                  await client.request('/v1/biology/process-ownership', { body: spec });
                  setSaved(true);
                  setOffset(0);
                  setRevision((v) => v + 1);
                } catch (err) {
                  setError(err);
                } finally {
                  setBusy(false);
                }
              }}
            >
              <fieldset disabled={!admin || busy} className="stack">
                <div className="form-grid">
                  <RecordIdentity />
                  <Field label="Evidence reference">
                    <input name="evidence" required maxLength={2000} />
                  </Field>
                </div>
                <p className="muted">
                  {t(
                    'Choose from the first 50 models per format with exactly matching context. Other exact references can be registered with the documented CLI.',
                  )}
                </p>
                {network.reactions.map((r) => (
                  <section className="process-owner-row" key={r.id}>
                    <h3>
                      <bdi>{r.label}</bdi>
                    </h3>
                    <div className="form-grid">
                      <Field label="Process owner">
                        <select
                          required
                          value={owners[r.id] || ''}
                          onChange={(e) => setOwners({ ...owners, [r.id]: e.target.value })}
                        >
                          <option value="">{t('Choose an owner or exclusion')}</option>
                          <option value="exclude">{t('Explicitly exclude this process')}</option>
                          {choices.map((m) => (
                            <option key={m.kind + identity(m)} value={`${m.kind}:${identity(m)}`}>
                              {m.title} · {m.kind} · {m.version}
                            </option>
                          ))}
                        </select>
                      </Field>
                      <Field label="Ownership rationale or exclusion reason">
                        <input
                          required
                          value={reasons[r.id] || ''}
                          maxLength={600}
                          onChange={(e) => setReasons({ ...reasons, [r.id]: e.target.value })}
                        />
                      </Field>
                    </div>
                  </section>
                ))}
                <button className="primary">
                  {t(busy ? 'Working…' : 'Save ownership declaration')}
                </button>
              </fieldset>
              {!admin && <p className="notice">{t('Read-only session')}</p>}
              {error !== undefined && <ErrorBox error={error} />}
              {saved && (
                <p role="status" className="notice success">
                  {t('Saved immutable record')}
                </p>
              )}
            </form>
          </details>
        )}
      </Boundary>
      <Boundary resource={records}>
        {(items) => (
          <>
            <h2>{t('Ownership declarations')}</h2>
            {items.length ? (
              items.map((p) => (
                <article key={identity(p.spec)} className="surface stack">
                  <h3>
                    <code>{identity(p.spec)}</code>
                  </h3>
                  <Meta
                    items={[
                      ['Process network', identity(p.spec.network)],
                      ['Assigned models', p.spec.assignments.length],
                      ['Excluded processes', p.spec.exclusions.length],
                      ['Approval', t(p.approval)],
                    ]}
                  />
                  <JsonDetails value={p} />
                </article>
              ))
            ) : (
              <Empty />
            )}
            <Pager offset={offset} count={items.length} setOffset={setOffset} />
          </>
        )}
      </Boundary>
    </div>
  );
}

function RecordIdentity() {
  return (
    <>
      <Field label="Record ID">
        <input name="id" required pattern="[a-z][a-z0-9_.-]{0,79}" maxLength={80} />
      </Field>
      <Field label="Version">
        <input name="version" defaultValue="1.0.0" required pattern="[0-9]+\.[0-9]+\.[0-9]+" />
      </Field>
    </>
  );
}

type ValueDraft = {
  state_id: string;
  kind: 'amount' | 'concentration' | 'count';
  value: string;
  unit: string;
};
function Frames({ network }: { network: Network }) {
  const { client, admin } = useSession();
  const t = useT();
  const selectable = network.states.filter((s) => s.kind !== 'set');
  const initial = (): ValueDraft => ({
    state_id: selectable[0].id,
    kind: 'amount',
    value: '',
    unit: 'nanomole',
  });
  const [values, setValues] = useState<ValueDraft[]>([initial()]);
  const [offset, setOffset] = useState(0);
  const [revision, setRevision] = useState(0);
  const [busy, setBusy] = useState(false);
  const [saved, setSaved] = useState(false);
  const [error, setError] = useState<unknown>();
  const records = useResource(
    (signal) =>
      client.request<Schema['MolecularStateFrame'][]>(
        `/v1/biology/state-frames?limit=20&offset=${offset}`,
        { signal },
      ),
    [client, offset, revision],
  );
  const update = (index: number, change: Partial<ValueDraft>) =>
    setValues((rows) => rows.map((r, i) => (i === index ? { ...r, ...change } : r)));
  async function submit(event: FormEvent<HTMLFormElement>) {
    event.preventDefault();
    const form = new FormData(event.currentTarget);
    setBusy(true);
    setSaved(false);
    setError(undefined);
    const body = {
      id: String(form.get('id')),
      version: String(form.get('version')),
      network: ref(network),
      instance_id: String(form.get('instance')),
      context: network.context,
      time: { value: Number(form.get('time')), unit: String(form.get('timeUnit')) },
      origin: String(form.get('origin')),
      values: values.map((v) => ({
        state_id: v.state_id,
        kind: v.kind,
        quantity: { value: Number(v.value), unit: v.unit },
      })),
      evidence: [String(form.get('evidence'))],
      limitations: [String(form.get('limitations'))],
    };
    try {
      await client.request('/v1/biology/state-frames', { body });
      setSaved(true);
      setOffset(0);
      setRevision((v) => v + 1);
    } catch (err) {
      setError(err);
    } finally {
      setBusy(false);
    }
  }
  return (
    <div className="stack">
      <p className="notice">
        {t(
          'State frames retain declared sample quantities in original units. Missing states remain unknown; frames do not run or resume simulations.',
        )}
      </p>
      {selectable.length > 0 && (
        <details className="surface">
          <summary>{t('Register state frame')}</summary>
          <form className="stack" onSubmit={submit}>
            <fieldset className="stack" disabled={!admin || busy}>
              <div className="form-grid">
                <RecordIdentity />
                <Field label="Instance ID">
                  <input name="instance" required pattern="[a-z][a-z0-9_.-]{0,79}" maxLength={80} />
                </Field>
                <Field label="Data origin">
                  <select name="origin">
                    <option value="synthetic">{t('Synthetic')}</option>
                    <option value="experimental">{t('Experimental')}</option>
                    <option value="model_output">{t('Model output')}</option>
                  </select>
                </Field>
                <Field label="Sample time">
                  <input name="time" type="number" min="0" step="any" defaultValue="0" required />
                </Field>
                <Field label="Time unit">
                  <input name="timeUnit" defaultValue="second" required maxLength={80} />
                </Field>
              </div>
              {values.map((v, i) => (
                <fieldset key={i} className="stack">
                  <legend>
                    {t('State value')} {i + 1}
                  </legend>
                  <div className="form-grid">
                    <Field label="Molecular state">
                      <select
                        value={v.state_id}
                        onChange={(e) => update(i, { state_id: e.target.value })}
                      >
                        {selectable.map((s) => (
                          <option key={s.id} value={s.id}>
                            {s.label}
                          </option>
                        ))}
                      </select>
                    </Field>
                    <Field label="Quantity kind">
                      <select
                        value={v.kind}
                        onChange={(e) => {
                          const kind = e.target.value as ValueDraft['kind'];
                          update(i, {
                            kind,
                            unit:
                              kind === 'count'
                                ? 'count'
                                : kind === 'amount'
                                  ? 'nanomole'
                                  : 'nanomole/liter',
                          });
                        }}
                      >
                        <option value="amount">{t('Amount')}</option>
                        <option value="concentration">{t('Volumetric concentration')}</option>
                        <option value="count">{t('Molecule count')}</option>
                      </select>
                    </Field>
                    <Field label="Value">
                      <input
                        type="number"
                        min="0"
                        step={v.kind === 'count' ? '1' : 'any'}
                        required
                        value={v.value}
                        onChange={(e) => update(i, { value: e.target.value })}
                      />
                    </Field>
                    <Field label="Original unit">
                      <input
                        value={v.unit}
                        required
                        maxLength={80}
                        onChange={(e) => update(i, { unit: e.target.value })}
                      />
                    </Field>
                  </div>
                  {values.length > 1 && (
                    <button
                      type="button"
                      className="secondary"
                      onClick={() => setValues((rows) => rows.filter((_, j) => j !== i))}
                    >
                      {t('Remove state value')} {i + 1}
                    </button>
                  )}
                </fieldset>
              ))}
              <button
                type="button"
                className="secondary"
                disabled={values.length >= 500}
                onClick={() => setValues((rows) => [...rows, initial()])}
              >
                {t('Add state value')}
              </button>
              <Field label="Evidence reference">
                <input name="evidence" required maxLength={2000} />
              </Field>
              <Field label="Limitations">
                <textarea name="limitations" required maxLength={2000} rows={3} />
              </Field>
              <button className="primary">{t(busy ? 'Working…' : 'Save state frame')}</button>
            </fieldset>
            {!admin && <p className="notice">{t('Read-only session')}</p>}
            {error !== undefined && <ErrorBox error={error} />}
            {saved && (
              <p className="notice success" role="status">
                {t('Saved immutable record')}
              </p>
            )}
          </form>
        </details>
      )}
      <Boundary resource={records}>
        {(items) => (
          <>
            <h2>{t('Retained state frames')}</h2>
            {items.length ? (
              items.map((f) => (
                <article key={identity(f.spec)} className="surface stack">
                  <h3>
                    <code>{identity(f.spec)}</code>
                  </h3>
                  <Meta
                    items={[
                      ['Process network', identity(f.spec.network)],
                      ['Instance ID', f.spec.instance_id],
                      ['Data origin', f.spec.origin],
                      ['Sample time', `${f.spec.time.value} ${f.spec.time.unit}`],
                      ['Approval', t(f.approval)],
                    ]}
                  />
                  <ul>
                    {f.spec.values.map((v) => (
                      <li key={v.state_id}>
                        <bdi>
                          {v.state_id} · {v.quantity.value} {v.quantity.unit} · {v.kind}
                        </bdi>
                      </li>
                    ))}
                  </ul>
                  <Evidence values={f.spec.evidence} />
                  <Limitations values={f.spec.limitations} />
                  <JsonDetails value={f} />
                </article>
              ))
            ) : (
              <Empty />
            )}
            <Pager offset={offset} count={items.length} setOffset={setOffset} />
          </>
        )}
      </Boundary>
    </div>
  );
}
