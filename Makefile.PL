use 5.014;
use strict;
use warnings;
use ExtUtils::MakeMaker;

WriteMakefile(
  NAME                => 'PZoom',
  AUTHOR              => q{Steffen Mueller <smueller@cpan.org>},
  VERSION_FROM        => 'lib/PZoom.pm',
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
        url => 'git://github.com/tsee/p5-PZoom.git',
        web => 'https://github.com/tsee/p5-PZoom',
        type => 'git',
      },
    },
  },
  depend => { Makefile => '$(VERSION_FROM)' },
  dist => { COMPRESS => 'gzip -9f', SUFFIX => 'gz', },
  clean => { FILES => 'PZoom-*' },
  #LIBS              => ['-Ltcc-install/lib', '-ltcc'], # e.g., '-lm'
  DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
  INC               => '-I. -Isrc',
  #OPTIMIZE          => $OPTIMIZE,
  OBJECT            => '$(O_FILES)',
);
