use strict;
use warnings;
use PLua;
use Data::Dumper;
use Benchmark qw(cmpthese);

# perl -Mblib author_tools/lua_perl_derivative_bench.pl 1000
#        Rate  perl   lua
# perl  389/s    --  -95%
# lua  7385/s 1799%    --

srand($ARGV[1] || 0);
my $n = $ARGV[0] || 1000;
my @x;
my @y;
my $x = 0;
my $y = 10;
my $dy = 0.1;
for (1..$n) {
  my $dx = 0.1 + 0.1*rand();
  push @x, ($x += $dx);
  $dy += -0.01 + 0.02*rand();
  push @y, ($y += $dy*$dx);
}

cmpthese(-1, {
  perl => sub {derivative2(\@x, \@y)},
  lua  => sub {lua_derivative2(\@x, \@y)},
});

# From  Math::Derivative
sub derivative2 {
  my ($x,$y,$yp1,$ypn)=@_;
  my $n=$#{$x};
  my ($i,@y2,@u);
  if (!defined $yp1) {
    $y2[0] = 0;
    $u[0] = 0;
  }
  else {
    $y2[0] = -0.5;
    $u[0] = ( 3/($x->[1]-$x->[0]) )
            * ( ($y->[1]-$y->[0])/($x->[1]-$x->[0])-$yp1 );
  }
  for ($i = 1; $i < $n; $i++) {
    my $sig = ( $x->[$i]-$x->[$i-1] )
              / ( $x->[$i+1]-$x->[$i-1] );
    my $p = $sig*$y2[$i-1]+2.0; 	
    $y2[$i] = ($sig-1.0) / $p;
    $u[$i] = ( 6.0*( ($y->[$i+1]-$y->[$i])/($x->[$i+1]-$x->[$i])-
               ( $y->[$i]-$y->[$i-1])/($x->[$i]-$x->[$i-1]) )
                 / ( $x->[$i+1]-$x->[$i-1] )
               - $sig*$u[$i-1]
             ) / $p;
  }

  my ($qn, $un);
  if (!defined $ypn) {
    $qn = 0;
    $un = 0;
  }
  else {
    $qn = 0.5;
    $un = ( 3.0/($x->[$n]-$x->[$n-1]) )
          * ( $ypn-($y->[$n]-$y->[$n-1])/($x->[$n]-$x->[$n-1]) );
  }
  $y2[$n] = ( $un-$qn*$u[$n-1] )
            / ( $qn*$y2[$n-1]+1.0 );
  for ($i = $n-1; $i >= 0; --$i) {
    $y2[$i] = $y2[$i] * $y2[$i+1] + $u[$i];
  }
  return @y2;
}

BEGIN {
  my $lua_deriv;
  lua {{{
    local derivative2 = function(x, y, yp1, ypn)
      local n = #x
      local y2 = {}
      local u = {}
      if (yp1 == nil) then
        y2[1] = 0
        u[1] = 0
      else
        y2[1] = -0.5
        u[1] = ( 3/(x[2]-x[1]) ) * ( (y[2]-y[1])/(x[2]-x[1])-yp1 )
      end
      for i = 2, n-1, 1 do
        local sig = ( x[i] - x[i-1] ) / ( x[i+1] - x[i-1] )
        local p = sig * y2[i-1] + 2.0
        y2[i] = (sig-1.0) / p
        local tmp = 6.0*( (y[i+1] - y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]) )
        u[i] = ( tmp / ( x[i+1]-x[i-1] ) - sig*u[i-1] ) / p;
      end

      local qn
      local un
      if (yp1 == nil) then
        qn = 0
        un = 0
      else
        qn = 0.5
        un = ( 3.0/(x[n]-x[n-1]) ) * ( ypn-(y[n]-y[n-1])/(x[n]-x[n-1]) )
      end
      y2[n] = ( un-qn*u[n-1] ) / ( qn*y2[n-1]+1.0 )

      for i = n-1, 1, -1 do
        y2[i] = y2[i] * y2[i+1] + u[i]
      end
      return y2
    end
    $lua_deriv = derivative2
  }}}

  *lua_derivative2 = $lua_deriv;
}
