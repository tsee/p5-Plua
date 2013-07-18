#!perl
use strict;
use warnings;
use Test::More;
use PLua;

plan tests => 10;

# Test Plua::Table API

my $tbl;
my $tbl2;

lua {{
  local table = { key = 5, foo = 12, tbl = {5, 8} }
  $tbl = table
}}

isa_ok($tbl, "PLua::Table", "Table-wrapping works");
is($tbl->get("foo"), 12, "Simple table member works");
isa_ok($tbl->get("tbl"), "PLua::Table", "Nested tables");
is($tbl->get("tbl")->get(1), 5, "Inner table access");
is($tbl->get("tbl")->get("1"), undef, "Inner table access using wrong type");
$tbl->set_int("foo", 13);
is($tbl->get("foo"), 13, "Simple table member works after set");
$tbl2 = $tbl->get("tbl");

my $hash = $tbl2->to_hash;
is_deeply($hash, {1 => 5, 2 => 8}, "Inner hash converted ok");

$hash = $tbl->to_hash(1);
is_deeply($hash, {key => 5, foo => 13, tbl => {1 => 5, 2 => 8}}, "Outer hash converted ok");

undef $tbl; # clean up manually to trigger potential issues before "Alive" below
# Test with parent table gone
is($tbl2->get(1), 5, "Inner table access after parent is gone");
undef $tbl2; # clean up manually to trigger potential issues before "Alive" below

pass("Alive");

