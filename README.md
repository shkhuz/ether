# ether
## NOTE: ETHER IS NO LONGER ACTIVELY DEVELOPED. 
A low-level general purpose programming language built to improve on C.

## Sematics and Syntax

### Hello World
```
main :: pub int {
    putsl("Hello, world!");
}
```

### Loops
```
main :: pub int {
    for i .. 100 {
        printfl("iteration: %", i);
    }
}
```

### Variables
```
main :: pub int {
    integer :: 102;
    printfl("integer is %", integer);

    float :: 436.36;
    printfl("float is %", float);

    letter :: 'e';
    printfl("letter is %", letter);
    
    str :: "hello";
    printfl("string is %", str);

    boolean :: false;
    printfl("boolean is %", boolean);
}
```

### Variable Type Specifier
```
main :: pub int {
    long_integer u64; // u64 is the type
    long_integer = __U64_MAX__;
    printfl("u64 max: %", long_integer);
}
```

### Arrays
```
    first_five_primes [5]int :: {
        2, 3, 5, 7, 11
    };

    for n .. arr_count(first_five_primes) {
        printfl("%: %", n, first_five_primes[n]);
    }
```

### Structs
```
struct vec2 {
    x f64;
    y f64;
}

main :: pub int {
    location vec2;
    location.x = 335.5;
    location.y = 1123.54;
    
    printfl("Entity location: (%, %)", location.x, location.y);
}
```

### Pointers
```
main :: pub int {
    integer :: 12;
    pointer ^int :: &integer; // pointer to 'integer'
    ^pointer = 5; // changing memory at 'integer'

    printfl("integer is %", integer); // 5
}
```
