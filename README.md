# toy-virtual-machine
[文章]("https://justinmeiners.github.io/lc3-vm/")的CPP实现
## some fix
* 判断是否是ansi terminal终端
~~~
"?[1;31mele ?[32mct ?[33mroni ?[35mX ?[36mtar ?[m".Replace('?', [char]27);
~~~
