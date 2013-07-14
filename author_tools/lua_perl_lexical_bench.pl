#!perl
use 5.14.0;
use warnings;
use blib;
use PLua;
use Benchmark::Dumb qw(cmpthese);

# Multiple ways of looping & setting/incrementing an integer:
# - Pure Lua (wickedly fast)
# - A loop in Lua accessing a Perl scalar (much slower, but faster then the others)
# - A loop in Perl, crossing into Lua each iteration, accessing the Perl scalar from there
#   (yet slower, but still faster than just Perl)
# - Plain old Perl (slowest)
#
# Quite surprising that crossing over into Lua and looking at Perl lexicals (scalars) from
# there is a bit faster than staying in Perl!
#
# Update: now access to lexicals should be even faster than listed below. Just haven't rerun
#         the benchmark.
#
#                           Rate     perl_only perl_loop_lua_lex lua_perl_lex lua_only
#perl_only            762+-1.5/s            --            -21.6%       -62.1%   -99.5%
#perl_loop_lua_lex  971.8+-2.6/s  27.53+-0.42%                --       -51.7%   -99.3%
#lua_perl_lex        2012.2+-2/s 164.07+-0.58%     107.05+-0.58%           --   -98.6%
#lua_only          147240+-670/s    19222+-96%        15051+-80%    7217+-34%       --

my $lex = 123;
cmpthese(150.1, {
  lua_only => sub {
    lua {{
      for i = 1, 10000, 1 do
        local foo = 123
        foo = foo + 1
      end
    }}
  },
  perl_only => sub {
    foreach (1..10000) {
      my $foo = 123;
      $foo = $foo + 1;
    }
  },
  perl_loop_lua_lex=> sub {
    foreach (1..10000) {
      lua {{
        local foo = $lex.int;
        foo = foo + 1
      }}
    }
  },
  lua_perl_lex => sub {
    lua {{
      for i = 1, 10000, 1 do
        local foo = $lex.int;
        foo = foo + 1
      end
    }}
  },
});

