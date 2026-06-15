# Strafe Helper Math

We use these symbols for the relevant values in `PM_Accelerate`:

* $`v \in \mathbb{R}^3`$: `pm->ps->velocity`, current velocity of the player
* $`w \in \mathbb{R}^3`$: `wishdir`, direction in which the player wishes to accelerate
* $`s \in \mathbb{R}`$: `wishspeed`, target speed that the player wishes to accelerate to
* $`a \in \mathbb{R}`$: `accel`, acceleration constant
* $`\Delta t \in \mathbb{R}`$: `pml.frametime`, time duration of this physics frame

Here, $`s`$, $`a`$ and $`\Delta t`$ can be assumed to be positive and $`w`$ to be normalized.

Then according to `PM_Accelerate`, the new velocity $`v^+ \in \mathbb{R}^3`$ depending on the user inputs $`s`$ and $`w`$ is given by

```math
\begin{align*}
v^+(s,w) &= \begin{cases}
v + \min\left\{sa \Delta t, s - v^\mathsf{T}w\right\} w &\text{ if } s - v^\mathsf{T}w > 0\text{,}\\
v &\text{ otherwise.}
\end{cases}
\end{align*}
```

## Horizontal Speed

To consider the horizontal components of vectors, we multiply with the matrix

```math
H := \begin{pmatrix}1& 0& 0\\0& 1& 0\\ 0& 0& 0\end{pmatrix}\text{.}
```

With that, the new horizontal speed can be written as $`\|H v^+(s,w)\|`$.
Ignoring the boring case $`s - v^\mathsf{T}w \leq 0`$, we find the expression

```math
\begin{align*}
\|H v^+(s,w)\|^2 &= \left(H v^+(s,w)\right)^\mathsf{T} \left(H v^+(s,w)\right) \\
&= \left(v^+(s,w)\right)^\mathsf{T} H v^+(s,w) \\
&= \|Hv\|^2 + 2 \min\left\{s a\Delta t, s - v^\mathsf{T}w\right\} v^\mathsf{T} H w + \min\left\{sa\Delta t, s - v^\mathsf{T}w\right\}^2 \|Hw\|^2 \text{.}
\end{align*}
```

By rearranging this we can separate the terms that are influenced by the horizontal looking direction and those that are unaffected by it:

```math
\begin{align}
\|H v^+(s,w)\|^2 &=\|Hv\|^2 + 2\min\{sa\Delta t, s - v^\mathsf{T}Hw - v_zw_z\} v^\mathsf{T}Hw\notag\\
&\quad + \min\{sa\Delta t, s - v^\mathsf{T}Hw - v_zw_z\}^2\|Hw\|^2 \text{.}
\end{align}
```

In this expression, the user-input dependent terms $`s`$, $`w_z`$ and $`\|Hw\|`$ are invariant to horizontal rotation.
The only other term that is user-input dependent is $`v^\mathsf{T}Hw`$.
And this term (after scaling) is *only* affected by horizontal rotation, as it is directly related to the angle $`\theta_H`$ between $`Hv`$ and $`Hw`$:

```math
\cos \theta_H =\frac{v^\mathsf{T} H w}{\|Hv\|\|Hw\|\text{.}}
```

This is exactly what we need for the strafe helper, as it is not supposed to suggest input movement keys that influence $`s`$, $`w_z`$ or $`\|Hw\|`$.
Instead, it takes those values as given and then suggests values for the horizontal looking direction.
And the most useful form to express this looking direction turns out to be $`\theta_H`$.

### Increasing Horizontal Speed

To find player inputs that lead to an increase in horizontal speed, we need to find $`s`$ and $`w`$ such that $`\|Hv^+(s,w)\| > \|Hv\|`$.
By definition we have $`v^+(s,w) = v$ for all $s - v^\mathsf{T}w \leq 0`$, so a mandatory condition is

