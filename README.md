# Jlang

This is an experimental language.

## Syntax Of language
```go
fn foo()
{
    b := 20;
    print(b);
}

fn main()
{
    foo();
    a := 10;
    b := "Hello";
    c := true;
    print(20);
}
```

## Build Instruction

1. Meson Build

First Setup your meson directory
```
$ meson setup build

```

Then change to the dir
```
$ cd build

```

Then build
```
$ meson compile

```

2. Makefile
```
$ make

```
