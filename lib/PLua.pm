package PLua;
use 5.14.0;
use strict;

use Carp qw(croak);
use XSLoader;

our $VERSION = '0.01';

SCOPE: {
  my $ident = qr/[a-zA-Z0-9_]+/;
  my $get_methods = qr/int|num|str/;
  my $set_methods = qr/set\s*\(\s*($ident)\s*\)/;

  sub _scan_lua_code {
    # Lua code may be modified in-place in $_[0]
    my %lexicals;
    while ($_[0] =~ /(\$$ident)\.(?:$get_methods|$set_methods/go) {
      $lexicals{$1} = undef;
    }
    # Must return hashref or else boom!
    return \%lexicals;
  }

  sub _munge_lua_code {
    # Lua code WILL be modified in-place in $_[0]
    # Filled lexical lookup hash in $_[1]
    my $lexicals = $_[1] || {};
    $_[0] =~ s/(\$$ident)\.($get_methods)\b/
        not(defined($lexicals->{$1}))
          ? croak("Could not find Perl lexical with name '$1' referenced from Lua block")
          : "perl.var_to_$2(" . $lexicals->{$1} . ")"
      /goe;

    $_[0] =~ s/(\$$ident)\.$set_methods\b/
        not(defined($lexicals->{$1}))
          ? croak("Could not find Perl lexical with name '$1' referenced from Lua block")
          : "perl.lua_val_to_sv(" . $lexicals->{$1} . ", $2)"
      /goe;
  }
}

XSLoader::load;

1;
__END__

=head1 NAME

PLua - Perl and Lua Make a Great Couple!

=head1 SYNOPSIS

  use PLua;

  my $foo = 12.3;
  lua {
    local bar = $foo.num
    ...
    $foo.set(bar)
  }
  
  etc.

=head1 DESCRIPTION

This Perl module aims at providing seamless integration between Perl and Lua.
At any place in your Perl code, you can now embed blocks of Lua code and from
within, access your Perl lexicals!

TODO write more documentation!

=head1 AUTHOR

Steffen Mueller, C<< <smueller at cpan dot org> >>

=head1 COPYRIGHT & LICENSE

Copyright 2013 Steffen Mueller.

This program is free software; you can redistribute it and/or modify it
under the same terms as perl 5.8.0 or at your choice, any later version
of perl.

See http://dev.perl.org/licenses/ for more information.

=cut
