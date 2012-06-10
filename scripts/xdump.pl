#!/usr/bin/perl
#
# xdump example program.
# Copyright (C) 2006-2012 Markus Armbruster
#
# Empire is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Connect to server, xdump everything into tables, then print the
# tables.
#
# A table is an array of records.
# A record is a hash mapping field names to values.
#
# This is just an example!  Error handling is anemic.  Tables and
# records just beg to be objects, but aren't.

use warnings;
use strict;
use Empire;
use Dumpvalue;

# Map table name to meta table reference
my %meta_by_name = ();
# Map table uid to meta table reference
my @meta_by_uid = ();
# Map table name to table reference
my %table_by_name = ();
# Map table uid to table reference
my @table_by_uid = ();

# The meta meta table
my @meta_meta;
# The meta-type symbol table
my @meta_type;

my $empire = new Empire(-host => '127.0.0.1',
			-port => 6664,
			-country => '1',
			-player => '1',
			-user => 'xdump.pl');
my $dumper = new Dumpvalue;

# xdump meta meta
{
    my $mmd = send_cmd('xdump meta meta');
    @meta_meta = ();
    $table_by_name{meta} = $meta_by_name{meta} = \@meta_meta;
    # find field names
    parse_xdump($mmd, undef, \&store_meta_meta_record);
    # parse for real (requires field names)
    parse_xdump($mmd, undef);
}

# xdump table
{
    # xdump meta table
    $meta_by_name{table} = [];
    parse_xdump(send_cmd('xdump meta table'), undef);
    # table's first field is record uid, and its table field is the table uid:
    my $tuid = $meta_by_name{table}->[0]->{table};
    $meta_by_uid[$tuid] = $meta_by_name{table};

    # xdump table
    $table_by_uid[$tuid] = $table_by_name{table} = [];
    parse_xdump(send_cmd('xdump table *'), $tuid);
}

# complete work for xdump meta meta
{
    # find the meta table uid
    my ($meta) = grep { defined $_ && $_->{name} eq 'meta' }
		      @{$table_by_name{table}};
    my $tuid = $meta->{uid};
    # enter it into @*_by_uid
    $meta_by_uid[$tuid] = $meta_by_name{meta};
    $table_by_uid[$tuid] = $table_by_name{meta};
}

# xdump meta-type
{
    # lookup record with name => 'type' in meta meta:
    my ($mmt) = grep {$_->{name} eq 'type'} @meta_meta;
    # its table field is uid of table meta-type:
    my $tuid = $mmt->{table};
    # lookup table meta-type's name:
    my $name = $table_by_name{table}->[$tuid]->{name};

    # xdump meta meta-type
    $meta_by_uid[$tuid] = $meta_by_name{$name} = [];
    parse_xdump(send_cmd("xdump meta $tuid"), $tuid);

    # xdump meta-type
    @meta_type = ();
    $table_by_uid[$tuid] = $table_by_name{$name} = \@meta_type;
    parse_xdump(send_cmd("xdump $tuid *"), $tuid);
}

# xdump rest
for my $tab (@{$table_by_name{table}}) {
    # skip holes:
    next unless exists $tab->{name};
    # skip tables already dumped:
    next if defined $meta_by_name{$tab->{name}};

    # xdump meta $tab
    $meta_by_uid[$tab->{uid}] = $meta_by_name{$tab->{name}} = [];
    parse_xdump(send_cmd("xdump meta $tab->{uid}"), $tab->{uid});

    # xdump $tab
    $table_by_uid[$tab->{uid}] = $table_by_name{$tab->{name}} = [];
    parse_xdump(send_cmd("xdump $tab->{uid} *"), $tab->{uid});
}

$dumper->dumpValue(\@table_by_uid);


sub send_cmd {
    my ($cmd) = @_;
    $empire->empcmd($cmd);
    my ($status, $data) = $empire->empreadline($cmd);
    die unless ($status eq $Empire::C_PROMPT);
    return $data;
}

sub parse_xdump {
    my ($data, $tuid, $store_record) = @_;
    $store_record = \&store_record unless defined $store_record;

    # parse header
    ($_, $data) = split(/\n/, $data, 2);
    /^XDUMP (meta )?(\S+) ([0-9])+$/
	or die;
    my $meta = defined $1;
    my $name = $2;
    my $timestamp = $3;

    # parse body
    my $n = 0;
    while ($data !~ /^\//) {
	($_, $data) = split(/\n/, $data, 2);
	my @fld = split(/ /);
	&$store_record($tuid, $meta, $name, $n, \@fld, $timestamp);
	$n++;
    }

    # parse footer
    $data =~ /^\/([0-9]+)$/
	or die;
    $n == $1
	or die;
}

sub store_record {
    my ($tuid, $meta, $name, $idx, $fld, $timestamp) = @_;
    my ($meta_table, $table);

    # find table to update and its meta table
    if ($meta) {
	$meta_table = \@meta_meta;
	$table = $meta_by_name{$name};
    } else {
	$meta_table = $meta_by_name{$name};
	$table = $table_by_name{$name};
	$idx = $fld->[0]
	    if has_record_uid($tuid);
    }

    # update record
    my $record = $table->[$idx] || {};
    my $fldidx = 0;
    for (my $i = 0; $i <= $#{$meta_table}; $i++) {
	my $n = is_array($meta_table->[$i]);
	my $j;
	do {
	    # update field
	    my $fldname = $meta_table->[$i]->{name};
	    die unless defined $fld->[$fldidx];
	    $record->{$fldname} = eval_fld($fld->[$fldidx++]);
	} while (++$j < $n);
    }
    $table->[$idx] = $record;
}

sub store_meta_meta_record {
    my ($tuid, $meta, $name, $idx, $fld, $timestamp) = @_;
    die unless $name eq 'meta';
    my $val = eval_fld($fld->[0]);
    die if $idx == 0 && $val ne 'name';
    $meta_meta[$idx] = {name => $val, len => 0};
}

sub eval_fld {
    my ($fld) = @_;
    if ($fld =~ /\A[-+.0-9]/) {
	# sloppy, doesn't flag malformed numbers
	return $fld + 0;
    } elsif ($fld =~ /\A\"(.*)\"/) {
	# sloppy, doesn't junk after string
	$fld = $1;
	$fld =~ s/\\([0-7][0-7][0-7])/ chr(oct($1)) /eg;
	return $fld;
    }
    die;
}

sub has_record_uid {
    # a table has record uids if the first field's table is the table's uid
    my ($tuid) = @_;
    die unless defined $tuid;
    die unless defined $meta_by_uid[$tuid];
    die unless defined $meta_by_uid[$tuid]->[0];
    die unless defined $meta_by_uid[$tuid]->[0]->{table};
    return $meta_by_uid[$tuid]->[0]->{table} == $tuid;
}

sub sym_by_value {
    my ($symtab, $value) = @_;
    my ($sym) = grep { $_->{value} == $value } @{$symtab};
    return $sym->{name};
}

sub is_array {
    my ($meta) = @_;
    my $len = $meta->{len};
    # non-zero $len means array, except for format 'c'
    if ($len == 0 || sym_by_value(\@meta_type, $meta->{type}) eq 'c') {
	return 0;
    }
    return $len;
}
