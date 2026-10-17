import { afterEach, describe, expect, it, vi } from 'vitest';
import { ApiError, Client, detailMessage, nativeSeries, safeLink } from './api';

afterEach(() => {
  vi.unstubAllGlobals();
});
describe('scientific output display', () => {
  it('retains physical units, quoted paths and identity exclusion', () => {
    const result = nativeSeries('"Time","IndividualId","Liver, drug"\n0,1,2.5\n1,1,1.2\n', {
      quantities: {
        Time: { unit: 'min' },
        'Liver, drug': { unit: 'µmol/L' },
      },
    });
    expect(result.time).toMatchObject({ unit: 'min', values: [0, 1] });
    expect(result.outputs).toEqual([{ name: 'Liver, drug', unit: 'µmol/L', values: [2.5, 1.2] }]);
  });
  it('uses COPASI species and model time units', () => {
    expect(
      nativeSeries('Time,A\n0,1\n1,0.9\n', {
        model_units: { time_unit: 's' },
        species: [{ name: 'A', unit: 'mM' }],
      }).outputs[0].unit,
    ).toBe('mM');
  });
  it.each([
    'Time,A\n0,1\n0,2',
    'Time,A\n0,1\n1,',
    'Time,A\n0,1\n1,NaN',
    'Time,A\n0,1\n1,Infinity',
    'Time,A\n0,1\n1,2,3',
  ])('rejects ambiguous or fabricated numerical data: %s', (csv) => {
    expect(() =>
      nativeSeries(csv, {
        model_units: { time_unit: 's' },
        species: [{ name: 'A', unit: 'mM' }],
      }),
    ).toThrow();
  });
  it('rejects missing physical metadata', () => {
    expect(() => nativeSeries('Time,A\n0,1\n1,2', {})).toThrow();
  });
});
describe('credential boundary', () => {
  it('sends credentials only to same-origin API paths with no redirects or cookies', async () => {
    const fetcher = vi.fn().mockResolvedValue(new Response('{"role":"reader"}'));
    vi.stubGlobal('fetch', fetcher);
    const client = new Client('fixture-only-read-token');
    await client.request('/v1/workspace/session');
    expect(fetcher.mock.calls[0][1]).toMatchObject({
      redirect: 'error',
      credentials: 'omit',
      cache: 'no-store',
      headers: { Authorization: 'Bearer fixture-only-read-token' },
    });
    await expect(client.request('https://unrelated.invalid/v1/data')).rejects.toThrow();
    expect(fetcher).toHaveBeenCalledTimes(1);
  });
  it('clears the active session on revoked credentials', async () => {
    vi.stubGlobal(
      'fetch',
      vi.fn().mockResolvedValue(
        new Response('{"detail":"Valid bearer token required"}', {
          status: 401,
        }),
      ),
    );
    const expired = vi.fn();
    await expect(new Client('fixture', expired).request('/v1/models')).rejects.toBeInstanceOf(
      ApiError,
    );
    expect(expired).toHaveBeenCalledOnce();
  });
  it('retains idempotency keys without silently retrying a mutation', async () => {
    const fetcher = vi.fn().mockRejectedValue(new Error('network'));
    vi.stubGlobal('fetch', fetcher);
    await expect(
      new Client('fixture').request('/v1/simulations', {
        body: { model: 'fixture' },
        key: 'same-key',
      }),
    ).rejects.toThrow('Cannot reach');
    expect(fetcher).toHaveBeenCalledTimes(1);
    expect(fetcher.mock.calls[0][1].headers['Idempotency-Key']).toBe('same-key');
  });
  it('treats source links as untrusted content', () => {
    expect(safeLink('javascript:alert(1)')).toBeUndefined();
    expect(safeLink('data:text/html,bad')).toBeUndefined();
    expect(safeLink('https://sbml.org/')).toBe('https://sbml.org/');
  });
  it('explains field-level server validation errors', () => {
    expect(detailMessage([{ loc: ['body', 'duration', 'unit'], msg: 'Unsupported unit' }])).toBe(
      'duration.unit: Unsupported unit',
    );
  });
});
