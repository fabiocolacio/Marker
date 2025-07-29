---
date: 2025-01-29 00:00:00
description: Navigate your document structure with the interactive markdown outline
tags: outline, navigation, sidebar, headers, structure
authors: marker
---

# Markdown Outline

The markdown outline is an interactive navigation feature in Marker's sidebar that displays your document's structure based on headings, allowing you to quickly jump to any section with a single click.

![Main window showing outline in sidebar](./media/00_main_window_showing_dual_pane_sidebar_outline_scratchpad.png)

## Overview

The outline automatically generates a hierarchical tree view of your document based on markdown headings (`#`, `##`, `###`, etc.), providing a bird's-eye view of your content structure and enabling quick navigation through long documents.

## Features

- **Automatic generation**: Updates in real-time as you type
- **Hierarchical display**: Shows heading levels with proper indentation
- **Click navigation**: Jump to any section instantly
- **Scroll synchronization**: Outline follows your current position
- **Collapsible sections**: Manage visibility of nested headings
- **Multi-document support**: Each document has its own outline

## Accessing the Outline

### Visibility Requirements
The outline is displayed in the **sidebar**. To view it:
1. Press **F9** to toggle the sidebar, or
2. Press **Ctrl+Shift+B**, or
3. Use the **gear menu** → **Sidebar**

### Location
The outline appears in the sidebar between the document tabs and the scratchpad, providing easy access while editing.

## How It Works

### Heading Detection
The outline automatically detects all markdown headings in your document:

```markdown
# Main Title (H1)
## Section 1 (H2)
### Subsection 1.1 (H3)
### Subsection 1.2 (H3)
## Section 2 (H2)
### Subsection 2.1 (H3)
#### Detail 2.1.1 (H4)
```

### Hierarchical Structure
Headings are displayed with appropriate indentation:
```
▼ Main Title
  ▼ Section 1
    • Subsection 1.1
    • Subsection 1.2
  ▼ Section 2
    ▼ Subsection 2.1
      • Detail 2.1.1
```

### Real-Time Updates
- **Dynamic refresh**: Outline updates as you add, remove, or modify headings
- **No manual refresh needed**: Changes appear automatically
- **Performance optimized**: Updates don't interrupt your writing flow

## Navigation Features

### Click Navigation
- **Single click**: Jump directly to any heading in your document
- **Smooth scrolling**: Editor scrolls to show the selected section
- **Focus retention**: Cursor moves to the beginning of the selected section

### Current Position Tracking
- **Visual indicators**: Current section may be highlighted
- **Auto-expand**: Parent sections expand to show your location
- **Breadcrumb awareness**: Always know where you are in the document

## Usage Tips

### Document Organization
- **Use consistent heading levels**: Create a logical hierarchy
- **Descriptive headings**: Make navigation intuitive
- **Avoid skipping levels**: Use H2 after H1, not H3
- **Balance depth**: Too many levels can clutter the outline

### Effective Navigation
- **Large documents**: Essential for navigating long texts
- **Quick reference**: Jump between sections while writing
- **Structure review**: See document organization at a glance
- **Reorganization aid**: Identify structure issues quickly

### Writing Workflow
1. **Plan with headings**: Create outline structure first
2. **Fill in content**: Write under each heading
3. **Navigate while writing**: Use outline to move between sections
4. **Review structure**: Check logical flow via outline

## Best Practices

### Heading Guidelines
```markdown
# Document Title (only one H1)

## Major Section
Introduction paragraph...

### Subsection
Content for subsection...

#### Detailed Point
Specific details...

## Another Major Section
More content...
```

### Navigation Strategies
- **Keyboard + outline**: Combine shortcuts with outline clicks
- **Split view advantage**: Keep outline visible while writing
- **Reference jumping**: Quickly check other sections
- **Progress tracking**: See completed vs remaining sections

## Advanced Features

### Multi-Document Workflow
- Each open document maintains its own outline
- Switch documents via tabs, outline updates automatically
- Outline state preserved when switching between files

### Integration with Other Features
- **Scroll sync**: When enabled, outline can follow preview scroll
- **Search coordination**: Find results can be navigated via outline
- **Export awareness**: Outline structure reflects final document

## Common Use Cases

### Academic Writing
- Navigate chapters and sections
- Jump between introduction, methodology, results
- Manage bibliography and appendices
- Track theorem and proof structures

### Technical Documentation
- API endpoint organization
- Tutorial step navigation
- Reference section access
- FAQ management

### Creative Writing
- Chapter navigation
- Scene jumping
- Character section tracking
- Plot point organization

### Note Taking
- Topic organization
- Quick idea access
- Meeting minute structure
- Research note navigation

## Troubleshooting

### Outline Not Showing
- Ensure sidebar is visible (F9)
- Check that document has headings
- Verify heading syntax is correct (`#` with space)

### Navigation Not Working
- Click directly on heading text
- Ensure document is saved if issues persist
- Check for syntax errors in markdown

### Performance Issues
- Very large documents may update slowly
- Consider breaking into smaller files
- Reduce heading depth if too complex

The markdown outline transforms Marker into a powerful document navigation tool, making it easy to work with large, complex documents while maintaining a clear view of your content structure.