# Body & uploads

## URL-encoded forms

For `Content-Type: application/x-www-form-urlencoded`, Blink parses fields into the request object (struct fields or the documented layout on `blink.Request`) in addition to preserving raw `Data` when useful for debugging.

## Multipart

For `multipart/form-data`:

- Text fields appear under documented property names on `blink.Request`.
- **Files** are written under a **configurable directory**, with **sanitized filenames**, **maximum size**, and **cleanup** rules (for example delete temporaries after the handler completes unless the API retains handles).

## Limits

Global and per-route **maximum body size** returns **413 Payload Too Large** when exceeded.

## Raw body

Handlers may still read **`Data`** directly when they bypass built-in parsers or use custom content types.
