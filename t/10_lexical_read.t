#!perl
use strict;
use warnings;
use PLua;

# Manual test output so we can easily mix Lua and Perl printing of tests

print "1..2\n";

my $foo = 42;
lua {{
  local f = $foo.int
  if f == 42 then
    print("ok - Lua sees Perl integer");
  else
    print("not ok - Lua sees Perl integer");
  end
}}

print "ok - Alive\n";

