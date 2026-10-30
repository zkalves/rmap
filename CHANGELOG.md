# Changelog

## [0.3.0](https://github.com/zkalves/rmap/compare/v0.2.0...v0.3.0) (2026-10-30)


### Features

* add configurable layout modes and comprehensive memory region editing. DOC-FLAG: ([3c654ec](https://github.com/zkalves/rmap/commit/3c654ecd9c2f29aa53dc9807dfa49b49cf08c062))
* Add decode-only address gating, move description to rightmost column, and enforce header minimum widths ([e6d684d](https://github.com/zkalves/rmap/commit/e6d684df39de11e1a8e9394dc32488652ab471fd))
* Add hierarchical software read/write locks accross block ,register and field levels ([b801ae5](https://github.com/zkalves/rmap/commit/b801ae59134adab8cf5ce31863cd43ab9014fda5))
* Added about window ([7b5f0d1](https://github.com/zkalves/rmap/commit/7b5f0d1ba473722d0c7d9395f4989b4bf3dad57c))
* Added features to code generator for the documentation templates ([82e801d](https://github.com/zkalves/rmap/commit/82e801d427740e9b979a11f671f64f334742f231))
* Added features to code generator for the documentation templates ([82e801d](https://github.com/zkalves/rmap/commit/82e801d427740e9b979a11f671f64f334742f231))
* Added python scripting support ([#6](https://github.com/zkalves/rmap/issues/6)) ([f8fffce](https://github.com/zkalves/rmap/commit/f8fffcead78c185c5f782aaebed76d9ad88a8900))
* Added release packages ([#17](https://github.com/zkalves/rmap/issues/17)) ([2dda775](https://github.com/zkalves/rmap/commit/2dda7758582c5f327cb7095bc89694f41249bdbf))
* Added semantic versioning ([34b27e9](https://github.com/zkalves/rmap/commit/34b27e9f9a735a8700049c7cafb4550358c3fd47))
* **cli:** initialize headless mode via -platform CLI argument instead of mutating QT_QPA_PLATFORM ([df0d573](https://github.com/zkalves/rmap/commit/df0d5739765427cf75f3e29ce5892b2aa9c50407))
* **codegen:** support Jinja expressions in template filenames and output paths ([691f2b8](https://github.com/zkalves/rmap/commit/691f2b8869aa875c2dce19cecb72394603933b8d))
* **config:** add vendor, library, and description metadata ([4cd68bd](https://github.com/zkalves/rmap/commit/4cd68bd1dffaf4bb08a3fd1d82544724fa897574))
* **format:** add CSV block-level lock roundtrip, lockstep docs, and example suite ([5f048d9](https://github.com/zkalves/rmap/commit/5f048d90a84228188887d5b042120ad378004643))
* **gui:** modular QDockWidget layout architecture, presets, and dock locking ([693131d](https://github.com/zkalves/rmap/commit/693131d691e003610abe6cf3874307a0bd68ff1c))
* **license:** migrate license to GNU GPLv3 with Output Exception ([bc0b5f4](https://github.com/zkalves/rmap/commit/bc0b5f4f072e1ce692d6528b3992ee9d9ab2734b))
* **model:** support multiple configuration profiles with reserved registers and fields ([4373e62](https://github.com/zkalves/rmap/commit/4373e6294ff778131db6cf527978d1a71fb6f62b))
* **packaging:** Added Docker, Homebrew, and Environment modules ([#21](https://github.com/zkalves/rmap/issues/21)) ([eecff18](https://github.com/zkalves/rmap/commit/eecff1881bf047ed5d62fe857117a3eeab1f9600))
* **rtl:** add error response parameters for RO, WO, and locked accesses ([a740515](https://github.com/zkalves/rmap/commit/a740515ad13947f71d4e14415f59e26fded7b243))
* **sim:** support Verilator 5.052 UVM simulation, waveform tracing, and 100% test coverage ([84c73a1](https://github.com/zkalves/rmap/commit/84c73a16b9a0a56142f12c05fc636ad8ca1aadff))
* Updated languages and themes ([0a08f00](https://github.com/zkalves/rmap/commit/0a08f007aae017499ffd8adafc1d64e3c577c25f))
* Updated UVM template ([#11](https://github.com/zkalves/rmap/issues/11)) ([bc91b46](https://github.com/zkalves/rmap/commit/bc91b4652495fcbd7c90123108db8d54649f900d))


### Bug Fixes

* **ci:** make UVM library download resilient with artifact caching and retries ([94988d5](https://github.com/zkalves/rmap/commit/94988d5061d2d422b3a60b6f79634cd90f5954ea))
* **ci:** resolve headless coverage discrepancy in desktop integration DOC-FLAG ([815e6c7](https://github.com/zkalves/rmap/commit/815e6c7addc8c6b7ade2eb33bae8272ccbce9062))
* **ci:** resolve Template Test (rtl_tb) and SPI UVM Verilator failures ([9fc9ac6](https://github.com/zkalves/rmap/commit/9fc9ac6902e2c289aa9af0936b7c47d99c47f4b5))
* Coverage improvements ([#9](https://github.com/zkalves/rmap/issues/9)) ([b72ddda](https://github.com/zkalves/rmap/commit/b72ddda0a17f1ece20c9ba7ee559fa9737123269))
* **gui:** wrap PropertiesDock container in QScrollArea for flexible sizing ([0b1197e](https://github.com/zkalves/rmap/commit/0b1197e03da34ba9d38cb4582230cccc627fea10))
* Implemented missing features ([#13](https://github.com/zkalves/rmap/issues/13)) ([56d0580](https://github.com/zkalves/rmap/commit/56d05800e51e0c4b81cd99ba541479fcf5a88752))
* integrate application icon tiers and auto-register FreeDesktop entry for Wayland and X11  taskbars ([e236460](https://github.com/zkalves/rmap/commit/e236460d5a010bedfa4dd9051ee906af0ee6dd43))
* Removed hw precedence from gui, it needs to be configured through custom parameters ([d7bbf44](https://github.com/zkalves/rmap/commit/d7bbf440a53bd6d46571d2765752e67f34520a6b))
* **security:** resolve CodeQL incomplete HTML end tag regex warning ([60770ce](https://github.com/zkalves/rmap/commit/60770cead1ffed5d50f906a122cc3da9d60564f1))


### Code Refactoring

* **examples:** reorganize into self-contained categorized environments and update repo references ([1a8ceef](https://github.com/zkalves/rmap/commit/1a8ceefda2634ae3ca4b1b9b3e5bc1298618d48f))


### Documentation

* Added missing file ([#16](https://github.com/zkalves/rmap/issues/16)) ([993df12](https://github.com/zkalves/rmap/commit/993df12d4116dd67d452a720065cc872ecf99d55))
* Improved GUI images ([170fa56](https://github.com/zkalves/rmap/commit/170fa56b8b79c12132c2f1c171e36e255ce4c81a))
* Removed coverage data from README.md ([fe57335](https://github.com/zkalves/rmap/commit/fe573354e09e3760b8b39e779faee716ec442170))
* Updated docs formating ([f1aa54b](https://github.com/zkalves/rmap/commit/f1aa54b9a031aaf03d8651bfd437d83d8a762083))
* Updated documentation flow ([#12](https://github.com/zkalves/rmap/issues/12)) ([6419ed3](https://github.com/zkalves/rmap/commit/6419ed3f2c038a42030ea2cb4a65ab279c33ad55))
* Updated image generators ([57394d7](https://github.com/zkalves/rmap/commit/57394d7bc66207337c5cc45a08290cfdd3028000))
* Updated template documentation ([e52d867](https://github.com/zkalves/rmap/commit/e52d867056f0f0e4aab551de08564d7420249808))
* Updated theme ([#15](https://github.com/zkalves/rmap/issues/15)) ([5a273fe](https://github.com/zkalves/rmap/commit/5a273fe27d353eadd0114e86e6f291f9c5251dd9))


### Tests

* Added coverage metrics ([#8](https://github.com/zkalves/rmap/issues/8)) ([2f8e79d](https://github.com/zkalves/rmap/commit/2f8e79d98dc413d9adfc43e9eb791e05229aac6e))
* **coverage:** achieve 100% line coverage in desktop integration error handling DOC-FLAG ([2e9f5ec](https://github.com/zkalves/rmap/commit/2e9f5ec2248f6a12033e22ac51216e1f80b43523))
* improve code coverage to 100% ([82e801d](https://github.com/zkalves/rmap/commit/82e801d427740e9b979a11f671f64f334742f231))
* restore 100% line and function test coverage across all subsystems DOC-FLAG ([f9c9982](https://github.com/zkalves/rmap/commit/f9c9982ce22e40b73db55b27398d4f721db86ed7))


### Continuous Integration

* **actions:** upgrade GitHub Actions to latest Node.js 24 runtimes ([e4671ef](https://github.com/zkalves/rmap/commit/e4671efe6719ccf2bc9c820d0ab2094eb281a5fa))
* enforce GCC 14 in CI and support gcov-14 in coverage engine ([82e801d](https://github.com/zkalves/rmap/commit/82e801d427740e9b979a11f671f64f334742f231))
* Fixed coverage reporting ([#14](https://github.com/zkalves/rmap/issues/14)) ([68e84d7](https://github.com/zkalves/rmap/commit/68e84d7c009e462233d2b738e5b826077e154111))
* Updated ci workflows ([45f6a1c](https://github.com/zkalves/rmap/commit/45f6a1c7807dca2c38c9dde164cf6cf51db3facc))
