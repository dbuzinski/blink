# Real-time & HTTPS

## WebSockets

Blink registers **WebSocket** endpoints alongside HTTP routes. Handlers receive connection lifecycle events (open, message, close, error) through documented MATLAB callbacks. **HTTP** and **WebSocket** routes coexist on one app; upgrade requests use the path and rules given in the reference.

If **backpressure** or slow consumers matter for an application, the reference describes what authors should assume when sending messages from MATLAB handlers.

## HTTPS

TLS is configured through **MATLAB-facing options**: paths to certificate and key material, or other supported stores, mapped to the server implementation. Exact name-value arguments appear on `serve` or `blink.Config`.

## HTTP/2

Support tracks the underlying native stack; when enabled, behavior is documented in release notes and the reference.
