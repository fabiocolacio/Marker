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

## Coding Style

The following guidelines only apply for the core parts of the app. Changes
to upstream libraries such as *hoedown* and *charter* should try to match the styles of those projects.

### General

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

### Header (.h) files

Function prototypes have the function name start at column 22, the the opening
parenthases at column 66, and the last asterisk for pointer types, at column 86:

```C
void                 marker_editor_window_set_tab_width          (MarkerEditorWindow *window,
                                                                  guint               value);
```

### Source (.c) files

In order to keep the code predictable and easy-to-navigate, items should
appear in the following order:

1. GPL header
2. Structures
3. Function prototypes
4. ``G_DEFINE_TYPE ()``
5. Enums
6. Static variables
7. Auxillary methods
8. Action callbacks
9. Action array
10. Signal callbacks
11. Interface implementations
12. Parent class overrides
13. ``class_init ()`` and ``init ()``
14. Public API

### Action Callbacks

Names for action callbacks should start with ``action_``:

```C
static void
action_zoom_in (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  /* ... */
}
```

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
## Releases

When making a release, do all of the following:

1. Update the version number in the ``meson.build``
2. Create a release on GitHub (see guidelines for release notes below).
3. Update the tag name in the flatpak manifest of the [flathub repo](https://github.com/flathub/com.github.fabiocolacio.marker)
    * If you do not have push access, create a pull request.
      Someone who has push access will merge it for you.

### Version Numbering

We use dates for version numbers in the format ``YYYY.MM.DD``

e.g. ``2018.01.28``.

### Release Notes

When making a release on GitHub, you will be prompted to write
release notes. The notes should list every change between the
previous release and the new one. The first section should list new features, and the second section should list bug fixes.

An easy way to check what has been changed since the previous release is by finding the previous release in GitHub, and clicking the link that says
"X commits to master since this release".