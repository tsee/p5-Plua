package PLua;
use 5.14.0;
use strict;

use Carp qw(croak);
use XSLoader;

our $VERSION = '0.01';
use PLua::Table qw(RECURSIVE SHALLOW);
use Exporter ();
our @ISA = qw(Exporter); # sigh
our @EXPORT_OK = (@PLua::Table::EXPORT_OK);
our %EXPORT_TAGS = (all => \@EXPORT_OK);

sub import {
  # Enable keywords in lexical scope ("Plua:kw" isn't
  # magical, it just needs to match XS)
  $^H{"PLua:kw"} = 1;
  __PACKAGE__->export_to_level(1, @_);
}

sub unimport {
  # and disable keywords!
  delete $^H{"PLua:kw"};
}

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

On Lua tables, see also the documentation of their OO interface at L<PLua::Table>.

=head2 Embedding Lua in Perl

As of this writing, C<PLua> has two main avenues for meshing Lua and Perl.
The first is using the C<lua_function> keyword which gives an
effective way to embed Lua I<functions> into Perl and exposing those functions
to Perl. To wit, the example from the synopsis:

  lua_function sum(ary) {
    -- This is Lua!
    local s = 0
    for i, num in ipairs(ary) do
      s = s + num
    end
    return s
  }
  
  # This is Perl!
  my $sum = sum([1..100]);

To Lua people, this might look a bit odd because Lua doesn't use curly braces
to delimit code blocks. Therefore, B<before C<PLua> is considered
production-ready, the syntax may change to use Lua's C<end> instead.>
The reason why that's not the case right now is that it would require a
full-blown lua parser to find the closing delimiter. Which brings us to
delimiters. The scanning for the end of the Lua code is really rather naive
now, so if your Lua code has any closing curly braces, you need to use more
curlies as delimiter:

  # THIS IS BROKEN
  lua_function make_table_broken() {
    local tbl = {} -- Lua will "end" here
  }

  # This is okay:
  lua_function make_table() {{
    local tbl = {}
  }}
  my $tbl = make_table();

You can use an arbitrary number of opening C<{> here.

The second way of interfacing between the two languages is inline Lua code
blocks:

  lua {
    local foo = 1
    for i = 1, 100, 1 do
      foo = foo + 1
    end
    print(foo)
  }

This will be compiled at Perl compile-time and inserted into the Perl
OP-tree as a custom OP for maximum run-time efficiency.
But it doesn't interact with Perl at all just yet. Thus, you can access
lexical (scalar) Perl variables directly in your Lua code as follows:

  my $n = 100;
  my $foo;
  lua {
    local foo = 1
    for i = 1, $n.int, 1 do
      foo = foo + 1
    end
    $foo = foo
  }
  say "foo is $foo (which should be " . ($n+1) . ")"

See how it's used both for reading and writing? For reading from a Perl
lexical, you use the C<.foo> notation where C<foo> is whatever type you
want to cast the Perl scalar as. More on that below.
On writing back to a Perl variable, C<PLua> will intuit the necessary
conversion for you. B<Warning: Due to the way that this is implemented
at the moment, the expression after C<$foo = > must not span more than
the currentl line. This will hopefully be addressed before a production-ready
release of PLua.>

The biggest danger of using the embedded-Perl-lexicals syntax is that
you B<must not use them in Lua functions>. This means that:

  my $x = 2;
  lua {
    function square(a)
      return a * a
    end
    $x = square($x.int)
  }

is just fine, but this is not:

  my $x = 2;
  lua {
    function square()
      -- Really BAD! Never use Perl lexicals in Lua functions
      $x = $x.int * $x.int
    end
  }

Why? Because those Lua function can be called from outside the scope of
the Perl lexical. See L</"THE CLOSURE GOTCHA"> for details.

=head2 Marshalling Data Between Lua and Perl

The general rule on migrating data between Lua and Perl is
that the language on the receiving end of
the conversion determines how things are converted.

For reading from Perl scalars in Lua blocks, you use the C<$foo.XXX>
syntax, where C<XXX> can be any of:

=over 2

=item I<int>

=item I<num>

=item I<str>

=item I<table>

=item I<any>

=back

C<int>, C<num>, C<str> convert to the respective basic
types (integers, floats, strings). C<table> requires the Perl
variable to hold a C<Plua::Table> object (see below). C<any>
in turn is the wildcard: It will attempt to intuit the type
of data in the Perl variable and convert accordingly.

C<any> supports integers, floats, strings, C<Plua::Table> objects,
array references, and hash references. The latter three types
can be arbitrarily nested and will all be converted to Lua
tables on the Lua side.

C<lua_function> arguments are converted with the C<any> conversion
logic, so all of the above are supported.

Lexical Perl variable assignments in Lua blocks also dynamically
support converting integers, floats, strings, and tables to Perl
data structures. The Lua tables turn into C<PLua::Table> objects.
These can then be (shallowly or recursively) converted to
Perl arrays or hashes, or be inspected manually. This intermediary
is necessary because Lua tables simply don't universally map to
any singe Perl type. They are a bit of a bastard child of a dictionary
and an array.

The same conversion happens to Lua values returned from C<lua_function>s
to Perl.

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

Code and ideas from a number of other modules has gone into C<PLua>.
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

Last but certainly not least, Dave Mitchell took the time to write up such a clear
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
