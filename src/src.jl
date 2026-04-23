#load "libc";
extern "raylib", path="libraylib.so";

struct foo {
    name: string;
    age: number;
}

fn main() {
    x := 42 + 20 * 30;
    print(x);
}
