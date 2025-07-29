---
date: 2025-01-29 00:00:00
description: Scientific markdown extensions and Beamer presentation support
tags: scidown, beamer, academic, scientific, presentations, latex
authors: marker
---

# SciDown - Scientific Markdown

Marker uses [SciDown](https://github.com/Mandarancio/scidown/) as its markdown parser, providing powerful extensions for scientific and academic writing, including support for Beamer presentations, academic document structures, and advanced cross-referencing.

![Beamer presentation example](./media/slides.png)

## Overview

SciDown extends standard markdown with features essential for academic and scientific documents:
- YAML headers for metadata
- Document classes (article, beamer)
- Cross-references and citations
- Numbered figures, tables, and equations
- Abstract sections
- Table of contents generation
- External document inclusion

## Document Classes

### Article Class

For academic papers and reports:

```markdown
---
title: "Quantum Computing: A Modern Approach"
author: "Dr. Jane Smith"
date: 2024-01-15
class: article
abstract: |
  This paper explores the fundamental principles of quantum computing
  and their practical applications in modern cryptography.
---

# Introduction

The field of quantum computing has evolved rapidly...
```

### Beamer Class

For presentations and slides:

```markdown
---
title: "Introduction to Machine Learning"
author: "Prof. John Doe"
date: 2024-01-20
class: beamer
theme: metropolis
---

# Overview

## What is Machine Learning?

- Subset of artificial intelligence
- Learns from data
- Makes predictions or decisions

---

# Types of Learning

## Supervised Learning

Training with labeled data...
```

## YAML Headers

### Basic Metadata

```yaml
---
title: "Document Title"
author: 
  - "First Author"
  - "Second Author"
date: 2024-01-15
keywords: [markdown, scidown, academic]
lang: en
---
```

### Advanced Options

```yaml
---
class: article          # or beamer
papersize: a4          # or letter, b4:3, b16:9 for beamer
fontsize: 12pt
geometry: margin=1in
bibliography: refs.bib
csl: ieee.csl
---
```

### Beamer-Specific Options

```yaml
---
class: beamer
theme: metropolis      # or default, Madrid, Berlin, etc.
colortheme: beaver     # or dolphin, orchid, rose, etc.
aspectratio: 169       # 16:9, or 43 for 4:3
navigation: true
---
```

## Document Structure

### Abstract

```markdown
---
title: "Research Paper"
abstract: |
  This is a multi-line abstract that describes
  the key findings and methodology of the paper.
---
```

Or inline:

```markdown
# Abstract {.abstract}

This section will be formatted as an abstract.
```

### Table of Contents

```markdown
# Table of Contents {.toc}

<!-- TOC will be auto-generated here -->

# Chapter 1
## Section 1.1
## Section 1.2

# Chapter 2
```

### Sections and Subsections

```markdown
# Chapter Title {#chap:intro}
## Section {#sec:background}
### Subsection
#### Subsubsection

Reference to Chapter {@chap:intro} and Section {@sec:background}.
```

## Cross-References

### Figures

```markdown
![Data visualization](plot.png){#fig:results caption="Experimental results showing correlation"}

As shown in Figure {@fig:results}, the correlation is significant.
```

### Tables

```markdown
| Method | Accuracy | Speed |
|--------|----------|-------|
| SVM    | 95%      | Fast  |
| DNN    | 98%      | Slow  |
{#tbl:comparison caption="Comparison of ML methods"}

Table {@tbl:comparison} summarizes the performance.
```

### Equations

```markdown
$$
E = mc^2
$$ {#eq:einstein}

Einstein's famous equation {@eq:einstein} shows mass-energy equivalence.
```

### Code Listings

```markdown
```python
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n-1) + fibonacci(n-2)
``` {#lst:fib caption="Recursive Fibonacci implementation"}

Listing {@lst:fib} shows a simple recursive approach.
```

## Beamer Presentations

