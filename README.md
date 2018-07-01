<a href="https://www.bigclown.com/"><img src="https://bigclown.sirv.com/logo.png" width="200" alt="BigClown Logo" align="right"></a>

# BigClown Firmware SDK

[![Travis](https://img.shields.io/travis/bigclownlabs/bcf-sdk/master.svg)](https://travis-ci.org/bigclownlabs/bcf-sdk)
[![Release](https://img.shields.io/github/release/bigclownlabs/bcf-sdk.svg)](https://github.com/bigclownlabs/bcf-sdk/releases)
[![License](https://img.shields.io/github/license/bigclownlabs/bcf-sdk.svg)](https://github.com/bigclownlabs/bcf-sdk/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/BigClownLabs.svg?style=social&label=Follow)](https://twitter.com/BigClownLabs)

This repository contains firmware SDK for:

* BigClown Core Module
* BigClown USB Dongle
* BigClown Cloony

## Introduction

Firmware SDK for Core Module is implemented in C (ISO C99) language.
It is the core foundation of all the firmware repositories and device projects.

In short it is a set of libraries and APIs which simplify embedded firmware development workflow.

All the drivers provide high-level abstraction of the underlying hardware and follow event-driven approach.

The whole library is documented using Doxygen and automatically generated at [sdk.bigclown.com](http://sdk.bigclown.com) with every commit/merge to master branch using Travis CI.

Firmware SDK also comes with Makefile recipe to build the firmware projects.

This repository is best integrated within each firmware project as a Git submodule so it is easy to update it to the most recent version and at the same time keep the know-to-work version of the firmware locked to the specific commit of the SDK.

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT/) - see the [LICENSE](LICENSE) file for details.

---

Made with &#x2764;&nbsp; by [**HARDWARIO s.r.o.**](https://www.hardwario.com/) in the heart of Europe.
