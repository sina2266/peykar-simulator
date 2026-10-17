import { renderToStaticMarkup } from 'react-dom/server';
import { expect, it } from 'vitest';
import { ErrorBox, JsonDetails } from './ui';
import { ApiError } from './api';

it('downloads inside editors cannot implicitly submit an enclosing form', () => {
  const html = renderToStaticMarkup(
    <form>
      <JsonDetails value={{ model: 'fixture' }} />
    </form>,
  );
  expect(html).toContain('<button type="button"');
});
it('distinguishes an immutable admission conflict from a lifecycle race', () => {
  expect(
    renderToStaticMarkup(
      <ErrorBox
        error={new ApiError(409, 'This immutable version already exists with different content')}
      />,
    ),
  ).toContain('Immutable version conflict');
  const html = renderToStaticMarkup(
    <ErrorBox error={new ApiError(409, 'Run is already terminal')} />,
  );
  expect(html).toContain('State conflict');
  expect(html).not.toContain('Use a new version');
});
