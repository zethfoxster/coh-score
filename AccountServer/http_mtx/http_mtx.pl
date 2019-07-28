#!/usr/bin/perl

use DBI;
use Net::HTTPServer;
use IO::Socket::INET;
use Term::ReadKey;
use Win32::Console::ANSI qw/ SetCloseButton /;

use constant HTTP_MTX_COMMAND_KEEPALIVE => 100;
use constant HTTP_MTX_COMMAND_APPLY_SKU => 101;

use constant HTTP_MTX_STATUS_OK => 100;
use constant HTTP_MTX_STATUS_OFFLINE => 101;
use constant HTTP_MTX_STATUS_BAD_SKU => 102;
use constant HTTP_MTX_STATUS_BAD_AMOUNT => 103;
use constant HTTP_MTX_STATUS_OVER_CLAIMED => 104;
use constant HTTP_MTX_STATUS_GRANT_LIMIT => 105;
use constant HTTP_MTX_STATUS_FAILED => 106;
use constant HTTP_MTX_DEAD_LINK => 200;

$config_id = $ARGV[0];
unless ($config_id)
{
	print "USAGE: http_mtx.pl CONFIG_ID\n";
	exit;
}

$dbh = DBI->connect('dbi:SQLite:dbname=http_mtx.db', '', '', { RaiseError => 1 }) or die $DBI::errstr;
$sth = $dbh->prepare("SELECT active,remote_ip,remote_port,server_ip,server_port FROM config WHERE id=$config_id");
$sth->execute();
($active,$remote_ip,$remote_port,$server_ip,$server_port) = $sth->fetchrow();
$sth->finish();
undef $sth;

if ($active)
{
	print "Accepting connections from $remote_ip on port $remote_port\nUsing AccountServer $server_ip on port $server_port\n";
	$mtx_socket = new IO::Socket::INET (PeerHost => $server_ip, PeerPort => $server_port, Proto => 'tcp');
	if ($mtx_socket)
	{
		$mtx_socket->autoflush(1);
		$server = new Net::HTTPServer(port => $remote_port, datadir => '.', log => 'STDOUT');
		$server->RegisterRegex("/mtx/.*", \&http_mtx);
		$server->RegisterRegex(".*", \&http_forbidden);
		$server->Start();
		SetCloseButton(0);
		while (1)
		{
			$server->Process(1);
			if (uc(ReadKey(-1)) eq 'Q')
			{
				last;
			}
			if (!&mtx_keepalive)
			{
				print "AccountServer did not respond to Keep-Alive, shutting down.";
				last;
			}
		}
		SetCloseButton(1);
		$server->Stop();
		$mtx_socket->close();
	}
	else
	{
		print "Unable to connect to the AccountServer.\n";
	}
}
else
{
	print "Configuration $config_id does not exist or is not active.\n";
}

$dbh->do("VACUUM");
$dbh->disconnect();
undef $dbh;

unless ($active)
{
	sleep(30);
}

