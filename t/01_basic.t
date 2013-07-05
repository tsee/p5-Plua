#!perl
use strict;
use warnings;
use Test::More tests => 1;
use PZoom;

my $foo = 1;
lua {
  local bar = 1;
  print("foooooo\n");
  print(bar);
}
warn $foo;

pass("Alive");

