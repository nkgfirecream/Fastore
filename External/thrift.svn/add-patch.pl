# /usr/bin/perl
#$Id$

$pwd = qx(echo %cd%);
chomp $pwd;
$pwd .= '\\patch\\';

@patches = grep { s!\Q$pwd\E!!g; s/\.diff$//; } qx(dir /s/b patch);
chomp @patches;
print "Existing patches:\n\t", join ("\n\t", @patches), $/;

@changes = grep { s!\Q$pwd\E!!g; ($a, $_) = split } qx(svn status  trunk);
#print join $/, @changes;

Change:
foreach my $chg (@changes) {
    foreach my $p (@patches) {
	next Change if $chg =~ /\Q$p\E/;
    }
    push @todo, $chg;
}

print "Other modified files:\n\t", 
    join ("\n\t", grep { s!trunk\\lib\\cpp\\src\\thrift\\!! } @todo ), $/;

for $f (@todo) {
    my @diff = qx(svn diff trunk/lib/cpp/src/thrift/$f);
    die unless @diff;
    my $name = "patch\\$f.diff";
    print "creating: $name\n";
    die if -e $name;
    open PATCH, ">$name" or die "cannot open file '$name': $!\n";
    print PATCH @diff;
    close PATCH;
}
