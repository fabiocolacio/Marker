---
date: 2025-01-29 00:00:00
description: Complete guide to exporting markdown documents in various formats
tags: export, formats, pdf, html, latex, pandoc, conversion
authors: marker
---

# Export Formats

Marker provides powerful export capabilities to convert your markdown documents into various formats suitable for different purposes. From web publishing to academic papers, Marker supports multiple output formats with professional quality.

## Overview

The export system in Marker leverages two main engines:
- **Built-in scidown processor**: For HTML, PDF, and LaTeX generation with scientific extensions
- **Pandoc integration**: For additional formats like RTF, ODT, and DOCX

## Accessing Export

### Export Dialog
- **Menu**: Gear menu (⚙️) → **Export**
- **Keyboard**: **Ctrl+E**
- **File Dialog**: Choose format by file extension when saving

### Command Line Export
```bash
marker -o output.pdf input.md
marker --output report.html document.md
```

## Supported Formats

### HTML Export
**Extension**: `.html`  
**Engine**: Built-in scidown processor  
**Features**:
- Clean, semantic HTML5 output
- Embedded CSS styling from selected theme
- Mathematical equations with KaTeX/MathJax
- Syntax-highlighted code blocks
- Mermaid diagram support
- Responsive design for web viewing
- Inline CSS for standalone HTML files

**Best for**: Web publishing, documentation websites, online sharing

### PDF Export
**Extension**: `.pdf`  
**Engine**: WebKit print engine  
**Features**:
- High-quality PDF generation
- Preserves formatting and styling
- Mathematical notation rendering
- Vector graphics for diagrams
- Configurable paper sizes (A4, Letter, B4:3, B16:9)
- Automatic page orientation (Portrait/Landscape for Beamer)
- Professional typography

**Best for**: Documents, reports, academic papers, presentations

### LaTeX Export
**Extension**: `.tex`  
**Engine**: Built-in scidown processor  
**Features**:
- Native LaTeX source generation
- Document class support (article, beamer, etc.)
- Mathematical equations in LaTeX syntax
- Bibliography and citation support
- Cross-references and internal linking
- Table formatting with proper alignment
- Figure and listing management
- Academic paper structure (abstract, sections, etc.)

**Best for**: Academic writing, journal submission, complex mathematical documents

### RTF Export
**Extension**: `.rtf`  
**Engine**: Pandoc (requires installation)  
**Features**:
- Rich Text Format compatible with most word processors
- Preserves basic formatting (bold, italic, headings)
- Table support
- Cross-platform compatibility
- Microsoft Word compatibility

**Best for**: Document sharing, word processor import, legacy systems

### ODT Export
**Extension**: `.odt`  
**Engine**: Pandoc (requires installation)  
**Features**:
- OpenDocument Text format
- LibreOffice/OpenOffice compatibility
- Full formatting preservation
- Table and image support
- Collaborative editing ready
- Open standard format

**Best for**: LibreOffice users, open-source workflows, collaborative editing

### DOCX Export
**Extension**: `.docx`  
**Engine**: Pandoc (requires installation)  
**Features**:
- Microsoft Word format
- Full compatibility with Word features
- Preserves formatting and structure
- Table and image support
- Track changes compatibility
- Industry standard format

**Best for**: Microsoft Office environments, professional document sharing

## Dependencies

### Built-in Formats
These formats work out-of-the-box:
- **HTML**: No additional dependencies
- **PDF**: Uses WebKit print engine (built-in)
- **LaTeX**: Built-in scidown processor

### Pandoc-based Formats
These formats require Pandoc installation:
- **RTF**: Requires `pandoc` command
- **ODT**: Requires `pandoc` command  
- **DOCX**: Requires `pandoc` command

#### Installing Pandoc
```bash
# Ubuntu/Debian
sudo apt install pandoc

# Fedora
sudo dnf install pandoc

# Arch Linux
sudo pacman -S pandoc
```

## Advanced Features

### Scientific Documents
- **Mathematical equations**: Full LaTeX math support in all formats
- **Citations and bibliography**: Automatic reference management
- **Cross-references**: Internal document linking
- **Figure captions**: Numbered figures with references
- **Table captions**: Professional table formatting
- **Abstracts**: Academic paper structure

### Presentation Mode
- **Beamer class**: LaTeX presentation format
- **Landscape orientation**: Automatic orientation for slides
- **Slide breaks**: Section-based slide generation
- **Aspect ratios**: Support for 4:3 and 16:9 formats

### Styling and Themes
- **CSS themes**: Custom styling for HTML and PDF export
- **Consistent appearance**: Maintained across formats where possible
- **Syntax highlighting**: Code blocks with proper coloring
- **Mathematical rendering**: KaTeX for fast, high-quality math

## Export Options

### Paper Sizes
- **A4**: Standard international paper size
- **Letter**: US standard paper size
- **B4:3**: 4:3 aspect ratio for presentations
- **B16:9**: 16:9 widescreen format

### Mathematical Rendering
- **KaTeX**: Fast client-side math rendering (default)
- **MathJax**: Alternative math rendering engine
- **LaTeX native**: Direct LaTeX output for .tex files

### Code Highlighting
- **highlight.js**: Syntax highlighting for code blocks
- **Language detection**: Automatic language recognition
- **Theme support**: Consistent with editor themes

## Quality and Performance

### Output Quality
- **Professional typography**: High-quality text rendering
- **Vector graphics**: Scalable diagrams and math
- **Consistent formatting**: Preserved across platforms
- **Print-ready**: Professional document quality

### Performance Optimization
- **Efficient processing**: Fast export for large documents
- **Memory management**: Optimized for large files
- **Background processing**: Non-blocking export operations
- **Error handling**: Graceful handling of conversion issues

## Tips and Best Practices

### Format Selection
- **Choose HTML** for web publishing and online sharing
- **Choose PDF** for final documents and presentations
- **Choose LaTeX** for academic submission or further LaTeX editing
- **Choose DOCX/ODT** for collaborative editing in word processors
- **Choose RTF** for maximum compatibility across platforms

### Document Preparation
- **Test exports early**: Verify formatting during writing
- **Use relative paths**: For images and references
- **Check dependencies**: Ensure Pandoc is installed for office formats
- **Preview before export**: Use Marker's preview to check appearance

### Troubleshooting
- **Pandoc not found**: Install Pandoc for RTF/ODT/DOCX support
- **Mathematical errors**: Check LaTeX syntax in equations
- **Image issues**: Verify image paths and formats
- **Formatting problems**: Review CSS theme compatibility

The export system in Marker provides professional-quality output suitable for everything from casual documentation to academic publishing, making it a complete solution for markdown-based document creation.