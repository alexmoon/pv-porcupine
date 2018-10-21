# pv-porcupine

A [Node.js](http://nodejs.org/) [addon](http://nodejs.org/api/addons.html) that provides a wrapper around the [Picovoice Porcupine](https://picovoice.ai/) wake word detection library.

**Note:** This is a Node.js library. It will not work in a browser.

## Installation

Install [Node.js](http://nodejs.org/) for your platform. This software has been developed against the long term stable (LTS) release. For ease of installation with other node packages, this package includes a copy of the dependent Porcupine library and so has no prerequisites. Build rules are defined for Mac and Linux platforms. Windows is not supported.

**Important:** Before using the library you will need to download or generate a keyword file. A number of pre-generated keyword files are available in the [Porcupine repository](https://github.com/picovoice/Porcupine), along with a tool to generate your own keywords.

Install the package with:

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

const detector = new porcupine.Porcupine('node_modules/pv-porcupine/Porcupine/lib/common/porcupine_params.pv', 'path/to/keyword.ppn');

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
    keywords: 'path/to/keyword.ppn'
});

detector.on('keyword', (chunk, keyword) => {
    console.log('Detected keyword');
});

fs.createReadStream('path/to/raw/audio/file').pipe(detector);
```

## Development

The Picovoice Porcupine libraries are included as a git submodule of this repository. When cloning the repository use `git clone --recurse-submodules` to get the Porcupine libraries in addition to this Node.js wrapper. If you have cloned this repository without the submodule, you can update the libraries by changing to the Porcupine subdirectory and using `git update --init`.

## License

This software uses libraries from the Picovoice Porcupine project. The [license terms for Porcupine](https://github.com/Picovoice/Porcupine/blob/master/LICENSE) are stated to be an [Apache 2.0](https://opensource.org/licenses/Apache-2.0).
