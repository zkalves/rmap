# Changelog

## [0.3.0](https://github.com/zkalves/rmap/compare/v0.2.0...v0.3.0) (2026-10-02)


### Features

* add configurable layout modes and comprehensive memory region editing. DOC-FLAG: ([ac631fc](https://github.com/zkalves/rmap/commit/ac631fc6d4680ea51c8534b5afc7b8f1dbaa0747))
* Add decode-only address gating, move description to rightmost column, and enforce header minimum widths ([76f3b38](https://github.com/zkalves/rmap/commit/76f3b38522c5aec329ffb98b2e791e8a095fe60a))
* Add hierarchical software read/write locks accross block ,register and field levels ([be9d689](https://github.com/zkalves/rmap/commit/be9d689a5b6a69861317ce7bfdc7c867712a3498))
* Added about window ([4c63e8d](https://github.com/zkalves/rmap/commit/4c63e8db41d8beb2a69ed394c21797fde600ad4f))
* Added features to code generator for the documentation templates ([03bdcd5](https://github.com/zkalves/rmap/commit/03bdcd55f8fcc25d759060d3d3bb0fc1d35d4262))
* Added features to code generator for the documentation templates ([03bdcd5](https://github.com/zkalves/rmap/commit/03bdcd55f8fcc25d759060d3d3bb0fc1d35d4262))
* Added python scripting support ([#6](https://github.com/zkalves/rmap/issues/6)) ([0fef89c](https://github.com/zkalves/rmap/commit/0fef89c6b28daecff6fe15008fb597957597e553))
* Added release packages ([#17](https://github.com/zkalves/rmap/issues/17)) ([06273d1](https://github.com/zkalves/rmap/commit/06273d127db2ce7d801b2bbd18f0a446cdf7c9df))
* Added semantic versioning ([78e6f56](https://github.com/zkalves/rmap/commit/78e6f5665b45263d0a5f1724ad205e405e530d1b))
* **config:** add vendor, library, and description metadata ([6c256dd](https://github.com/zkalves/rmap/commit/6c256dd3c3c8e6c9c96bc0a2b53afdb4f9d4150f))
* **format:** add CSV block-level lock roundtrip, lockstep docs, and example suite ([d1555aa](https://github.com/zkalves/rmap/commit/d1555aa51e5737889cd0e8af3cb2273b739f358f))
* **packaging:** Added Docker, Homebrew, and Environment modules ([#21](https://github.com/zkalves/rmap/issues/21)) ([1e52b37](https://github.com/zkalves/rmap/commit/1e52b37e8b18686c033ae2ae02d1ed8314c45da1))
* **rtl:** add error response parameters for RO, WO, and locked accesses ([6f839be](https://github.com/zkalves/rmap/commit/6f839be5a4f4736a5090f9643c8ccca8327d366f))
* Updated languages and themes ([4672384](https://github.com/zkalves/rmap/commit/4672384dcad64d5755dc1e527d445ec02b60128d))
* Updated UVM template ([#11](https://github.com/zkalves/rmap/issues/11)) ([76efcd8](https://github.com/zkalves/rmap/commit/76efcd84a2cee9c1f06880e78e69288a485d549a))


### Bug Fixes

* **ci:** resolve headless coverage discrepancy in desktop integration DOC-FLAG ([3bbafd5](https://github.com/zkalves/rmap/commit/3bbafd5635b8f479644c0757601a404e49116b40))
* Coverage improvements ([#9](https://github.com/zkalves/rmap/issues/9)) ([9d7cf02](https://github.com/zkalves/rmap/commit/9d7cf02d180f53499db1cea51182aef37736a200))
* Implemented missing features ([#13](https://github.com/zkalves/rmap/issues/13)) ([2bd8352](https://github.com/zkalves/rmap/commit/2bd83522d1e24fbbf78a812d745dddeb50f4d3ec))
* integrate application icon tiers and auto-register FreeDesktop entry for Wayland and X11  taskbars ([9098e82](https://github.com/zkalves/rmap/commit/9098e82af21612271548dfac5cd7343e4bbfdddc))
* Removed hw precedence from gui, it needs to be configured through custom parameters ([b2b637c](https://github.com/zkalves/rmap/commit/b2b637ca88ecb47efb88f8754195962b8833ba7a))
* **security:** resolve CodeQL incomplete HTML end tag regex warning ([acfb391](https://github.com/zkalves/rmap/commit/acfb3914c332b24a176c8742db1dd5659b1119d6))


### Code Refactoring

* **examples:** reorganize into self-contained categorized environments and update repo references ([4cb95e3](https://github.com/zkalves/rmap/commit/4cb95e3b92d0aef200b2e5243c25f61fde55ecec))


### Documentation

* Added missing file ([#16](https://github.com/zkalves/rmap/issues/16)) ([8820627](https://github.com/zkalves/rmap/commit/8820627a0950f2b379c5aeb7d1b87ccc8146ab32))
* Improved GUI images ([ae6eacf](https://github.com/zkalves/rmap/commit/ae6eacf8f0be60b2897d86ec07cdde9141be35d1))
* Removed coverage data from README.md ([9b09e91](https://github.com/zkalves/rmap/commit/9b09e91d0ffe400cab302ede5bd0c661c124369e))
* Updated docs formating ([bd622e3](https://github.com/zkalves/rmap/commit/bd622e37f194fc0702a325420939afa4b720b7ab))
* Updated documentation flow ([#12](https://github.com/zkalves/rmap/issues/12)) ([a7a399c](https://github.com/zkalves/rmap/commit/a7a399cbbfcb9c75026b1862527fc5a90f46e5df))
* Updated image generators ([2c68110](https://github.com/zkalves/rmap/commit/2c681104237cb1da156c6434b972947eb7a84508))
* Updated template documentation ([397dc28](https://github.com/zkalves/rmap/commit/397dc2807738f98c42111ab5ae3d3e8bc2dfb991))
* Updated theme ([#15](https://github.com/zkalves/rmap/issues/15)) ([e78a7c0](https://github.com/zkalves/rmap/commit/e78a7c0e71bf4146a8305da324bb4539ef554d87))


### Tests

* Added coverage metrics ([#8](https://github.com/zkalves/rmap/issues/8)) ([7fb0617](https://github.com/zkalves/rmap/commit/7fb0617a25f55ec504affbba33a8931821120be9))
* **coverage:** achieve 100% line coverage in desktop integration error handling DOC-FLAG ([9ff1b5f](https://github.com/zkalves/rmap/commit/9ff1b5fbd1f3f51ec6c663fb1f407c173a2aab3a))
* improve code coverage to 100% ([03bdcd5](https://github.com/zkalves/rmap/commit/03bdcd55f8fcc25d759060d3d3bb0fc1d35d4262))
* restore 100% line and function test coverage across all subsystems DOC-FLAG ([eddf988](https://github.com/zkalves/rmap/commit/eddf9881ed741d68102071a6d1c1495e5bbfec1c))


### Continuous Integration

* enforce GCC 14 in CI and support gcov-14 in coverage engine ([03bdcd5](https://github.com/zkalves/rmap/commit/03bdcd55f8fcc25d759060d3d3bb0fc1d35d4262))
* Fixed coverage reporting ([#14](https://github.com/zkalves/rmap/issues/14)) ([456cf89](https://github.com/zkalves/rmap/commit/456cf89ce845159b72846a8bad3d7abd828ce07b))
* Updated ci workflows ([05fea82](https://github.com/zkalves/rmap/commit/05fea823d4e821a45432f278d8a3a3a824b94b78))