```math
\begin{align*}
v^\mathsf{T}w &< s \\
\Leftrightarrow v^\mathsf{T}Hw + v_z w_z &< s\\
\Leftrightarrow \frac{v^\mathsf{T}Hw}{\|Hv\|\|Hw\|} &< \frac{s - v_z w_z}{\|Hv\|\|Hw\|} \\
\Leftrightarrow \cos \theta_H &< \frac{s - v_z w_z}{\|Hv\|\|Hw\|}\text{.}
\end{align*}
```

So assuming that that is given, we have $`\min\{sa\Delta t, s - v^\mathsf{T}w\} > 0`$ and therefore

```math
\begin{align*}
\|H^+(s,w)\| &> \|Hv\| \\
\Leftrightarrow \|Hv^+(s,w)\|^2 &> \|Hv\|^2 \\
\Leftrightarrow 2 \min\left\{s a\Delta t, s - v^\mathsf{T}w\right\} v^\mathsf{T} H w &> - \min\left\{sa\Delta t, s - v^\mathsf{T}w\right\}^2 \|Hw\|^2 \\
\Leftrightarrow 2 v^\mathsf{T}Hw &> - \min\{sa\Delta t, s - v^\mathsf{T}w\}\|Hw\|^2 \\
\Leftrightarrow 2 v^\mathsf{T}Hw &> \max\{-sa\Delta t, v^\mathsf{T}w - s\}\|Hw\|^2 \\
\Leftrightarrow 2 v^\mathsf{T}Hw > -sa\Delta t\|Hw\|^2 \quad &\wedge \quad 2 v^\mathsf{T}Hw > (v^\mathsf{T}w - s)\|Hw\|^2\text{.}
\end{align*}
```

#### Case 1

```math
\begin{align*}
2 v^\mathsf{T}Hw &> -sa\Delta t\|Hw\|^2 \\
\Leftrightarrow \frac{v^\mathsf{T}Hw}{\|Hw\|} &> - \frac{1}{2} sa\Delta t\|Hw\| \\
\Leftrightarrow \cos \theta_H &> -\frac{1}{2}sa\Delta t \frac{\|Hw\|}{\|Hv\|}
\end{align*}
```

#### Case 2

```math
\begin{align*}
2 v^\mathsf{T}Hw &> (v^\mathsf{T}w - s)\|Hw\|^2 \\
\Leftrightarrow 2 v^\mathsf{T}Hw &> (v^\mathsf{T}Hw + v_z w_z - s) \|Hw\|^2 \\
\Leftrightarrow (2 - \|Hw\|^2) v^\mathsf{T}Hw &> (v_zw_z - s)\|Hw\|^2 \\
\Leftrightarrow \frac{v^\mathsf{T}Hw}{\|Hw\|} &> \frac{v_zw_z - s}{2 - \|Hw\|^2} \|Hw\| \\
\Leftrightarrow \cos \theta_H &> \frac{v_zw_z - s}{2 - \|Hw\|^2} \frac{\|Hw\|}{\|Hv\|}
\end{align*}
```

So together, assuming that $`\cos \theta_H < \frac{s - v_z w_z}{\|Hv\|\|Hw\|}`$ holds, we have

```math
\|Hv^+(s,w)\| > \|Hv\| \quad\Leftrightarrow\quad \cos \theta_H > \max\left\{-\frac{1}{2}sa\Delta t, \frac{v_zw_z - s}{2 - \|Hw\|^2} \right\} \frac{\|Hw\|}{\|Hv\|}\text{.}
```

In summary, the horizontal speed increases iff

```math
\arccos\left(\frac{s - v_z w_z}{\|Hv\|\|Hw\|}\right) < \theta_H < \arccos \left( \max\left\{-\frac{1}{2}sa\Delta t, \frac{v_zw_z - s}{2 - \|Hw\|^2} \right\} \frac{\|Hw\|}{\|Hv\|} \right)
```

where either side is meant to be unbounded if the respective argument is outside of the domain of $`\arccos`$.

