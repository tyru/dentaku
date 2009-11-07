use strict;
use warnings;

use Test::More;
use Test::Output;
use FileHandle;
use POSIX ();



our $EXE = "./dentaku";
our $OPT_F = "stack";

# capture output.
sub run_dentaku {
    my $expr = shift;
    print qx( echo '$expr' | $EXE -f '$OPT_F' );
}

sub rx_int {
    my $int = POSIX::floor(shift);
    $int > 0 ?
        qr/\A \+ ? $int \Z/x
      : qr/\A      $int \Z/x
}
sub rx_float {
    my $float = shift;
    my ($i, $f) = split /\./, "$float";
    $f = 0 unless defined $f;
    $float > 0 ? 
        qr/\A \+ ? $i \. $f \Z/x
      : qr/\A      $i \. $f \Z/x
}

sub calc_int {
    my ($expr, $result) = @_;
    stdout_like { run_dentaku($expr) } rx_int($result),, "$OPT_F: '$expr' is '$result'";
}
sub calc_float {
    my ($expr, $result) = @_;
    stdout_like { run_dentaku($expr) } rx_float($result),, "$OPT_F: '$expr' is '$result'";
}


my @f_opts = qw(stack parser);
my @tests = (
    sub {
        calc_int("1", 1);
    },
    sub {
        calc_int("10", 10);
    },
    sub {
        calc_int("1-1", 0);
    },
    sub {
        calc_int("-1-1", -2);
    },
    sub {
        calc_int("-1+1", 0);
    },
    sub {
        calc_int("1*1", 1);
    },
    sub {
        calc_int("1*0", 0);
    },
    sub {
        calc_int("1+2*(3+4)+5", 20);
    },
    sub {
        calc_int("1+2+(3+4)*5", 38);
    },
    sub {
        calc_int("1*2-(3+4)*5", -33);
    },
    sub {
        calc_int("2*((3+4)*5)", 70);
    },
    sub {
        calc_int("2+((3+4)*5)", 37);
    },
    sub {
        calc_float("7/((3+4*1)*5)", 0.2);
    },
    sub {
        calc_int("7/(3+4)*5", 5);
    },
    sub {
        calc_int("1-7/(3+4)*5", -4);
    },
    sub {
        calc_int("-1-1-1", -3);
    },
    sub {
        calc_int("-1-1-1-1", -4);
    },
    sub {
        calc_int("-1-1+1", -1);
    },
    sub {
        calc_int("-1", -1);
    },
    sub {
        calc_int("+1", 1);
    },
    sub {
        calc_int("2.5*4", 10);
    },
    sub {
        calc_float("2.5*3", 7.5);
    },
    sub {
        calc_int("3*3+4*4", 25);
    },
    sub {
        calc_int("(3*3+4*4)/5", 5);
    },
    sub {
        calc_int("-(1+1)", -2);
    },
    sub {
        calc_int("+(1+1)", 2);
    },
    sub {
        calc_int("+(1+2)+(3+4)", 10);
    },
    sub {
        calc_int("-(1+2)+(3+4)", 4);
    },
);
plan tests => scalar @tests * @f_opts;

for my $f (@f_opts) {
    local $OPT_F = $f;
    $_->() for @tests;
}
