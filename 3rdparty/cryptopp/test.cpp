// test.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#include "md5.h"
#include "sha.h"
#include "ripemd.h"
#include "files.h"
#include "rng.h"
#include "hex.h"
#include "gzip.h"
#include "default.h"
#include "rsa.h"
#include "randpool.h"
#include "ida.h"
#include "socketft.h"

#include "validate.h"
#include "bench.h"

#include <iostream>

#if (_MSC_VER >= 1000)
#include <crtdbg.h>		// for the debug heap
#endif

#if defined(__MWERKS__) && defined(macintosh)
#include <console.h>
#endif

USING_NAMESPACE(CryptoPP)
USING_NAMESPACE(std)

const int MAX_PHRASE_LENGTH=250;

void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed);
char *RSAEncryptString(const char *pubFilename, const char *seed, const char *message);
char *RSADecryptString(const char *privFilename, const char *ciphertext);
void RSASignFile(const char *privFilename, const char *messageFilename, const char *signatureFilename);
bool RSAVerifyFile(const char *pubFilename, const char *messageFilename, const char *signatureFilename);

void DigestFile(const char *file);

string EncryptString(const char *plaintext, const char *passPhrase);
string DecryptString(const char *ciphertext, const char *passPhrase);

void EncryptFile(const char *in, const char *out, const char *passPhrase);
void DecryptFile(const char *in, const char *out, const char *passPhrase);

void SecretShareFile(int threshold, int nShares, const char *filename, const char *seed);
void SecretRecoverFile(int threshold, const char *outFilename, char *const *inFilenames);

void InformationDisperseFile(int threshold, int nShares, const char *filename);
void InformationRecoverFile(int threshold, const char *outFilename, char *const *inFilenames);

void GzipFile(const char *in, const char *out, int deflate_level);
void GunzipFile(const char *in, const char *out);

void ForwardTcpPort(const char *sourcePort, const char *destinationHost, const char *destinationPort);

bool Validate(int);

#ifdef __BCPLUSPLUS__
int cmain(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _CRTDBG_LEAK_CHECK_DF
	// Turn on leak-checking
	int tempflag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tempflag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tempflag );
#endif

#if defined(__MWERKS__) && defined(macintosh)
	argc = ccommand(&argv);
