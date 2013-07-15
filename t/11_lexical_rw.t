#!perl
use strict;
use warnings;
use Test::More;
use PLua;

plan tests => 5;

# Test basic lexical access for simple types
my $integer = 42;
my $number = 1.23;
my $string = "fooo";

lua {
  local i = $integer.int
  $integer = i + 1

  $number = $number.num + 1

  local s = $string.str
  $string = s .. "bar"
}

is($integer, 43, "Setting Perl SV from Lua integer");
ok($number+1e-6 > 2.23 && $number-1e-6 < 2.23, "Setting Perl SV from Lua number");
is($string, "fooobar", "Setting Perl SV from Lua string");

# Test that accessing invalid lexical throws exception
ok(!eval q{
    lua {
      \$doesntexist = 12
    }
    1
  },
  "Lua binding throws exception on non-existant lexical\n"
);

pass("Alive");

