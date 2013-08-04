#!perl
use 5.14.0;
use warnings;
use Test::More;
use PLua;
use lib 'lib', 't/lib';
use PLua::Test;

plan tests => 10;

compile_pass_ok("lua{}", "empty Lua block");
compile_pass_ok("lua{ }", "almost empty Lua block");
compile_pass_ok("lua {}", "empty Lua block with space prefix");
compile_pass_ok("lua {\n}", "almost empty Lua block with newline");
compile_pass_ok("lua\n{\n}", "almost empty Lua block with newline prefix");
compile_pass_ok("lua{\nlocal foo = 1}", "Lua block with actual code, beginning newline");
compile_pass_ok("lua{local foo = 1\n}", "Lua block with actual code, ending newline");
compile_pass_ok("lua{local foo = 1}", "Lua block with actual code, no newlines");
compile_pass_ok("lua { local foo = 1 } ", "Lua block with actual code, no newlines but spaces");

pass("Alive");
