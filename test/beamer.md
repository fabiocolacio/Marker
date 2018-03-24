---
class:beamer
title: A Simple Presentation
author: Martino Ferrari
paper: 4:3
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

# Conclusion

As shown in figure (#fig:1) the output is cool