#!perl
use strict;
use warnings;
use PLua;

use Test::More;

plan tests => 1;

lua_function foo () {
  return
}

pass("Alive");

lua_function bar(  ){return}

pass("Alive");

lua_function baz( buz ){
  return
}

pass("Alive");

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