#endif

	try
	{
		char command;

		if (argc < 2)
			command = 'h';
		else
			command = argv[1][0];

		switch (command)
		{
		case 'g':
		  {
			char seed[1024], privFilename[128], pubFilename[128];
			unsigned int keyLength;

			cout << "Key length in bits: ";
			cin >> keyLength;

			cout << "\nSave private key to file: ";
			cin >> privFilename;

			cout << "\nSave public key to file: ";
			cin >> pubFilename;

			cout << "\nRandom Seed: ";
			ws(cin);
			cin.getline(seed, 1024);

			GenerateRSAKey(keyLength, privFilename, pubFilename, seed);
			return 0;
		  }
		case 'r':
		  {
			switch (argv[1][1])
			{
			case 's':
				RSASignFile(argv[2], argv[3], argv[4]);
				return 0;
			case 'v':
			  {
				bool verified = RSAVerifyFile(argv[2], argv[3], argv[4]);
				cout << (verified ? "valid signature" : "invalid signature") << endl;
				return 0;
			  }
			default:
			  {
				char privFilename[128], pubFilename[128];
				char seed[1024], message[1024];

				cout << "Private key file: ";
				cin >> privFilename;

				cout << "\nPublic key file: ";
				cin >> pubFilename;

				cout << "\nRandom Seed: ";
				ws(cin);
				cin.getline(seed, 1024);

				cout << "\nMessage: ";
				cin.getline(message, 1024);

				char *ciphertext = RSAEncryptString(pubFilename, seed, message);
				cout << "\nCiphertext: " << ciphertext << endl;

				char *decrypted = RSADecryptString(privFilename, ciphertext);
				cout << "\nDecrypted: " << decrypted << endl;

				delete [] ciphertext;
				return 0;
			  }
			}
		  }
		case 'm':
			DigestFile(argv[2]);
			return 0;
		case 't':
		  {
			// VC60 workaround: use char array instead of std::string to workaround MSVC's getline bug
			char passPhrase[MAX_PHRASE_LENGTH], plaintext[1024];

			cout << "Passphrase: ";
			cin.getline(passPhrase, MAX_PHRASE_LENGTH);

			cout << "\nPlaintext: ";
			cin.getline(plaintext, 1024);

			string ciphertext = EncryptString(plaintext, passPhrase);
			cout << "\nCiphertext: " << ciphertext << endl;

			string decrypted = DecryptString(ciphertext.c_str(), passPhrase);
			cout << "\nDecrypted: " << decrypted << endl;

			return 0;
		  }
		case 'e':
		case 'd':
		  {
			char passPhrase[MAX_PHRASE_LENGTH];
			cout << "Passphrase: ";
			cin.getline(passPhrase, MAX_PHRASE_LENGTH);
			if (command == 'e')
				EncryptFile(argv[2], argv[3], passPhrase);
			else
				DecryptFile(argv[2], argv[3], passPhrase);
			return 0;
		  }
		case 's':
			if (argv[1][1] == 's')
			{
				char seed[1024];
				cout << "\nRandom Seed: ";
				ws(cin);
				cin.getline(seed, 1024);
				SecretShareFile(atoi(argv[2]), atoi(argv[3]), argv[4], seed);
			}
			else
				SecretRecoverFile(argc-3, argv[2], argv+3);
			return 0;
		case 'i':
			if (argv[1][1] == 'd')
				InformationDisperseFile(atoi(argv[2]), atoi(argv[3]), argv[4]);
			else
				InformationRecoverFile(argc-3, argv[2], argv+3);
			return 0;
		case 'v':
			return !Validate(argc>2 ? atoi(argv[2]) : 0);
		case 'b':
			if (argc<3)
				BenchMarkAll();
			else
				BenchMarkAll((float)atof(argv[2]));
			return 0;
		case 'z':
			GzipFile(argv[3], argv[4], argv[2][0]-'0');
			return 0;
		case 'u':
			GunzipFile(argv[2], argv[3]);
			return 0;
		case 'f':
			ForwardTcpPort(argv[2], argv[3], argv[4]);
			return 0;
		default:
			FileSource usage("usage.dat", true, new FileSink(cout));
			return 1;
		}
	}
	catch(CryptoPP::Exception &e)
	{
		cout << "CryptoPP::Exception caught: " << e.what() << endl;
		return -1;
	}
	catch(std::exception &e)
	{
		cout << "std::exception caught: " << e.what() << endl;
		return -2;
	}
	catch(...)
	{
		cout << "unknown exception caught" << endl;
		return -3;
	}
}

void GenerateRSAKey(unsigned int keyLength, const char *privFilename, const char *pubFilename, const char *seed)
{
	RandomPool randPool;
	randPool.Put((byte *)seed, strlen(seed));

	RSAES_OAEP_SHA_Decryptor priv(randPool, keyLength);
	HexEncoder privFile(new FileSink(privFilename));
	priv.DEREncode(privFile);
	privFile.MessageEnd();

	RSAES_OAEP_SHA_Encryptor pub(priv);
	HexEncoder pubFile(new FileSink(pubFilename));
	pub.DEREncode(pubFile);
	pubFile.MessageEnd();
}

char *RSAEncryptString(const char *pubFilename, const char *seed, const char *message)
{
	FileSource pubFile(pubFilename, true, new HexDecoder);
	RSAES_OAEP_SHA_Encryptor pub(pubFile);

	if (strlen(message) > pub.MaxPlainTextLength())
	{
		cerr << "message too long for this key\n";
		abort();
	}

	RandomPool randPool;
	randPool.Put((byte *)seed, strlen(seed));

	char *outstr = new char[2*pub.CipherTextLength()+1];
	pub.Encrypt(randPool, (byte *)message, strlen(message), (byte *)outstr);

	HexEncoder hexEncoder;
	hexEncoder.Put((byte *)outstr, pub.CipherTextLength());
	hexEncoder.MessageEnd();
	hexEncoder.Get((byte *)outstr, 2*pub.CipherTextLength());

	outstr[2*pub.CipherTextLength()] = 0;
	return outstr;
}

