#!perl -w

$hsrc = 'c:/src/Common/entity/character_attribs.h';
$csrc = 'c:/src/Common/entity/character_attribs.c';


# Sections supported
#
# ParseCharacterAttributes
# IS_HITPOINTS
# CharacterAttributesTable
# ParseCharacterAttributesTable
#
# ModBase
# CalcCur
#
# TRANSPOSE_ARRAY
# CLAMP_MAX
# CLAMP_STRENGTH
# CLAMP_CUR

@attribs = ();
%Add = ();
%TimesMax = ();
%Absolute = ();

die "$hsrc isn't writable. Check it out first." if(!-w $hsrc);
die "$csrc isn't writable. Check it out first." if(!-w $csrc);

open IN, "<$hsrc";

while(<IN>)
{
	if(m/typedef struct CharacterAttributes/)
	{
		last;
	}
}

while(<IN>)
{
	if(m'^\s*float\s+f([^\s]+);.*//(.*)')
	{
		my $attrib = $1;
		my $args = $2;
		my $size = 0;
		my $index = '';

		if($attrib=~m/\[/)
		{
			($attrib, $size) = $attrib=~m/([^\[]+)\[([^\]]+)\]/;
			$size{$attrib} = $size;
		}
		else
		{
			$size{$attrib} = 1;
		}

		push @attribs, $attrib;

		my @args = split /,/, $args;
		foreach $a (@args)
		{
			my ($param, $val) = $a=~m/\s*([^\s:]+)[\s:]*([^\s]*)/;
			if($val)
			{
				$$param{$attrib} = $val;
			}
			else
			{
				$$param{$attrib}++;
			}
		}
	}
	elsif(m'^(#.*)')
	{
		my $attrib = $1;
		push @attribs, $attrib;
		$size{$attrib} = 1;
	}
	elsif(m'} CharacterAttributes;')
	{
		last;
	}
}

Process($hsrc);
Process($csrc);


sub Process
{
	my $fname = shift;

	my $oldslash = $/;
	$/ = undef; # slurp it all

	open IN, "<$fname" or die "Can't open $fname!";
	my $input = <IN>;
	close IN;

	$input =~ s"([ \t]*)// START ([^\s]+).*?// END \2[ \t]*\n"'qq($1// START $2\n).'.$2.'.qq($1// END $2\n)'"sgee;

	open IN, ">$fname" or die "Can't open $fname!";
	print IN $input;
	close IN;

	$/ = $oldslash;
}


#
#
#
sub ParseCharacterAttributes
{
	my $ret;

	foreach $attrib (@attribs)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			if($attrib=~m/^#/)
			{
				$ret .= "$attrib\n";
			}
			else
			{
				my $indexName='';
				my $index='';

				if($size{$attrib}>1)
				{
					$indexName = sprintf("%02d", $i);
					$index = sprintf("[%d]", $i);
				}

				$ret .= sprintf("\t{ %-18s TOKEN_F32,                      offsetof(CharacterAttributes, f%-18s },\n", qq("$attrib$indexName",), "$attrib$index)");
				if(exists($Synonym{$attrib}))
				{
					$ret .= sprintf("\t{ %-18s TOKEN_F32|TOKEN_REDUNDANTNAME,  offsetof(CharacterAttributes, f%-18s },\n", qq("$Synonym{$attrib}$indexName",), "$attrib$index)");
				}
			}
		}
	}

	return $ret;
}

sub IS_HITPOINTS
{
	my $ret;
	my $first = 1;

	$ret .= "#define IS_HITPOINTS(x) ( \\\n";

	foreach $attrib (keys %HitPoints)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			my $indexName='';
			my $index='';

			if($size{$attrib}>1)
			{
				$indexName = sprintf("%02d", $i);
				$index = sprintf("[%d]", $i);
			}

			if($first)
			{
				$first = 0;
				$ret .= "\t   ";
			}
			else
			{
				$ret .= "\t|| ";
			}
			$ret .= "(x)==offsetof(CharacterAttributes, f$attrib$index) \\\n";
		}
	}
	$ret .= "\t)\n";

	return $ret;
}

