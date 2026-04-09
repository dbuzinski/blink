# Request & response

## Request

- **`Parameters`** — Struct of route parameters from `:name` segments; values are character vectors.
- **`Query`** — Struct built from the query string. Keys are mapped to **valid MATLAB field names**; duplicate keys follow a single documented policy (for example last value wins).
- **`Headers`** — Request headers available as a struct with normalized field names, plus **case-insensitive** logical access as documented on `blink.Request` (so authors do not depend on header spelling).
- **`Data`** — Raw body as `string` for handlers that read unparsed content.
- **Parsed bodies** — When content types are handled by built-in parsers, parsed fields appear on `blink.Request` as documented in [Body & uploads](body-uploads.md), in addition to or instead of raw `Data` depending on type.

## Response

- **`StatusCode`** — HTTP status (default 200).
- **`Data`** — Body as character data for text; binary responses use the documented binary or uint8 representation.
- **`ContentType`** — Sets `Content-Type` when provided; otherwise sensible defaults apply from body or route.
- **`Headers`** — Arbitrary response headers, merged with framework-set headers; later rules for duplicates are documented on `blink.Response`.
- **Helpers** — Documented helpers for **redirects** (`Location` and status), **JSON** encoding, and similar common cases.

## Cookies

- **Request:** The `Cookie` header is parsed into a stable MATLAB representation (struct or table) for name/value lookup.
- **Response:** Authors set **`Set-Cookie`** through response header helpers or dedicated cookie APIs.

## Streaming

Request and response bodies are **buffered** through the handler unless the documentation for a specific API advertises streaming. When streaming exists, handler contracts state **buffer lifetime** and whether `req`/`resp` may be used after the handler returns.
