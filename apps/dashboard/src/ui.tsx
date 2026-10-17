import { useEffect, useId, useRef, useState, type DependencyList, type ReactNode } from 'react';
import { ApiError, download, safeLink, type PlotData } from './api';
import { useT } from './i18n';

export function useResource<T>(
  load: (signal: AbortSignal) => Promise<T>,
  deps: DependencyList,
  poll = 0,
) {
  const [data, setData] = useState<T>();
  const [error, setError] = useState<unknown>();
  const [busy, setBusy] = useState(true);
  const [revision, refresh] = useState(0);
  useEffect(() => {
    const ctrl = new AbortController();
    let active = true,
      pending = false;
    setData(undefined);
    setError(undefined);
    setBusy(true);
    const run = async () => {
      if (pending || !active) return;
      pending = true;
      try {
        const value = await load(ctrl.signal);
        if (active) {
          setData(value);
          setError(undefined);
        }
      } catch (e) {
        if (active) setError(e);
      } finally {
        pending = false;
        if (active) setBusy(false);
      }
    };
    void run();
    const timer = poll
      ? setInterval(() => {
          if (!document.hidden) void run();
        }, poll)
      : undefined;
    return () => {
      active = false;
      ctrl.abort();
      clearInterval(timer);
    };
    // Each caller supplies explicit resource identities, never a changing loader.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [...deps, revision]);
  return { data, error, busy, refresh: () => refresh((r) => r + 1) };
}
export type Resource<T> = ReturnType<typeof useResource<T>>;
export function ErrorBox({ error, retry }: { error: unknown; retry?: () => void }) {
  const t = useT();
  const status = error instanceof ApiError ? error.status : -1;
  const immutable =
    error instanceof Error && /immutable version|different content/i.test(error.message);
  const heading =
    status === 403
      ? 'Access denied'
      : status === 409
        ? immutable
          ? 'Immutable version conflict'
          : 'State conflict'
        : status === 0
          ? 'Service unavailable'
          : 'Request failed';
  return (
    <div className="notice error" role="alert">
      <strong>{t(heading)}</strong>
      <p>{error instanceof Error ? error.message : t('Request failed')}</p>
      {status === 403 && <p>{t('Use an administrator token to make changes.')}</p>}
      {status === 409 && (
        <p>
          {t(
            immutable
              ? 'Use a new version or reload the existing record before retrying.'
              : 'Refresh the run state before retrying.',
          )}
        </p>
      )}
      {retry && (
        <button type="button" onClick={retry}>
          {t('Retry')}
        </button>
      )}
    </div>
  );
}
export function Loading() {
  const t = useT();
  return (
    <div className="loading" role="status">
      <span className="pulse" />
      {t('Loading workspace…')}
    </div>
  );
}
export function Empty({ text = 'No records on this page.' }: { text?: string }) {
  const t = useT();
  return (
    <div className="empty">
      <Icon name="layers" />
      <p className="empty-title">{t('Nothing here yet')}</p>
      <p>{t(text)}</p>
    </div>
  );
}
export function Boundary<T>({
  resource,
  children,
}: {
  resource: Resource<T>;
  children: (data: T) => ReactNode;
}) {
  return (
    <>
      {resource.error ? (
        <ErrorBox error={resource.error} retry={resource.refresh} />
      ) : resource.busy || resource.data === undefined ? (
        <Loading />
      ) : (
        children(resource.data)
      )}
    </>
  );
}
export function Field({
  label,
  children,
  help,
}: {
  label: string;
  children: ReactNode;
  help?: string;
}) {
  const t = useT();
  return (
    <label className="field">
      <span>{t(label)}</span>
      {children}
      {help && <small>{t(help)}</small>}
    </label>
  );
}
export function PageHeading({
  title,
  description,
  eyebrow,
}: {
  title: string;
  description: string;
  eyebrow: string;
}) {
  const t = useT();
  return (
    <header className="page-heading">
      <p className="eyebrow">{t(eyebrow)}</p>
      <h1>{t(title)}</h1>
      <p>{t(description)}</p>
    </header>
  );
}
export function Tabs({
  tabs,
  selected,
  onSelect,
  label = 'Details',
}: {
  tabs: string[];
  selected: string;
  onSelect: (tab: string) => void;
  label?: string;
}) {
  const t = useT();
  return (
    <nav className="tabs" aria-label={t(label)}>
      {tabs.map((tab) => (
        <button
          key={tab}
          aria-current={tab === selected ? 'page' : undefined}
          onClick={() => onSelect(tab)}
        >
          {t(tab)}
        </button>
      ))}
    </nav>
  );
}
export function Badge({ value }: { value: string }) {
  const t = useT();
  return (
    <span
      className={`badge ${['succeeded', 'running', 'queued', 'failed', 'cancelled', 'skipped'].includes(value) ? value : ''}`}
    >
      <span aria-hidden="true" />
      {t(value)}
    </span>
  );
}
export function Meta({ items }: { items: [string, ReactNode][] }) {
  const t = useT();
  return (
    <dl className="metadata">
      {items.map(([label, value]) => (
        <div key={label}>
          <dt>{t(label)}</dt>
          <dd>{value ?? '—'}</dd>
        </div>
      ))}
    </dl>
  );
}
export function JsonDetails({ value, label = 'Details' }: { value: unknown; label?: string }) {
  const t = useT();
  return (
    <details className="json-details">
      <summary>{t(label)}</summary>
      <pre dir="ltr">{JSON.stringify(value, null, 2)}</pre>
      <button type="button" onClick={() => download('peykar-record.json', value)}>
        {t('Save JSON')}
      </button>
    </details>
  );
}
export function Evidence({ values }: { values?: string[] }) {
  const t = useT();
  return !values?.length ? null : (
    <section className="evidence">
      <h3>{t('Evidence')}</h3>
      <ul>
        {values.map((v, i) => (
          <li key={i}>
            {safeLink(v) ? (
              <a href={safeLink(v)} target="_blank" rel="noreferrer noopener">
                {v}
                <span className="sr-only"> (external)</span>
              </a>
            ) : (
              <bdi>{v}</bdi>
            )}
          </li>
        ))}
      </ul>
    </section>
  );
}
export function Limitations({ values }: { values?: string[] }) {
  const t = useT();
  return !values?.length ? null : (
    <div className="notice">
      <strong>{t('Limitations')}</strong>
      <ul>
        {values.map((s, i) => (
          <li key={i}>
            <bdi>{s}</bdi>
          </li>
        ))}
      </ul>
    </div>
  );
}
export function Pager({
  offset,
  count,
  size = 20,
  setOffset,
}: {
  offset: number;
  count: number;
  size?: number;
  setOffset: (o: number) => void;
}) {
  const t = useT();
  return (
    <div className="pager">
      <span>{count ? `${offset + 1}–${offset + count}` : '0'}</span>
      <button disabled={offset === 0} onClick={() => setOffset(Math.max(0, offset - size))}>
        {t('Previous')}
      </button>
      <button disabled={count < size} onClick={() => setOffset(offset + size)}>
        {t('Next')}
      </button>
    </div>
  );
}
export function Icon({ name }: { name: string }) {
  const paths: Record<string, ReactNode> = {
    biology: (
      <>
        <circle cx="7" cy="6" r="3" />
        <circle cx="17" cy="12" r="3" />
        <circle cx="7" cy="19" r="2" />
        <path d="m9.6 7.6 4.8 2.8M14.5 14 9 18M7 9v8" />
      </>
    ),
    layers: (
      <>
        <path d="m12 3 9 5-9 5-9-5 9-5ZM3 12l9 5 9-5M3 16l9 5 9-5" />
      </>
    ),
    data: (
      <>
        <ellipse cx="12" cy="5" rx="8" ry="3" />
        <path d="M4 5v14c0 4 16 4 16 0V5M4 12c0 4 16 4 16 0" />
      </>
    ),
    experiment: (
      <>
        <path d="M9 3h6M10 3v7L4 20h16l-6-10V3M8 14h8" />
      </>
    ),
    chart: (
      <>
        <path d="M3 3v18h18M6 16l4-6 4 3 6-8" />
      </>
    ),
    check: (
      <>
        <path d="m12 3 8 3v6c0 5-8 9-8 9s-8-4-8-9V6l8-3Z" />
        <path d="m8 12 3 3 5-6" />
      </>
    ),
    arrow: <path d="M4 12h16m-6-6 6 6-6 6" />,
  };
  return (
    <svg
      className="icon"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="1.6"
      strokeLinecap="round"
      strokeLinejoin="round"
      aria-hidden="true"
    >
      {paths[name] || paths.layers}
    </svg>
  );
}
export function JsonForm({
  clientAction,
  label,
  initial,
  title,
  disabled = false,
  onDone,
}: {
  clientAction: (v: unknown) => Promise<unknown>;
  label: string;
  initial: unknown;
  title: string;
  disabled?: boolean;
  onDone?: (v: unknown) => void;
}) {
  const t = useT();
  const [text, setText] = useState(JSON.stringify(initial, null, 2));
  const [error, setError] = useState<unknown>();
  const [result, setResult] = useState<unknown>();
  const [busy, setBusy] = useState(false);
  return (
    <form
      className="stack"
      onSubmit={async (e) => {
        e.preventDefault();
        setBusy(true);
        setError(undefined);
        setResult(undefined);
        try {
          const result = await clientAction(JSON.parse(text));
          setResult(result);
          onDone?.(result);
        } catch (err) {
          setError(err);
        } finally {
          setBusy(false);
        }
      }}
    >
      <Field label={label}>
        <textarea
          className="code"
          dir="ltr"
          rows={16}
          value={text}
          onChange={(e) => {
            setText(e.target.value);
            setResult(undefined);
          }}
          required
          spellCheck={false}
          maxLength={1500000}
        />
      </Field>
      {error !== undefined && <ErrorBox error={error} />}
      {result !== undefined && (
        <div className="notice success" role="status">
          <strong>{t('Saved immutable record')}</strong>
          <JsonDetails value={result} />
        </div>
      )}
      <button className="primary" disabled={disabled || busy}>
        {t(busy ? 'Working…' : title)}
      </button>
    </form>
  );
}
export function Plot({ data }: { data: PlotData }) {
  const t = useT();
  const id = useId();
  const [output, setOutput] = useState(0);
  const [index, setIndex] = useState(0);
  const [table, setTable] = useState(false);
  const [offset, setOffset] = useState(0);
  const plotRoot = useRef<HTMLElement>(null);
  const [compact, setCompact] = useState(true);
  useEffect(() => {
    const element = plotRoot.current;
    if (!element) return;
    const observer = new ResizeObserver((entries) =>
      setCompact(entries[0].contentRect.width < 620),
    );
    observer.observe(element);
    return () => observer.disconnect();
  }, []);
  const left = compact ? 90 : 70,
    plotWidth = compact ? 300 : 790;
  const series = data.outputs[output] || data.outputs[0];
  const time = data.time.values;
  const values = series.values;
  const xmin = time[0],
    xmax = time[time.length - 1];
  let ymin = values.reduce((a, b) => Math.min(a, b), Infinity),
    ymax = values.reduce((a, b) => Math.max(a, b), -Infinity);
  if (ymin === ymax) {
    const delta = Math.abs(ymin) * 0.05 || 1;
    ymin -= delta;
    ymax += delta;
  }
  const x = (v: number) => left + ((v - xmin) / (xmax - xmin)) * plotWidth;
  const y = (v: number) => 280 - ((v - ymin) / (ymax - ymin)) * 240;
  const stride = Math.max(1, Math.ceil(time.length / 1200));
  const points = time
    .map((v, i) => (i % stride === 0 || i === time.length - 1 ? `${x(v)},${y(values[i])}` : ''))
    .filter(Boolean)
    .join(' ');
  const selected = Math.min(index, time.length - 1);
  const fmt = (v: number) => Number(v.toPrecision(5)).toString();
  return (
    <section ref={plotRoot} className="plot">
      <div className="plot-toolbar">
        <Field label="Output">
          <select value={output} onChange={(e) => setOutput(Number(e.target.value))}>
            {data.outputs.map((s, i) => (
              <option key={s.name} value={i}>
                {s.name} · {s.unit}
              </option>
            ))}
          </select>
        </Field>
        <div className="sample-readout">
          <strong>
            <bdi>{fmt(values[selected])}</bdi>
          </strong>
          <span>
            <bdi>
              {series.unit} · {fmt(time[selected])} {data.time.unit}
            </bdi>
          </span>
        </div>
      </div>
      <svg
        className={`scientific-chart ${compact ? 'compact' : ''}`}
        viewBox={compact ? '0 0 430 340' : '0 0 920 340'}
        role="img"
        aria-labelledby={`${id}-title ${id}-desc`}
      >
        <title id={`${id}-title`}>
          {series.name} ({series.unit})
        </title>
        <desc id={`${id}-desc`}>
          {time.length} {t('Samples')}. {t('Time')}: {xmin}–{xmax} {data.time.unit}. {ymin}–{ymax}{' '}
          {series.unit}.
        </desc>
        {[0, 1, 2, 3, 4].map((i) => (
          <g key={i}>
            <line
              className="gridline"
              x1={left}
              x2={left + plotWidth}
              y1={40 + i * 60}
              y2={40 + i * 60}
            />
            <text x={left - 12} y={44 + i * 60} textAnchor="end">
              {Number((ymax - (i / 4) * (ymax - ymin)).toPrecision(3))}
            </text>
            <text x={left + (i * plotWidth) / 4} y="306" textAnchor="middle">
              {!compact || i % 2 === 0
                ? Number((xmin + (i / 4) * (xmax - xmin)).toPrecision(3))
                : ''}
            </text>
          </g>
        ))}
        <polyline className="data-line" points={points} />
        <line
          className="sample-line"
          x1={x(time[selected])}
          x2={x(time[selected])}
          y1="40"
          y2="280"
        />
        <circle className="data-dot" cx={x(time[selected])} cy={y(values[selected])} r="5" />
        <text x={left + plotWidth / 2} y="334" textAnchor="middle">
          {t('Time')} ({data.time.unit})
        </text>
      </svg>
      <Field label="Sample">
        <input
          type="range"
          min="0"
          max={time.length - 1}
          value={selected}
          onChange={(e) => setIndex(Number(e.target.value))}
          aria-valuetext={`${time[selected]} ${data.time.unit}: ${values[selected]} ${series.unit}`}
        />
      </Field>
      <p className="muted">
        {time.length} {t('Samples')} ·{' '}
        {t('Sampled values only. No cell geometry or movement is reconstructed.')}
      </p>
      {stride > 1 && (
        <p className="muted">
          {t('Plot decimated for display; the table and downloads retain all samples.')}
        </p>
      )}
      <button onClick={() => setTable(!table)} aria-expanded={table}>
        {t(table ? 'Hide data' : 'Show data')}
      </button>
      {table && (
        <>
          <div className="table-scroll" tabIndex={0} role="region" aria-label={t('Data table')}>
            <table>
              <caption>{t('Data table')}</caption>
              <thead>
                <tr>
                  <th>
                    {t('Time')} ({data.time.unit})
                  </th>
                  <th>
                    {series.name} ({series.unit})
                  </th>
                </tr>
              </thead>
              <tbody>
                {time.slice(offset, offset + 20).map((v, i) => (
                  <tr key={i}>
                    <td>{v}</td>
                    <td>{values[offset + i]}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
          <Pager offset={offset} count={Math.min(20, time.length - offset)} setOffset={setOffset} />
        </>
      )}
    </section>
  );
}
export function useAction() {
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<unknown>();
  const keys = useRef(new Map<string, string>());
  async function act<T>(body: unknown, fn: (key: string) => Promise<T>): Promise<T | undefined> {
    if (busy) return;
    setBusy(true);
    setError(undefined);
    // A deliberate retry of the same payload retains its key after an uncertain
    // network response. A successful action permits a new intentional execution.
    const fingerprint = JSON.stringify(body);
    let key = keys.current.get(fingerprint);
    if (!key) {
      key = crypto.randomUUID();
      keys.current.set(fingerprint, key);
    }
    try {
      const value = await fn(key);
      keys.current.delete(fingerprint);
      return value;
    } catch (e) {
      setError(e);
      return undefined;
    } finally {
      setBusy(false);
    }
  }
  return { busy, error, act };
}
