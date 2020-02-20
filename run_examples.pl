#!/usr/bin/env perl
# perl -MCPAN -e "install Capture::Tiny"
use strict;
use warnings;
use Capture::Tiny 'capture_merged';
use Math::BigFloat ':constant';
use Getopt::Long 'GetOptions';

# Set bash path.
# Must use bash keyword time for millisecond precision, as
# GNU and zsh time return centiseconds.
use constant BASH => 'bash';

# Set Python3 path.
use constant PYTHON_3 => 'python3';

# Set desired debugging tool.
use constant DEBUGGER => 'lldb';

# Regex for floating numbers.
use constant RE_FLOATING => '[+-]?([0-9]*[.])?[0-9]+';

sub get_time;
sub run_debug;
sub run_time;
sub run_tests;

## INITIALIZATION ##
my $executable;
my $experiment_to_run;

# Options
my $debug     = '';
my $verbose   = '';
my $yes       = '';
my $visualize = '';
my $start     = 1;
my $end       = 50;
my $prefix;
my $visualizer;
my $test;
my $experiment;
my $timeout;

GetOptions(
    'debug'          => \$debug,
    'verbose|v'      => \$verbose,
    'yes'            => \$yes,
    'visualize|see'  => \$visualize,
    'test=s'         => \$test,
    'start=i'        => \$start,
    'prefix=s'       => \$prefix,
    'visualizer=s'   => \$visualizer,
    'end=i'          => \$end,
    'experiment|e=i' => \$experiment,
    'timeout|t=s'    => \$timeout
);

$prefix //= 'test' if defined $test;
$prefix //= 'exp3';

$visualizer //= '../code/visualize.py' if defined $test;
$visualizer //= '../visualize.py';

$executable = shift or die "No executable given!\n";

## INPUT CHECKING ##
die "No experiment given to visualize. Use -e.\n"
    if $visualize and not defined $experiment;

die "No experiment given to debug. Use -e.\n"
    if $debug and not defined $experiment;

die "Start >= end.\n" if $start >= $end;

die "Invalid argument to -test: $test\n"
    unless not defined $test
    or $test =~ m/cbs|pp/i;

die "Invalid argument to -timeout: $timeout. "
    . "Format is same as timeout command, sans hours and days. \n"
    unless not defined $timeout
    or $timeout =~ m/^(?!$)([0-9]+m)?([0-9]+s)?$/;

## SCRIPT BEGIN ##
print "Running $executable on"
    . ( defined $experiment
    ? " experiment $experiment"
    : " experiments $start-$end" )
    . ( defined $timeout 
    ? " with timeout $timeout" 
    : '' ) 
    . "\n";

$experiment-- if defined $experiment;

my @experiments = map {"${prefix}_$_"} ( $start .. $end );

$experiment_to_run = $experiments[$experiment] if defined $experiment;

if ( !$yes ) {
    print 'Delete path files and run [Y/n]? ';
    <STDIN> =~ m/y$|yes$|^\Z/i or exit(1);
}
unlink glob '*.paths.txt';

if ( $debug && $experiment != -1 ) {
    run_debug $experiment_to_run;
    exit(0);
}

if ( defined $experiment ) {
    run_time $experiment_to_run;
    system
        "${\PYTHON_3} $visualizer $experiment_to_run.txt $experiment_to_run.paths.txt"
        if $visualize;
    exit(0);
}

my @cpu_times;
my @costs;
my @num_paths;

foreach (@experiments) {
    print "Experiment $_\n" if $verbose;

    my $out;
    if ($debug) {
        ($out) = capture_merged \&run_debug;
    }
    else {
        ($out) = capture_merged \&run_time;
    }

    print "$out\n" if $verbose;

    my $this_num_paths = () = $out =~ /a[0-9]+:.*-->.*/mg if $test;

    my $sys_time  = get_time 'sys',  $out;
    my $user_time = get_time 'user', $out;

    die "Could not find time on experiment $_. Output was:\n\n$out\n"
        unless defined $sys_time and defined $user_time;

    my $cpu_time = $sys_time + $user_time;

    my ($sum_of_cost) = $out =~ m/Sum of cost: ([+-]?[0-9]+)/mg;
    $sum_of_cost //= -1;

    push @cpu_times, $cpu_time;
    push @costs,     $sum_of_cost;
    push @num_paths, $this_num_paths if $test;
}

print "Times:\n" . ( join ",", @cpu_times ) . "\n";
print "Costs:\n" . ( join ",", @costs ) . "\n";

run_tests if $test;

## FUNCTIONS BEGIN ##
sub get_time {
    my $type = shift;
    my $in   = shift;

    my $this_time;
    my ( $min, $sec ) = $in =~ m/$type\W*([0-9]+)m(${\RE_FLOATING})s/mg;

    if ( defined $min && defined $sec ) {
        $this_time = 1000.0 * ( ( 60.0 * $min ) + $sec );
    }
    return $this_time;
}

sub run_debug {
    my $ex = $_ // shift;
    return system "${\DEBUGGER} ./$executable $ex.txt $ex.paths.txt";
}

sub run_time {
    my $ex  = $_ // shift;
    my $cmd = "./$executable $ex.txt $ex.paths.txt";

    if ( defined $timeout ) {
        return system "${\BASH} -c 'time timeout $timeout $cmd'";
    }
    return system "${\BASH} -c 'time $cmd'";
}

sub run_tests {
    print "Testing "
        . ( $test =~ m/cbs/i ? 'CBS' : 'Prioritized Planning' ) . "\n";

    open my $csv, '<', 'sum-of-costs.csv'
        or die "Could not find sum-of-costs.csv\n";

    local $/ = "\n";
    my @correct_sums = <$csv>;

    close $csv;

    @correct_sums = @correct_sums[ 2 .. $#correct_sums ];

    @correct_sums = map { [ split ',', $_ ] } @correct_sums;

    my $num_incorrect = 0;
    foreach my $exp ( $start .. $end ) {
        my $suggested_sum
            = $costs[ $exp - 1 ] == -1
            ? -1
            : $costs[ $exp - 1 ] - $num_paths[ $exp - 1 ];
        my $correct_sum
            = $test =~ m/cbs/i
            ? $correct_sums[ $exp - 1 ][1]
            : $correct_sums[ $exp - 1 ][2];

        if ( $suggested_sum != $correct_sum ) {
            print "Incorrect on experiment $exp:\n"
                . "\tCorrect: $correct_sum\tResult: $suggested_sum\n";

            ++$num_incorrect;
        }
    }

    print "Incorrect on $num_incorrect experiments.\n";

    return;
}

__END__

=head1 NAME
run_examples.pl - run exp3 examples

=head1 SYNOPSIS
run_examples.pl [options] [executable]

    Options:
        - debug         Run an example using the given debugger. Requires experiment number.
        - yes           Assume yes to all user queries. Deletes path files.
        - verbose       Print outputs for each exeriment.
        - visualize     Visualize output. Requires experiment number.
        - experiment x  The number of the experiment to run (1-50).
        - timeout s     Terminate execution after time. Ex: -t 5s
        - test cbs|pp   Run on testing cases instead. Compare to given path lengths.

=head1 AUTHOR
Jeremy Nuttall

=cut
