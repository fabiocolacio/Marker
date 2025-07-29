---
date: 2025-01-29 00:00:00
description: Create data visualizations and plots using Charter syntax
tags: charter, plotting, graphs, charts, data-visualization
authors: marker
---

# Charter Plotting

Charter is a simple charting library integrated into Marker that allows you to create line graphs, bar graphs, and scatter plots directly in your markdown documents using a simple, intuitive syntax.

## Overview

Charter provides a lightweight way to visualize data without leaving your markdown editor. Charts are rendered inline in the preview and exported to all output formats, making them perfect for reports, presentations, and documentation.

## Basic Syntax

Charter plots are created using code blocks with the `charter` language identifier:

````markdown
```charter
title: Sample Chart
x-label: X Axis
y-label: Y Axis

Line Dataset: 1,4,9,16,25
Bar Dataset: 5,10,15,20,25
```
````

## Chart Types

### Line Charts

Create line graphs for continuous data visualization:

````markdown
```charter
title: Monthly Sales
x-label: Month
y-label: Revenue (k$)
x-scale: Jan,Feb,Mar,Apr,May,Jun

Sales 2024: 45,52,48,58,63,71
Sales 2023: 42,48,44,51,55,62
```
````

**Features:**
- Multiple line series
- Custom colors per series
- Point markers
- Smooth or straight lines

### Bar Charts

Display categorical data with bar graphs:

````markdown
```charter
title: Product Comparison
x-label: Products
y-label: Units Sold
type: bar

Product A: 150
Product B: 230
Product C: 180
Product D: 210
```
````

**Features:**
- Vertical or horizontal bars
- Grouped or stacked bars
- Custom bar colors
- Value labels

### Scatter Plots

Show relationships between variables:

````markdown
```charter
title: Height vs Weight
x-label: Height (cm)
y-label: Weight (kg)
type: scatter

Dataset: (160,55),(165,60),(170,65),(175,72),(180,78),(185,85)
```
````

**Features:**
- Point size customization
- Multiple series
- Trend lines
- Point labels

## Chart Configuration

### Title and Labels

````markdown
```charter
title: Chart Title
subtitle: Optional subtitle
x-label: X Axis Label
y-label: Y Axis Label
```
````

### Axis Configuration

#### Scale Types
````markdown
```charter
x-scale: linear  # Default
y-scale: log     # Logarithmic scale
```
````

#### Custom Scales
````markdown
```charter
x-scale: Q1,Q2,Q3,Q4      # Custom labels
y-scale: 0,25,50,75,100   # Custom values
```
````

#### Axis Ranges
````markdown
```charter
x-min: 0
x-max: 100
y-min: -10
y-max: 50
```
````

### Data Series

#### Named Series
````markdown
```charter
Series A: 1,2,3,4,5
Series B: 5,4,3,2,1
```
````

#### Multiple Data Formats
````markdown
```charter
# Comma-separated values
Dataset 1: 10,20,30,40,50

# Space-separated values
Dataset 2: 10 20 30 40 50

# Coordinate pairs (for scatter)
Points: (1,10),(2,20),(3,30)
```
````

### Styling Options

#### Colors
````markdown
```charter
color-scheme: blue,red,green,orange
# or
Series A [color:blue]: 1,2,3,4,5
Series B [color:red]: 5,4,3,2,1
```
````

#### Chart Dimensions
````markdown
```charter
width: 600
height: 400
```
````

## Advanced Examples

### Multi-Series Line Chart
````markdown
```charter
title: Website Traffic Analysis
subtitle: Daily Unique Visitors
x-label: Day
y-label: Visitors (thousands)
x-scale: Mon,Tue,Wed,Thu,Fri,Sat,Sun

Desktop: 45,48,52,55,58,42,38
Mobile: 65,70,75,78,82,95,92
Tablet: 12,13,14,15,16,18,17
```
````

### Grouped Bar Chart
````markdown
```charter
title: Quarterly Sales by Region
x-label: Quarter
y-label: Sales (M$)
type: bar
group: true

North America: 45,52,48,58
Europe: 38,41,39,44
Asia: 55,58,62,68
```
````

### Scientific Data Plot
````markdown
```charter
title: Temperature vs Pressure
x-label: Temperature (°C)
y-label: Pressure (kPa)
type: scatter
show-trend: true

Experiment 1: (20,101),(25,105),(30,110),(35,116),(40,123)
Experiment 2: (20,102),(25,106),(30,111),(35,117),(40,124)
Theory: (20,101.3),(25,105.8),(30,110.3),(35,116.8),(40,123.3)
```
````

## Integration with Markdown

### Inline References
```markdown
As shown in the chart below, sales increased by 15% in Q4.

```charter
title: Quarterly Sales Growth
...
```

The data clearly demonstrates the upward trend.
```

### Figure Captions
Using scidown syntax:
```markdown
```charter
title: Population Growth
...
``` {#fig:population caption="Population growth over 50 years"}

According to Figure {@fig:population}, the growth is exponential.
```

## Best Practices

### Data Preparation
- **Clean data**: Remove invalid values
- **Consistent format**: Use same separator throughout
- **Appropriate scale**: Choose linear or log based on data
- **Meaningful labels**: Clear, concise axis labels

### Chart Selection
- **Line charts**: Time series, continuous data
- **Bar charts**: Categorical comparisons
- **Scatter plots**: Correlations, relationships
- **Keep it simple**: Don't overcrowd with too many series

### Visual Design
- **Color contrast**: Ensure readability
- **Font sizes**: Large enough for export
- **White space**: Don't fill every pixel
- **Consistency**: Use same style throughout document

## Troubleshooting

### Chart Not Rendering
- Check syntax: Ensure proper `charter` language tag
- Verify data format: Commas, spaces, or coordinates
- Look for errors: Check preview for error messages

### Data Issues
- **Missing points**: Use null or skip values
- **Scale problems**: Set manual min/max
- **Label overlap**: Reduce number of labels
- **Performance**: Limit data points for large datasets

### Export Problems
- **PDF**: Charts export as images
- **HTML**: Interactive features may be limited
- **LaTeX**: Converts to appropriate format
- **Resolution**: High-quality vector output

## Examples by Use Case

### Business Reports
````markdown
```charter
title: Monthly Revenue Breakdown
type: bar
stack: true

Product Sales: 450,480,520,490,510,530
Service Revenue: 150,160,170,180,190,200
Licensing: 50,55,60,58,62,65
```
````

### Scientific Papers
````markdown
```charter
title: Reaction Rate vs Temperature
x-label: Temperature (K)
y-label: Rate Constant (s⁻¹)
type: scatter
y-scale: log

Experimental: (298,0.001),(308,0.003),(318,0.009),(328,0.027)
Theoretical: (298,0.0011),(308,0.0032),(318,0.0093),(328,0.0271)
```
````

### Educational Materials
````markdown
```charter
title: Understanding Quadratic Functions
x-label: x
y-label: f(x)
x-scale: -5,-4,-3,-2,-1,0,1,2,3,4,5

f(x) = x²: 25,16,9,4,1,0,1,4,9,16,25
f(x) = -x²: -25,-16,-9,-4,-1,0,-1,-4,-9,-16,-25
```
````

## Performance Tips

- **Limit data points**: 100-200 points maximum for smooth rendering
- **Simplify complex charts**: Break into multiple smaller charts
- **Cache results**: Preview updates only when chart code changes
- **Optimize for export**: Test in target format

Charter integration in Marker makes it easy to add professional data visualizations to your documents without switching to external tools, maintaining your writing flow while creating compelling visual narratives.