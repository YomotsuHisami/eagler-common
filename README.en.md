# eagler-common

[中文](README.md)

Shared C++ Runtime infrastructure, browser bridges, and source-level testkits for eagler-touhou games. See [LICENSE](LICENSE).

The library owns title-independent networking, input scheduling and repair, rollback storage primitives, and diagnostic tools. Games own object layouts, snapshot inventories, simulation rules, native Replay semantics, and lifecycle adapters.

```text
cmake -S . -B build/tests -DEAGLER_COMMON_BUILD_TESTS=ON
cmake --build build/tests --config Release
ctest --test-dir build/tests -C Release --output-on-failure
```

The Windows native build passed 34 CTest cases, including graphics state and motion-record formats. Consumer browser and gameplay acceptance is separate. Games pin their own common revision; upgrading this checkout does not upgrade those consumers. The maintenance guide records the extracted portable code's sources and compatibility limits.

Maintenance documentation is in Chinese: [maintenance](docs/MAINTENANCE.md), [browser bridges](browser/README.md), [input Replay format](docs/input-replay.md), [Presentation Lab](testkit/presentation-lab/README.md), and [Replay Verifier](testkit/replay-verifier/README.md).
