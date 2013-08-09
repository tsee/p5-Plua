use 5.014;
use strict;
use warnings;
use ExtUtils::MakeMaker;
use Getopt::Long qw(GetOptions);

# --no-jit disables JIT even if available, otherwise when enabled,
# falls through to non-JIT, standard Lua if "Alien::LuaJIT" isn't
# available.
GetOptions(
  'jit!' => \(my $use_jit = 1),
);

my $OPTIMIZE;
my $DEFINE = '';
if ($ENV{DEBUG}) {
  # gccism but all known people who've tried to debug PLua use gcc
  $OPTIMIZE = '-DDEBUG -O0 -g -Wall -Wextra -Wno-unused-function';
}
else {
  $OPTIMIZE = '-DNDEBUG';
}

use Alien::Lua;
my $alien = Alien::Lua->new(luajit => $use_jit);

my $libs = $alien->libs;
my $cflags = $alien->cflags;
if ($alien->luajit) {
  $DEFINE .= " -DPLU_LUAJIT";
  say "Using Alien::LuaJIT-configured luajit";
}
elsif (!$use_jit) {
  say "Explicitly disabled use of luajit";
}
else {
  say "Alien::LuaJIT not available, using standard Alien::Lua";
}

WriteMakefile(
  NAME                => 'PLua',
  AUTHOR              => q{Steffen Mueller <smueller@cpan.org>},
  VERSION_FROM        => 'lib/PLua.pm',
  CONFIGURE_REQUIRES => {
    'ExtUtils::MakeMaker' => '6.56',
    'Alien::Lua' => '5.1.0', # min 5.1.0
  },
  LICENSE => 'perl',
  PL_FILES            => {},
  BUILD_REQUIRES => {
    'Test::More' => 0,
  },
  PREREQ_PM => {
    'XSLoader' => 0,
    'Carp' => 0,
  },
  MIN_PERL_VERSION => '5.14.0',
  META_MERGE => {
    'meta-spec' => { version => 2 },
    resources => {
      repository => {
        url => 'git://github.com/tsee/p5-PLua.git',
        web => 'https://github.com/tsee/p5-PLua',
        type => 'git',
      },
    },
  },
  depend => { Makefile => '$(VERSION_FROM)' },
  dist => { COMPRESS => 'gzip -9f', SUFFIX => 'gz', },
  clean => { FILES => 'PLua-*' },
  DEFINE            => $DEFINE,
  LIBS              => $libs,
  INC               => "-I. $cflags",
  OPTIMIZE          => $OPTIMIZE,
  OBJECT            => '$(O_FILES)',
);
