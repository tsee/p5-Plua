#!perl
use 5.14.0;
use warnings;
use PLua;
use Test::More;

# Almost c/p from synopsis, just to double-check that the
# synopsis isn't lying (which it was prior to this).

  # Seamlessly embed Lua functions into your Perl code as if
  # they were Perl functions!
  lua_function fnord (a, b, c) {
    return a + b * c
  }
  is(fnord(12, 2, 3), 18); # prints 18

  # Pass in or return composite data structures: PLua does the
  # marshalling for you!
  lua_function sum (ary) {
    local s = 0
    for i, num in ipairs(ary) do
      s = s + num
    end
    return s
  }
  is(sum([5..9]), 35); # prints 35

  # Even better, embed Lua code directly in Perl without having
  # to call a function. You can even access Perl lexicals!
  my $foo = 12.3;
  lua {
    local bar = $foo.num
    -- more Lua code...
    bar = bar + 1
    $foo = bar
  }
  is($foo, 13.3); # prints 13.3

  # You can convert Lua tables to Perl!
  my $luatable;
  lua {{
    local mytbl = { foo = "bar", inner = {5, 8} }
    $luatable = mytbl
  }}

  is($luatable->get("foo"), "bar"); # prints "bar"

  use PLua qw(RECURSIVE);
  my $ref = {
    'inner' => {
      '1' => '5',
      '2' => '8'
    },
    'foo' => "bar",
  };
  is_deeply($luatable->to_hash(RECURSIVE), $ref); # 1 indicates recursive table conversion

  # Similarly for arrays (but Lua arrays start at index 1 by default):
  my $array = $luatable->get("inner")->to_array();
  is_deeply($array, [undef, 5, 8]);
  # $array = [undef, 5, 8];
  
  # But often, you want this:
  $array = $luatable->get("inner")->to_array_shifted();
  is_deeply($array, [5,8]);
  # $array = [5, 8];

  # You can create Lua tables from Perl explicitly!
  my $tbl = PLua::Table->new;
  $tbl->set_str("foo" => "bar");
  my $t;
  lua {
    -- print($tbl.table.foo)
    $t = $tbl.table.foo
  }
  is($t, "bar");

done_testing;
