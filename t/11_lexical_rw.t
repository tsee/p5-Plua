#!perl
use strict;
use warnings;
use Test::More;
use PLua;

plan tests => 9;

# Test basic lexical access for simple types
my $integer = 42;
my $number = 1.23;
my $string = "fooo";
my $undef = 1;
my $tbl;

lua {{
  local i = $integer.int
  $integer = i + 1

  $number = $number.num + 1

  local s = $string.str
  $string = s .. 'bar'

  local table = { key = 5, foo = 12, tbl = {5, 8} }
  $tbl = table

  $undef = nil
}}

is($integer, 43, "Setting Perl SV from Lua integer");
is($undef, undef, "Setting Perl SV from Lua nil");
ok($number+1e-6 > 2.23 && $number-1e-6 < 2.23, "Setting Perl SV from Lua number");
is($string, "fooobar", "Setting Perl SV from Lua string");
isa_ok($tbl, "PLua::Table", "Table-wrapping works");
is($tbl->get("foo"), 12, "Simple table member works");
isa_ok($tbl->get("tbl"), "PLua::Table", "Nested tables");

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

