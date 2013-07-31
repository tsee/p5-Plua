#!perl
use strict;
use warnings;
use Test::More;
use PLua;
plan tests => 8;
use Devel::Peek;

my $y = -1;
SCOPE: {
  my $x = 1;
  lua {
    function foo()
      $y = $x.int + 1
    end
  }

  sub increment_x {$x++}
  sub get_x {$x}
}

is(get_x(), 1, "x intialized as 1");
is($y, -1, "y initialized as -1");
lua {
  foo()
}
is(get_x(), 1, "x still one after Lua call");
is($y, 2, "y is x+1=2 after Lua call");
increment_x();
is(get_x(), 2, "x incremented to 2");
is($y, 2, "y still 2 after X increment");
lua {
  foo()
}
is($y, 3, "y is x+1=3 after Lua call");

pass("Alive");

