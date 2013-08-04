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

# FIXME just do it in XS...
sub _install_sub {
  my ($name, $sub) = @_;
  my ($pkg) = caller();
  no strict;
  *{"${pkg}::$name"} = $sub;
}

XSLoader::load;

1;
__END__

=head1 NAME

PLua - Perl and Lua Make a Great Couple!

=head1 SYNOPSIS

  use 5.14.0;
  use PLua;
  
  # Seamlessly embed Lua functions into your Perl code as if
  # they were Perl functions!
  lua_function fnord (a, b, c) {
    return a + b * c
  }
  say fnord(12, 2, 3); # prints 18

  # Pass in or return composite data structures: PLua does the
  # marshalling for you!
  lua_function sum (ary) {
    local s = 0
    for i, num in ipairs(ary) do
      s = s + num
    end
    return s
  }
  say sum([5..9]); # prints 35
  
  # Even better, embed Lua code directly in Perl without having
  # to call a function. You can even access Perl lexicals!
  my $foo = 12.3;
  lua {
    local bar = $foo.num
    -- more Lua code...
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

  use PLua qw(RECURSIVE); # get RECURSIVE symbol
  use Data::Dumper;
  print Dumper $luatable->to_hash(RECURSIVE);
  # $VAR1 = {
  #         'inner' => {
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
  $tbl->set_str("foo" => "bar");
  lua {
    print($tbl.table.foo)
    -- Prints "bar"
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
of the implementation of Perl and Lua closures is that getting this
to work requires a better man than him.

Consider the following code:

  {
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
their scope. If you need to do that, send a well-tested patch,
or work around it. Sorry. Nowadays, with Lua functions being first-class
objects in Perl, this shouldn't be too painful to work around in most
cases.

=head1 ACKNOWLEDGMENTS

Code and ideas from a number of other modules has gone into PLua.
Several people have graciously given advice and support. Apologies
if this is not a complete list.

Rafael Garcia-Suarez and Reini Urban have provided patches including
improving the build process.

Tassilo von Parseval wrote L<Inline::Lua> which does much of what
this module does and is more than just a spiritual precursor.

Lukas Mai wrote L<Function::Parameters> which was very helpful in
my learning perl's custom keyword and lexer API. A number of functions
are almost verbatim copies.

Rob Hoelz' advice on Lua as a language and in terms of embedding
has been invaluable and prevented falling into a number of very
spiky traps.

Last but not least, Dave Mitchell took the time to write up such a clear
explanation of how lexicals and closures are implemented in perl that
even I could follow. His ability to convey very intricate technical
concepts is really quite humbling.

=head1 AUTHOR

Steffen Mueller, C<< <smueller at cpan dot org> >>

=head1 COPYRIGHT & LICENSE

Copyright 2013 Steffen Mueller.

This program is free software; you can redistribute it and/or modify it
under the same terms as perl 5.8.0 or at your choice, any later version
of perl.

See http://dev.perl.org/licenses/ for more information.

=cut
