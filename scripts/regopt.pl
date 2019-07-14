#!/usr/bin/env perl
use Regexp::Optimizer;
my $regexpr = @ARGV[0];
print Regexp::Optimizer->new->optimize(qr/$regexpr/);
