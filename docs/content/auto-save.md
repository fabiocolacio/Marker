---
date: 2025-01-29 00:00:00
description: Automatic document saving to prevent data loss
tags: auto-save, saving, backup, preferences
authors: marker
---

# Auto-Save Feature

Marker provides comprehensive auto-save functionality to ensure your work is never lost. With both periodic and event-based saving options, you can write with confidence knowing your documents are protected.

## Overview

The auto-save feature offers two complementary modes:
- **Periodic auto-save**: Saves documents at regular intervals
- **Focus-out auto-save**: Saves when you switch away from Marker

Both modes work together to provide maximum protection while maintaining performance.

## Configuration

Access auto-save settings through **Preferences → Window** tab.

![Window preferences showing auto-save options](./media/05_editor_preferences.png)

### Available Options

#### Auto-save on Focus Out
- **Setting**: "Auto save on focus out"
- **Default**: Enabled
- **Behavior**: Automatically saves all modified documents when:
  - You switch to another application
  - The Marker window loses focus
  - You minimize the window

#### Periodic Auto-save
- **Setting**: "Auto save periodically"
- **Default**: Enabled
- **Behavior**: Saves all modified documents at regular intervals
- **Interval**: Configurable (10-600 seconds)
- **Default interval**: 300 seconds (5 minutes)

## How It Works

### Periodic Saving
```
Every [interval] seconds:
├── Check all open documents
├── Identify modified documents
├── Save each modified document
└── Continue timer
```

### Focus-Out Saving
```
When window loses focus:
├── Detect focus change event
├── Check all open documents
├── Save all modified documents
└── Allow focus change
```

### Smart Saving
- **Only saves modified files**: Unchanged files are skipped
- **Preserves file paths**: Uses existing save locations
- **No interruption**: Saves happen in background
- **Visual feedback**: Status bar shows save operations

## Benefits

### Data Protection
- **Prevent work loss**: Never lose more than a few minutes of work
- **Power failure protection**: Regular saves minimize data loss
- **Crash recovery**: Recent saves available after unexpected closure
- **Multi-document safety**: All open files are protected

### Workflow Enhancement
- **Worry-free writing**: Focus on content, not saving
- **Natural save points**: Focus-out saves at task switches
- **Consistent state**: Documents always reflect recent work
- **Version safety**: Compatible with version control

## Configuration Examples

### Maximum Protection Setup
```
☑ Auto save on focus out
☑ Auto save periodically
   Interval: 60 seconds
```
Best for: Critical documents, unreliable systems

### Balanced Setup (Default)
```
☑ Auto save on focus out
☑ Auto save periodically
   Interval: 300 seconds (5 minutes)
```
Best for: General use, most users

### Performance Setup
```
☑ Auto save on focus out
☐ Auto save periodically
```
Best for: Large files, slower systems

### Manual Control Setup
```
☐ Auto save on focus out
☐ Auto save periodically
```
Best for: Users who prefer manual save control

## Usage Scenarios

### Academic Writing
- Long thesis chapters saved regularly
- Research notes protected during alt-tab research
- Citations preserved when switching to reference manager

### Code Documentation
- API docs saved when testing code
- README updates preserved during commits
- Tutorial writing protected during example testing

### Note Taking
- Meeting notes saved automatically
- Ideas captured without save anxiety
- Quick notes preserved when switching tasks

### Blogging
- Draft posts saved regularly
- Content preserved during research
- Multiple posts protected simultaneously

## Technical Details

### Save Triggers
1. **Timer-based**: Background timer for periodic saves
2. **Event-based**: GTK focus-out signal detection
3. **User-initiated**: Manual save still available (Ctrl+S)

### File Handling
- **Atomic saves**: Temporary file + rename for safety
- **Encoding preservation**: Maintains file encoding
- **Permission retention**: Preserves file permissions
- **Metadata preservation**: Keeps file timestamps

### Performance Optimization
- **Efficient checking**: Only scans for modifications
- **Batch operations**: Saves multiple files efficiently
- **Non-blocking**: UI remains responsive during saves
- **Smart scheduling**: Avoids save conflicts

## Best Practices

### Interval Selection
- **Frequent editing**: 60-120 seconds
- **Normal editing**: 300 seconds (default)
- **Light editing**: 600 seconds
- **Consider file size**: Larger files may need longer intervals

### Workflow Integration
- **Trust the system**: Let auto-save work in background
- **Manual saves**: Still use Ctrl+S for important checkpoints
- **Version control**: Commit at logical points, not every auto-save
- **Backup strategy**: Auto-save complements, doesn't replace backups

## Interaction with Other Features

### Manual Save (Ctrl+S)
- Works alongside auto-save
- Resets auto-save timer
- Immediate save guarantee
- Visual confirmation provided

### Save As (Ctrl+Shift+S)
- Unaffected by auto-save
- Creates new file normally
- Auto-save applies to new file
- Original file unchanged

### Export Functions
- Independent of auto-save
- Source file saved before export
- Export doesn't trigger auto-save
- Clean separation of concerns

## Troubleshooting

### Auto-save Not Working
1. Check preferences are enabled
2. Verify file has been saved initially
3. Ensure file permissions allow writing
4. Check disk space availability

### Performance Issues
- Increase auto-save interval
- Disable periodic save, keep focus-out only
- Check for very large files
- Monitor system resources

### Save Conflicts
- Auto-save respects file locks
- Version control conflicts handled normally
- Network drive delays considered
- External modifications detected

## Tips

### For New Users
- Start with default settings
- Monitor status bar for save notifications
- Trust the system but verify initially
- Adjust intervals based on experience

### For Power Users
- Customize intervals per workflow
- Combine with version control
- Use focus-out for natural saves
- Set shorter intervals for critical work

The auto-save feature in Marker provides peace of mind while maintaining the flexibility to work the way you want, ensuring your writing is always protected without interrupting your creative flow.
