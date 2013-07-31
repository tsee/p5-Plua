#!perl
use strict;
use Test::More tests => 3;
use PLua;

# trigger unsupported-lua-type exceptions to test stack-balancing
# in exception handling

my $err;
my $ok = eval {
  my $x;
  lua {
    local f = function ()
    end
    local c = coroutine.create(f)
    $x = c
  }
  1
} or do {
  $err = $@ || 'Zombie error';
};
ok(!$ok);
like($err, qr/cannot be converted/i);


pass("Alive");

