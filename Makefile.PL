use 5.014;
use strict;
use warnings;
use ExtUtils::MakeMaker;

my $OPTIMIZE;
my $DEFINE = '';
if ($ENV{DEBUG}) {
  $OPTIMIZE = '-DDEBUG -O0 -g -Wall -Wextra -Wno-unused-function';
}
else {
  $OPTIMIZE = '-DNDEBUG';
}

# If the environment variable LUAJIT_DIR is defined, build with luajit.
# Else, build with regular lua, to be found in LUA_DIR, ~/lua, or /usr.

my $lua_inst_dir = $ENV{LUAJIT_DIR} || $ENV{LUA_DIR} || $ENV{HOME}."/lua";
if ($ENV{LUAJIT_DIR}) {
    print "Building with luajit\n";
    $DEFINE .= ' -DPLU_LUAJIT';
}
if (-d $lua_inst_dir) {
    print "lua found in $lua_inst_dir\n";
}
else {
    $lua_inst_dir = '';
}
my $libdir = $lua_inst_dir ? "-L$lua_inst_dir/lib " : "";
my $incdir = "-I" . ($lua_inst_dir // '/usr') . "/include" . ($ENV{LUAJIT_DIR} ? "/luajit-2.0" : "");

WriteMakefile(
  NAME                => 'PLua',
  AUTHOR              => q{Steffen Mueller <smueller@cpan.org>},
  VERSION_FROM        => 'lib/PLua.pm',
  CONFIGURE_REQUIRES => {
    'ExtUtils::MakeMaker' => '6.56',
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
  LIBS              => $ENV{LUAJIT_DIR} ? ["$libdir -lluajit-5.1"] : ["$libdir -llua"],
  INC               => "$incdir -I.",
  OPTIMIZE          => $OPTIMIZE,
  OBJECT            => '$(O_FILES)',
);
