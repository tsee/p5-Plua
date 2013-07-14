package PLua;
use 5.14.0;
use strict;

use Carp qw(croak);
use XSLoader;

our $VERSION = '0.01';

SCOPE: {
  my $ident = qr/[a-zA-Z0-9_]+/;
  my $methods = qr/int|num/; # TODO implement str conversion (and complex types)
  #my $methods = qr/int|num|str/;

  sub _scan_lua_code {
    # Lua code may be modified in-place in $_[0]
    my %lexicals;
    while ($_[0] =~ /(\$$ident)\.(?:$methods)\b/go) {
      $lexicals{$1} = undef;
    }
    # Must return hashref or else boom!
    return \%lexicals;
  }

  sub _munge_lua_code {
    # Lua code WILL be modified in-place in $_[0]
    # Filled lexical lookup hash in $_[1]
    my $lexicals = $_[1] || {};
    $_[0] =~ s/(\$$ident)\.($methods)\b/
        not(defined($lexicals->{$1}))
          ? croak("Could not find Perl lexical with name '$1' referenced from Lua block")
          : "perl.var_to_$2(" . $lexicals->{$1} . ")"
      /goe;
  }
}

XSLoader::load;

1;
__END__

=head1 NAME

PLua - blah

=head1 SYNOPSIS

  use PLua;

=head1 DESCRIPTION

=head1 AUTHOR

Steffen Mueller, C<< <smueller at cpan dot org> >>

=head1 COPYRIGHT & LICENSE

Copyright 2013 Steffen Mueller.

This program is free software; you can redistribute it and/or modify it
under the same terms as perl 5.8.0 or at your choice, any later version
of perl.

See http://dev.perl.org/licenses/ for more information.

=cut
