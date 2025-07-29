---
date: 2025-01-29 00:00:00
description: Write mathematical equations and formulas using LaTeX syntax
tags: math, latex, katex, mathjax, equations, formulas
authors: marker
---

# Mathematical Syntax

Marker provides comprehensive mathematical notation support through LaTeX syntax, enabling you to write complex mathematical expressions, equations, and formulas that render beautifully in both preview and exported documents.

## Overview

Mathematical expressions in Marker are powered by:
- **KaTeX**: Fast, high-quality math rendering (default)
- **MathJax**: Alternative rendering engine with broader LaTeX support
- **LaTeX Export**: Native LaTeX output for academic publishing

## Basic Math Syntax

### Inline Math

Use single dollar signs `$...$` for inline mathematical expressions within text.

```markdown
The quadratic formula is $x = \frac{-b \pm \sqrt{b^2-4ac}}{2a}$ where $a \neq 0$.
```

**Renders as**: The quadratic formula is $x = \frac{-b \pm \sqrt{b^2-4ac}}{2a}$ where $a \neq 0$.

### Display Math

Use double dollar signs `$$...$$` for centered, display-style equations.

```markdown
$$
\int_{-\infty}^{\infty} e^{-x^2} dx = \sqrt{\pi}
$$
```

**Renders as**:
$$
\int_{-\infty}^{\infty} e^{-x^2} dx = \sqrt{\pi}
$$

### Alternative Display Syntax

You can also use `\[...\]` for display math:

```markdown
\[
E = mc^2
\]
```

**Renders as**:
\[
E = mc^2
\]

## Basic Mathematical Elements

### Greek Letters

```markdown
$\alpha, \beta, \gamma, \Delta, \Theta, \Lambda, \Omega$
$\pi, \phi, \psi, \chi, \rho, \sigma, \tau, \mu, \nu$
```

**Renders as**: $\alpha, \beta, \gamma, \Delta, \Theta, \Lambda, \Omega$, $\pi, \phi, \psi, \chi, \rho, \sigma, \tau, \mu, \nu$

### Basic Operations

```markdown
$a + b - c \times d \div e$
$a \cdot b \ast c \pm d \mp e$
```

**Renders as**: $a + b - c \times d \div e$, $a \cdot b \ast c \pm d \mp e$

### Fractions

```markdown
$\frac{a}{b}$, $\frac{numerator}{denominator}$, $\frac{1}{2}$
```

**Renders as**: $\frac{a}{b}$, $\frac{numerator}{denominator}$, $\frac{1}{2}$

### Superscripts and Subscripts

```markdown
$x^2$, $x^{n+1}$, $x_i$, $x_{i,j}$, $x_i^2$
```

**Renders as**: $x^2$, $x^{n+1}$, $x_i$, $x_{i,j}$, $x_i^2$

### Square Roots

```markdown
$\sqrt{x}$, $\sqrt[n]{x}$, $\sqrt{x^2 + y^2}$
```

**Renders as**: $\sqrt{x}$, $\sqrt[n]{x}$, $\sqrt{x^2 + y^2}$

## Advanced Mathematical Constructs

### Summations and Products

```markdown
$$
\sum_{i=1}^{n} i = \frac{n(n+1)}{2}
$$

$$
\prod_{i=1}^{n} i = n!
$$
```

**Renders as**:
$$
\sum_{i=1}^{n} i = \frac{n(n+1)}{2}
$$

$$
\prod_{i=1}^{n} i = n!
$$

### Integrals

```markdown
$$
\int_0^1 x^2 dx = \frac{1}{3}
$$

$$
\oint_C \mathbf{F} \cdot d\mathbf{r} = \iint_S (\nabla \times \mathbf{F}) \cdot d\mathbf{S}
$$
```

**Renders as**:
$$
\int_0^1 x^2 dx = \frac{1}{3}
$$

$$
\oint_C \mathbf{F} \cdot d\mathbf{r} = \iint_S (\nabla \times \mathbf{F}) \cdot d\mathbf{S}
$$

