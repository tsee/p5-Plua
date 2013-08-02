#!perl
use strict;
use warnings;
use PLua;

use Test::More;

plan tests => 8;

lua_function foo () {
  return 2
}

pass("Alive");
is(foo(), 2);

lua_function bar(  ){return}

pass("Alive");

lua_function baz( buz ){
  return buz + 1
}

pass("Alive");
is(baz(12), 13);

lua_function
 foo2(a, b, c) {
  return
}

pass("Alive");

lua_function X1 (a,
                 b, c, ...)
{
  return
}

pass("Alive");

lua_function t(...) {
  return
}

pass("Alive");

