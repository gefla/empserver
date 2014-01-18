#!/usr/bin/perl

# Assumed initial state:
# nats POGO, 1..5, $MAXNOC-1
# sects
#     x>0,y>0   own 1
#     x<-1,y>0  own 2
#     x>0,y<0   own 3
#     x<-1,y<0  own 0
#     all wilderness, rest sea
#     owned sectors have 1m 1c
# units cs #0..4, f1 #0..4, sup #0..4, 10kt #0..4, all in 1,-1 owned by 3

use warnings;
use strict;

my $MAXNOC = 99;
my $xmax = 31;
my $ymax = 15;
my $STAT_GOD = 5;
my $INT_MAX = 0x7fffffff;
my $INT_MIN = -$INT_MAX - 1;

my %ef2edit = (
    sect => 'l',
    ship => 's',
    plane => 'p',
    land => 'u',
    nuke => 'n',
    nat => 'c',
);

my %edit2setres = (
    i => 'i',
    g => 'g',
    c => 'o',
    f => 'f',
    u => 'u',
);

my %edit2setsect = (
    o => 'ow',
    O => 'ol',
    i => 'i',
    g => 'g',
    c => 'oi',
    e => 'e',
    M => 'mi',
    m => 'mo',
    a => 'a',
    w => 'w',
    f => 'f',
    u => 'u',
);

sub edit {
    my ($ef, $id, $key, @rest) = @_;
    my $args = join(' ', @rest);
    print "edit $ef2edit{$ef} $id $key $args\n";
}

sub iedit {
    my ($ef, $id, @rest) = @_;
    print "edit $ef2edit{$ef} $id\n";
    for my $inp (@rest) {
	print "$inp\n";
    }
    print "\n";
}

sub next_id {
    my ($ef, $id) = @_;
    if ($ef ne 'sect') {
        return $id + 1;
    }
    my ($x, $y) = split /,/, $id;
    $x += 2;
    return "$x,$y";
}

sub edit_int1 {
    my ($ef, $id, $key, $lob, $upb) = @_;
    edit($ef, $id, $key, $lob);
    $id = next_id($ef, $id);
    edit($ef, $id, $key, $lob - 1)
	if ($lob > $INT_MIN);
    $id = next_id($ef, $id);
    edit($ef, $id, $key, $upb);
    $id = next_id($ef, $id);
    edit($ef, $id, $key, $upb + 1)
	if ($upb < $INT_MAX);
}

sub edit_int {
    my ($ef, $id, @rest) = @_;
    for my $it (@rest) {
	edit_int1($ef, $id, @$it);
    }
}

sub setres {
    my ($id, $key, $val) = @_;
    print "setres $key $id $val\n";
}

sub setsect {
    my ($id, $key, $val) = @_;
    print "setsect $key $id $val\n";
}

sub give {
    my ($id, $key, $val) = @_;
    print "give $key $id $val\n";
}

sub swaps {
    my ($id1, $id2) = @_;
    print "swaps $id1 $id2\ny\n";
}

## Sector

# invalid key
edit('sect', '0,0', '@', 0);
setres('0,0', '@', 0);
setsect('0,0', '@', 0);
give('0,0', '@', 0);

# own oldown che_target
for my $key ('o', 'O', 'X') {
    edit('sect', '1,7', $key, 0);
    edit('sect', '1,7', $key, -1);
    edit('sect', '3,7', $key, $MAXNOC - 1);
    edit('sect', '3,7', $key, $MAXNOC);
}
edit('sect', '5,7', 'o', 2);
for my $key ('ow', 'ol') {
    setsect('7,7', $key, 0);
    setsect('7,7', $key, -1);
    setsect('9,7', $key, $MAXNOC - 1);
    setsect('9,7', $key, $MAXNOC);
}
setsect('11,7', 'ow', 2);

# x,y (copy)
edit('sect', '1,7', 'L', '5,-7');
edit('sect', '3,-7', 'L', '3,-7', 'L', '1,0');

