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

type Package = Schema['VocabularyPackage'];
type Term = Schema['VocabularyTermRecord'];

export function VocabularyExplorer() {
  const { client } = useSession();
  const [offset, setOffset] = useState(0);
  const [selected, setSelected] = useState('');
  const sources = useResource(
    (signal) =>
      client.request<Package[]>(`/v1/biology/vocabularies?limit=20&offset=${offset}`, { signal }),
    [client, offset],
  );
  return (
    <div className="stack">
      <div className="notice">
        {useT()(
          'Ontology classes and identifier annotations are declarations. They do not execute mechanisms or establish biological equivalence.',
        )}
      </div>
      <Boundary resource={sources}>
        {(items) =>
          items.length ? (
            <>
              <Field label="Vocabulary snapshot">
                <select
                  value={selected || identity(items[0].spec)}
                  onChange={(e) => setSelected(e.target.value)}
                >
                  {items.map((p) => (
                    <option key={identity(p.spec)} value={identity(p.spec)}>
                      {p.spec.title} · {p.spec.version}
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
                const source = items.find((p) => identity(p.spec) === selected) || items[0];
                return <TermBrowser key={identity(source.spec)} source={source} />;
              })()}
            </>
          ) : (
            <Empty />
          )
        }
      </Boundary>
    </div>
  );
}

function TermBrowser({ source }: { source: Package }) {
  const { client } = useSession();
  const t = useT();
  const [draft, setDraft] = useState('');
  const [mode, setMode] = useState('search');
  const [status, setStatus] = useState('current');
  const [offset, setOffset] = useState(0);
  const [query, setQuery] = useState({ text: '', mode: 'search', status: 'current' });
  const [selected, setSelected] = useState<string>();
  const [error, setError] = useState<unknown>();
  const [busy, setBusy] = useState(false);
  const base = '/v1/biology/vocabularies/' + versionPath(source.spec);
  const terms = useResource(
    async (signal) =>
      query.mode === 'exact'
        ? [
            await client.request<Term>(base + '/term?term_id=' + encodeURIComponent(query.text), {
              signal,
            }),
          ]
        : await client.request<Term[]>(
            `${base}/terms?q=${encodeURIComponent(query.text)}&limit=20&offset=${offset}${query.status === 'all' ? '' : '&obsolete=' + String(query.status === 'obsolete')}`,
            { signal },
          ),
    [client, base, query, offset],
  );
  const lookup = (id: string) => {
    setDraft(id);
    setMode('exact');
    setQuery({ text: id, mode: 'exact', status: 'all' });
    setOffset(0);
    setSelected(id);
  };
  return (
    <div className="stack">
      <section className="surface stack">
        <h2>
          <bdi>{source.spec.title}</bdi>
        </h2>
        <Meta
          items={[
            ['Identity', <code>{identity(source.spec)}</code>],
            ['Revision', <bdi>{source.spec.revision}</bdi>],
            ['Domain', source.spec.domain],
            ['License', source.spec.license],
            ['Source', <Evidence values={[source.spec.source_url, ...source.spec.attribution]} />],
            ['Terms', source.term_count.toLocaleString()],
            ['Obsolete terms', source.obsolete_count.toLocaleString()],
            ['Unresolved parents', source.unresolved_parent_ids.length],
          ]}
        />
        <div className="action-row">
          {(['raw', 'normalized'] as const).map((kind) => (
            <button
              disabled={busy}
              key={kind}
              onClick={async () => {
                setBusy(true);
                setError(undefined);
                try {
                  const raw = await client.request<ArrayBuffer>(base + '/artifacts/' + kind, {
                    bytes: true,
                  });
                  download(source.spec.id + '-' + kind, raw, 'application/octet-stream');
                } catch (e) {
                  setError(e);
                } finally {
                  setBusy(false);
                }
              }}
            >
              {t(kind === 'raw' ? 'Download original source' : 'Download normalized terms')}
            </button>
          ))}
        </div>
        {error !== undefined && <ErrorBox error={error} />}
        <Limitations values={source.spec.limitations} />
        <JsonDetails value={source} />
      </section>
      <form
        className="form-grid"
        onSubmit={(e) => {
          e.preventDefault();
          setQuery({ text: draft, mode, status });
          setOffset(0);
          setSelected(undefined);
        }}
      >
        <Field label="Term name or identifier">
          <input
            value={draft}
            maxLength={200}
            required={mode === 'exact'}
            onChange={(e) => setDraft(e.target.value)}
          />
        </Field>
        <Field label="Lookup mode">
          <select value={mode} onChange={(e) => setMode(e.target.value)}>
            <option value="search">{t('Search names and IDs')}</option>
            <option value="exact">{t('Exact ID or alias')}</option>
          </select>
        </Field>
        <Field label="Term status">
          <select
            value={status}
            disabled={mode === 'exact'}
            onChange={(e) => setStatus(e.target.value)}
          >
            <option value="current">{t('Current terms')}</option>
            <option value="obsolete">{t('Obsolete terms')}</option>
            <option value="all">{t('All terms')}</option>
          </select>
        </Field>
        <button className="primary">{t('Search')}</button>
      </form>
      <Boundary resource={terms}>
        {(records) => {
          const current = records.find(
            (r) => r.term.id === selected || r.requested_id === selected,
          );
          return (
            <div className="split-view">
              <div className="record-list">
                {records.length ? (
                  records.map((r) => (
                    <button
                      key={r.term.id}
                      className={`record ${current?.term.id === r.term.id ? 'selected' : ''}`}
                      onClick={() => setSelected(r.term.id)}
                    >
                      <strong>
                        <bdi>{r.term.label}</bdi>
                      </strong>
                      <span className="mono">{r.term.id}</span>
                      <span>
                        {t(r.term.obsolete ? 'Obsolete' : 'Current')}
                        {r.matched_alias ? ' · ' + t('Matched alias') : ''}
                      </span>
                    </button>
                  ))
                ) : (
                  <Empty text="No matching terms in this snapshot." />
                )}
                {query.mode === 'search' && (
                  <Pager
                    offset={offset}
                    count={records.length}
                    setOffset={(v) => {
                      setOffset(v);
                      setSelected(undefined);
                    }}
                  />
                )}
              </div>
              <div className="inspector">
                {current ? (
                  <TermDetail record={current} lookup={lookup} />
                ) : (
                  <Empty text="Choose a term to inspect its original declarations." />
                )}
              </div>
            </div>
          );
        }}
      </Boundary>
    </div>
  );
}

