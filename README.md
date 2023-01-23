# The ??? Programming language

## Features

- Type checker
- Functions
- Lambdas
- Recursion
- Basic datatypes: i64, string
- Conditionals: if, else
- Loops: while

## Example

```
fn fib(n: i64) -> i64 {
	let i = 0;
	let p0 = 0;
	let p1 = 1;
	while(i < n){
		let p = p0 + p1;
		p0 = p1;
		p1 = p;
		i = i + 1;
	}
	p0;
}

fn main() -> i64 {
    let k = 0;
    while(k <= 90){
        println("fib(", k, ") = ", fib(k));
        k = k + 1;
    }
	0;
}

```

## Contributing

Not currently interested in contributions to the codebase.