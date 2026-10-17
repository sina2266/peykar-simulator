import { copyFile, mkdir, readFile, writeFile } from 'node:fs/promises';
const target = new URL('../../packages/platform/src/peykar/web_assets/licenses/', import.meta.url);
await mkdir(target, { recursive: true });
const notices = [];
for (const name of ['react', 'react-dom', 'scheduler']) {
  const spec = JSON.parse(
    await readFile(new URL(`node_modules/${name}/package.json`, import.meta.url), 'utf8'),
  );
  const text = await readFile(new URL(`node_modules/${name}/LICENSE`, import.meta.url), 'utf8');
  const retained = await readFile(new URL(`third-party/${name}.LICENSE`, import.meta.url), 'utf8');
  if (text !== retained) throw new Error(`Retained ${name} notice differs`);
  await copyFile(
    new URL(`third-party/${name}.LICENSE`, import.meta.url),
    new URL(`${name}.LICENSE.txt`, target),
  );
  notices.push({
    name,
    version: spec.version,
    license: spec.license,
    notice: `${name}.LICENSE.txt`,
  });
}
await writeFile(new URL('index.json', target), JSON.stringify(notices, null, 2) + '\n');
