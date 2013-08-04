package  # Hide from PAUSE
  PLua::Test;
use 5.14.0;
use warnings;
use PLua ();
use Test::More;
use Exporter 'import';

our @EXPORT = qw(
  compile_pass_ok
  compile_fail_ok
);
our %EXPORT_TAGS = (all => \@EXPORT);

sub compile_pass_ok {
  my $code = shift;
  my $name = shift;
  my @retval;
  my $ok = eval "\@retval = do{ $code }; 1";
  my $err = $@||'Zombie Error';
  if (defined $name) {
    ok($ok, $name);
  } else {
    ok($ok);
  }
  note("Error was: $err") if !$ok;
  return($ok, \@retval);
}

sub compile_fail_ok {
  my $code = shift;
  my $name = shift;
  my $ok = eval "do{ $code }; 1";
  my $err = $@||'Zombie Error';
  if (defined $name) {
    ok(!$ok, $name);
  } else {
    ok(!$ok);
  }
  note("Error was: $err") if !$ok;
  return !$ok;
}


1;
