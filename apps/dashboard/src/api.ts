import type { components } from './generated/api';
export type Schema = components['schemas'];
export type Model = Schema['ModelPackage'];
export type NativeModel = Schema['NativeModelPackage'];
export type Input<T> = T extends readonly (infer U)[]
  ? Input<U>[]
  : T extends object
    ? {
        [K in keyof T as K extends 'schema_version' ? never : K]: Input<T[K]>;
      } & { schema_version?: '1.0.0' }
    : T;
export type Plan = Input<Schema['SimulationPlan']>;
export type NativePlan = Input<Schema['NativeRunSpec']>;
export type Ref = { id: string; version: string };
export type CatalogItem = Ref & { title: string; digest: string };
export const ref = (r: Ref): Ref => ({ id: r.id, version: r.version });
export const identity = (r: Ref) => `${r.id}@${r.version}`;
export const versionPath = (r: Ref) =>
  `${encodeURIComponent(r.id)}/versions/${encodeURIComponent(r.version)}`;
export type RunKind =
  | 'simulations'
  | 'native-runs'
  | 'experiment-runs'
  | 'hybrid-runs'
  | 'batch-runs';
export const runKinds: RunKind[] = [
  'simulations',
  'native-runs',
  'hybrid-runs',
  'experiment-runs',
  'batch-runs',
];
export const kindNames: Record<RunKind, string> = {
  simulations: 'Reference',
  'native-runs': 'Native',
  'hybrid-runs': 'Scalar flow',
  'experiment-runs': 'Reference DAG',
  'batch-runs': 'Cohort',
};
export type Run =
  | Schema['RunRecord']
  | Schema['NativeRunRecord']
  | Schema['HybridFlowRunRecord']
  | Schema['ExperimentRunRecord']
  | Schema['BatchRunRecord'];
export const terminal = (s: string) => ['succeeded', 'failed', 'cancelled'].includes(s);
export class ApiError extends Error {
  constructor(
    public status: number,
    message: string,
  ) {
    super(message);
  }
}
export function detailMessage(value: unknown): string {
  if (typeof value === 'string') return value;
  if (Array.isArray(value))
    return value
      .map((v) => `${v.loc?.slice(1).join('.') || 'Request'}: ${v.msg || 'Invalid value'}`)
      .join('\n');
  return 'The request could not be completed.';
}
export class Client {
  constructor(
    private token: string,
    private expired: () => void = () => {},
  ) {}
  async request<T>(
    path: string,
    options: {
      body?: unknown;
      key?: string;
      signal?: AbortSignal;
      text?: boolean;
      bytes?: boolean;
    } = {},
  ): Promise<T> {
    if (!path.startsWith('/v1/') || path.includes('://'))
      throw new Error('Only workspace API paths are allowed.');
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 20000);
    const abort = () => controller.abort();
    options.signal?.addEventListener('abort', abort, { once: true });
    if (options.signal?.aborted) controller.abort();
    try {
      const response = await fetch(path, {
        method: options.body === undefined ? 'GET' : 'POST',
        redirect: 'error',
        cache: 'no-store',
        credentials: 'omit',
        headers: {
          Authorization: `Bearer ${this.token}`,
          ...(options.body === undefined ? {} : { 'Content-Type': 'application/json' }),
          ...(options.key ? { 'Idempotency-Key': options.key } : {}),
        },
        body: options.body === undefined ? undefined : JSON.stringify(options.body),
        signal: controller.signal,
      });
      if (!response.ok) {
        if (response.status === 401) this.expired();
        const problem = await response.json().catch(() => ({}));
        throw new ApiError(response.status, detailMessage(problem.detail));
      }
      if (options.bytes) return (await response.arrayBuffer()) as T;
      return options.text ? ((await response.text()) as T) : ((await response.json()) as T);
    } catch (e) {
      if (e instanceof ApiError) throw e;
      if (options.signal?.aborted) throw e;
      throw new ApiError(
        0,
        controller.signal.aborted
          ? 'Request timed out. Check the service and try again.'
          : 'Cannot reach the API. Check the connection and try again.',
      );
    } finally {
      clearTimeout(timeout);
      options.signal?.removeEventListener('abort', abort);
    }
  }
}
export function download(name: string, value: unknown, type = 'application/json') {
  const blob = new Blob(
    [
      value instanceof ArrayBuffer || typeof value === 'string'
        ? value
        : JSON.stringify(value, null, 2),
    ],
    {
      type,
    },
  );
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = name;
  a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}
