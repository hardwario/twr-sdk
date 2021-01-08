<a href="https://www.hardwario.com/"><img src="https://www.hardwario.com/ci/assets/hw-logo.svg" width="200" alt="HARDWARIO Logo" align="right"></a>

# HARDWARIO Firmware SDK

[![Travis](https://img.shields.io/travis/hardwario/twr-sdk/master.svg)](https://travis-ci.org/hardwario/twr-sdk)
[![Release](https://img.shields.io/github/release/hardwario/twr-sdk.svg)](https://github.com/hardwario/twr-sdk/releases)
[![License](https://img.shields.io/github/license/hardwario/twr-sdk.svg)](https://github.com/hardwario/twr-sdk/blob/master/LICENSE)
[![Twitter](https://img.shields.io/twitter/follow/hardwario_en.svg?style=social&label=Follow)](https://twitter.com/hardwario_en)

This repository contains firmware SDK for:

* HARDWARIO Core Module
* HARDWARIO Radio Dongle
* HARDWARIO Cloony

## Introduction

Firmware SDK for Core Module is implemented in C (ISO C99) language.
It is the core foundation of all the firmware repositories and device projects.

In short it is a set of libraries and APIs which simplify embedded firmware development workflow.

All the drivers provide high-level abstraction of the underlying hardware and follow event-driven approach.

The whole library is documented using Doxygen and automatically generated at [sdk.hardwario.com](http://sdk.hardwario.com) with every commit/merge to master branch using Travis CI.

Firmware SDK also comes with Makefile recipe to build the firmware projects.

This repository is best integrated within each firmware project as a Git submodule so it is easy to update it to the most recent version and at the same time keep the know-to-work version of the firmware locked to the specific commit of the SDK.

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT/) - see the [LICENSE](LICENSE) file for details.

---

Made with &#x2764;&nbsp; by [**HARDWARIO s.r.o.**](https://www.hardwario.com/) in the heart of Europe.
