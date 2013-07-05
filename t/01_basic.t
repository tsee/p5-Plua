#!perl
use strict;
use warnings;
use Test::More tests => 1;
use PZoom;

my $foo = 1;
lua {
  asd
}
warn $foo;

pass("Alive");