# effic mobil iron gmin fertil oil uran work loyalty che pstage ptime
# fallout avail mines road rail defense
sub sect_int {
    for my $it (@_) {
	my ($key, $lob, $upb) = @$it;
	edit_int1('sect', '1,1', $key, $lob, $upb);
	my $rkey = $edit2setres{$key};
	if (defined $rkey) {
	    setres('2,2', $rkey, $lob);
	    setres('4,2', $rkey, $lob - 1);
	    setres('6,2', $rkey, $upb);
	    setres('8,2', $rkey, $upb + 1);
	}
	my $skey = $edit2setsect{$key};
	if (defined $skey) {
	    setsect('1,3', $skey, $INT_MIN);
	    setsect('3:7,3', $skey, 1);
	    setsect('5,3', $skey, $INT_MAX);
	    setsect('7,3', $skey, -1);
	}
    }
}
sect_int(
    ['e', 0, 100],
    ['m', -127, 127],
    ['i', 0, 100],
    ['g', 0, 100],
    ['f', 0, 100],
    ['c', 0, 100],
    ['u', 0, 100],
    ['w', 0, 100],
    ['l', 0, 127],
    ['x', 0, 255],
    ['p', 0, 4],
    ['t', 0, 32767],
    ['F', 0, 9999],
    ['a', 0, 9999],
    ['M', 0, 32767],
    ['R', 0, 100],
    ['r', 0, 100],
    ['d', 0, 100],
);

# special case: unowned sector
edit('sect', '-1,-1', 'i', 50);
setres('-1,-1', 'g', 50);
setsect('-1,-1', 'f', 50);

# special case: mines in occupied sector
setsect('1:3,-3', 'ol', 1);
setsect('1,-3', 'mi', 1);
edit('sect', '3,-3', 'M', 1);

# dist
edit('sect', '2,4', 'D', '4,4');
edit('sect', '4,4', 'D', '4,4');

# des newdes
for my $key ('s', 'S') {
    edit('sect', '6:8,4', $key, '+');
    edit('sect', '6,4', $key, '+');
    edit('sect', '8,4', $key, ',');
}

# multiple arguments
edit('sect', '1,5', 'm', 1, 'a', 1);

# interactive edit
iedit('sect', '3,5', 'm 2', 'a 1');
iedit('sect', '5,5', ' ');
iedit('sect', '7:9,5', 'e 1', '', 'e 2');

# give
give('2,6', 'l', $INT_MIN);
give('4:8,6', 'c', 1);
give('6,6', 'c', $INT_MAX);
give('8,6', 'c', -1);

# swapsector
swaps('-2,2', '2,-2');

## Ship, plane, land unit, nuke

for my $ef ('ship', 'plane', 'land', 'nuke') {
    # invalid key
    edit($ef, 0, '@', 0);
    # own
    edit($ef, 0, 'O', 0);
    edit($ef, 0, 'O', -1);
    edit($ef, 1, 'O', $MAXNOC - 1);
    edit($ef, 1, 'O', $MAXNOC);
    edit($ef, 2, 'O', 2);
    edit($ef, 4, 'O', 0, 'O', '3');
    # uid (copy)
    edit($ef, 3, 'U', 5, 'U', 3);
    edit($ef, 0, 'U', 0, 'U', -1);
    # x,y
    my $key = $ef eq 'plane' ? 'l' : 'L';
    edit($ef, 2, $key, '3,-1');
    edit($ef, 3, $key, '1,-1');
}

# ship: type
edit('ship', 6, 't', 'lc', 'O', 1, 't', 'lc', 't', 'hc', 'g', 8, 't', 'lc');

# ship: effic mobil tech pstage ptime milit
edit_int('ship', 2, (
    ['E', 0, 100],
    ['M', -127, 127],
    ['T', 0, 32767],
    ['a', 0, 4],
    ['b', 0, 32767],
    ['m', 0, 50],
));

# plane: type
edit('plane', 6, 'T', 'mb', 'O', 1, 'T', 'mb', 'T', 'hb', 'T', 'mb');

