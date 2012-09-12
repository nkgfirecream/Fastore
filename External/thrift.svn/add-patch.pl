# /usr/bin/perl
#$Id$

use Getopt::Std;
use File::Basename;

getopts('pxv', \%opts);

sub up_to_date
{ my($patch, $src) = @_;
  my ($patch_mtime, $src_mtime);
  my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
      $atime,$ctime,$blksize,$blocks);
  
  ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
   $atime,$patch_mtime,$ctime,$blksize,$blocks)
      = stat($patch) or die "no such patch: '$patch'\n";

  ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
   $atime,$src_mtime,$ctime,$blksize,$blocks)
      = stat($src) or die "no such source file: '$patch'\n";

  return $src_mtime < $patch_mtime;
}

$pwd = qx(echo %cd%);
chomp $pwd;
$pwd .= '\\patch\\';

stat "$pwd" or die qq(no such dir "$pwd"\n);
@patches = grep { s!\Q$pwd\E!!g; s/\.diff$//; } qx(dir /s/b $pwd);
chomp @patches;
print "Existing patches:\n\t", join ("\n\t", @patches), $/;

@changes = grep { 
    s!\Q$pwd\E!!g; 
    my $status;
    ($status, $_) = split; 
    $status eq 'M' }  qx(svn status  trunk);
#print join $/, @changes;

Change:
foreach my $src (@changes) {
    foreach my $p (@patches) {
	if( $src =~ /\Q$p\E/ ) {
	    next Change if up_to_date( "${pwd}$p.diff", $src );
	}
    }
    push @todo, $src;
}

print "Files modified since patch was generated:\n\t", 
    join ("\n\t", grep { s!trunk\\lib\\cpp\\src\\thrift\\!! } @todo ), $/;

if( ! $opts{x} ) {
    exit 0;
}

print join $/, @todo if $opts{v};

for $f (@todo) {
    next unless $f =~ /\.(tcc|cpp|h)$/;
    my @diff = qx(svn diff trunk/lib/cpp/src/thrift/$f);
    die "bad: $f" unless @diff;
    my $name = "patch\\$f.diff";
    my $verb = (-e $name)? 'overwriting:' : 'creating:';
    printf "%-16s $name\n", $verb;
    my $dir = dirname $name;
    if( ! stat $dir ) {
	mkdir $dir or die "could not create $dir\n";
    }
    open PATCH, ">$name" or die "cannot open file '$name': $!\n";
    print PATCH @diff;
    close PATCH;
}
