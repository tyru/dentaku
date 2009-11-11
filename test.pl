use strict;
use warnings;

use Test::More;
use Test::Output;
use FileHandle;
use POSIX ();



our $PROG = join ' ', map { qq('$_') } @ARGV;

# capture output.
sub run_dentaku {
    my $expr = shift;
    print qx( echo '$expr' | $PROG );
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
    stdout_like { run_dentaku($expr) } rx_int($result),, "'$expr' is '$result'";
}
sub calc_float {
    my ($expr, $result) = @_;
    stdout_like { run_dentaku($expr) } rx_float($result),, "'$expr' is '$result'";
}


my @tests = (
    sub {
        calc_int("1", 1);
    },
    sub {
        calc_int("10", 10);
    },
    sub {
        calc_int("1-1", 1-1);
    },
    sub {
        calc_int("-1-1", -1-1);
    },
    sub {
        calc_int("-1+1", -1+1);
    },
    sub {
        calc_int("1*1", 1*1);
    },
    sub {
        calc_int("1*0", 1*0);
    },
    sub {
        calc_int("1+2*(3+4)+5", 1+2*(3+4)+5);
    },
    sub {
        calc_int("1+2+(3+4)*5", 1+2+(3+4)*5);
    },
    sub {
        calc_int("1*2-(3+4)*5", 1*2-(3+4)*5);
    },
    sub {
        calc_int("2*((3+4)*5)", 2*((3+4)*5));
    },
    sub {
        calc_int("2+((3+4)*5)", 2+((3+4)*5));
    },
    sub {
        calc_float("7/((3+4*1)*5)", 7/((3+4*1)*5));
    },
    sub {
        calc_int("7/(3+4)*5", 7/(3+4)*5);
    },
    sub {
        calc_int("1-7/(3+4)*5", 1-7/(3+4)*5);
    },
    sub {
        calc_int("-1-1-1", -1-1-1);
    },
    sub {
        calc_int("-1-1-1-1", -1-1-1-1);
    },
    sub {
        calc_int("-1-1+1", -1-1+1);
    },
    sub {
        calc_int("-1", -1);
    },
    sub {
        calc_int("+1", 1);
    },
    sub {
        calc_int("2.5*4", 2.5*4);
    },
    sub {
        calc_float("2.5*3", 2.5*3);
    },
    sub {
        calc_int("3*3+4*4", 3*3+4*4);
    },
    sub {
        calc_int("(3*3+4*4)/5", (3*3+4*4)/5);
    },
    sub {
        calc_int("-(1+1)", -(1+1));
    },
    sub {
        calc_int("+(1+1)", +(1+1));
    },
    sub {
        calc_int("+(1+2)+(3+4)", +(1+2)+(3+4));
    },
    sub {
        calc_int("-(1+2)+(3+4)", -(1+2)+(3+4));
    },
    sub {
        calc_int("3^3", 3**3);
    },
);
plan tests => scalar @tests;
$_->() for @tests;
