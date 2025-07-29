---
date: 2025-01-29 00:00:00
description: Monitor document statistics and editor state at a glance
tags: status-bar, statistics, interface, information
authors: marker
---

# Status Bar

The status bar at the bottom of the Marker window provides real-time information about your document and editor state. It displays essential statistics and indicators to help you track your writing progress and document properties.

## Overview

The status bar is always visible at the bottom of the window and shows:
- Document statistics (lines, words, characters)
- Cursor position information
- Document encoding
- File save status
- Editor mode indicators

## Status Bar Elements

### Document Statistics

#### Line Count
- **Display**: "Lines: X"
- **Updates**: Real-time as you type
- **Includes**: All lines including empty ones
- **Use case**: Track document length

#### Word Count
- **Display**: "Words: X"
- **Updates**: Live counting as you write
- **Definition**: Space-separated text units
- **Use case**: Meeting word count requirements

#### Character Count
- **Display**: "Characters: X"
- **Options**: With or without spaces
- **Updates**: Instant feedback
- **Use case**: Character limit compliance

### Cursor Information

#### Line and Column
- **Display**: "Ln X, Col Y"
- **Line**: Current line number (1-based)
- **Column**: Current character position in line
- **Updates**: As cursor moves

#### Selection Info
When text is selected:
- **Selected lines**: Number of lines in selection
- **Selected words**: Word count in selection
- **Selected characters**: Character count in selection

### File Information

#### Encoding Indicator
- **Default**: UTF-8
- **Display**: Shows current file encoding
- **Purpose**: Ensure proper character handling
- **Changes**: When opening files with different encodings

#### Save Status
- **Saved**: No indicator (clean state)
- **Modified**: Asterisk (*) or "Modified" label
- **Auto-saved**: Brief notification when auto-save triggers
- **Read-only**: Lock icon or "Read-only" label

### Editor State

#### Insert/Overwrite Mode
- **Insert**: "INS" indicator
- **Overwrite**: "OVR" indicator
- **Toggle**: Insert key switches modes
- **Default**: Insert mode

#### Language Mode
- **Display**: Current syntax highlighting mode
- **Default**: Markdown
- **Changes**: Based on file type
- **Purpose**: Confirms correct syntax highlighting

## Using Status Bar Information

### Writing Goals

#### Word Count Tracking
Monitor your progress toward word count goals:
- Blog posts (typically 500-2000 words)
- Articles (1000-5000 words)
- Academic papers (specified requirements)
- Daily writing targets

#### Document Length
Track document size for:
- Page estimates (roughly 250 words per page)
- Reading time estimates (200-250 words per minute)
- Content planning and structure
- Splitting large documents

### Editing Efficiency

#### Cursor Position
Use line/column info for:
- Navigating to specific locations
- Debugging markdown syntax issues
- Coordinating with collaborators
- Finding referenced line numbers

#### Selection Statistics
Helpful for:
- Extracting sections to new documents
- Checking paragraph lengths
- Ensuring consistent section sizes
- Meeting specific length requirements

## Customization

### Display Options
While Marker doesn't offer status bar customization, the information displayed is carefully chosen for markdown editing workflows.

### Visibility
- Always visible in normal window mode
- Hidden in fullscreen mode (F11)
- Returns when exiting fullscreen
- No option to permanently hide

## Practical Applications

### Academic Writing
- Track word counts for assignments
- Monitor abstract length (typically 150-250 words)
- Ensure introduction/conclusion balance
- Meet publication requirements

### Blogging
- Optimize post length for SEO (1500+ words)
- Create consistent content sizes
- Track series article lengths
- Plan content structure

### Technical Documentation
- Keep sections concise
- Monitor code block impact on length
- Track README file size
- Ensure comprehensive coverage

### Creative Writing
- Daily word count goals
- Chapter length consistency
- Scene size tracking
- Overall manuscript progress

## Tips and Tricks

### Quick Checks
- Glance at status bar for document stats
- No need to run separate word count tools
- Real-time feedback on writing progress
- Instant selection statistics

### Writing Productivity
- Set word count goals before starting
- Check progress periodically
- Use selection stats to balance sections
- Track daily writing output

### Document Planning
- Estimate final document size early
- Plan sections based on word allocation
- Monitor verbose sections
- Ensure adequate detail throughout

## Integration with Features

### With Auto-save
- Save indicator shows when auto-save triggers
- Modified indicator between saves
- Peace of mind with save status
- No manual checking needed

### With Find/Replace
- Line numbers help locate search results
- Navigate to specific line numbers
- Track changes across document
- Verify replacement counts

### With Export
- Word counts remain consistent across formats
- Character counts may vary with formatting
- Use for print page estimates
- Plan layout before export

## Common Scenarios

### Meeting Word Limits
For a 1000-word article:
1. Check current count in status bar
2. Calculate remaining words needed
3. Plan sections to fill gap
4. Monitor as you write

### Balancing Document Sections
1. Select each section individually
2. Note word count in status bar
3. Adjust lengths for balance
4. Achieve consistent structure

### Collaborative Editing
1. Share line numbers for feedback
2. Reference specific positions
3. Track changes by line count
4. Coordinate edits efficiently

## Understanding Statistics

### Word Count Methodology
- Counts space-separated units
- Includes numbers as words
- Markdown syntax counted as words
- Code blocks included in count

### Character Count Details
- All visible characters counted
- Spaces optional in count
- Line breaks included
- Markdown formatting included

### Line Count Specifics
- Empty lines counted
- Wrapped lines count as one
- Code blocks count all lines
- Headers count as single lines

## Best Practices

### Regular Monitoring
- Check stats at document milestones
- Track progress during long sessions
- Verify requirements before submission
- Use for time estimates

### Goal Setting
- Set realistic daily word goals
- Track weekly/monthly progress
- Adjust goals based on statistics
- Celebrate milestones

### Document Organization
- Keep similar documents similar lengths
- Use stats for content planning
- Balance sections appropriately
- Maintain consistency across projects

The status bar in Marker provides essential information at a glance, helping you stay informed about your document without interrupting your writing flow. Whether tracking word counts, monitoring file status, or navigating by line numbers, the status bar is your constant companion in the writing process.