# plane: effic mobil range tech
edit_int('plane', 2, (
    ['e', 0, 100],
    ['m', -127, 127],
    ['r', 0, 9],
    ['t', 50, 32767],
));

# land: type
edit('land', 6, 'T', 'art', 'O', 1, 'T', 'art', 'T', 'hat', 'g', 12,
     'T', 'art');

# land: effic mobil tech harden retreat milit
edit_int('land', 2, (
    ['e', 0, 100],
    ['M', -127, 127],
    ['t', 50, 32767],
    ['F', 0, 127],
    ['Z', 0, 100],
    ['m', 0, 25],
));

# nuke: type
edit('nuke', 6, 't', '15kt', 'O', 1, 't', '15kt', 't', '50kt', 't', '15kt');

# nuke: tech
edit_int('nuke', 2, (
    ['T', 280, 32767],
));

# fleet, wing, army
sub unit_group {
    my ($ef, $key) = @_;
    edit($ef, 2, $key, '~');
    edit($ef, 3, $key, 'a');
}
unit_group('ship', 'F');
unit_group('plane', 'w');
unit_group('land', 'a');

# rpath, rflags
for my $ef ('ship', 'land') {
    edit($ef, 2, 'R', '""');
    edit($ef, 3, 'R', 'jj');
    # Take care to have only valid bits set in final state
    edit($ef, 2, 'W', 0, 'W', 1);
    edit($ef, 3, 'W', 513, 'W', 1030, 'W', 2);
}

# plane: flags
# Take care to have only valid bits set in final state
edit('plane', 2, 'f', 4);

# carrier
sub unit_carrier {
    my ($ef, $key1, $key2) = @_;
    edit($ef, 2, $key1, -1, $key1, 9999);
    edit($ef, 3, $key1, 3);
    edit($ef, 4, $key1, 4, $key2, 4) if defined $key2;
}
unit_carrier('plane', 's', 'y');
unit_carrier('land', 'S', 'Y');
unit_carrier('nuke', 'p');

# special case: move carrier's cargo away
edit('plane', 4, 'l', '5,1');

# special case: load teleports to carrier
edit('land', 4, 'S', 2);

# interactive edit
iedit('ship', 0, 'M 2', 'm 1', 'f 1');
iedit('ship', 0, 'R n', 'R ""');
iedit('plane', 0, 'm 2', 'y -1');
iedit('land', 0, 'M 2', 'Y -1');
iedit('nuke', 0, 'S a', 'p -1');

## Nation

# invalid key
edit('nat', 0, '@', 0);

# btus reserve timeused money

edit_int('nat', 1, (
    ['b', 0, 640],
    ['m', 0, 2147483647],
    ['u', 0, 86400],
    ['M', -2147483648, 2147483647],
));

# tgms
# Take care to use ones that won't receive further telegrams
edit('nat', 6, 't', -1);
edit('nat', 7, 't', 65536);

# tlev rlev elev hlev
for my $key ('T', 'R', 'E', 'H') {
    edit('nat', 1, $key, 3.14);
    edit('nat', 2, $key, 100);
    edit('nat', 3, $key, -1);
}

# cnam
edit('nat', 1, 'n', 'POGO');
edit('nat', 2, 'n', '2');
edit('nat', 3, 'n', 'drei');

# pnam
edit('nat', 1, 'r', 1);
edit('nat', 2, 'r', '012345678901234567890123456789');

# xcap,ycap xorg,yorg
for my $key ('c', 'o') {
    edit('nat', 1, $key, '0,0');
    edit('nat', 2, $key, '2,0');
}

# status
# Refs to nats with status 0 are invalid, take care to use
# unreferenced ones for that
edit('nat', 6, 's', 0);
edit('nat', 7, 's', -1);
edit('nat', 4, 's', $STAT_GOD);
edit('nat', 5, 's', $STAT_GOD + 1);

# interactive edit
iedit('nat', 5, 'b 640', 'T 1');

## Epilog: read telegrams (they're not in xdump)
for my $cnum (0, 1, 2, 3, 4, 5, 98) {
    print "read $cnum\n";
}
