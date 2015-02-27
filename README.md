# event-msgpack
An event-driven implementation of a MessagePack subset in C

## Use
Just drop event-msgpack.{c,h} into your source directory.
Some conditional compilation directives are available. These directives should only be to disable features.

## Conditional compilation directives

| Directive |                          |
|-----------|--------------------------|
| NO_READER | Disable reading features |
| NO_WRITER | Disable writing features |
