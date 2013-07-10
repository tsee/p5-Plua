use 5.014;
use strict;
use warnings;
use ExtUtils::MakeMaker;

my $OPTIMIZE;
if ($ENV{DEBUG}) {
  $OPTIMIZE = '-DDEBUG -O0 -Wall -Wextra';
}
else {
  $OPTIMIZE = '-DNDEBUG';
}

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
  LIBS              => ['-lluajit-5.1'], # e.g., '-lm'
  INC               => '-I.',
  #LIBS              => ['-L/home/tsee/perl/luajit-install/lib -lluajit-5.1'], # e.g., '-lm'
  #INC               => '-I/home/tsee/perl/luajit-install/include/luajit-2.0 -I.',
  OPTIMIZE          => $OPTIMIZE,
  OBJECT            => '$(O_FILES)',
);