### Limits

```markdown
$$
\lim_{x \to 0} \frac{\sin x}{x} = 1
$$

$$
\lim_{n \to \infty} \left(1 + \frac{1}{n}\right)^n = e
$$
```

**Renders as**:
$$
\lim_{x \to 0} \frac{\sin x}{x} = 1
$$

$$
\lim_{n \to \infty} \left(1 + \frac{1}{n}\right)^n = e
$$

### Matrices

```markdown
$$
A = \begin{pmatrix}
a_{11} & a_{12} & a_{13} \\
a_{21} & a_{22} & a_{23} \\
a_{31} & a_{32} & a_{33}
\end{pmatrix}
$$

$$
\begin{vmatrix}
a & b \\
c & d
\end{vmatrix} = ad - bc
$$
```

**Renders as**:
$$
A = \begin{pmatrix}
a_{11} & a_{12} & a_{13} \\
a_{21} & a_{22} & a_{23} \\
a_{31} & a_{32} & a_{33}
\end{pmatrix}
$$

$$
\begin{vmatrix}
a & b \\
c & d
\end{vmatrix} = ad - bc
$$

### Systems of Equations

```markdown
$$
\begin{cases}
x + y = 3 \\
2x - y = 0
\end{cases}
$$

$$
\begin{align}
f(x) &= x^2 + 2x + 1 \\
&= (x + 1)^2
\end{align}
$$
```

**Renders as**:
$$
\begin{cases}
x + y = 3 \\
2x - y = 0
\end{cases}
$$

$$
\begin{align}
f(x) &= x^2 + 2x + 1 \\
&= (x + 1)^2
\end{align}
$$

## Scientific and Engineering Notation

### Chemical Formulas

```markdown
$\mathrm{H_2O}$, $\mathrm{CO_2}$, $\mathrm{C_6H_{12}O_6}$
$\mathrm{Ca^{2+}}$, $\mathrm{SO_4^{2-}}$
```

**Renders as**: $\mathrm{H_2O}$, $\mathrm{CO_2}$, $\mathrm{C_6H_{12}O_6}$, $\mathrm{Ca^{2+}}$, $\mathrm{SO_4^{2-}}$

### Units and Measurements

```markdown
$10\,\mathrm{kg}$, $25\,\mathrm{m/s}$, $3.14 \times 10^8\,\mathrm{m/s}$
$\mathrm{J \cdot mol^{-1} \cdot K^{-1}}$
```

**Renders as**: $10\,\mathrm{kg}$, $25\,\mathrm{m/s}$, $3.14 \times 10^8\,\mathrm{m/s}$, $\mathrm{J \cdot mol^{-1} \cdot K^{-1}}$

### Scientific Constants

```markdown
$c = 3.0 \times 10^8\,\mathrm{m/s}$
$h = 6.626 \times 10^{-34}\,\mathrm{J \cdot s}$
$N_A = 6.022 \times 10^{23}\,\mathrm{mol^{-1}}$
```

**Renders as**: $c = 3.0 \times 10^8\,\mathrm{m/s}$, $h = 6.626 \times 10^{-34}\,\mathrm{J \cdot s}$, $N_A = 6.022 \times 10^{23}\,\mathrm{mol^{-1}}$

## Fonts and Styling

### Mathematical Fonts

```markdown
$\mathbb{R}$ (blackboard bold)
$\mathcal{L}$ (calligraphic)  
$\mathfrak{g}$ (fraktur)
$\mathrm{text}$ (roman)
$\mathbf{v}$ (bold)
$\mathit{italic}$ (italic)
```

**Renders as**: $\mathbb{R}$, $\mathcal{L}$, $\mathfrak{g}$, $\mathrm{text}$, $\mathbf{v}$, $\mathit{italic}$

### Text in Math Mode

```markdown
$$
P(\text{heads}) = \frac{1}{2}
$$

$$
\text{if } x > 0 \text{ then } f(x) = \sqrt{x}
$$
```

