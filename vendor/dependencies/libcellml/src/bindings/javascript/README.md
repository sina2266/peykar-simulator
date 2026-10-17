
# libcellml.js

Run libCellML in the browser.

## Installation

``` bash
npm install libcellml.js
```

## Usage

### In the browser

Create a file **index.js** with content:

``` javascript
const http = require('http');
const libCellMLModule = require('libcellml.js/libcellml.common');
http.createServer(function (req, res) {
  libCellMLModule().then(libcellml => {
    res.writeHead(200, {'Content-Type': 'text/html'});
    res.write('Version of libCellML: ');
    res.write(libcellml.versionString());
    res.end();
  })
}).listen(8080);
```

Serve the file

``` bash
node index.js
```

Open **localhost:8080** in a browser.
You should see:

```
Version of libCellML: 0.2.0
```

Written on the page.

### In Vue application

Start a vue project with:

```bash
vue create vue-libcellml
```

Choose the default option in the setup.
Change into the newly created project:

```bash
cd vue-libcellml
```

Install libcellml.js:

```bash
yarn add libcellml.js
```

Modify **src/main.js** to be:

```javascript
import Vue from 'vue'
import App from './App.vue'

import libCellMLModule from 'libcellml.js'
import libCellMLWasm from 'libcellml.js/libcellml.wasm'

Vue.config.productionTip = false
Vue.prototype.$libcellml = null

const init = async () => {
  Vue.prototype.$libcellml = await new libCellMLModule({locateFile(path, prefix) {
    if(path.endsWith('.wasm')) {
      return libCellMLWasm
    }
    return prefix + path
  }});
  /* eslint-disable no-new */
  new Vue({
    render: h => h(App),
  }).$mount('#app');
};
 
init();
```

Create a **vue.config.js** file and set the contents to:

```javascript
module.exports = {
  chainWebpack: config => {
    config.module
      .rule('wasm')
      .type('javascript/auto')
      .test(/\.wasm$/)
      .use('file-loader')
      .loader('file-loader')
      .end()
  }
}
```

Modify the contents of **src/components/HelloWorld.vue** to be:

```vue
<template>
  <div class="hello">
    <h1>{{ msg }}</h1>
    <p>
      libCellML version: {{ versionMsg }}
    </p>
  </div>
</template>

<script>
export default {
  name: 'HelloWorld',
  props: {
    msg: String
  },
  data: function() {
    return {
      versionMsg: this.$libcellml.versionString()
    }
  }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
h3 {
  margin: 40px 0 0;
}
ul {
  list-style-type: none;
  padding: 0;
}
li {
  display: inline-block;
  margin: 0 10px;
}
a {
  color: #42b983;
}
</style>
```

Serve the application:

```bash
yarn serve
```

Open **localhost:8000** in a browser.
You should see:

![Vue app showing libcellml version](src/bindings/javascript/vue-app-with-libcellml.png)
