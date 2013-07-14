#!perl
use strict;
use warnings;
use PLua;

# Manual test output so we can easily mix Lua and Perl printing of tests

print "1..5\n";

# Test basic lexical access for simple types
my $integer = 42;
my $number = 1.23;
my $string = "fooo";

lua {
  local i = $integer.int
  if i == 42 then
    print("ok - Lua sees Perl integer")
  else
    print("not ok - Lua sees Perl integer")
  end

  local n = $number.num
  if (n-1e-6 < 1.23) and (n+1e-6 > 1.23) then
    print("ok - Lua sees Perl number")
  else
    print("not ok - Lua sees Perl number")
  end

  local s = $string.str
  if s == "fooo" then
    print("ok - Lua sees Perl string")
  else
    print("not ok - Lua sees Perl string")
  end
}

# Test that accessing invalid lexical throws exception
print "not " if eval "
    lua {
      local foo = \$doesntexist.int
    }
    1
  ";

print "ok - Lua binding throws exception on non-existant lexical\n";

print "ok - Alive\n";

