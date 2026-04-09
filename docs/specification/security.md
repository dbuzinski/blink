# Security

## Sessions

Blink documents supported patterns for **signed cookies**, **server-side sessions**, or both, so authors can choose an approach that fits deployment and threat model.

## CSRF

When applications use **cookie-based** authentication or sessions, Blink documents **CSRF** mitigations (for example synchronizer tokens, double-submit cookies, or SameSite policies) as part of the recommended request flow.

## CORS

The **CORS** behavior for browser `fetch` and `XMLHttpRequest`—allowed origins, methods, headers, and **credentials**—is configured in one place and described in [Middleware & errors](middleware-errors.md).

Server-to-server HTTP (no browser) is unaffected by CORS; documentation clarifies this distinction.

## Secrets

User-facing documentation warns against placing **secrets**, **keys**, or **private MATLAB source** inside **static** directories or templates meant for the public web.
