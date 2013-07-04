#!perl
use strict;
use warnings;
use Test::More tests => 1;
use PZoom;

my $foo = 1;
zoom <<HERE;
HERE
warn $foo;

pass("Alive");

