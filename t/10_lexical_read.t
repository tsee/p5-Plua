#!perl
use strict;
use warnings;
use PLua;

# Manual test output so we can easily mix Lua and Perl printing of tests

print "1..4\n";

my $integer = 42;
lua {
  local f = $integer.int
  if f == 42 then
    print("ok - Lua sees Perl integer")
  else
    print("not ok - Lua sees Perl integer")
  end
}

my $number = 1.23;
lua {
  local f = $number.num
  if (f-1e-6 < 1.23) and (f+1e-6 > 1.23) then
    print("ok - Lua sees Perl number")
  else
    print("not ok - Lua sees Perl number")
  end
}

my $string = "fooo";
lua {
  local f = $string.str
  if f == "fooo" then
    print("ok - Lua sees Perl string")
  else
    print("not ok - Lua sees Perl string")
  end
}

print "ok - Alive\n";

