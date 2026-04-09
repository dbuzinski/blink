# Request (`blink.Request`)

Handlers receive a `blink.Request` with the following properties.

## `Parameters`

`struct` — Route parameters from path segments (see [Routing](routing.md)). Field names match `:name` segments in the route pattern; values are character vectors.

## `Query`

`struct` — Query-string parameters from the URL (the part after `?`).

Keys are mapped to **valid MATLAB struct field names** (for example characters that cannot appear in identifiers are normalized). The exact mapping and duplicate-key policy are defined in [Specification: Request & response](specification/request-response.md).

If the query string is empty, you still get a struct (possibly with no fields).

## `Headers`

`struct` (and documented accessors) — HTTP request headers with **case-insensitive** logical access so authors do not depend on how the client spelled header names.

## `Data`

`string` — Raw request body.

- For `GET` or `HEAD` requests this is typically empty.
- For JSON APIs, decode with `jsondecode(req.Data)` when the content type is JSON.

For parsed form or multipart bodies, see [Body & uploads](specification/body-uploads.md) and the reference for additional fields on `blink.Request`.

!!! note "Scalar empty"

    `Data` is a scalar `string` that may be empty when there is no body.
