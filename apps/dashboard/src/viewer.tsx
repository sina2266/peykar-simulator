import { useState } from 'react';
import {
  type RunKind,
  type Run,
  type Schema,
  type PlotData,
  runKinds,
  kindNames,
  terminal,
  download,
  nativeSeries,
} from './api';
import { useSession } from './session';
import { useT } from './i18n';
import {
  useResource,
  useAction,
  Boundary,
  PageHeading,
  Field,
  Empty,
  Badge,
  Pager,
  Meta,
  JsonDetails,
  Plot,
  Limitations,
  ErrorBox,
} from './ui';

export function Viewer({
  selectedKind,
  selectedId,
}: {
  selectedKind?: RunKind;
  selectedId?: string;
}) {
  const { client, openRun } = useSession();
  const t = useT();
  const [kind, setKind] = useState<RunKind>(selectedKind || 'simulations');
  const [offset, setOffset] = useState(0);
  const [lookup, setLookup] = useState('');
  const runs = useResource(
    (s) =>
      client.request<Run[]>(`/v1/${kind}?limit=20&offset=${offset}`, {
        signal: s,
      }),
    [client, kind, offset],
    5000,
  );
  if (selectedId && selectedKind) return <RunDetail kind={selectedKind} id={selectedId} />;
  return (
    <>
      <PageHeading
        eyebrow="Simulation Viewer"
        title="Computed results, with provenance"
        description="Resume persisted runs. Inspect status, artifacts and actual sampled quantities."
      />
      <div className="section-tools">
        <Field label="Queue">
          <select
            value={kind}
            onChange={(e) => {
              setKind(e.target.value as RunKind);
              setOffset(0);
            }}
          >
            {runKinds.map((k) => (
              <option key={k} value={k}>
                {t(kindNames[k])}
              </option>
            ))}
          </select>
        </Field>
        <form
          className="lookup"
          onSubmit={(e) => {
            e.preventDefault();
            openRun(kind, lookup);
          }}
        >
          <Field label="Run ID">
            <input
              required
              maxLength={100}
              value={lookup}
              onChange={(e) => setLookup(e.target.value)}
            />
          </Field>
          <button>{t('Open run')}</button>
        </form>
      </div>
      <Boundary resource={runs}>
        {(rows) => (
          <section className="surface">
            <div className="section-tools">
              <h2>{t(kindNames[kind])}</h2>
              <button onClick={runs.refresh}>{t('Refresh')}</button>
            </div>
            {rows.length ? (
              <div className="table-scroll">
                <table>
                  <thead>
                    <tr>
                      <th>{t('Run ID')}</th>
                      <th>{t('Status')}</th>
                      <th>{t('Created')}</th>
                      <th>{t('Details')}</th>
                    </tr>
                  </thead>
                  <tbody>
                    {rows.map((r) => (
                      <tr key={r.id}>
                        <td>
                          <code>{r.id}</code>
                        </td>
                        <td>
                          <Badge value={r.status} />
                        </td>
                        <td>
                          <time dateTime={r.created_at}>
                            {new Date(r.created_at).toLocaleString()}
                          </time>
                        </td>
                        <td>
                          <button
                            onClick={() => openRun(kind, r.id)}
                            aria-label={`${t('Open run')} ${r.id}`}
                          >
                            {t('Open run')} →
                          </button>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            ) : (
              <Empty />
            )}
            <Pager offset={offset} count={rows.length} setOffset={setOffset} />
          </section>
        )}
      </Boundary>
    </>
  );
}
function RunDetail({ kind, id }: { kind: RunKind; id: string }) {
  const { client, admin, openRun } = useSession();
  const t = useT();
  const action = useAction();
  const [confirm, setConfirm] = useState(false);
  const record = useResource(
    (s) =>
      client.request<Run>(`/v1/${kind}/${encodeURIComponent(id)}`, {
        signal: s,
      }),
    [client, kind, id],
    2500,
  );
  const events = useResource(
    (s) =>
      client.request<
        {
          sequence: number;
          occurred_at: string;
          event: string;
          detail: string;
        }[]
      >(`/v1/${kind}/${encodeURIComponent(id)}/events`, { signal: s }),
    [client, kind, id],
    3000,
  );
  const [tab, setTab] = useState('Results');
  return (
    <>
      <PageHeading
        eyebrow={kindNames[kind]}
        title="Computed results, with provenance"
        description="Resume persisted runs. Inspect status, artifacts and actual sampled quantities."
      />
      <Boundary resource={record}>
        {(r) => (
          <>
            <section className="surface">
              <div className="section-tools">
                <div>
                  <Badge value={r.status} />
                  <h2 className="run-id">
                    <bdi>{r.id}</bdi>
                  </h2>
                </div>
                {!terminal(r.status) && (
                  <button
                    className="danger"
                    disabled={!admin || r.cancel_requested}
                    onClick={() => setConfirm(true)}
                  >
                    {t(r.cancel_requested ? 'Cancellation requested' : 'Cancel run')}
                  </button>
                )}
              </div>
              <Meta
                items={[
                  ['Created', new Date(r.created_at).toLocaleString()],
                  ['Updated', new Date(r.updated_at).toLocaleString()],
                  [
                    'Manifest SHA-256',
                    'manifest_digest' in r ? (
                      <code>{r.manifest_digest}</code>
                    ) : 'flow_digest' in r ? (
                      <code>{r.flow_digest}</code>
                    ) : 'batch_digest' in r ? (
                      <code>{r.batch_digest}</code>
                    ) : (
                      <code>{r.experiment_digest}</code>
                    ),
                  ],
                  ['Result SHA-256', 'result_digest' in r ? <code>{r.result_digest}</code> : null],
                ]}
              />
              {r.error && (
                <div className="notice error" role="alert">
                  <strong>{t('failed')}</strong>
                  <p>{r.error}</p>
                </div>
              )}
              {r.status === 'queued' && (
                <div className="notice">
                  <strong>{t('Waiting for a worker')}</strong>
                  <p>
                    {t(
                      'The queue is durable. Check that the appropriate worker or native agent is running.',
                    )}
                  </p>
                </div>
              )}
              {confirm && (
                <div className="notice" role="group" aria-label={t('Confirm cancellation')}>
                  <strong>{t('Confirm cancellation')}</strong>
                  <p>
                    {t(
                      'Pending children will be skipped; running children receive a cancellation request.',
                    )}
                  </p>
                  <div className="actions">
                    <button
                      className="danger"
                      disabled={action.busy}
                      onClick={() =>
                        void action.act({ kind, id }, async () => {
                          const result = await client.request(
                            `/v1/${kind}/${encodeURIComponent(id)}/cancel`,
                            { body: {} },
                          );
                          setConfirm(false);
                          record.refresh();
                          return result;
                        })
                      }
                    >
                      {t('Confirm cancellation')}
                    </button>
                    <button onClick={() => setConfirm(false)}>{t('Keep running')}</button>
                  </div>
                </div>
              )}
              {action.error !== undefined && <ErrorBox error={action.error} />}
              {('nodes' in r || 'members' in r) && (
                <div className="child-grid">
                  {('nodes' in r ? r.nodes : r.members).map((n) => (
                    <article key={'node_id' in n ? n.node_id : n.member_id}>
                      <div>
                        <strong>{'node_id' in n ? n.node_id : n.member_id}</strong>
                        <Badge value={n.status} />
                      </div>
                      {n.run_id && (
                        <button
                          onClick={() =>
                            openRun(
                              'queue' in n && n.queue === 'native' ? 'native-runs' : 'simulations',
                              n.run_id!,
                            )
                          }
                        >
                          {t('Open run')} →
                        </button>
                      )}
                      {'transfer_digest' in n &&
                        typeof n.transfer_digest === 'string' &&
                        'node_id' in n && <TransferDetail id={id} node={n.node_id} />}
                    </article>
                  ))}
                </div>
              )}
            </section>
            <nav className="tabs" aria-label={t('Details')}>
              {['Results', 'Events', 'Manifest'].map((v) => (
                <button
                  key={v}
                  aria-current={tab === v ? 'page' : undefined}
                  onClick={() => setTab(v)}
                >
                  {t(v)}
                </button>
              ))}
            </nav>
            {tab === 'Results' ? (
              <RunOutput kind={kind} record={r} />
            ) : tab === 'Events' ? (
              <Boundary resource={events}>
                {(items) => (
                  <ol className="timeline">
                    {items.map((e) => (
                      <li key={e.sequence}>
                        <span className="timeline-dot" />
                        <div>
                          <strong>{e.event}</strong>
                          <time dateTime={e.occurred_at}>
                            {new Date(e.occurred_at).toLocaleString()}
                          </time>
                          {e.detail !== '{}' && <pre>{e.detail}</pre>}
                        </div>
                      </li>
                    ))}
                  </ol>
                )}
              </Boundary>
            ) : (
              <Manifest kind={kind} record={r} />
            )}
          </>
        )}
      </Boundary>
    </>
  );
}
function Manifest({ kind, record }: { kind: RunKind; record: Run }) {
  const { client } = useSession();
  const t = useT();
  const resource = useResource(
    async (s) => {
      if (kind === 'simulations' || kind === 'native-runs')
        return await client.request(`/v1/${kind}/${encodeURIComponent(record.id)}/manifest`, {
          signal: s,
        });
      const reference =
        'flow' in record
          ? record.flow
          : 'batch' in record
            ? record.batch
            : 'experiment' in record
              ? record.experiment
              : null;
      const type =
        kind === 'hybrid-runs' ? 'hybrid-flows' : kind === 'batch-runs' ? 'batches' : 'experiments';
      return reference
        ? await client.request(
            `/v1/${type}/${encodeURIComponent(reference.id)}/versions/${encodeURIComponent(reference.version)}`,
            { signal: s },
          )
        : record;
    },
    [client, kind, record.id],
  );
  return (
    <Boundary resource={resource}>
      {(value) => (
        <section className="surface">
          <h2>{t('Manifest')}</h2>
          <button onClick={() => download(`peykar-${record.id}-manifest.json`, value)}>
            {t('Save JSON')}
          </button>
          <pre className="manifest" dir="ltr">
            {JSON.stringify(value, null, 2)}
          </pre>
        </section>
      )}
    </Boundary>
  );
}
function RunOutput({ kind, record }: { kind: RunKind; record: Run }) {
  const { client, openRun } = useSession();
  const t = useT();
  const ready = record.status === 'succeeded' || (kind === 'batch-runs' && terminal(record.status));
  const resource = useResource(
    async (signal) => {
      if (!ready) return null;
      const result = await client.request<unknown>(
        `/v1/${kind}/${encodeURIComponent(record.id)}/results`,
        { signal },
      );
      if (kind === 'native-runs') {
        const [csv, metadata] = await Promise.all([
          client.request<string>(
            `/v1/native-runs/${encodeURIComponent(record.id)}/artifacts/native.csv`,
            { signal, text: true },
          ),
          client.request<Parameters<typeof nativeSeries>[1]>(
            `/v1/native-runs/${encodeURIComponent(record.id)}/artifacts/quantities.json`,
            { signal },
          ),
        ]);
        try {
          return { result, plot: nativeSeries(csv, metadata), csv, plotError: undefined };
        } catch (plotError) {
          return { result, plot: undefined, csv, plotError };
        }
      }
      return {
        result,
        plot: kind === 'simulations' ? (result as PlotData) : undefined,
        csv: undefined,
        plotError: undefined,
      };
    },
    [client, kind, record.id, record.status],
  );
  return (
    <Boundary resource={resource}>
      {(value) =>
        value === null ? (
          <Empty text="Results become available after successful execution. Failed and cancelled jobs publish no invented output." />
        ) : (
          <section className="surface">
            <div className="section-tools">
              <h2>{t('Results')}</h2>
              <button onClick={() => download(`peykar-${record.id}-results.json`, value.result)}>
                {t('Save JSON')}
              </button>
            </div>
            {value.plotError !== undefined && <ErrorBox error={value.plotError} />}
            {value.plot && <Plot data={value.plot} />}
            {kind === 'native-runs' && (
              <div className="actions">
                <button
                  onClick={() => download(`peykar-${record.id}-native.csv`, value.csv!, 'text/csv')}
                >
                  CSV
                </button>
                <JsonDetails
                  label="Artifacts"
                  value={(value.result as Schema['NativeRunResult']).artifacts}
                />
              </div>
            )}
            {kind === 'batch-runs' && (
              <BatchSummary report={value.result as Schema['BatchReport']} />
            )}
            {kind === 'hybrid-runs' && (
              <div className="stack">
                {(
                  value.result as {
                    nodes: {
                      node_id: string;
                      queue: string;
                      run_id: string;
                      terminal_values: Record<string, { value: number; unit: string }>;
                    }[];
                  }
                ).nodes.map((n) => (
                  <article className="result-node" key={n.node_id}>
                    <h3>{n.node_id}</h3>
                    <Meta
                      items={Object.entries(n.terminal_values).map(([name, q]) => [
                        name,
                        `${q.value} ${q.unit}`,
                      ])}
                    />
                    <button
                      onClick={() =>
                        openRun(n.queue === 'native' ? 'native-runs' : 'simulations', n.run_id)
                      }
                    >
                      {t('View result')} →
                    </button>
                  </article>
                ))}
              </div>
            )}
            <Limitations values={(value.result as { limitations?: string[] }).limitations} />
            <JsonDetails value={value.result} />
          </section>
        )
      }
    </Boundary>
  );
}
function BatchSummary({ report }: { report: Schema['BatchReport'] }) {
  const t = useT();
  return (
    <>
      <div className="notice">
        Within-case terminal sample variability only. Sample SD is absent for fewer than two
        successful replicates. Failed, skipped and cancelled members are excluded; no confidence
        interval or biological uncertainty is inferred.
      </div>
      {report.groups.map((g) => (
        <article className="result-node" key={`${g.case_id}:${g.grid_index}`}>
          <div className="section-tools">
            <h3>
              {g.case_id} / {g.grid_index}
            </h3>
            <Badge value={g.complete ? 'succeeded' : 'incomplete'} />
          </div>
          <p>
            {g.successful_replicates} / {g.expected_replicates} successful ·{' '}
            {g.excluded_member_ids.length} excluded
          </p>
          <div className="table-scroll">
            <table>
              <thead>
                <tr>
                  <th>{t('Output')}</th>
                  <th>Mean</th>
                  <th>Sample SD</th>
                  <th>Min / max</th>
                  <th>{t('Unit')}</th>
                </tr>
              </thead>
              <tbody>
                {g.endpoints.map((e) => (
                  <tr key={e.output}>
                    <td>{e.output}</td>
                    <td>{e.mean}</td>
                    <td>{e.sample_standard_deviation ?? '—'}</td>
                    <td>
                      {e.minimum} / {e.maximum}
                    </td>
                    <td>{e.unit}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <JsonDetails
            value={{
              parameters: g.parameters,
              excluded_member_ids: g.excluded_member_ids,
            }}
          />
        </article>
      ))}
    </>
  );
}
function TransferDetail({ id, node }: { id: string; node: string }) {
  const { client } = useSession();
  const [open, setOpen] = useState(false);
  const resource = useResource(
    async (s) =>
      open
        ? await client.request(
            `/v1/hybrid-runs/${encodeURIComponent(id)}/nodes/${encodeURIComponent(node)}/transfers`,
            { signal: s },
          )
        : null,
    [client, id, node, open],
  );
  return (
    <>
      <button onClick={() => setOpen((v) => !v)} aria-expanded={open}>
        Transfer provenance
      </button>
      {open && <Boundary resource={resource}>{(v) => <JsonDetails value={v} />}</Boundary>}
    </>
  );
}
