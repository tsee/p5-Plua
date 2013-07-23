#!perl
use 5.14.0;
use warnings;
use PLua;

print "1..3\n";
print "ok 1 - this should come first\n";
lua { print("ok 2 - lua's print works") };
print "ok 3 - made it so far\n";
