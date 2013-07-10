#!perl
use strict;
use warnings;
use Test::More tests => 1;
use PZoom;

# Run some simple math in Lua (hint: some 10x faster on my laptop for 1e6 iterations)

lua {{
  -- Table to hold statistical functions
  stats={}

  -- Get the mean value of a table
  function stats.mean( t )
    local sum = 0
    local count= 0

    for k,v in ipairs(t) do
      sum = sum + v
      count = count + 1
    end

    return (sum / count)
  end

  -- Get the standard deviation of a table
  function stats.standardDeviation( t )
    local m
    local vm
    local sum = 0
    local count = 0
    local result

    m = stats.mean( t )

    for k,v in pairs(t) do
      vm = v - m
      sum = sum + (vm * vm)
      count = count + 1
    end

    result = math.sqrt(sum / (count-1))

    return result
  end

}}

sub mean {
  my $sum = 0;
  my $c = 0;
  foreach (@{$_[0]}) {
    $sum += $_;
    ++$c;
  }
  return $sum/$c;
}

sub standard_deviation {
  my $sum = 0;
  my $count = 0;
  my $m = mean($_[0]);

  foreach (@{$_[0]}) {
    $sum += ($_ - $m)**2;
    ++$count;
  }

  return sqrt($sum / ($count-1));
}
  

if (not @ARGV > 1) {
  for (1..($ARGV[0] || 10000)) {
    lua {{
      local foo = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                   21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                   39, 40 }
      local std = stats.standardDeviation(foo)
    }}
  }
}
else {
  for (1..($ARGV[0] || 10000)) {
    my $ary = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                39, 40 ];
    my $std = standard_deviation($ary);
  }
}

pass("Alive");

