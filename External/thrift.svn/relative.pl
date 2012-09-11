#! /usr/bin/perl

$pwd = qx(cd);
chomp $pwd;
$pwd  .= '\\';

while (<>) {
    s/\Q$pwd\E//g;
    print;
}
