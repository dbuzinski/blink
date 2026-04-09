# Stability & versioning

## Public API

The supported surface for application authors is:

- Documented types under **`blink.*`** (for example `blink.App`, `blink.Request`, `blink.Response`, `blink.Route`, `blink.Config`).
- **Handler signatures** and documented properties and methods.

**Not public:** `blink.internal.*` and any undocumented packages—those interfaces may change without a migration guide.

## Documentation layout

- The **[User guide](application.md)** explains how to build applications.
- The **[Specification](specification/index.md)** gives precise HTTP and API behavior.
- **[Features](features.md)** summarizes capabilities at a glance.

## Versioning

Blink aligns **toolbox or package versions** with documented changes. Breaking changes to the public API ship with **migration notes** and version bumps according to the project’s semver or release policy.

## Deprecation

When a public API is retired, releases note the **replacement**, **timeline**, and **migration** steps before removal.
