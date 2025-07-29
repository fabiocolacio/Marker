---
date: 2025-01-29 00:00:00
description: Quick access to your 10 most recently opened markdown files
tags: files, workflow, productivity, menu
authors: marker
---

# Recent Files

The recent files feature provides quick access to your most recently opened markdown documents, helping you maintain an efficient workflow by reducing the time spent navigating to frequently used files.

## Overview

Marker tracks up to **10 recently opened files** and displays them in an easily accessible menu. This feature is perfect for users who work with multiple documents or frequently switch between projects.

## Features

- **10 file history**: Tracks your 10 most recently opened markdown files
- **Automatic tracking**: Files are added automatically when opened
- **Persistent storage**: Recent files list persists between application sessions
- **Smart ordering**: Most recently opened files appear at the top
- **Duplicate handling**: Opening the same file moves it to the top of the list
- **File validation**: Non-existent files are filtered out automatically

## Accessing Recent Files

### Menu Access
1. Click the **gear menu** (⚙️) in the top-right corner
2. Navigate to **Recent Files**
3. Select any file from the submenu to open it

### File Display Format
Recent files are displayed showing:
- **Filename**: The base name of the file
- **Recent order**: Most recent files appear first

## Usage Examples

### Daily Workflow
```
Recent Files:
├── project-notes.md        ← Today's work
├── meeting-minutes.md      ← Yesterday's meeting
├── README.md              ← Project documentation
├── changelog.md           ← Release notes
└── draft-article.md       ← Work in progress
```

### Project Switching
Quickly switch between different projects by accessing their main files from the recent files menu, eliminating the need to navigate through the file system.

## Technical Details

- **Storage limit**: Maximum of 10 files tracked
- **File types**: Only markdown files are tracked (.md, .markdown, etc.)
- **Storage method**: File paths stored in GSettings configuration
- **Update behavior**: List updates automatically when files are opened
- **Cleanup**: Invalid file paths are removed automatically

## Automatic Management

The recent files list manages itself by:
- **Adding new files**: When you open a file, it's added to the top
- **Moving existing files**: Opening a previously opened file moves it to the top
- **Removing old files**: When the list exceeds 10 files, the oldest is removed
- **Validating paths**: Non-existent files are filtered out when displaying the menu

## Privacy and Storage

- Recent files paths are stored locally in your system configuration
- No file content is stored, only file paths
- List persists until manually cleared or application data is reset
- No network or cloud synchronization

## Tips

- **Quick project access**: Pin important project files by opening them regularly
- **File organization**: Use descriptive filenames since only the basename is shown
- **Cleanup**: Recent files automatically clean up invalid paths
- **Workflow optimization**: Organize your most important files to stay in the recent list

## Related Features

- **File menu**: Standard file operations (Open, Save, etc.)
- **Sidebar**: File outline and navigation within documents
- **Export options**: Convert recent files to various formats