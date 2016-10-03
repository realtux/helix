# helix 0.0.1-alpha
helix is a fun and mostly useless toy language written in c. helix is based on an earlier project of mine called [axis](https://github.com/ebrian/axis) which was written in node.

##### what does helix look like?
```go
// single comment
output::print('hello world')

// function declaration
fn say_hi() {
    output::print('hi :)')
}

// variables
name = 'brian'

// conditionals
if name == 'brian' {
    output::print('hi brian')
}

// math
output::print('2 + 2 =')
output::print(2+2);
```

##### installation
```
git clone https://github.com/ebrian/helix
cd helix
make
sudo make install
```

##### usage
```
helix your_file.hl
```

## change log

#### 0.0.1-alpha
- language boilerplate
- functions
- variables
- simple math
- concatenation
- return values, early return, implicit return
- runaway recursion checking
