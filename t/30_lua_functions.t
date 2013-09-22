#!perl
use 5.14.0;
use warnings;
use Test::More;
use PLua;
use Scalar::Util qw(reftype);
plan tests => 28;

sub check_lua_function {
  my ($func, $name) = @_;
  isa_ok($func, 'PLua::Function', "$name: function is PLua::Function");
  is(reftype($func), 'CODE', "$name: function is CODE ref");
}

my $func;
my @rvs;
my $rv;
my $name;


##############
$name = "void fun(void)";
lua {
  local f = function ()
    return
  end
  $func = f
}

check_lua_function($func, $name);
@rvs = $func->();
is(scalar(@rvs), 0, "$name returns nothing");
@rvs = $func->(qw(a b c));
is(scalar(@rvs), 0, "$name returns nothing, even with parameters");

##############
$name = "void fun(a)";
lua {
  local f = function (a)
    return
  end
  $func = f
}

check_lua_function($func, $name);
@rvs = $func->();
is(scalar(@rvs), 0, "$name returns nothing");
@rvs = $func->(qw(a b c));
is(scalar(@rvs), 0, "$name returns nothing, even with parameters");

##############
$name = "void fun(a, b)";
lua {
  local f = function (a, b)
    return
  end
  $func = f
}

check_lua_function($func, $name);
@rvs = $func->();
is(scalar(@rvs), 0, "$name returns nothing");
@rvs = $func->(qw(a b c));
is(scalar(@rvs), 0, "$name returns nothing, even with parameters");

##############
$name = "scalar fun(a, b)";
lua {
  local f = function (a, b)
    return a+b
  end
  $func = f
}

check_lua_function($func, $name);
@rvs = ();
my $status = eval {
  @rvs = $func->(); # Lua error!
  1
};
my $err = $@ || 'Zombie Error';
ok(!$status, "$name Lua-errors");
like($err, qr/nil value/i, "$name error looks sane"); # an undef error
is(scalar(@rvs), 0, "$name returns nothing in error condition");
@rvs = $func->(12, 13);
is(scalar(@rvs), 1, "$name returns one thing, with parameters");
is($rvs[0], 25, "$name returns right thing, with parameters");
$rv = $func->(12, 13);
is($rv, 25, "$name returns right thing (perl scalar context), with parameters");

##############
$name = "array fun(a, b)";
lua {
  local f = function (a, b)
    return a, b, a+b
  end
  $func = f
}

check_lua_function($func, $name);
@rvs = $func->(5..9); # Lua error!
is_deeply(\@rvs, [5,6,11], "$name returns right list in list context");
$rv = $func->(12, 13);
is($rv, 25, "$name returns right thing (perl scalar context), with parameters");

###############
# Returning Lua function from Lua function
lua {
  local f = function(a)
    local f2 = function(b)
      return a+b
    end
    return f2
  end
  $func = f
}
my $f2 = $func->(42);
isa_ok($f2, "PLua::Function");
is($f2->(2), 44);

##############
# Passing Lua function to lua function
SCOPE: {
  my $x;
  lua {
    local f = $f2.any
    $x = f(3)
  }
  is($x, 45);
}

pass("Alive");
