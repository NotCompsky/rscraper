#!/usr/bin/env perl

# NOTE: Input must be valid regex, but some regex patterns cause the optimiser to skip the entire string. E.g. '(?:foo|bar)'. See https://bisqwit.iki.fi/source/regexopt.html for more.

use Regexp::Optimizer;
my $regexpr = @ARGV[0];
print Regexp::Optimizer->new->optimize(qr/$regexpr/);