**Renders as**:
$$
P(\text{heads}) = \frac{1}{2}
$$

$$
\text{if } x > 0 \text{ then } f(x) = \sqrt{x}
$$

## Spacing and Alignment

### Manual Spacing

```markdown
$a\,b$ (thin space)
$a\;b$ (medium space)  
$a\quad b$ (quad space)
$a\qquad b$ (double quad space)
```

**Renders as**: $a\,b$, $a\;b$, $a\quad b$, $a\qquad b$

### Multi-line Equations

```markdown
$$
\begin{aligned}
\nabla \cdot \mathbf{E} &= \frac{\rho}{\epsilon_0} \\
\nabla \cdot \mathbf{B} &= 0 \\
\nabla \times \mathbf{E} &= -\frac{\partial \mathbf{B}}{\partial t} \\
\nabla \times \mathbf{B} &= \mu_0\mathbf{J} + \mu_0\epsilon_0\frac{\partial \mathbf{E}}{\partial t}
\end{aligned}
$$
```

**Renders as**:
$$
\begin{aligned}
\nabla \cdot \mathbf{E} &= \frac{\rho}{\epsilon_0} \\
\nabla \cdot \mathbf{B} &= 0 \\
\nabla \times \mathbf{E} &= -\frac{\partial \mathbf{B}}{\partial t} \\
\nabla \times \mathbf{B} &= \mu_0\mathbf{J} + \mu_0\epsilon_0\frac{\partial \mathbf{E}}{\partial t}
\end{aligned}
$$

## Configuration and Preferences

### Rendering Engine Selection

Access via **Preferences** → **Preview**:
- **KaTeX** (default): Fast, lightweight, good browser support
- **MathJax**: More complete LaTeX support, slower rendering

### Quality Settings

- **High-quality rendering**: Vector-based output for crisp display
- **Export compatibility**: Math renders in all export formats
- **Print quality**: Professional typography in PDF exports

## Advanced Features

### Numbered Equations

When using scidown's scientific extensions:

```markdown
$$
E = mc^2
$$ {#eq:einstein}

As shown in equation (@eq:einstein), mass and energy are equivalent.
```

### Custom Macros

Define custom commands for repeated expressions:

```markdown
$$
\newcommand{\R}{\mathbb{R}}
\newcommand{\norm}[1]{\left\| #1 \right\|}
$$

For $x \in \R^n$, we have $\norm{x} \geq 0$.
```

### Complex Examples

#### Fourier Transform
```markdown
$$
\mathcal{F}\{f(t)\} = F(\omega) = \int_{-\infty}^{\infty} f(t) e^{-i\omega t} dt
$$
```

#### Schrödinger Equation
```markdown
$$
i\hbar\frac{\partial}{\partial t}\Psi(\mathbf{r},t) = \hat{H}\Psi(\mathbf{r},t)
$$
```

#### Statistical Distribution
```markdown
$$
f(x|\mu,\sigma^2) = \frac{1}{\sigma\sqrt{2\pi}} e^{-\frac{1}{2}\left(\frac{x-\mu}{\sigma}\right)^2}
$$
```

## Tips and Best Practices

### Writing Mathematics
- **Preview frequently**: Check rendering as you write
- **Use parentheses**: Group complex expressions clearly
- **Consistent notation**: Maintain consistent variable naming
- **Proper spacing**: Use `\,` for thin spaces in units

### Performance
- **Inline vs display**: Use appropriate math mode for context
- **Complex expressions**: Break very long equations into parts
- **Testing**: Verify math renders in preview before export

### Accessibility
- **Alt text**: Provide text descriptions for complex equations
- **Context**: Explain mathematical notation when first introduced
- **Readability**: Use clear, standard mathematical conventions

Mathematical notation in Marker provides the tools needed for academic writing, scientific documentation, and technical communication, with professional-quality output suitable for publication and presentation.