char *RSADecryptString(const char *privFilename, const char *ciphertext)
{
	FileSource privFile(privFilename, true, new HexDecoder);
	RSAES_OAEP_SHA_Decryptor priv(privFile);

	HexDecoder hexDecoder;
	hexDecoder.Put((byte *)ciphertext, strlen(ciphertext));
	hexDecoder.MessageEnd();
	SecByteBlock buf(priv.CipherTextLength());
	hexDecoder.Get(buf, priv.CipherTextLength());

	char *outstr = new char[priv.MaxPlainTextLength()+1];
	unsigned messageLength = priv.Decrypt(buf, (byte *)outstr);
	outstr[messageLength] = 0;
	return outstr;
}

void RSASignFile(const char *privFilename, const char *messageFilename, const char *signatureFilename)
{
	FileSource privFile(privFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Signer priv(privFile);
	NullRNG rng;	// RSASSA_PKCS1v15_SHA_Signer ignores the rng. Use a real RNG for other signature schemes!
	FileSource f(messageFilename, true, new SignerFilter(rng, priv, new HexEncoder(new FileSink(signatureFilename))));
}

bool RSAVerifyFile(const char *pubFilename, const char *messageFilename, const char *signatureFilename)
{
	FileSource pubFile(pubFilename, true, new HexDecoder);
	RSASSA_PKCS1v15_SHA_Verifier pub(pubFile);

	FileSource signatureFile(signatureFilename, true, new HexDecoder);
	if (signatureFile.MaxRetrievable() != pub.SignatureLength())
		return false;
	SecByteBlock signature(pub.SignatureLength());
	signatureFile.Get(signature, signature.size);

	VerifierFilter *verifierFilter = new VerifierFilter(pub);
	verifierFilter->PutSignature(signature);
	FileSource f(messageFilename, true, verifierFilter);

	byte result = 0;
	f.Get(result);
	return result == 1;
}

void DigestFile(const char *filename)
{
	MD5 md5;
	HashFilter md5Filter(md5, new HexEncoder);
	SHA sha;
	HashFilter shaFilter(sha, new HexEncoder);
	RIPEMD160 ripemd;
	HashFilter ripemdFilter(ripemd, new HexEncoder);
	SHA256 sha256;
	HashFilter sha256Filter(sha256, new HexEncoder);

	ChannelSwitch *channelSwitch;
	FileSource file(filename, false, channelSwitch = new ChannelSwitch);
	channelSwitch->AddDefaultRoute(md5Filter);
	channelSwitch->AddDefaultRoute(shaFilter);
	channelSwitch->AddDefaultRoute(ripemdFilter);
	channelSwitch->AddDefaultRoute(sha256Filter);
	file.PumpAll();

	FileSink sink(cout);
	cout << "\nMD5: ";
	md5Filter.TransferTo(sink);
	cout << "\nSHA-1: ";
	shaFilter.TransferTo(sink);
	cout << "\nRIPEMD-160: ";
	ripemdFilter.TransferTo(sink);
	cout << "\nSHA-256: ";
	sha256Filter.TransferTo(sink);
}

string EncryptString(const char *instr, const char *passPhrase)
{
	string outstr;

	DefaultEncryptorWithMAC encryptor(passPhrase, new HexEncoder(new StringSink(outstr)));
	encryptor.Put((byte *)instr, strlen(instr));
	encryptor.MessageEnd();

	return outstr;
}

string DecryptString(const char *instr, const char *passPhrase)
{
	string outstr;

	HexDecoder decryptor(new DefaultDecryptorWithMAC(passPhrase, new StringSink(outstr)));
	decryptor.Put((byte *)instr, strlen(instr));
	decryptor.MessageEnd();

	return outstr;
}

void EncryptFile(const char *in, const char *out, const char *passPhrase)
{
	FileSource f(in, true, new DefaultEncryptorWithMAC(passPhrase, new FileSink(out)));
}

void DecryptFile(const char *in, const char *out, const char *passPhrase)
{
	FileSource f(in, true, new DefaultDecryptorWithMAC(passPhrase, new FileSink(out)));
}

void SecretShareFile(int threshold, int nShares, const char *filename, const char *seed)
{
	assert(nShares<=1000);

	RandomPool rng;
	rng.Put((byte *)seed, strlen(seed));

	ChannelSwitch *channelSwitch;
	FileSource source(filename, false, new SecretSharing(rng, threshold, nShares, channelSwitch = new ChannelSwitch));

	vector_member_ptrs<FileSink> fileSinks(nShares);
	string channel;
	for (unsigned int i=0; i<nShares; i++)
	{
		char extension[5] = ".000";
		extension[1]='0'+byte(i/100);
		extension[2]='0'+byte((i/10)%10);
		extension[3]='0'+byte(i%10);
		fileSinks[i].reset(new FileSink((string(filename)+extension).c_str()));

		channel = WordToString<word32>(i);
		fileSinks[i]->Put((byte *)channel.data(), 4);
		channelSwitch->AddRoute(channel, *fileSinks[i], BufferedTransformation::NULL_CHANNEL);
	}

	source.PumpAll();
}

void SecretRecoverFile(int threshold, const char *outFilename, char *const *inFilenames)
{
	assert(threshold<=1000);

	SecretRecovery recovery(threshold, new FileSink(outFilename));

	vector_member_ptrs<FileSource> fileSources(threshold);
	SecByteBlock channel(4);
	unsigned int i;
	for (i=0; i<threshold; i++)
	{
		fileSources[i].reset(new FileSource(inFilenames[i], false));
		fileSources[i]->Pump(4);
		fileSources[i]->Get(channel, 4);
		fileSources[i]->Attach(new ChannelSwitch(recovery, string((char *)channel.ptr, 4)));
	}

	while (fileSources[0]->Pump(256))
		for (i=1; i<threshold; i++)
			fileSources[i]->Pump(256);

	for (i=0; i<threshold; i++)
		fileSources[i]->PumpAll();
}

void InformationDisperseFile(int threshold, int nShares, const char *filename)
{
	assert(nShares<=1000);

	ChannelSwitch *channelSwitch;
	FileSource source(filename, false, new InformationDispersal(threshold, nShares, channelSwitch = new ChannelSwitch));

	vector_member_ptrs<FileSink> fileSinks(nShares);
	string channel;
	for (unsigned int i=0; i<nShares; i++)
	{
		char extension[5] = ".000";
		extension[1]='0'+byte(i/100);
		extension[2]='0'+byte((i/10)%10);
		extension[3]='0'+byte(i%10);
		fileSinks[i].reset(new FileSink((string(filename)+extension).c_str()));

		channel = WordToString<word32>(i);
		fileSinks[i]->Put((byte *)channel.data(), 4);
		channelSwitch->AddRoute(channel, *fileSinks[i], BufferedTransformation::NULL_CHANNEL);
	}

	source.PumpAll();
}

void InformationRecoverFile(int threshold, const char *outFilename, char *const *inFilenames)
{
	assert(threshold<=1000);

	InformationRecovery recovery(threshold, new FileSink(outFilename));

	vector_member_ptrs<FileSource> fileSources(threshold);
	SecByteBlock channel(4);
	unsigned int i;
	for (i=0; i<threshold; i++)
	{
		fileSources[i].reset(new FileSource(inFilenames[i], false));
		fileSources[i]->Pump(4);
		fileSources[i]->Get(channel, 4);
		fileSources[i]->Attach(new ChannelSwitch(recovery, string((char *)channel.ptr, 4)));
	}

	while (fileSources[0]->Pump(256))
		for (i=1; i<threshold; i++)
			fileSources[i]->Pump(256);

	for (i=0; i<threshold; i++)
		fileSources[i]->PumpAll();
}

void GzipFile(const char *in, const char *out, int deflate_level)
{
	FileSource(in, true, new Gzip(new FileSink(out), deflate_level));
}

void GunzipFile(const char *in, const char *out)
{
	FileSource(in, true, new Gunzip(new FileSink(out)));
}

void ForwardTcpPort(const char *sourcePortName, const char *destinationHost, const char *destinationPortName)
{
#ifdef SOCKETS_AVAILABLE
	SocketsInitializer sockInit;

	Socket sockListen, sockSource, sockDestination;

	int sourcePort = Socket::PortNameToNumber(sourcePortName);
	int destinationPort = Socket::PortNameToNumber(destinationPortName);

	sockListen.Create();
	sockListen.Bind(sourcePort);

	cout << "Listing on port " << sourcePort << ".\n";
	sockListen.Listen();

	sockListen.Accept(sockSource);
	cout << "Connection accepted on port " << sourcePort << ".\n";

	cout << "Making connection to " << destinationHost << ", port " << destinationPort << ".\n";
	sockDestination.Create();
	sockDestination.Connect(destinationHost, destinationPort);

	cout << "Connection made to " << destinationHost << ", starting to forward.\n";

	SocketSource out(sockSource, false, new SocketSink(sockDestination));
	SocketSource in(sockDestination, false, new SocketSink(sockSource));

	while (!(out.EofReceived() && in.EofReceived()))
	{
		fd_set fds;
		FD_ZERO(&fds);
		if (!out.EofReceived())
			FD_SET(out, &fds);
		if (!in.EofReceived())
			FD_SET(in, &fds);
		select(FD_SETSIZE, &fds, NULL, NULL, NULL);

		if (FD_ISSET(out, &fds))
		{
			out.TimedPump(0);
			if (out.EofReceived())
			{
				cout << "EOF received on source socket.\n";
				out.PumpAll();	// this will shutdown the attached SocketSink
			}
		}
		if (FD_ISSET(in, &fds))
		{
			in.TimedPump(0);
			if (in.EofReceived())
			{
				cout << "EOF received on destination socket.\n";
				in.PumpAll();	// this will shutdown the attached SocketSink
			}
		}
	}
#else
	cout << "Sockets not available on this system.\n";
	exit(-1);
#endif
}

bool Validate(int alg)
{
	switch (alg)
	{
	case 1: return TestSettings();
	case 2: return TestOS_RNG();
	case 3: return MD5Validate();
	case 4: return SHAValidate();
	case 5: return DESValidate();
	case 6: return IDEAValidate();
	case 7: return ARC4Validate();
	case 8: return RC5Validate();
	case 9: return BlowfishValidate();
	case 10: return Diamond2Validate();
	case 11: return ThreeWayValidate();
	case 12: return BBSValidate();
	case 13: return DHValidate();
	case 14: return RSAValidate();
	case 15: return ElGamalValidate();
	case 16: return DSAValidate();
	case 17: return HAVALValidate();
	case 18: return SAFERValidate();
	case 19: return LUCValidate();
	case 20: return RabinValidate();
	case 21: return BlumGoldwasserValidate();
	case 22: return ECPValidate();
	case 23: return EC2NValidate();
	case 24: return MD5MACValidate();
	case 25: return GOSTValidate();
	case 26: return TigerValidate();
	case 27: return RIPEMDValidate();
	case 28: return HMACValidate();
	case 29: return XMACCValidate();
	case 30: return SHARKValidate();
	case 32: return LUCDIFValidate();
	case 33: return LUCELGValidate();
	case 34: return SEALValidate();
	case 35: return CASTValidate();
	case 36: return SquareValidate();
	case 37: return RC2Validate();
	case 38: return RC6Validate();
	case 39: return MARSValidate();
	case 40: return RWValidate();
	case 41: return MD2Validate();
	case 42: return NRValidate();
	case 43: return MQVValidate();
	case 44: return RijndaelValidate();
	case 45: return TwofishValidate();
	case 46: return SerpentValidate();
	case 47: return CipherModesValidate();
	case 48: return CRC32Validate();
	case 49: return ECDSAValidate();
	case 50: return XTRDHValidate();
	case 51: return SKIPJACKValidate();
	case 52: return SHA2Validate();
	case 53: return PanamaValidate();
	case 54: return Adler32Validate();
	default: return ValidateAll();
	}
}
