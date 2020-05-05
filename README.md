# ether
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