sub http_mtx
{
	$req = shift;
	$res = $req->Response();

	if ($req->Env('REMOTE_ADDR') eq $remote_ip)
	{
		my $authid = int($req->Env('authid'));
		my $coupon = $req->Env('coupon');

		# This entry constrant ensures that the authid and coupon can't be used
		# for an SQL injection, so we are free to put them directly into queries.
		if (($authid > 0) && ($coupon =~ /^[0-9A-Z]+$/) && (length($coupon) < 17))
		{
			my $errormsg = '';
			my $logerror = 0;
			my $sth = $dbh->prepare("SELECT rewardgroup,uses,reward,random,daily,expiration,expiration < date('now') FROM coupons WHERE name='$coupon'");
			$sth->execute();
			my ($rewardgroup, $uses, $reward, $random, $daily, $expiration, $expired) = $sth->fetchrow();
			my $rows = $sth->rows();
			$sth->finish();

			if ($rows < 1)
			{
				$errormsg = "The coupon $coupon does not exist.";
				$logerror = 1;
			}
			else
			{
				$sth = $dbh->prepare("SELECT applied,message,date('now') FROM ledger WHERE authid='$authid' AND rewardgroup='$rewardgroup'");
				$sth->execute();
				my ($applied,$message,$today) = $sth->fetchrow();
				my $used = $sth->rows();
				$sth->finish();

				if (($used > 0) && !$daily)
				{
					$errormsg = "You already used the coupon $rewardgroup on $applied. It can only be used once per account.";
					$errormsg .= '<br>The message below is a record of the reward applied to your account by this coupon.';
					$errormsg .= "<br><br><br><br>$message<br><br><br><br><small>Coupon $coupon expires on $expiration and has $uses uses left.</small>";
				}
				elsif (($used > 0) && $daily && ($applied eq $today))
				{
					$errormsg = "You already used the coupon $rewardgroup today. It can be used once per day until $expiration.";
					$errormsg .= '<br>The message below is a record of the last reward applied to your account by this coupon.';
					$errormsg .= "<br><br><br><br>$message<br><br><br><br><small>Coupon $coupon expires on $expiration and has $uses uses left.</small>";
				}
				elsif ($uses < 1)
				{
					$errormsg = "The coupon $coupon has been used the maximum number of times.";
				}
				elsif ($expired)
				{
					$errormsg = "The coupon $coupon was only valid until $expiration.";
				}
				else
				{
					# Okay, if we got this far we can fetch a reward for the player.
					if ($random > 0)
					{
						$random = int(rand($random));
					}
					$sth = $dbh->prepare("SELECT sku,amount,message FROM rewards WHERE reward='$reward' AND idx='$random'");
					$sth->execute();
					my ($sku,$amount,$message) = $sth->fetchrow();
					$rows = $sth->rows();
					$sth->finish();

					if ($rows < 1)
					{
						# Somehow we picked an invalid reward. Maybe the random column is larger
						# than the number of items avaialable in the rewards table.
						$errormsg = "Could not find the reward $reward with random index $random. Please report this error.";
						$logerror = 1;
					}
					elsif ($amount < 1)
					{
						# Stupid amount in the rewards table.
						$errormsg = 'The reward $reward is trying to grant a zero or negative amount of $sku. Please report this error.';
						$logerror = 1;
					}
					else
					{
						# Everything is OK on our end, time to talk to AccountServer.

						my $mtx = &mtx_apply($authid, $sku, $amount);
						if ($mtx == HTTP_MTX_STATUS_OK)
						{
							# Finally! Everything went OK, echo the message and update the ledger.
							$uses--;
							$res->Body("$message<br><br><br><br><small>Coupon $coupon expires on $expiration and has $uses uses left.</small>");

							if ($used > 0)
							{
								# Daily coupon being refreshed, update the existing entry.
								$dbh->do("UPDATE ledger SET applied=date('now'),message='$message' WHERE authid='$authid' AND rewardgroup='$rewardgroup'");
							}
							else
							{
								# First time using this coupon, just INSERT it.
								$dbh->do("INSERT INTO ledger (authid,rewardgroup,applied,message) VALUES ($authid,'$rewardgroup',date('now'),'$message')");
							}

							# If the reward group ends with *, remove one use from every coupon in the group.
							if ($rewardgroup =~ /\*$/)
							{
								$dbh->do("UPDATE coupons SET uses=uses-1 WHERE rewardgroup='$rewardgroup'");
							}
							else
							{
								$dbh->do("UPDATE coupons SET uses=uses-1 WHERE name='$coupon'");
							}
						}
						elsif ($mtx == HTTP_MTX_STATUS_OFFLINE)
						{
							$errormsg = 'Your game account is not online. The Account Server cannot grant items to offline accounts.';
						}
						elsif ($mtx == HTTP_MTX_STATUS_BAD_SKU)
						{
							$errormsg = "The Account Server does not recognize the item $sku. Please report this error.";
							$logerror = 1;
						}
						elsif ($mtx == HTTP_MTX_STATUS_BAD_AMOUNT)
						{
							$errormsg = "The Account Server refused to grant $amount units of $sku. Please report this error.";
							$logerror = 1;
						}
						elsif ($mtx == HTTP_MTX_STATUS_OVER_CLAIMED)
						{
							$errormsg = "You have claimed more units of $sku than you actually have. Please report this error.";
							$logerror = 1;
						}
						elsif ($mtx == HTTP_MTX_STATUS_GRANT_LIMIT)
						{
							$errormsg = "You have already received the maximum number of $sku. Please report this error.";
							$logerror = 1;
						}
					}
				}
			}

			if ($errormsg ne '')
			{
				if ($logerror == 1)
				{
					$dbh->do("INSERT INTO errors (authid,coupon,message,datetime) VALUES ($authid,'$coupon','$errormsg',datetime('now'))");
				}
				$res->Body("<b>ERROR:</b> $errormsg");
			}
			return $res;
		}
	}

	&log_forbidden;
	return $res;
 }

 sub http_forbidden
{
	$req = shift;
	$res = $req->Response();
	&log_forbidden;
	return $res;
 }

sub log_forbidden
{
	# Request did not come from the forums or is invalid; someone might be poking around.
	$res->Code(403);
	$res->Body('<h1>Forbidden</h1>');

	# Log everything.
	open(LOGFILE,'>>http_mtx_forbidden.log');
	foreach my $var (keys(%{$req->Header()}))
	{
		print LOGFILE ($var . ': ' . $req->Header($var) . "\n");
	}
	print LOGFILE "\n";
	foreach my $var (keys(%{$req->Env()}))
	{
		print LOGFILE ($var . ' = ' . $req->Env($var) . "\n");
	}
	print LOGFILE "\n\n";
	close(LOGFILE);
}

sub mtx_keepalive
{
	my $buf = pack("vc", 3, HTTP_MTX_COMMAND_KEEPALIVE);
	my $size = $mtx_socket->send($buf);
	$mtx_socket->recv($buf, 3);
	($size, $buf) = unpack("vc", $buf);
	unless (($size == 3) && ($buf == HTTP_MTX_STATUS_OK))
	{
		return 0;
	}
	return 1;
}

sub mtx_apply
{
	my ($mtx_authid, $mtx_sku, $mtx_amount) = @_;

	my $buf = pack("vcVV", length($mtx_sku) + 12, HTTP_MTX_COMMAND_APPLY_SKU, $mtx_authid, $mtx_amount) . $mtx_sku . pack("c", 0);
	my $size = $mtx_socket->send($buf);

	$mtx_socket->recv($buf, 3);
	($size, $buf) = unpack("vc", $buf);
	unless ($size == 3)
	{
		return HTTP_MTX_DEAD_LINK;
	}
	return $buf;
}
