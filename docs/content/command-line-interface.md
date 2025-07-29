---
date: 2025-01-29 00:00:00
description: Complete guide to using Marker from the command line
tags: cli, command-line, options, export, batch-processing
authors: marker
---

# Command Line Interface

Marker provides a comprehensive command-line interface for both interactive editing and batch processing tasks. You can launch Marker with specific view modes, process multiple files, and perform automated exports.

## Basic Usage

```bash
marker [OPTIONS] [FILES...]
```

## Command Overview

```bash
$ marker --help
Usage:
  marker [OPTION…]

Help Options:
  -h, --help                 Show help options
  --help-all                 Show all help options
  --help-gapplication        Show GApplication options
  --help-gtk                 Show GTK+ Options

Application Options:
  -e, --editor               Open in editor-only mode
  -p, --preview              Open in preview-only mode
  -d, --dual-pane            Open in dual-pane mode
  -w, --dual-window          Open in dual-window mode
  -n, --new-window           Open files in a new window
  -o, --output               Export the given markdown document as the given output file
  --display=DISPLAY          X display to use
```

## Command Options

### View Mode Options

#### `-e, --editor`
**Open in editor-only mode**

Launch Marker showing only the markdown editor, hiding the preview pane.

```bash
# Open file in editor-only mode
marker -e document.md

# Open multiple files in editor-only mode
marker --editor file1.md file2.md file3.md
```

**Use cases**:
- Distraction-free writing
- Working on content without preview
- Low-resource environments
- Large document editing

#### `-p, --preview`
**Open in preview-only mode**

Launch Marker showing only the rendered preview, hiding the editor.

```bash
# Open file in preview-only mode
marker -p presentation.md

# View multiple files as preview-only
marker --preview *.md
```

**Use cases**:
- Document review and presentation
- Reading finished documents
- Checking formatting and layout
- Presentation mode for meetings

#### `-d, --dual-pane`
**Open in dual-pane mode**

Launch Marker with editor and preview side-by-side (horizontal layout).

```bash
# Open in dual-pane mode
marker -d document.md

# Multiple files in dual-pane
marker --dual-pane chapter*.md
```

**Use cases**:
- Simultaneous editing and preview
- Real-time formatting feedback
- Standard editing workflow
- Wide screen optimization

#### `-w, --dual-window`
**Open in dual-window mode**

Launch editor and preview in separate windows for multi-monitor setups.

```bash
# Open in dual-window mode
marker -w document.md

# Dual-window for multiple files
marker --dual-window project-files/*.md
```

**Use cases**:
- Multi-monitor setups
- Maximum screen real estate
- Independent window positioning
- Professional presentations

### File Handling Options

#### `-n, --new-window`
**Open files in a new window**

Forces Marker to create a new application instance instead of using existing windows.

```bash
# Always create new window
marker -n document.md

# New window with specific view mode
marker -n -e notes.md

# Multiple files in new window
marker --new-window *.md
```

**Use cases**:
- Working on multiple projects simultaneously
- Isolating different document types
- Independent application instances
- Multi-project workflows

### Export Options

#### `-o, --output`
**Export markdown document to specified output file**

Perform batch export without opening the GUI interface.

```bash
# Export to PDF
marker -o document.pdf document.md

# Export to HTML
marker -o report.html report.md

# Export to LaTeX
marker -o paper.tex academic-paper.md

# Export to Word format
marker -o presentation.docx slides.md
```

**Supported output formats**:
- `.pdf` - PDF document
- `.html` - HTML webpage
- `.tex` - LaTeX source
- `.rtf` - Rich Text Format
- `.odt` - OpenDocument Text
- `.docx` - Microsoft Word

### System Options

#### `--display=DISPLAY`
**Specify X display to use**

Set the X11 display for GUI output (Linux/Unix systems).

```bash
# Run on specific display
marker --display=:1.0 document.md

# Remote display via SSH X forwarding
marker --display=localhost:10.0 file.md
```

## File Operations

### Opening Files

#### Single File
```bash
# Open one file
marker document.md
```

