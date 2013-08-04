#!perl
use strict;
use warnings;
use PLua;

use Test::More;

plan tests => 4;

# First, test basic parse/execute
subtest "Basic parse/execute" => sub {
  plan tests => 7;
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
};

SCOPE: {
  package Foo;
  lua_function foofoo () {
    return 42
  }
  package main;
  is(Foo::foofoo(), 42, "lua_function uses current package");
}

subtest "Argument list corner cases" => sub {
  plan tests => 1;
  SKIP: {skip "Test not written yet" => 1}
};

pass("Alive");

