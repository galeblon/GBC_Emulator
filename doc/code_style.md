# C code style

Throughout the project's C code the [Linux kernel coding style](https://www.kernel.org/doc/html/v4.10/process/coding-style.html) is used with following amendments:

## 1) Indentation

Indentation is realised with tab characters, so indentation width is to be set on the side of the developer.

## 4) Naming

### Global variables

Module-scope variable names should be prefixed with `g_`.
Program-scope variables should be avoided.

### Function names

Function names should be prefixed with `<name of module>_`.
Functions not belonging to modules public interface should also be prefixed with `_`.

## 7) Centralized exiting of functions

`goto` instructions should not be used.