#### Multiple Files
```bash
# Open multiple specific files
marker chapter1.md chapter2.md chapter3.md

# Open all markdown files in directory
marker *.md

# Open files with glob patterns
marker docs/**/*.md
```

#### Directory Opening
```bash
# Open a directory (will load all .md files)
marker /path/to/markdown-project/
```

### File Type Support

Marker automatically detects and opens these markdown file extensions:
- `.md` - Standard markdown
- `.markdown` - Full extension
- `.mdown` - Alternative extension
- `.mkd` - Short form
- `.mkdn` - Variant

## Advanced Usage Examples

### Batch Export Workflows

#### Convert Multiple Files
```bash
# Export all markdown files to PDF
for file in *.md; do
    marker -o "${file%.md}.pdf" "$file"
done

# Export with specific naming
marker -o "final-report-$(date +%Y%m%d).pdf" report.md
```

#### Automated Documentation Pipeline
```bash
#!/bin/bash
# Build documentation pipeline

INPUT_DIR="docs/source"
OUTPUT_DIR="docs/build"

# Export to multiple formats
for md_file in "$INPUT_DIR"/*.md; do
    basename=$(basename "$md_file" .md)
    
    # Generate HTML
    marker -o "$OUTPUT_DIR/$basename.html" "$md_file"
    
    # Generate PDF
    marker -o "$OUTPUT_DIR/$basename.pdf" "$md_file"
    
    # Generate LaTeX for further processing
    marker -o "$OUTPUT_DIR/$basename.tex" "$md_file"
done
```

### Multi-Project Workflows

#### Project-Specific Windows
```bash
# Open different projects in separate windows
marker -n project-a/*.md &
marker -n project-b/*.md &
marker -n personal-notes/*.md &
```

#### Development Environment Setup
```bash
# Setup development documentation environment
marker -d README.md CONTRIBUTING.md &
marker -p --new-window docs/api/*.md &
marker -e --new-window drafts/
```

### Integration with Other Tools

#### Git Workflow Integration
```bash
# Edit files changed in last commit
git diff --name-only HEAD~1 | grep '\.md$' | xargs marker -d

# Review markdown files in PR
gh pr diff | grep '\.md$' | xargs marker -p
```

#### Build System Integration
```bash
# Makefile integration
docs: $(MARKDOWN_FILES)
	for file in $(MARKDOWN_FILES); do \
		marker -o "build/$$(basename $$file .md).pdf" "$$file"; \
	done
```

## Environment Variables

### Configuration
- `GSETTINGS_SCHEMA_DIR` - Override GSettings schema directory
- `XDG_CONFIG_HOME` - Configuration file location
- `DISPLAY` - X11 display (alternative to --display)

### Development
```bash
# Run development version
GSETTINGS_SCHEMA_DIR=build ./build/marker
```

## Exit Codes

- `0` - Success
- `1` - General error
- `2` - Invalid command line arguments
- `3` - File not found or permission error
- `4` - Export error

## Performance Considerations

### Large File Handling
```bash
# For very large files, use editor-only mode
marker -e large-document.md

# Batch processing with memory limits
ulimit -m 1048576 marker -o output.pdf input.md
```

### System Resources
- Editor-only mode uses less memory
- Preview-only mode reduces CPU for large documents
- Dual-window mode requires more system resources
- Batch export operations are memory-efficient

## Troubleshooting

### Common Issues

#### Permission Errors
```bash
# Check file permissions
ls -la document.md

# Run with proper permissions
chmod 644 document.md
marker document.md
```

#### Display Issues
```bash
# Check X11 forwarding (SSH)
echo $DISPLAY
xauth list

# Alternative display specification
marker --display=:0 document.md
```

#### Export Failures
```bash
# Check Pandoc installation (for office formats)
which pandoc
pandoc --version

# Verify output directory exists
mkdir -p output/
marker -o output/document.pdf document.md
```

### Debug Information
```bash
# Run with debug output
G_MESSAGES_DEBUG=all marker document.md

# Check system libraries
ldd /usr/bin/marker
```

The command-line interface makes Marker suitable for both interactive editing and automated processing workflows, providing flexibility for individual users and integration into larger documentation and publishing systems.