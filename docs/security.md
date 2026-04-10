# Security

This section defines Blink’s security model and the security-related defaults we want the framework to ship with.

## Secure defaults (proposed)

Topics to specify:

- Safe header handling (case-insensitive lookups, canonicalization)
- Output encoding guidance (XSS prevention patterns)
- Cookie defaults for sessions (httpOnly, secure, sameSite)
- CSRF strategy for form posts (when sessions are enabled)

## Input handling (proposed)

Topics to specify:

- Body size limits (global + per-route overrides)
- Upload handling limits and temporary storage rules
- Content type parsing rules (strict vs permissive)

## Authentication & authorization (proposed)

Topics to specify:

- Middleware patterns for auth
- How to attach a “current user” to request context
- Recommended approach for API keys / bearer tokens

## Transport security (proposed)

Topics to specify:

- Running behind TLS terminators (reverse proxies)
- HSTS and related headers
- Trust proxy / forwarded headers behavior

