# Middleware & errors

## Middleware

Blink composes **middleware** with route handlers: functions that run in order and may end the request early or pass control to the next layer. Registration uses the public API on `blink.App` (for example a chain of registered callables with a defined **next** step). Middleware runs before the matched route handler unless the reference documents a different ordering for a specific hook.

Typical uses: logging, authentication, shared headers, and **CORS**.

## Global error handlers

Applications register **error handlers** that convert uncaught MATLAB errors into HTTP **status**, **body**, and headers. Configuration distinguishes **development** (richer error detail) from **production** (safe client-facing messages).

## CORS

CORS is configured so that:

- **OPTIONS** preflight requests receive the required `Access-Control-*` headers within the middleware or framework layer.
- Ordinary responses include `Access-Control-*` headers when the app enables cross-origin access.

If an application defines an explicit **OPTIONS** route for a path, the interaction between that route and CORS middleware follows the ordering rules documented for `blink.App`.

## Logging

Optional **request logging** (access-style logs) hooks into the same lifecycle as middleware, without requiring authors to use native code.
