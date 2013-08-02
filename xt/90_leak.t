#!perl
use 5.14.0;
use warnings;
use PLua;
use Test::More;
use Linux::Smaps::Tiny qw(get_smaps_summary);

my $many_iters = 1000000;
my $many_init_iters = 1000;
my $many_limit = 10;

plan tests => 10;

not_too_much_growth_many(
  "Empty Lua block doesn't leak"
  => sub {
    lua { }
  },
);

not_too_much_growth_many(
  "Lua block reading scalars"
  => sub {
    my $x = 1;
    my $y = "str";
    my $z = [1..5];
    my $tbl = PLua::Table->new;
    lua {
      local foo = $x.int
      local bar = $y.str
      local baz = $z.any
      local t = $tbl.table
    }
  },
);

SCOPE: {
  my $x = 1;
  not_too_much_growth_many(
    invert => 1,
    count => 10000,
    "Lua block appending to scalar DOES leak"
    => sub {
      lua {
        local foo = $x.str
        $x = foo .. "1"
      }
    },
  );
}

not_too_much_growth_many(
  "Lua block writing scalars"
  => sub {
    my ($x, $y);
    lua {{
      $x = 123
      $y = "foo"
    }}
  },
);

not_too_much_growth_many(
  "Lua block writing table"
  => sub {
    my ($z, $tbl);
    lua {{
      $z = {1, 2, 3}
      local t = {[0] = "bar"}
      $tbl = t
    }}
    my $h = $tbl->to_hash;
    $h = $tbl->to_array;
    $h = $z->to_hash;
    $h = $z->to_array;
  },
);

not_too_much_growth_many(
  "Lua block creating function"
  => sub {
    lua {{
      local fun = function ()
      end
    }}
  },
);

not_too_much_growth_many(
  "Lua block writing function"
  => sub {
    my ($sub);
    lua {{
      local fun = function ()
      end
      $sub = fun
    }}
  },
);

SCOPE: {
  my $sub;
  lua {{
    local fun = function (a)
      return a + 1
    end
    $sub = fun
  }}
  not_too_much_growth_many(
    "Lua block calling anon function"
    => sub {
      my $x = $sub->(12);
      $sub->(12123);
    },
  );
}

lua_function notleaky (a, ...) {
  return a + 1
}
not_too_much_growth_many(
  "Lua block calling function"
  => sub {
    my $x = notleaky(12, 13, 14);
    notleaky(1..10);
  },
);

pass("Alive");

sub not_too_much_growth_many {
  not_too_much_growth(
    count => $many_iters,
    initial => $many_init_iters,
    limit => $many_limit,
    @_
  );
}

sub not_too_much_growth {
  my $sub = pop @_;
  my $name = pop @_;
  my %params = @_;
  my $count  = $params{count} || 1;
  my $icount = $params{initial} || 0;
  my $limit  = $params{limit} // 1;
  my $invert = $params{invert};

  for (1..$icount) {
    $sub->();
  }
  my $before = get_smaps_summary();
  for (1..$count) {
    $sub->();
  }
  my $after = get_smaps_summary();
  my $diff = abs($after->{Size} - $before->{Size});
  if ($invert) {
    ok(($diff >= abs($limit)), $name);
  }
  else {
    ok(($diff < abs($limit)), $name);
  }
}

