# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

# Claude Code Configuration

IMPORTANT: Before performing any task or modifying code, you MUST read the `AGENTS.md` file in the root directory. It contains the build/test commands, build options, CMake macros (`cf_lib`/`cf_app`/`cf_test_old`/`cf_remote`), code-generation tooling, the test framework, and common mistakes.

- Always cross-reference `AGENTS.md` for build commands and architectural decisions.
- Do not duplicate content from `AGENTS.md` here. This file covers only the big-picture architecture that spans multiple files/modules.

# Architecture

cflib is a C++20 library (+ companion JavaScript in `js/`) for building networked Web 2.0 backends and for efficient C++↔C++ communication. The unifying idea: **mark a class or service, and the build generates the serialization and remote-call plumbing for you** — across C++ and JavaScript.

## Module layering

Strictly layered (see `MODULES`; build order in `AGENTS.md`):

```
base → util → {crypt, serialize, db} → {net, dao}
```

- `base` — foundational types (`String`, `ByteArray`, `SharedPtr`, flags, containers). No dependencies. Provides the shared PCH `cflib/base.h`.
- `util` — runtime substrate: event loop, threading, timers, logging, the test framework, and the build-time tools (`ser`, `bin2src`, `jscombiner`).
- `serialize` — BER/ASN.1 (de)serialization and the codegen engine.
- `crypt` — Botan-backed TLS (client/server/sessions/credentials) and password hashing. Requires exceptions.
- `db` — schema + Postgres/SQLite backends (only built with `ENABLE_PSQL`/`ENABLE_SQLITE`); `migrationmoped` migration tool.
- `net` — TCP/TLS transport, HTTP server/client, and the RMI (remote method invocation) + RSig (remote signal) system.
- `dao` — data-access objects bridging `db` + `serialize`.

## Threading & event model (`cflib/util`)

Everything async is built on **libev** wrapped by `LibEVThreadLoop`. The central pattern is `ThreadVerify` (`threadverify.h`): an object is pinned to one event loop (`Net` or `Worker` type). Methods guard themselves with `verifyThreadCall(...)` / `verifySyncedThreadCall(...)` — if called from the wrong thread, the call is marshalled onto the owning loop via `execLater` and a `Functor` instead of running inline. This gives single-threaded reasoning inside each object without explicit locking. `MainLoop` (`mainloop.h`) runs the process's primary loop.

When touching net/RMI code, respect this discipline: do not call an object's methods directly across threads — route through its `ThreadVerify` machinery.

## Serialization (`cflib/serialize`)

- Annotate a class with the `SERIALIZE_CLASS` macro (from `serialize.h`); members are declared `serialized`. The codegen scans `.h` files for these markers and emits `*_ser.cpp` into `target_autogen/`, implementing `serialize`/`deserialize`/`serializeTypeInfo` over a BER (ASN.1) wire format (`serializeber.h`, `impl/ber.h`).
- Polymorphic hierarchies use `SERIALIZE_IS_BASE` / `SERIALIZE_BASE` + `RegisterClass` for class-id based construction (`createByClassId`).
- `SerializeTypeInfo` is reflection metadata that drives the *remote* codegen below.
- Wire format is inspectable with `asn1dump`.

## RMI / RSig — the remote layer (`cflib/net` + codegen)

This is the heart of the framework. A service class derives from `RMIService<C>` / `RMIServiceBase` and uses the `rmi` macro; methods become remotely callable and `RSig` members become signals the server can push to subscribed clients.

The two-step build (documented in `AGENTS.md` under "Remote Interface Generation"):
1. The app is built with `ENABLE_SER` + `DAO`, generating local serialization/service handlers.
2. `cf_remote(app)` runs `app --export remote/`, which uses the registered `SerializeTypeInfo` to emit **client stubs** into `remote/`:
   - C++ remote services — `cflib/serialize/generate/cppremoteservices.cpp`
   - JavaScript ES modules — `cflib/serialize/generate/javascript.cpp` (consumed alongside `js/net/rmi.js`, `rsig.js`, `ber.js`)
   - HTML API docs — `cflib/serialize/generate/apidoc.cpp`

   See `examples/chatserver/` for the full shape: `services/`, `dao/`, and a generated `remote/` tree containing C++ stubs, `*.js`, and `apidoc/`. The same service definition thus drives a C++ server, C++ clients, browser clients, and documentation from one source of truth.

## JavaScript companion (`js/`)

Browser-side runtime mirroring the C++ net layer: `js/net/` implements RMI, RSig, BER, and AJAX so a generated `*.js` service stub can call into the C++ server over the same protocol.

# Conventions

- Module sources live in `cflib/<module>/`; private implementation in a nested `impl/`; tests in `cflib/<module>/<module>_test/`.
- English only in code and comments (no non-ASCII identifiers/text).
- After adding new source files, **re-run cmake** — file globs are resolved at configure time.
