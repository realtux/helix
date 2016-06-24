# blang 0.0.1-alpha
blang is a fun and mostly useless toy language written in c. blang is based on an earlier project of mine called [axis](https://github.com/ebrian/axis) which was written in node. it was too slow and wouldn't scale.

##### what does blang look like?
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
git clone https://github.com/ebrian/blang
cd blang
make
sudo make install
```

##### usage
```
blang your_file.bl
```

## change log

#### 0.0.1-alpha
- language boilerplate
