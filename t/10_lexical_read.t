#!perl
use strict;
use warnings;
use PLua;

# Manual test output so we can easily mix Lua and Perl printing of tests

print "1..14\n";

# Test basic lexical access for simple types
my $integer = 42;
my $number = 1.23;
my $string = "fooo";
my $ary = [7, 3, "foo"];
my $hash = {foo => 5, bar => "baz", 7 => 2};

lua {
  function fail (msg)
    print("not ok - " .. msg)
    return 0
  end
  function pass (msg)
    print("ok - " .. msg)
    return 1
  end
  function ok (test, msg)
    if test then
      pass(msg)
    else
      fail(msg)
    end
    return test
  end

  local i = $integer.int
  ok(i == 42, "Lua sees Perl integer")

  local n = $number.num
  ok(((n-1e-6 < 1.23) and (n+1e-6 > 1.23)),
      "Lua sees Perl number")

  local s = $string.str
  ok(s == "fooo", "Lua sees Perl string")

  local a = $ary.any
  ok(type(a) == "table", "Lua sees Perl Array as Table")
  ok(table.getn(a) == 3, "Lua table has right length")
  ok(a[1] == 7, "Lua table elem 1");
  ok(a[2] == 3, "Lua table elem 2");
  ok(a[3] == "foo", "Lua table elem 3");

  local h = $hash.any
  ok(type(h) == "table", "Lua sees Perl Hash as Table")
  ok(h["foo"] == 5, "Lua table elem 1");
  ok(h["bar"] == "baz", "Lua table elem 2");
  ok(h["7"] == 2, "Lua table elem 3");
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