### Slide Separation

Use `---` (three dashes) to separate slides:

```markdown
# Introduction

Welcome to this presentation

---

# Main Content

## First Point

- Important detail
- Supporting information

---

# Conclusion

Thank you for your attention!
```

### Slide Layouts

#### Two-Column Layout

```markdown
# Two Column Slide {.columns}

::: {.column width="50%"}
Left column content
- Point 1
- Point 2
:::

::: {.column width="50%"}
Right column content
- Point A
- Point B
:::
```

#### Incremental Reveals

```markdown
# Progressive Disclosure

::: incremental
- First point appears
- Then second point
- Finally third point
:::
```

### Speaker Notes

```markdown
# Main Slide

Content visible to audience

::: notes
Speaker notes only visible in presenter mode:
- Remember to mention X
- Emphasize Y
- Time check: 5 minutes
:::
```

## Scientific Features

### Citations

```markdown
According to recent research [@smith2023; @jones2024], quantum computing
shows promise. Smith's work [-@smith2023] specifically addresses...

# References {.bibliography}
```

### Theorems and Proofs

```markdown
::: {.theorem #thm:pythagorean}
**Pythagorean Theorem**: In a right triangle, $a^2 + b^2 = c^2$
:::

::: {.proof}
Consider a right triangle with sides $a$, $b$, and hypotenuse $c$...
:::

Theorem {@thm:pythagorean} is fundamental to geometry.
```

### Definitions and Examples

```markdown
::: {.definition}
**Machine Learning**: A subset of AI that enables systems to learn
from data without explicit programming.
:::

::: {.example}
A spam filter learning from email patterns is an example of ML.
:::
```

## External Document Inclusion

### Include Other Markdown Files

```markdown
# Chapter 1

{include:introduction.md}

# Chapter 2

{include:methodology.md}
```

### Include Code Files

```markdown
# Implementation

```{include:python src/algorithm.py}
```
```

## Advanced Features

### Custom Blocks

```markdown
::: {.warning}
**Caution**: This method requires significant computational resources.
:::

::: {.note}
Additional optimization techniques are discussed in Appendix A.
:::
```

### Metadata for Sections

```markdown
# Methods {#sec:methods .nonumber}

This section won't be numbered in the TOC.

## Data Collection {.unnumbered}

This subsection also won't be numbered.
```

### Page Breaks

```markdown
Content on page 1

\pagebreak

Content on page 2
```

## Best Practices

### Document Organization
- Use YAML headers for all metadata
- Create consistent heading hierarchy
- Use meaningful reference labels
- Keep abstracts concise

### Beamer Presentations
- Limit content per slide
- Use progressive disclosure wisely
- Test aspect ratios before presenting
- Include speaker notes for complex topics

### Scientific Writing
- Label all figures, tables, and equations
- Use cross-references instead of "above/below"
- Maintain bibliography file
- Follow journal-specific formatting

## Export Considerations

### LaTeX Export
- SciDown features translate directly
- Custom commands preserved
- Bibliography handled natively
- Beamer exports as LaTeX beamer

### PDF Export
- High-quality output via LaTeX
- Proper formatting maintained
- Cross-references resolved
- Presentations export as slides

### HTML Export
- Interactive features where possible
- MathJax/KaTeX for equations
- Responsive design
- Slide navigation for presentations

## Troubleshooting

### Common Issues

#### References Not Working
- Ensure unique IDs
- Use `{#label}` syntax correctly
- Check for typos in references

#### Beamer Slides Not Separating
- Use exactly three dashes `---`
- Ensure blank lines around separators
- Check YAML header has `class: beamer`

#### Missing Figures/Tables
- Verify file paths
- Check image formats
- Ensure captions use proper syntax

The SciDown integration in Marker transforms it from a simple markdown editor into a powerful tool for academic writing, scientific documentation, and professional presentations, all while maintaining the simplicity and readability of markdown.