/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

const libCellMLModule = require('libcellml.js/libcellml.common')
let libcellml = null

describe("Model tests", () => {
  beforeAll(async () => {
        libcellml = await libCellMLModule();
    });
  test('Checking Model name.', () => {
    const m = new libcellml.Model()

    expect(m.name()).toBe('');

    m.setName('model-test')
    expect(m.name()).toBe('model-test');
  });
  test('Checking Model has resolved imports.', () => {
    const m = new libcellml.Model()

    expect(m.hasUnresolvedImports()).toBe(false);
  });
  test('Checking Model is defined.', () => {
    const m = new libcellml.Model()

    expect(m.isDefined()).toBe(true);
  });
})
