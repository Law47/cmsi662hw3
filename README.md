# CMSI662 Homework #3

This repository contains secure, expandable, array-based stacks of strings in C, C++, and Java.

## Prerequisites

Install these tools if they are not already available:

- `gcc` (C compiler)
- `g++` (C++ compiler)
- `javac` and `java` (JDK)

## 1. C Module

Compile and run:

```bash
gcc -DSTRING_STACK_SELF_TEST c/string_stack.c -o c/string_stack
./c/string_stack
```

## 2. C++ Class

Compile and run:

```bash
g++ -DSECURE_STRING_STACK_SELF_TEST -x c++ cpp/SecureStringStack.hpp -o cpp/SecureStringStack
./cpp/SecureStringStack
```

## 3. Java Class

Compile and run:

```bash
javac java/SecureStringStack.java
java -cp java SecureStringStack
```