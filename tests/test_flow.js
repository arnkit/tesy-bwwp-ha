// Runs the "Decode + HA Discovery" function of archive/node-red-waveshare/flow.json against the captured frames.
// Usage: node tests/test_flow.js   (run from the repository root)
const fs = require('fs');
const assert = require('assert');
const flow = JSON.parse(fs.readFileSync('archive/node-red-waveshare/flow.json', 'utf8'));
const fn = flow.find(n => n.id === 'tesy_fn').func;
const F = new Function('msg', 'node', 'context', 'Buffer', fn);
const ctx = {}; const context = { get: k => ctx[k], set: (k, v) => { ctx[k] = v; } };
const node = { status() {} };

let out;
for (const addr of [1001, 1091, 2001]) {
  const rtu = Buffer.from(fs.readFileSync(`tests/frames/frame_${addr}.hex`, 'utf8').trim().split(/\s+/).map(h => parseInt(h, 16)));
  const body = rtu.subarray(0, rtu.length - 2);                       // the Waveshare strips the CRC ...
  const mbap = Buffer.from([0, 1, 0, 0, body.length >> 8, body.length & 255]);   // ... and prepends an MBAP header
  out = F({ payload: Buffer.concat([mbap, body]) }, node, context, Buffer);
}
assert(out && out[0], 'no state published after three blocks');
const st = JSON.parse(out[0].payload);
assert.strictEqual(st.soll, 58);
assert.strictEqual(st.modus, 'ECO');
assert.strictEqual(st.t2019, 22);
assert.strictEqual(st.o08, 'OFF');
assert.strictEqual(st.clock, '24.11.2026 09:45');

const disc = F({ payload: 'go', topic: 'discover' }, node, context, Buffer);
assert(disc && disc[1].length > 100, 'discovery configs missing');
console.log('flow test passed:', disc[1].length, 'discovery configs');