sub CharacterAttributesTable
{
	my $ret;

	foreach $attrib (@attribs)
	{
		if($attrib=~m/^#/)
		{
			$ret .= "$attrib\n";
		}
		else
		{
	 		my $size='';
	 		if($size{$attrib}>1)
	 		{
	 			$size = sprintf("[%d]", $size{$attrib});
	 		}

	 		$ret .= "\tfloat *pf$attrib$size;\n";
		}
	}
	return $ret;
}

sub DimReturnsLookup
{
	my $ret;

	foreach $attrib (@attribs)
	{
		if($attrib=~m/^#/)
		{
			$ret .= "$attrib\n";
		}
		else
		{
			my $size='';
			if($size{$attrib}>1)
			{
				$size = sprintf("[%d]", $size{$attrib});
			}

			$ret .= "\tDimReturns *p$attrib$size;\n";
		}
	}
	return $ret;
}

sub ParseCharacterAttributesTable
{
	my $ret;

	foreach $attrib (@attribs)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			if($attrib=~m/^#/)
			{
				$ret .= "$attrib\n";
			}
			else
			{
				my $indexName='';
				my $index='';

				if($size{$attrib}>1)
				{
					$indexName = sprintf("%02d", $i);
					$index = sprintf("[%d]", $i);
				}

				$ret .= sprintf("\t{ %-18s TOKEN_F32ARRAY, offsetof(CharacterAttributesTable, pf%-18s },\n", qq("$attrib$indexName",), "$attrib$index)");
				if(exists($Synonym{$attrib}))
				{
					$ret .= sprintf("\t{ %-18s TOKEN_F32ARRAY|TOKEN_REDUNDANTNAME,  offsetof(CharacterAttributesTable, pf%-18s },\n", qq("$Synonym{$attrib}$indexName",), "$attrib$index)");
				}
			}
		}
	}

	return $ret;
}

sub ModBase
{
	my $ret;

	foreach $attrib (@attribs)
	{
		if($attrib=~m/^#/)
		{
			$ret .= "$attrib\n";
		}
		else
		{
			if($size{$attrib}>1)
			{
				$ret .= sprintf("\t{ ");
				for(my $i=0; $i<$size{$attrib}; $i++)
				{
					$ret .= "$ModBase{$attrib}, ";
				}
				$ret .= "}, // f$attrib\n";
			}
			else
			{
				$ret .= sprintf("\t$ModBase{$attrib}, // f$attrib\n");
			}
		}
	}

	return $ret;
}

sub Values
{
	my $ret;
	my $val = shift;

	foreach $attrib (@attribs)
	{
		if($attrib=~m/^#/)
		{
			$ret .= "$attrib\n";
		}
		else
		{
			if($size{$attrib}>1)
			{
				$ret .= sprintf("\t{ ");
				for(my $i=0; $i<$size{$attrib}; $i++)
				{
					$ret .= "$val, ";
				}
				$ret .= "}, // f$attrib\n";
			}
			else
			{
				$ret .= sprintf("\t$val, // f$attrib\n");
			}
		}
	}

	return $ret;
}

sub CalcCur
{
	my $ret;

	foreach $attrib (@attribs)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			if($attrib=~m/^#/)
			{
				$ret .= "$attrib\n";
			}
			else
			{
				my $index='';

				if($size{$attrib}>1)
				{
					$index = sprintf("[%d]", $i);
				}

				if($attrib=~m/^#/)
				{
					$ret .= "$attrib\n";
				}
				else
				{
					$op = '*';
					if(exists($Add{$attrib}))
					{
						$op = '+';
					}

					if(exists($HitPoints{$attrib}))
					{
						$dest = 'HitPoints';
					}
					elsif(exists($SkillPoints{$attrib}))
					{
						$dest = 'SkillPoints';
					}
					elsif(exists($StatusPoints{$attrib}))
					{
						$ret .= sprintf("\tp->attrCur.f%-18s $op= (p->attrCur.f%-18s = pset->pattrMod->f%-18s", 'StatusPoints', $attrib.$index, $attrib.$index);
					}
					else
					{
						$dest = $attrib.$index;
					}

					if(!exists($StatusPoints{$attrib}))
					{
						$ret .= sprintf("\tp->attrCur.f%-18s $op= pset->pattrMod->f%-18s", $dest, $attrib.$index);
					}
					if(exists($TimesMax{$attrib}))
					{
						$ret .= sprintf(" * pset->pattrMax->f%-18s", $dest);
					}
					if(exists($Absolute{$attrib}))
					{
						$ret .= sprintf(" + pset->pattrAbsolute->f%-18s", $attrib.$index);
					}

					if(exists($StatusPoints{$attrib}))
					{
						$ret .= ")";
					}
					$ret =~ s/\s+$//s;
					$ret .= ";\n";

					if(exists($PlusAbsolute{$attrib}))
					{
						$ret .= sprintf("\tp->attrCur.f%-18s += pset->pattrAbsolute->f%-18s", $dest, $attrib.$index);
						$ret =~ s/\s+$//s;
						$ret .= ";\n";
					}
				}
			}
		}
	}

	return $ret;

}

sub Generic
{
	my $macro = shift;
	my $ret;

	foreach $attrib (@attribs)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			my $index='';

			if($size{$attrib}>1)
			{
				$index = sprintf("[%d]", $i);
			}

			if($attrib=~m/^#/)
			{
				$ret .= "$attrib\n";
			}
			else
			{
				$ret .= "\t";
				if(exists($macro->{$attrib}))
				{
					$ret .= "// ";
				}
				$ret .= sprintf("$macro(f$attrib$index);\n");
			}
		}
	}

	return $ret;
}

sub SetModBase
{
	my $macro = shift;
	my $ret;

	foreach $attrib (@attribs)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			if($attrib=~m/^#/)
			{
				$ret .= "$attrib\n";
			}
			else
			{
				my $index='';

				if($size{$attrib}>1)
				{
					$index = sprintf("[%d]", $i);
				}

				$ret .= "\t";
				if(exists($macro->{$attrib}))
				{
					$ret .= "// ";
				}
				$ret .= sprintf("pattr->f%-18s = %s;\n", $attrib.$index, $ModBase{$attrib});
			}
		}
	}

	return $ret;
}

sub DumpStrAttribs
{
	my $macro = shift;
	my $ret;

	foreach $attrib (@attribs)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			next if(exists($NoDump{$attrib}));

			if($attrib=~m/^#/)
			{
				$ret .= "$attrib\n";
			}
			else
			{
				my $index='';

				if($size{$attrib}>1)
				{
					$index = sprintf("[%d]", $i);
				}

				$ret .= "\t";
				if(exists($macro->{$attrib}))
				{
					$ret .= "// ";
				}

				$ret .= sprintf("DUMP_ATTR_STR($attrib$index);\n");
			}
		}
	}

	return $ret;
}


sub DumpAttribs
{
	my $macro = shift;
	my $ret;

	foreach $attrib (@attribs)
	{
		for(my $i=0; $i<$size{$attrib}; $i++)
		{
			next if(exists($NoDump{$attrib}));

			if($attrib=~m/^#/)
			{
				$ret .= "$attrib\n";
			}
			else
			{
				my $index='';
				my $extra = '';

				if($size{$attrib}>1)
				{
					$index = sprintf("[%d]", $i);
				}

				if(exists($DumpAttribs{$attrib}))
				{
					$extra = '_'.$DumpAttribs{$attrib};
				}

				$ret .= sprintf("\tDUMP_ATTR$extra($attrib$index);\n");
			}
		}
	}

	return $ret;
}
