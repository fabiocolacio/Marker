---
date: 2025-01-29 00:00:00
description: Use the integrated scratchpad for quick notes and temporary content
tags: sidebar, notes, productivity, text-area
authors: marker
---

# Scratchpad

The scratchpad is a persistent text area located in the sidebar that provides a convenient space for quick notes, temporary content, and ideas while working on your markdown documents.

## Overview

Located at the bottom of the sidebar below the document outline, the scratchpad offers a dedicated space for jotting down thoughts, code snippets, or any temporary content without switching between applications or creating new files.

## Features

- **Persistent storage**: Text automatically saves and persists between application sessions
- **Global access**: Same scratchpad content is available across all open documents
- **Compact design**: Shows 10 visible lines with automatic scrolling for longer content
- **Auto-save**: Changes are automatically saved as you type
- **Always available**: Accessible whenever the sidebar is visible

## Accessing the Scratchpad

### Visibility Requirements
The scratchpad is only visible when the **sidebar** is open. If you don't see it:
1. Press **F9** to toggle the sidebar, or
2. Use the **View** menu → **Sidebar**

### Toggle Scratchpad
- **Menu**: Gear menu (⚙️) → **Scratchpad**
- **Keyboard**: **Ctrl+Shift+S**

## Usage Examples

### Quick Notes
```
TODO: Add math section to Chapter 3
Research: LaTeX table formatting
Meeting notes: Discuss export options
```

### Code Snippets
```python
# Remember this syntax
def process_markdown(text):
    return scidown.parse(text)
```

### Temporary Content
Use the scratchpad to:
- Store text snippets you might want to paste later
- Keep track of references and links
- Draft section outlines before adding to documents
- Store formatting examples or syntax references

## Technical Details

- **Storage**: Content is saved in GSettings and persists between sessions
- **Capacity**: No practical limit on text length (scrolls automatically)
- **Performance**: Minimal impact on application performance
- **Scope**: Scratchpad content is global (not per-document)

## Privacy and Data

- Scratchpad content is stored locally in your system's configuration
- Content persists until manually cleared or application data is reset
- No network synchronization or cloud storage

## Tips

- **Quick access**: Keep commonly used text snippets in the scratchpad
- **Reference storage**: Store links, citations, or research notes
- **Draft space**: Use for outlining before committing to your main document
- **Cleanup**: Regularly clear old content to keep it organized

## Related Features

- **Sidebar**: Container for outline and scratchpad
- **Document Outline**: Navigate document structure above scratchpad
- **Recent Files**: Quick access to previously opened documents