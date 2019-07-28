my %ents = ();

open RULES, ">rules.txt";
print RULES <<END;
AuctionClient:Put,xid
AuctionClient:Change,inf fee
AuctionClient:Get,xid
END
close RULES;

my $infile = $ARGV[0];
my $outfile = $infile;
$outfile=~s|.*[/\\]+([^/\\]+)|$1|;

print qq(logfilter.exe -rules rules.txt -cryptic "$infile" "$outfile.log" \n);
system qq(logfilter.exe -rules rules.txt -cryptic "$infile" "$outfile.log");

print "opening $outfile.log\n";
open FILE, "<$outfile.log" or die "couldn't open $outfile.log";

while(<FILE>)
{
	my $line = $_;
	my $type;
	my $item;
	my $stored;
	my $other;
	my $id;

	my ($ent) = $line=~m/[^"]+"([^"]+):/i; # match all but auth
#	print "$ent\n";

	($id,$type,$item,$stored,$other,$storedinf,$status) = ParseItem($line);

	if($line=~m/AuctionClient:Get/i)
	{
		# MapHandleXactGetInfStored
		# 	RemInfStored
		# 		Get: getting sale. player with %i inf should have been given %i influece, but was given %i"
		# 		| Get: get sale. player was given %i inf.
		# 
		# MapHandleXactGetInv
		# 	RemAmtStored
		# 		Get: get succeeded. got item: %s, %d, %d
		# 		| Get: got inspiration: %s
		# 		| Get: got enhancement: %s, %d
		# 	& RemInfStored
		# 		Get: getting sale. player with %i inf should have been given %i influece, but was given %i"
		# 		| Get: get sale. player was given %i inf.
		# 
		# MapHandleXactGetAmtStored
		# 	RemAmtStored
		# 		Get: get succeeded. got item: %s, %d, %d
		# 		| Get: got inspiration: %s
		# 		| Get: got enhancement: %s, %d
		# 

		# 070509 00:00:00 City_03_01_1:172.30.131.23 "Mind's Shadow"    0 [AuctionClient:Get] (xid=6022188)get succeeded. got item: S_Brass, 0, 2 Item is '11 S_Brass'Context:"<&\r\nAucInvItem\r\n	ID 479\r\n	Item "11 S_Brass"\r\n	Status Stored\r\n	StoredCount 2\r\nEnd\r\n\r\n&>"
		if($line=~m/get sale/i)
		{
			# 070509 00:00:01 V_City_02_01_1:172.30.131.23 "Sulaiman"         8262 [AuctionClient:Get] (xid=4063217)get sale. player was given 45 inf. Item is '11 S_ArmorShard'Context:"<&\r\nAucInvItem\r\n	ID 1254\r\n	Item "11 S_ArmorShard"\r\n	Status Sold\r\n	StoredInf 50\r\n	OtherCount 10\r\n	Price 5\r\nEnd\r\n\r\n&>"
			my ($inf) = $line=~m/get sale. player was given ([0-9]+)/i;
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{inf} -= $inf;

			$ents{$ent}{$id}{count} = $stored;
		}
		elsif($line=~m/getting sale/i)
		{
			# 070509 00:00:01 V_City_02_01_1:172.30.131.23 "Sulaiman"         8262 [AuctionClient:Get] (xid=4063217)get sale. player was given 45 inf. Item is '11 S_ArmorShard'Context:"<&\r\nAucInvItem\r\n	ID 1254\r\n	Item "11 S_ArmorShard"\r\n	Status Sold\r\n	StoredInf 50\r\n	OtherCount 10\r\n	Price 5\r\nEnd\r\n\r\n&>"

			my ($inf) = $line=~m/was given ([0-9]+)/i;
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{inf} -= $inf;
		}
		elsif($line=~m/get succeeded/i)
		{
			# 070509 00:00:00 City_03_01_1:172.30.131.23 "Mind's Shadow"    0 [AuctionClient:Get] (xid=6022188)get succeeded. got item: S_Brass, 0, 2 Item is '11 S_Brass'Context:"<&\r\nAucInvItem\r\n	ID 479\r\n	Item "11 S_Brass"\r\n	Status Stored\r\n	StoredCount 2\r\nEnd\r\n\r\n&>"
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{count} -= $stored;

			if($status=~m/bidding/i)
			{
				$ents{$ent}{$id}{inf} = $storedinf;
			}
		}
		elsif($line=~m/got inspiration/i)
		{
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{count} = 0;
		}
		elsif($line=~m/got enhancement/i)
		{
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{count} = 0;
		}
	}
	elsif($line=~m/AuctionClient:Change/i)
	{
		# MapHandleXactChangeInvStatus
		# 	Change: selling item. %d inf fee removed from player;
		# 
		if($line=~m/selling item/i)
		{
			# 070509 00:00:00 V_City_03_01_1:172.30.131.19 "Adam We"          0 [AuctionClient:Change] (xid=4063190)selling item. 50 inf fee removed from player Item is '11 S_Claw'Context:"<&\r\nAucInvItem\r\n	ID 928\r\n	Item "11 S_Claw"\r\n	Status ForSale\r\n	StoredCount 10\r\n	Price 5\r\nEnd\r\n\r\n&>"
			my ($inf) = $line=~m/selling item\. ([0-9]+)/i;

			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{inf} += $inf;
		}
	}
	elsif($line=~m/AuctionClient:Put/i)
	{
		# MapHandleXactAddInv
		#	AddInfStored
		#		bid placed. removed %d inf.
		# 	| Put: Placed additional bid for item. removed %d inf fee.
		# 	| AddAmtStored
		# 		VerifyAddAmtStored
		# 			Put: Removed item from inventory: %s, %d, %d
		# 		| Put: Success. Removed inspiration from inventory: %s at %d,%d
		# 		| Put: Success. Removed enhancement from inventory: %s at %d
		# 
		if($line=~m/bid placed/i)
		{
			# 070509 00:00:02 City_03_01_1:172.30.131.23 "SpyderDwagon"     0 [AuctionClient:Put] (xid=6022205)bid placed. removed 2400 inf. Item is '11 S_InanimateCarbonRod'Context:"<&\r\nAucInvItem\r\n	ID 226\r\n	Item "11 S_InanimateCarbonRod"\r\n	Status Bidding\r\n	StoredInf 2400\r\n	OtherCount 4\r\n	Price 600\r\nEnd\r\n\r\n&>"
			my ($inf) = $line=~m/bid placed. removed ([0-9]+)/i;

			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{inf} += $inf;
			$ents{$ent}{$id}{other} += $other;
		}
		elsif($line=~m/Placed additional/)
		{
			my ($inf) = $line=~m/removed ([0-9]+)/i;

			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{inf} += $inf;
		}
		elsif($line=~m/Removed item/)
		{
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{count} += $stored;
		}
		elsif($line=~m/Removed inspiration/)
		{
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{count} = 1;
		}
		elsif($line=~m/Removed enhancement/)
		{
			$ents{$ent}{$id}{item} = $item;
			$ents{$ent}{$id}{count} = 1;
		}
	}
}
close FILE;

open OUT, ">$ARGV[1]" or die "couldn't open $ARGV[1]";


foreach my $ent (sort keys %ents)
{
	my $inf = 0;
	my %stuff = ();
	foreach my $id (sort keys %{ $ents{$ent} })
	{
		if($ents{$ent}{$id}{item})
		{
			if($ents{$ent}{$id}{count} && $ents{$ent}{$id}{count}>0)
			{
				if($ents{$ent}{$id}{item}=~m/^salvage/)
				{
					$stuff{$ents{$ent}{$id}{item}} += $ents{$ent}{$id}{count};
				}
				else
				{
					for(my $i=$ents{$ent}{$id}{count}; $i>0; $i--)
					{
						print OUT qq(/csroffline "$ent" $ents{$ent}{$id}{item}\n);
					}
				}
			}
			if($ents{$ent}{$id}{inf} && $ents{$ent}{$id}{inf}>0)
			{
				$inf+=$ents{$ent}{$id}{inf};
			}
		}
	}

	foreach my $item (sort keys %stuff)
	{
		if($stuff{$item}>0)
		{
			print OUT qq(/csroffline "$ent" $item $stuff{$item}\n);
		}
	}

	if($inf>0)
	{
		print OUT qq(/csroffline "$ent" influence_add $inf\n);
	}
}
close OUT;

sub ParseItem
{
	my $line = shift;
	
	# 070509 00:21:30 City_02_01_1:172.30.131.22 "Streblon"         7737 [AuctionClient:Put] (xid=6048092)Removed item from inventory: S_RuneboundArmor, 0, 1 Item is '11 S_RuneboundArmor'Context:"<&\r\nAucInvItem\r\n	ID 351\r\n	Item "11 S_RuneboundArmor"\r\n	Status Stored\r\n	StoredCount 1\r\n	MapSideIdx 23\r\nEnd\r\n\r\n&>"
	my ($id,$type,$item) = $line=~m/AucInvItem.*ID ([0-9]+).*Item "([0-9]+) ([^"]+)"/i;
	my ($stored) = $line=~m/StoredCount ([0-9]+)/i;
	my ($other) = $line=~m/OtherCount ([0-9]+)/i;
	my ($storedinf) = $line=~m/StoredInf ([0-9]+)/i;
	my ($status) = $line=~m/Status ([A-Za-z]+)/i;

	if($type == 2)
	{
		# inspiration
		my ($set, $power) = $item=~m/Inspirations\.([^.]+)\.([^ .]+)/i;

		$item = qq(inspire $set $power);
	}
	elsif($type == 5)
	{
		# enhancement
		my ($set, $power, $level) = $item=~m/Boosts\.([^.]+)\.([^ .]+) ([0-9]+)/i;
		$level = 0 if(!$level);
		$level++;

		$item = qq(boost $set $power $level);
	}
	elsif($type == 11)
	{
		# salvage
		$item = qq(salvage_grant $item);
	}
	elsif($type == 12)
	{
		# recipe
		my ($recipe) = $item=~m/([^ ]+)/i;
		$item = qq(rw_recipe $recipe);
	}
	else
	{
		print qq("Unknown type $type: $line");
	}

	return ($id,$type,$item,$stored,$other,$storedinf,$status);
}
