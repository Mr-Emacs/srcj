#load "libc";

struct Array {
    items: *Any;
    count: number;
    capacity: number;
    fn new(self, type: Any) -> self {
        Array { *items = type, count = 0, capacity = 0 }
    }
    
    fn push(self, item: Any) {
        if count >= capacity {
            if capacity == 0 { capacity = 256; }
            else { capacity *= 2 };
            items = libc::realloc(items, items.size() * capacity);
        }
        *(items + count) = item;
        count += 1;
    }

    fn get(self, index: number) -> Any {
        return *(items + index);
    }

    fn set(self, index: number, item: number) {
        *(items + index) = item;
    }
}

fn main() {
    xs := Array.new(number);
    x := 0;
    i := 0;
    
    while x < 20 {
        xs.push(x);
    }
    
    while i < xs.count {
        print(xs.get(i));
    }
}
