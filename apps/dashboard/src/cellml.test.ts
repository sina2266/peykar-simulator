import { afterEach, expect, it, vi } from 'vitest';
import { cellmlSeries } from './cellml';
import { Client } from './api';

afterEach(() => vi.unstubAllGlobals());
it('plots original CellML unit identifiers and component-qualified quantities', () => {
  const data = cellmlSeries('time,membrane/V\n0,-65\n0.5,-62\n', {
    voi: { units: 'millisecond' },
    columns: [{ component: 'membrane', name: 'V', units: 'millivolt' }],
  });
  expect(data.time).toEqual({ name: 'Time', unit: 'millisecond', values: [0, 0.5] });
  expect(data.outputs[0]).toEqual({ name: 'membrane/V', unit: 'millivolt', values: [-65, -62] });
});
it('rejects missing metadata or nonfinite CellML samples', () => {
  const metadata = { voi: { units: 'second' }, columns: [] };
  expect(() => cellmlSeries('time,decay/x\n0,1\n1,0.9\n', metadata)).toThrow('units');
  expect(() => cellmlSeries('time,decay/x\n0,1\n1,nan\n', metadata)).toThrow('Nonfinite');
});
it('downloads XML bytes without stripping its BOM or changing line endings', async () => {
  const raw = new Uint8Array([239, 187, 191, 60, 47, 62, 13, 10]);
  vi.stubGlobal('fetch', vi.fn().mockResolvedValue(new Response(raw)));
  const value = await new Client('fixture').request<ArrayBuffer>(
    '/v1/cellml-models/x/versions/1.0.0/content',
    { bytes: true },
  );
  expect(new Uint8Array(value)).toEqual(raw);
});
