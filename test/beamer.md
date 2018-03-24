---
class:beamer
title: A Simple Presentation
author: Martino Ferrari
paper: A4
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

```charter
plot:
  x: range: 1 10 100
  y: math: x^-3/2
```


---

# Conclusion

As shown in previous slides....