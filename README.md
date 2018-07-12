# pv-porcupine

A [Node.js](http://nodejs.org/) [addon](http://nodejs.org/api/addons.html) that provides a wrapper around the [Picovoice Porcupine](https://picovoice.ai/) library, enabling an application to record and play audio with cross platform support. With this library, you can create a writable [node.js stream](https://nodejs.org/dist/latest-v8.x/docs/api/stream.html) that can receive data piped from other streams, such as system audio, files and network connections.

**Note:** This is a server side library. It will not work in a browser.

## Installation

Install [Node.js](http://nodejs.org/) for your platform. This software has been developed against the long term stable (LTS) release. For ease of installation with other node packages, this package includes a copy of the dependent Porcupine library and so has no prerequisites.

`pv-porcupine` is designed to be `require`d or `import`ed to use from your own application to provide async processing. For example:

    npm install --save pv-porcupine

## Using pv-porcupine

If you are using regular Node.js, include the library with:

```javascript
const { default: porcupine, PorcupineStream } = require('pv-portaudio');
```

If you are using TypeScript, definitions have been provided and you can import the library with:

```typescript
import porcupine, { PorcupineStream } from 'pv-portaudio';
```

### Basic parameters

```javascript
const porcupine = require('pv-porcupine').default;

console.log(porcupine.version());
console.log(porcupine.sampleRate());
console.log(porcupine.frameLength());
```

An example of the output is:

```javascript
'1.3.0'
16000
512
```

### Creating a Porcupine object

```javascript
const porcupine = require('pv-porcupine').default;

const detector = new porcupine.Porcupine('node_modules/pv-porcupine/Porcupine/lib/common/porcupine_params.pv', 'Porcupine/resources/keyword_files/alexa_mac.ppn');

const buffer = Buffer.alloc(2 * porcupine.frameLength());
// Fill the buffer with audio data...
console.log(detector.process(buffer));

// When done, call destroy to free resources
detector.destroy();
```

### Using a stream

```javascript
const { PorcupineStream } = require('pv-porcupine');
const fs = require('fs');

const detector = new PorcupineStream({
    modelFilePath: 'node_modules/pv-porcupine/Porcupine/lib/common/porcupine_params.pv',
    keywords: 'Porcupine/resources/keyword_files/alexa_mac.ppn'
});

detector.on('keyword', (chunk, keyword) => {
    console.log('Detected keyword');
});

fs.createReadStream('path/to/raw/audio/file').pipe(detector);
```

## License

This software uses libraries from the Picovoice Porcupine project. The [license terms for Porcupine](https://github.com/Picovoice/Porcupine/blob/master/LICENSE) are stated to be an [Apache 2.0](https://opensource.org/licenses/Apache-2.0).
