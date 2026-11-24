# Contributing

The goal of this project was to get the heat pump's data (consumption, temperatures, states) into Home Assistant with a history, and to understand the BWWP better. That is achieved, so there is no active development roadmap. Contributions, other models and register findings are welcome, but issues and pull requests are handled on a best-effort basis.

Guidelines

* Do not add copyrighted material (TESY manuals, brochure images).
* Keep new entities `disabled_by_default: true` unless they are broadly useful (RAM on the ESP8266).
* Run `tests/run.sh` and `node tests/test_flow.js` before opening a PR. If you add registers, add them to `docs/registers.md`.
* Names that are not verified must contain `(?)` or `(unclear)`.
* Reports that it works (or not) on other TESY BWWP models are useful in an issue: model, controller board label, regs 2012/2014 (DSP/PFC software version).