### Maximizing Horizontal Speed

Based on $`(1)`$ we introduce $`f:\mathbb{R}\rightarrow\mathbb{R}`$ with

```math
f(x) := \|Hv\|^2 + 2\min\{sa\Delta t, s-x-v_zw_z\}x + \min\{sa\Delta t, s - x - v_zw_z\}^2 \|Hw\|^2
```

to simplify notation.
For the slope of this function we find

```math
\begin{align*}
f'(x) &=
\begin{cases}
2sa\Delta t & \text{ for } x < (1-a\Delta t)s - v_z w_z \\
-4x + 2(s-v_z w_z) - 2(s-x-v_zw_z)\|Hw\|^2 & \text{ for } x > (1-a\Delta t)s - v_z w_z
\end{cases} \\
&=
\begin{cases}
2sa\Delta t & \text{ for } x < (1-a\Delta t)s - v_z w_z \\
2(\|Hw\|^2 - 2)x + 2(s - v_z w_z)(1 - \|Hw\|^2) & \text{ for } x > (1-a\Delta t)s - v_z w_z\text{.}
\end{cases}
\end{align*}
```

Now $`2sa\Delta t`$ is always positive.
For the other case, we have

```math
\|Hw\|^2 - 2 = \|w\|^2 - w_z^2 - 2 \leq \|w\|^2 - 2 = -1\text{,}
```

which means that the whole term will become negative for large enough $`x`$.
To determine exactly at what $`x`$, we set the term equal to $`0`$:

```math
\begin{align*}
0 &= 2(\|Hw\|^2 - 2)x + 2(s - v_z w_z)(1 - \|Hw\|^2) \\
\Leftrightarrow x &= \frac{1 - \|Hw\|^2}{2 - \|Hw\|^2} (s-v_zw_z) \\
&= \frac{1 - (\|w\|^2 - w_z^2)}{2 - (\|w\|^2 - w_z^2)}(s - v_z w_z) \\
&= \frac{w_z^2}{1 + w_z^2} ( s - v_z w_z)
\end{align*}
```

So we have

```math
\begin{align*}
f(x) \begin{cases}
\text{monotonously increasing} & \text{ for } x < \max\left\{(1-a\Delta t)s - v_z w_z, \frac{w_z^2}{1 + w_z^2} ( s - v_z w_z)\right\} \\
\text{monotonously decreasing} & \text{ for } x > \max\left\{(1-a\Delta t)s - v_z w_z, \frac{w_z^2}{1 + w_z^2} ( s - v_z w_z)\right\}
\end{cases}\text{.}
\end{align*}
```

Since $`f`$ is contiuous, that means $`\max_{x\in\mathbb{R}} f(x) = f(x^*)`$ where

```math
x^* := \max\left\{(1-a\Delta t)s - v_z w_z, \frac{w_z^2}{1 + w_z^2} ( s - v_z w_z)\right\}\text{.}
```

So comparing $`f(x)`$ to $`(1)`$, we basically relabeled $`v^\mathsf{T} H w`$ as $x$, so

```math
\begin{align*}
v^\mathsf{T} H w &= x\\
\Rightarrow \frac{v^\mathsf{T} H w}{\|Hv\|\|Hw\|} &= \frac{x}{\|Hv\|\|Hw\|} \\
\Rightarrow \cos \theta_H &= \frac{x}{\|Hv\|\|Hw\|}\text{.}
\end{align*}
```

That means that $`(1)`$ is maximized for

```math
\begin{align*}
\cos \theta^*_H &= \min\left\{1, \frac{x^*}{\|Hv\|\|Hw\|}\right\} \\
\Rightarrow \theta^*_H &= \arccos\left(\min\left\{1,\frac{\max\left\{(1-a\Delta t)s - v_z w_z, \frac{w_z^2}{1 + w_z^2} ( s - v_z w_z)\right\}}{\|Hv\|\|Hw\|}\right\}\right)\text{.}
\end{align*}
```