function TermDetail({ record, lookup }: { record: Term; lookup: (id: string) => void }) {
  const t = useT();
  const term = record.term;
  return (
    <div className="stack wrap-identifier">
      <h2>
        <bdi>{term.label}</bdi>
      </h2>
      {term.obsolete && (
        <div className="notice">
          {t(
            'Obsolete term. Replacement declarations are shown verbatim and never followed automatically.',
          )}
        </div>
      )}
      <Meta
        items={[
          ['Identity', <code>{term.id}</code>],
          ['Requested identifier', <code>{record.requested_id}</code>],
          ['Matched alias', String(record.matched_alias)],
          ['Namespace', term.namespace || '—'],
          ['Species', term.species || t('Not declared by the source')],
          ['Artifact SHA-256', <code>{record.term_digest}</code>],
        ]}
      />
      {term.definition && (
        <section>
          <h3>{t('Source definition')}</h3>
          <p className="preserve-lines">
            <bdi>{term.definition}</bdi>
          </p>
        </section>
      )}
      <section>
        <h3>{t('Declared parents')}</h3>
        {term.parents.length ? (
          <div className="action-row">
            {term.parents.map((p) => (
              <button key={p} onClick={() => lookup(p)}>
                {p}
              </button>
            ))}
          </div>
        ) : (
          <p>{t('No declared parents')}</p>
        )}
      </section>
      <JsonDetails label="Original term declarations" value={term} />
      {term.synonyms.length > 0 && (
        <section>
          <h3>{t('Original synonyms and scope')}</h3>
          <ul>
            {term.synonyms.map((s, i) => (
              <li key={i}>
                <bdi>{s}</bdi>
              </li>
            ))}
          </ul>
        </section>
      )}
      {term.replaced_by.length + term.consider.length > 0 && (
        <section>
          <h3>{t('Replacement annotations')}</h3>
          <JsonDetails value={{ replaced_by: term.replaced_by, consider: term.consider }} />
        </section>
      )}
      {term.relationships.length > 0 && (
        <section>
          <h3>{t('Declared relationships')}</h3>
          <div
            className="table-scroll"
            tabIndex={0}
            role="region"
            aria-label={t('Declared relationships')}
          >
            <table>
              <thead>
                <tr>
                  <th>{t('Relation')}</th>
                  <th>{t('Target')}</th>
                </tr>
              </thead>
              <tbody>
                {term.relationships.map((r, i) => (
                  <tr key={i}>
                    <td>{r.relation}</td>
                    <td>
                      <bdi>{r.target}</bdi>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </section>
      )}
      <Evidence values={term.xrefs} />
    </div>
  );
}

export function VocabularyMappings() {
  const { client } = useSession();
  const t = useT();
  const [tab, setTab] = useState('Annotations');
  const [offset, setOffset] = useState(0);
  const resource = useResource<
    (Schema['EntityVocabularyMapping'] | Schema['VocabularyMappingConflict'])[]
  >(
    (signal) =>
      tab === 'Annotations'
        ? client.request<Schema['EntityVocabularyMapping'][]>(
            `/v1/biology/vocabulary-mappings?limit=20&offset=${offset}`,
            { signal },
          )
        : client.request<Schema['VocabularyMappingConflict'][]>(
            `/v1/biology/vocabulary-mapping-conflicts?limit=20&offset=${offset}`,
            { signal },
          ),
    [client, tab, offset],
  );
  return (
    <div className="stack">
      <div className="notice">
        {t(
          'Annotations remain unreviewed. Competing exact mappings are preserved as conflicts; neither declaration is automatically approved or removed.',
        )}
      </div>
      <Tabs
        label="Annotation records"
        tabs={['Annotations', 'Mapping conflicts']}
        selected={tab}
        onSelect={(v) => {
          setTab(v);
          setOffset(0);
        }}
      />
      <button onClick={resource.refresh}>{t('Refresh')}</button>
      <Boundary resource={resource}>
        {(records) => (
          <>
            {records.length ? (
              records.map((r) => (
                <article className="surface stack" key={identity('spec' in r ? r.spec : r)}>
                  <h2 className="wrap-identifier">
                    <bdi>{identity('spec' in r ? r.spec : r)}</bdi>
                  </h2>
                  {'spec' in r ? (
                    <>
                      <Meta
                        items={[
                          ['Graph', <code>{identity(r.spec.graph)}</code>],
                          ['Entity', <code>{r.spec.entity_id}</code>],
                          ['Target', <code>{r.spec.term_id}</code>],
                          ['Relation', r.spec.relation],
                          ['Review state', r.approval],
                          [
                            'Context',
                            <bdi>
                              {r.spec.context.species} · {r.spec.context.biological_system}
                            </bdi>,
                          ],
                        ]}
                      />
                      <p>
                        <bdi>{r.spec.rationale}</bdi>
                      </p>
                      <Evidence values={r.spec.evidence} />
                    </>
                  ) : (
                    <Meta
                      items={[
                        ['Reason', r.reason],
                        ['Review state', r.resolution],
                        [
                          'Annotations',
                          r.mappings.map((m) => <code key={identity(m)}>{identity(m)} </code>),
                        ],
                      ]}
                    />
                  )}
                  <JsonDetails value={r} />
                </article>
              ))
            ) : (
              <Empty />
            )}
            <Pager offset={offset} count={records.length} setOffset={setOffset} />
          </>
        )}
      </Boundary>
      <MappingForm onDone={resource.refresh} />
    </div>
  );
}

function MappingForm({ onDone }: { onDone: () => void }) {
  const { client, admin } = useSession();
  const t = useT();
  const options = useResource(
    async (signal) => ({
      graphs: await client.request<Schema['BiologicalGraphSpec'][]>('/v1/biology/graphs', {
        signal,
      }),
      vocabularies: await client.request<Package[]>('/v1/biology/vocabularies?limit=50', {
        signal,
      }),
    }),
    [client],
  );
  return (
    <details className="surface editor">
      <summary>{t('Annotate an entity')}</summary>
      <Boundary resource={options}>
        {(data) =>
          data.graphs.length && data.vocabularies.length ? (
            <MappingEditor
              graphs={data.graphs}
              vocabularies={data.vocabularies}
              admin={admin}
              submit={(body) => client.request('/v1/biology/vocabulary-mappings', { body })}
              onDone={onDone}
            />
          ) : (
            <Empty text="Register a graph and admit a vocabulary snapshot first." />
          )
        }
      </Boundary>
    </details>
  );
}
function MappingEditor({
  graphs,
  vocabularies,
  admin,
  submit,
  onDone,
}: {
  graphs: Schema['BiologicalGraphSpec'][];
  vocabularies: Package[];
  admin: boolean;
  submit: (body: Schema['EntityVocabularyMappingSpec']) => Promise<unknown>;
  onDone: () => void;
}) {
  const t = useT();
  const [graphId, setGraphId] = useState(identity(graphs[0]));
  const [vocabulary, setVocabulary] = useState(identity(vocabularies[0].spec));
  const graph = graphs.find((g) => identity(g) === graphId) || graphs[0];
  const [entity, setEntity] = useState(graph.entities[0]?.id || '');
  const [id, setId] = useState('my.entity-annotation');
  const [version, setVersion] = useState('1.0.0');
  const [term, setTerm] = useState('');
  const [relation, setRelation] =
    useState<Schema['EntityVocabularyMappingSpec']['relation']>('related_match');
  const [rationale, setRationale] = useState('');
  const [evidence, setEvidence] = useState('');
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<unknown>();
  const [result, setResult] = useState<unknown>();
  const handle = async (e: FormEvent) => {
    e.preventDefault();
    setBusy(true);
    setError(undefined);
    setResult(undefined);
    try {
      const selected = vocabularies.find((v) => identity(v.spec) === vocabulary)!;
      const body: Schema['EntityVocabularyMappingSpec'] = {
        schema_version: '1.0.0',
        id,
        version,
        graph: { ...ref(graph), schema_version: '1.0.0' },
        entity_id: entity,
        vocabulary: { ...ref(selected.spec), schema_version: '1.0.0' },
        term_id: term,
        relation,
        context: graph.context,
        rationale,
        evidence: evidence
          .split('\n')
          .map((s) => s.trim())
          .filter(Boolean),
      };
      setResult(await submit(body));
      onDone();
    } catch (e) {
      setError(e);
    } finally {
      setBusy(false);
    }
  };
  return (
    <form className="stack" onSubmit={handle}>
      <div className="form-grid">
        <Field label="Identity">
          <input
            required
            value={id}
            disabled={!admin || busy}
            onChange={(e) => setId(e.target.value)}
          />
        </Field>
        <Field label="Version">
          <input
            required
            value={version}
            disabled={!admin || busy}
            onChange={(e) => setVersion(e.target.value)}
          />
        </Field>
        <Field label="Graph">
          <select
            value={graphId}
            disabled={!admin || busy}
            onChange={(e) => {
              setGraphId(e.target.value);
              setEntity(graphs.find((g) => identity(g) === e.target.value)?.entities[0]?.id || '');
            }}
          >
            {graphs.map((g) => (
              <option key={identity(g)} value={identity(g)}>
                {g.title} · {g.version}
              </option>
            ))}
          </select>
        </Field>
        <Field label="Entity">
          <select
            value={entity}
            disabled={!admin || busy}
            onChange={(e) => setEntity(e.target.value)}
          >
            {graph.entities.map((e) => (
              <option key={e.id} value={e.id}>
                {e.label} · {e.kind}
              </option>
            ))}
          </select>
        </Field>
        <Field label="Vocabulary snapshot">
          <select
            value={vocabulary}
            disabled={!admin || busy}
            onChange={(e) => setVocabulary(e.target.value)}
          >
            {vocabularies.map((v) => (
              <option key={identity(v.spec)} value={identity(v.spec)}>
                {v.spec.title} · {v.spec.version}
              </option>
            ))}
          </select>
        </Field>
        <Field label="Canonical term ID">
          <input
            required
            maxLength={200}
            value={term}
            disabled={!admin || busy}
            onChange={(e) => setTerm(e.target.value)}
          />
        </Field>
        <Field label="Relation">
          <select
            value={relation}
            disabled={!admin || busy}
            onChange={(e) => setRelation(e.target.value as typeof relation)}
          >
            {(
              [
                'related_match',
                'exact_match',
                'broad_match',
                'narrow_match',
                'rejected_match',
              ] as const
            ).map((r) => (
              <option key={r} value={r}>
                {r}
              </option>
            ))}
          </select>
        </Field>
      </div>
      <Meta
        items={[
          [
            'Context',
            <bdi>
              {graph.context.species} · {graph.context.biological_system}
            </bdi>,
          ],
        ]}
      />
      <Field label="Mapping rationale">
        <textarea
          required
          maxLength={4000}
          value={rationale}
          disabled={!admin || busy}
          onChange={(e) => setRationale(e.target.value)}
        />
      </Field>
      <Field label="Evidence, one reference per line">
        <textarea
          required
          value={evidence}
          disabled={!admin || busy}
          onChange={(e) => setEvidence(e.target.value)}
        />
      </Field>
      <button className="primary" disabled={!admin || busy}>
        {t(busy ? 'Saving…' : 'Register annotation')}
      </button>
      {error !== undefined && <ErrorBox error={error} />}
      {result !== undefined && (
        <div role="status">
          <p>{t('Saved immutable record')}</p>
          <JsonDetails value={result} />
        </div>
      )}
    </form>
  );
}
