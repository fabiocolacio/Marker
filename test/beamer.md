---
class:beamer
title: A Simple Presentation
author: Martino Ferrari
paper: 4:3
---

@toc

---

# Introduction

The results will be presented in equation ((#eq1))

---

# Formula

 
Some equation:

@equation(eq1)
f(x)=\sum_{i=0}^nx^{-i}
@/
where $x\in\mathbb{R}$ is the independent variable and $n\in\mathbb{N}$ is a constant.

---

# Result

@figure(fig:1)
```charter
title: f(x)
height: 300
plot:
  x: range: 1 10 100
  y: math: 2*x^-1+1
  ls: --
```
@caption(approximation of equation 1 with $n=3$)
@/


---

# Table

| Column A. | Column B.|
| --------- | -------- |
| Element A.1 | Element B.1 |
| Element A.2 | Element B.2 |
| Element A.3 | Element B.3 |
| Element A.4 | Element B.4 |

---

# Conclusion

As shown in figure (#fig:1) the output is cool