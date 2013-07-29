package PLua;
use 5.14.0;
use strict;

use Carp qw(croak);
use XSLoader;

our $VERSION = '0.01';
use PLua::Table qw(RECURSIVE SHALLOW);
use Exporter 'import';
our @EXPORT_OK = (@PLua::Table::EXPORT_OK);
our %EXPORT_TAGS = (all => \@EXPORT_OK);

SCOPE: {
  my $ident = qr/[a-zA-Z0-9_]+/;
  my $get_methods = qr/int|num|str|table|any/;
  my $set_methods = qr/set\s*\(\s*($ident)\s*\)/;

  # Lookup for Lua function name to call for the given Perl => Lua conversion type
  my %get_meth_to_funcname = (
    'int'   => 'var_to_int',
    'num'   => 'var_to_num',
    'str'   => 'var_to_str',
    'table' => 'var_to_table',
    'any'   => 'var_to_luaval',
  );

  sub _scan_lua_code {
    # Lua code may be modified in-place in $_[0]
    my %lexicals;
    while ($_[0] =~ /(\$$ident)\.(?:$get_methods)/go) {
      $lexicals{$1} = undef;
    }
    while ($_[0] =~ /(\$$ident)\s*=\s*\S/go) {
      $lexicals{$1} = undef;
    }
    # Must return hashref or else boom!
    return \%lexicals;
  }

  sub _munge_lua_code {
    # Lua code WILL be modified in-place in $_[0]
    # Filled lexical lookup hash in $_[1]
    my $lexicals = $_[1] || {};
    $_[0] =~ s/(\$$ident)\s*\.\s*($get_methods)\b/
        not(defined($lexicals->{$1}))
          ? croak("Could not find Perl lexical with name '$1' referenced from Lua block")
          : "perl.$get_meth_to_funcname{$2}(" . $lexicals->{$1} . ")"
      /goe;

    $_[0] =~ s/(\$$ident)\s*=\s*([^\n]+)/
        not(defined($lexicals->{$1}))
          ? croak("Could not find Perl lexical with name '$1' referenced from Lua block")
          : "perl.lua_val_to_sv(" . $lexicals->{$1} . ", ($2))"
      /goe;
  }
}

XSLoader::load;

1;
__END__

=head1 NAME

PLua - Perl and Lua Make a Great Couple!

=head1 SYNOPSIS

  use 5.14.0;
  use PLua;
  
  my $foo = 12.3;
  lua {
    local bar = $foo.num
    ...
    bar = bar + 1
    $foo = bar
  }
  say $foo; # prints 13.3

  # You can convert Lua tables to Perl!
  my $luatable;
  lua {{
    local mytbl = { foo = "bar", inner = {5, 8} }
    $luatable = mytbl
  }}
  
  say $luatable->get("foo"); # prints "bar"
  
  use Data::Dumper;
  print Dumper $luatable->to_hash(1); # 1 indicates recursive table conversion
  # $VAR1 = {
  #         'tbl' => {
  #                    '1' => '5',
  #                    '2' => '8'
  #                  },
  #         'foo' => "bar",
  #       };
  
  # Similarly for arrays (but Lua arrays start at index 1 by default):
  my $array = $luatable->get("inner")->to_array();
  # $array = [undef, 5, 8];
  
  # But often, you want this:
  $array = $luatable->get("inner")->to_array_shifted();
  # $array = [5, 8];

  # You can create Lua tables from Perl explicitly!
  my $tbl = PLua::Table->new;
  $tbl->set_string("foo" => "bar");
  lua {
    print($tbl.table.foo)
  }

=head1 DESCRIPTION

B<This module is considered to be very highly experimental. It is not
considered production-quality code by its authors. If you intend to
disregard this warning and use it in production then please get in
touch to discuss what's blocking safe production use.>

This Perl module aims at providing seamless integration between Perl and Lua
and efficiently so. At any place in your Perl code, you can use C<PLua> to embed
blocks of Lua code and from within, access your Perl lexicals! (But see the
closure gotcha below.)

TODO write more documentation!

On Lua tables, see also: L<PLua::Table>.

=head1 THE CLOSURE GOTCHA

In a nutshell, Lua functions aren't Perl closures. Maybe this
can be addressed one day, but for the time being, the author's understanding
of the implementation of Perl closures isn't strong enough.

Consider the following code:

  if (1) {
    my $bar = 12;
    lua {
      function foo()
        local tmp = $baz.int + 1
        $baz = tmp
        return tmp
      end
    }
  }
  
  lua {
    print foo()
  }

What do you expect that to print? Clue: The C<foo()> definition
is within the scope of the Perl lexical, but the call is not.
C<PLua> performs the lookup of the lexical at compile time, but
it doesn't have the (quite complex) special magic hot sauce that
is Perl's closure behaviour.

So in a nutshell, this code may or may not print "13" as you might
expect. I could also print something else or quite possibly segfault.

So don't use Lua functions to provide access to Perl lexicals outside
their scope. If you need to do that, send a patch, pay me to work on
supporting this (just kidding), or work around it. Sorry.

=head1 AUTHOR

Steffen Mueller, C<< <smueller at cpan dot org> >>

=head1 COPYRIGHT & LICENSE

Copyright 2013 Steffen Mueller.

This program is free software; you can redistribute it and/or modify it
under the same terms as perl 5.8.0 or at your choice, any later version
of perl.

See http://dev.perl.org/licenses/ for more information.

=cut
