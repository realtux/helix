# helix 0.0.1-alpha
helix is a fun and mostly useless toy language written in c. helix is based on an earlier project of mine called [axis](https://github.com/ebrian/axis) which was written in node.

##### what does helix look like?
```go
// new function
fn say_hello(msg) {
  if msg { // truthy/falsy conditionals
    return msg;
  } else {
    return 'hello!';
  }
}

// set a variable
var name = 'blang';

// do a conditional
if name == 'blang' {
  out say_hello(); // argument optional
}
```

##### installation
```
# you know the drill
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