export function safeLink(value: string): string | undefined {
  try {
    const url = new URL(value);
    return ['http:', 'https:'].includes(url.protocol) ? url.href : undefined;
  } catch {
    return undefined;
  }
}
export function defaultPlan(model: Model): Plan {
  return {
    schema_version: '1.0.0',
    model: { ...ref(model) },
    context: model.context,
    duration: { value: 10, unit: 'second' },
    sample_interval: { value: 1, unit: 'second' },
    parameters: [],
    interventions: [],
    seed: 42,
    budget: { max_samples: 10001, wall_seconds: 60 },
    purpose: 'Exploratory research run',
  };
}
export type PlotData = {
  time: { unit: string; values: number[] };
  outputs: { name: string; unit: string; values: number[] }[];
};
// Strict CSV reader, including quoted OSPSuite quantity paths. Missing numeric data
// never becomes zero. Physical unit metadata is mandatory for every plotted column.
export function nativeSeries(
  text: string,
  metadata: {
    model_units?: { time_unit?: string; time?: string };
    quantities?: Record<string, { unit: string }>;
    species?: { name: string; unit: string }[];
  },
): PlotData {
  const rows: string[][] = [];
  let row: string[] = [],
    field = '',
    quoted = false;
  const source = text.replace(/^\uFEFF/, '');
  for (let i = 0; i < source.length; i++) {
    const c = source[i];
    if (c === '"') {
      if (quoted && source[i + 1] === '"') {
        field += '"';
        i++;
      } else quoted = !quoted;
    } else if (c === ',' && !quoted) {
      row.push(field);
      field = '';
    } else if ((c === '\n' || c === '\r') && !quoted) {
      if (c === '\r' && source[i + 1] === '\n') i++;
      row.push(field);
      if (row.some((s) => s.length)) rows.push(row);
      row = [];
      field = '';
    } else field += c;
  }
  if (quoted) throw new Error('Malformed native CSV.');
  if (field.length || row.length) {
    row.push(field);
    rows.push(row);
  }
  const names = rows.shift();
  if (
    !names ||
    rows.length < 2 ||
    new Set(names).size !== names.length ||
    rows.some((r) => r.length !== names.length)
  )
    throw new Error('Invalid native sample grid.');
  const units: Record<string, string> = Object.fromEntries(
    (metadata.species || []).map((s) => [s.name, s.unit]),
  );
  for (const [name, data] of Object.entries(metadata.quantities || {})) units[name] = data.unit;
  units.Time ||= metadata.model_units?.time_unit || metadata.model_units?.time || '';
  const columns = names.map((name, index) => ({
    name,
    unit: units[name],
    values: rows.map((r) => {
      if (!r[index].trim() || !Number.isFinite(Number(r[index])))
        throw new Error('Nonfinite or missing native output.');
      return Number(r[index]);
    }),
  }));
  const time = columns.find((c) => c.name === 'Time');
  const outputs = columns.filter((c) => c.name !== 'Time' && c.name !== 'IndividualId');
  if (
    !time?.unit ||
    !outputs.length ||
    outputs.some((c) => !c.unit) ||
    time.values.some((v, i, all) => i > 0 && v <= all[i - 1])
  )
    throw new Error(
      'Native units or time order are ambiguous. Download the original artifacts for inspection.',
    );
  return { time, outputs };
}
