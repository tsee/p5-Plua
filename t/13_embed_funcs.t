#!perl
use strict;
use warnings;
use PLua;
use lib 'lib', 't/lib';
use PLua::Test;

use Test::More;

plan tests => 4;

# First, test basic parse/execute
subtest "Basic parse/execute" => sub {
  plan tests => 9;
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
   foo4(a, b,c --foo ) {
     ) {
    return
  }

  pass("Alive");


  lua_function
   foo2(a, b, --foo
       c) {
    return
  }

  pass("Alive");

  lua_function foo3(a, b, -- bar!
                    c -- baz
  ) {
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
  plan tests => 6;

  my ($ok, @rv);

  lua_function t1(a) {
    return a+1
  }
  ($ok, @rv) = compile_pass_ok("t1(123)", "Simple function call ok");
  ok(scalar(@rv)==1 && $rv[0] == 124, "Function call result ok");

  ($ok, @rv) = compile_pass_ok("t1(123, 131)", "Simple function call with too many params ignores extra params");
  ok(scalar(@rv)==1 && $rv[0] == 124, "Function call result ok");

  lua_function t2(a) {
    return a
  }
  ($ok, @rv) = compile_pass_ok("t2()", "Simple function call with too few params pads with undef");
  ok(scalar(@rv)==1 && !defined($rv[0]), "Function call result ok");

  #lua_function t3(a, b, ...) {{
  #  return ...
  #}}
  #
  #my @x = t3(1,2,3,4);
  #use Data::Dumper;
  #warn Dumper @x;
};

pass("Alive");

