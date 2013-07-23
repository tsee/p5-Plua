use 5.014;
use strict;
use warnings;
use ExtUtils::MakeMaker;

my $OPTIMIZE;
if ($ENV{DEBUG}) {
  $OPTIMIZE = '-DDEBUG -O0 -g -Wall -Wextra -Wno-unused-function';
}
else {
  $OPTIMIZE = '-DNDEBUG';
}

my $luajit_inst_dir = $ENV{LUAJIT_DIR};
my $libdir = $luajit_inst_dir ? "-L$luajit_inst_dir/lib " : "";
my $incdir = "-I" . ($luajit_inst_dir || '/usr') . "/include/luajit-2.0";

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
  DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
  LIBS              => ["$libdir -lluajit-5.1"],
  INC               => "$incdir -I.",
  OPTIMIZE          => $OPTIMIZE,
  OBJECT            => '$(O_FILES)',
);
