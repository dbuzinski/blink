# Specification (overview)

These pages define how Blink behaves from the **application author’s** perspective: HTTP semantics, request and response models, middleware, configuration, security, TLS, WebSockets, and runtime rules.

The [User guide](../application.md) is the primary tutorial path; this section is the **normative** reference for behavior and API shape.

| Topic | Document |
|-------|----------|
| Verbs, 404/405, route groups, host routing | [HTTP & routing](http-routing.md) |
| Request/response shape, headers, cookies, redirects | [Request & response](request-response.md) |
| Forms and file uploads | [Body & uploads](body-uploads.md) |
| Middleware, errors, CORS | [Middleware & errors](middleware-errors.md) |
| Config, listen options, shutdown | [Config & lifecycle](config-lifecycle.md) |
| Sessions, CSRF, CORS matrix | [Security](security.md) |
| WebSockets, HTTPS | [Real-time & HTTPS](realtime-https.md) |
| Threading, blocking, handler lifetime | [Execution model](execution-model.md) |

Cross-cutting details—default **error bodies** (HTML vs JSON) for 404/405/500, **static file** MIME and cache behavior, **multipart** temp files and cleanup, **testing** patterns without blocking the main session, and **package boundaries** between `blink` and optional toolboxes—are covered in the relevant pages above.
