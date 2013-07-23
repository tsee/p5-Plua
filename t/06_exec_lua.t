#!perl
use 5.14.0;
use warnings;
use PLua;

print "1..5\n";
print "ok 1 - this should come first\n";
lua { print("ok 2 - lua's print works") };
print "ok 3 - made it so far\n";

my $ok = eval 'lua { print("ok 4 - lua block in eval works") } 1';
print "not " if not $ok;
print "ok 5 - evalling lua code looks ok from perl\n";

