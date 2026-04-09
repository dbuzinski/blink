# Execution model

## Handler context

Documentation states **which execution context** invokes route handlers (for example the thread or worker pool tied to the embedded runtime) so authors know what may run concurrently.

## Blocking work

**Blocking** MATLAB code in a handler delays work tied to that request and may affect throughput; the docs describe recommended patterns for **long CPU or I/O** (background jobs, queues, or documented async APIs).

## Request and response lifetime

Unless a specific API says otherwise, **`req` and `resp` are only valid for the duration of the handler**. Do not retain handles to request or response objects for deferred callbacks without documented support.

## MATLAB runtime

Handlers run as ordinary MATLAB calls in the documented runtime; authors do not manage engine or native threads directly.
