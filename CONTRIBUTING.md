# Contributing

Issues and Pull Requests are welcome!

There are only a few rules:

1. Be respectful in comments/discussions
2. When submitting a patch for an issue, reference the issue in your pull request
   by typing a ``#`` followed by the issue number, and describe what you did to
   resolve the issue.
3. If you would like to solve several issues at once, they should all have
   separate pull requests.
4. Follow the coding style below when submitting code

# Coding Style

The following guidelines only apply for the core parts of the app. Changes
to upstream libraries such as *hoedown* and *charter* should try to match the styles of those projects.

## General

* Indentations are done with 2 spaces
* There is always a space before opening parenthases
* Curly braces are on their own lines
* When declaring pointer-type variables, the ``*`` is paired with the variable name
* Since we are working heavily with gnome libraries, use glib types and functions over C standards when possible
  * Use ``gchar *`` instead of ``char *``
  * Use ``g_malloc ()`` and ``g_free ()`` over ``malloc ()`` and ``free ()``
  * Use ``g_print ()`` and ``g_err ()``
  * etc...
* Namespacing
  * All filenames are prefaced with ``marker-``
  * All public types are prefaced with ``Marker``
  * All other public symbols are prefaced with ``marker_``

## Header (.h) files

Function prototypes will look the same as their paired functions in the
source files, just without a body:

```C
// Prototype in .h file:
Foo *
marker_foo_new (Bar     *bar,
                GError **err);

// Implementation in .c file:
Foo *
marker_foo_new (Bar     *bar,
                GError **err)
{
  /* ,,, */
}
```

## Source (.c) files

In order to keep the code predictable and easy-to-navigate, items should
appear in the following order:

1. GPL header
2. Structures
3. Function prototypes
4. ``G_DEFINE_TYPE ()``
5. Enums
6. Static variables
7. Auxillary methods
8. Signal callbacks
9. Interface implementations
10. Parent class overrides
11. ``class_init ()`` and ``init ()``
12. Public API

### Auxillary Methods

Names for auxillary methods do not have the ``marker_`` prefix because they
are not part of the public API:

```C
static void
print_hello_world ()
{
  g_print ("Hello World!\n");
}
```

### Signal Callbacks

Names for signal callbacks should be the past-tense of the signal name
without the ``marker_`` prefix, but followed by ``_cb``:

```C
static void
size_allocated_cb (GtkWidget     *widget,
                   GtkAllocation *allocation,
                   gpointer       user_data)
{
  /* ... */
}

```