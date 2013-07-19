#!perl
use strict;
use warnings;
use Test::More;
use PLua;

plan tests => 16;

# Test Plua::Table API

my $tbl;
my $tbl2;

lua {{
  local table = { key = 5, foo = 12, tbl = {5, 8}, nasty = {[0] = 2, [1] = 3} }
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
is_deeply($hash, {key => 5, foo => 13, tbl => {1 => 5, 2 => 8}, nasty => {0 => 2, 1 => 3}}, "Outer hash converted ok");

my $ary = $tbl2->to_array;
is_deeply($ary, [undef, 5, 8], "Unshifted array");
$ary = $tbl2->to_array_shifted;
is_deeply($ary, [5, 8], "Shifted array");

$ary = $tbl->get("nasty")->to_array;
is_deeply($ary, [2, 3], "Unshifted funny array");
$ary = $tbl->get("nasty")->to_array_shifted;
is_deeply($ary, [3], "Unshifted funny array");

undef $tbl; # clean up manually to trigger potential issues before "Alive" below
# Test with parent table gone
is($tbl2->get(1), 5, "Inner table access after parent is gone");
undef $tbl2; # clean up manually to trigger potential issues before "Alive" below


# Test the other direction: Perl's XS Lua table to Lua
SCOPE: {
  my $table = PLua::Table->new;
  my $table2;
  $table->set_int("foo", 42);
  lua {{
    local tbl = $table.table
    local tbl2 = {}
    tbl2.bar = tbl.foo + 1
    $table2 = tbl2
  }}
  is_deeply($table2->to_hash, {bar => 43}, "Table roundtrip");
}

SCOPE: {
  # direct chaining ought to work!
  my $table = PLua::Table->new;
  $table->set_int("foo", 42);
  my $bar;
  lua {
    $bar = $table.table.foo
  }
  is($bar, 42, "Nasty chaining");
}

pass("Alive");

