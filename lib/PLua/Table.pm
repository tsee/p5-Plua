package PLua::Table;
use 5.14.0;
use strict;

require PLua;

use Exporter 'import';
use constant SHALLOW => 0;
use constant RECURSIVE => 1;
our @EXPORT_OK = qw(RECURSIVE SHALLOW);
our %EXPORT_TAGS = (all => \@EXPORT_OK);

1;
__END__

=head1 NAME

PLua::Table - Access Lua tables from Perl

=head1 SYNOPSIS

  use 5.14.0;
  use PLua;
  use PLua::Table qw(SHALLOW RECURSIVE);
  
  # You can convert Lua tables to Perl!
  my $luatable;
  lua {{
    local mytbl = { foo = "bar", inner = {5, 8} }
    $luatable = mytbl
  }}
  
  say $luatable->get("foo"); # prints "bar"
  
  use Data::Dumper;
  print Dumper $luatable->to_hash(RECURSIVE);
  # $VAR1 = {
  #         'tbl' => {
  #                    '1' => '5',
  #                    '2' => '8'
  #                  },
  #         'foo' => "bar",
  #       };
  
  # Similarly for arrays (but Lua arrays start at index 1 by default):
  my $array = $luatable->get("inner")->to_array(SHALLOW);
  # Note: SHALLOW conversion is also the default behaviour
  # $array = [undef, 5, 8];
  
  # But often, you want this:
  $array = $luatable->get("inner")->to_array_shifted(SHALLOW);
  # $array = [5, 8];

  # You can create Lua tables from Perl explicitly!
  my $tbl = PLua::Table->new;
  $tbl->set_string("foo" => "bar");
  lua {
    print($tbl.table.foo)
  }

=head1 DESCRIPTION

This class represents a Lua table in Perl. Please make sure you understand
how Lua tables work within Lua before you attempt to make sense of
this document or use the class in your code. Lua tables are curious creatures.

Instances of this class are automatically created when using PLua blocks in
your Perl code and assign a Lua variable to a Perl scalar.
Please note that due to the way that Lua tables and the Lua API work,
Lua tables are tied to the Lua interpreter they were created within.
At this point, that has little to no bearing on use within PLua, but
as more lower-level C<lua_State> APIs are exposed to Perl, this may
change.

=head1 METHODS

=head2 new

Constructor, returns a new C<PLua::Table> object.

=head2 get

Given a key (which can be numeric or a string, which is NOT the same thing
in Lua, so beware), fetches, converts and returns the value from the table.
Subtables are returned as C<PLua::Table> objects.

  my $foo = $tbl->get("key");

=head2 set_int

Reverse of get, but for an integer value (other values
will be cast to integers):

  $tbl->set_int("key", 2);

=head2 set_num

Reverse of get, but for a floating point value (which may also be
an integer or other things that can be cast):

  $tbl->set_num("key", 2.3);

=head2 set_str

Reverse of get, but for a string value:

  $tbl->set_str("key", 2.3);

You guessed it. Non-string things will be stringified.

=head2 set_table

The provided value needs to be a C<PLua::Table> object.

=head2 to_hash

Convert full table to a Perl hash. Keep in mind that Lua
tables can have things other than strings as keys.
This means that some Lua tables can't be converted
into sane Perl hashes (the conversion croaks).
Other things that can be stringified could conceivably
produce collisions in strings (C<1> vs. C<"1">).

If the first argument is true, values that are
tables themselves are converted recursively.
C<PLua> and C<PLua::Table> optionally export
named constants C<RECURSIVE> and C<SHALLOW>
for making this nicer on the eyes.

=head2 to_array

Convert the numeric keys in a table (others are cast)
and their values to a Perl array.

A table C<{5,6,7}> results in C<[undef,5,6,7]> since
Lua tables ("arrays") are 1-based.

Supports recursive conversion like C<to_hash> based
on the first argument to C<to_array>.

=head2 to_array_shifted

Same as C<to_array>, but shifts off the first
array element, which is usually undefined.

=head2 keys

Convert all keys to Perl values and returns them as
an array reference.

=head2 values

Convert all values to Perl values and returns them as
an array reference.

=head2 objlen

Returns the length of the array part of a Lua table.
Make sure to read up on the meaning of this in the Lua
documentation!

=head1 AUTHOR

Steffen Mueller, C<< <smueller at cpan dot org> >>

=head1 COPYRIGHT & LICENSE

Copyright 2013 Steffen Mueller.

This program is free software; you can redistribute it and/or modify it
under the same terms as perl 5.8.0 or at your choice, any later version
of perl.

See http://dev.perl.org/licenses/ for more information.

=cut
