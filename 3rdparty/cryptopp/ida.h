#ifndef CRYPTOPP_IDA_H
#define CRYPTOPP_IDA_H

#include "mqueue.h"
#include "filters.h"
#include "channels.h"
#include <map>
#include <vector>

NAMESPACE_BEGIN(CryptoPP)

/// base class for secret sharing and information dispersal
class RawIDA : public Filter, public BufferedTransformationWithAutoSignal
{
public:
	RawIDA(unsigned int threshold, BufferedTransformation *outQueue=NULL);

	unsigned int GetThreshold() const {return m_threshold;}
	void AddOutputChannel(word32 channelId);
	void ChannelData(word32 channelId, const byte *inString, unsigned int length);
	unsigned int InputBuffered(word32 channelId) const;

	void ChannelPut(const std::string &channel, byte inByte)
		{ChannelData(StringToWord<word32>(channel), &inByte, 1);}
	void ChannelPut(const std::string &channel, const byte *inString, unsigned int length)
		{ChannelData(StringToWord<word32>(channel), inString, length);}
	void ChannelMessageEnd(const std::string &channel, int propagation=-1);

	void Put(byte inByte) {}
	void Put(const byte *inString, unsigned int length) {}

protected:
	virtual void FlushOutputQueues();
	virtual void OutputMessageEnds();

	unsigned int InsertInputChannel(word32 channelId);
	unsigned int LookupInputChannel(word32 channelId) const;
	void ComputeV(unsigned int);
	void PrepareInterpolation();
	void ProcessInputQueues();

	std::map<word32, unsigned int> m_inputChannelMap;
	std::map<word32, unsigned int>::iterator m_lastMapPosition;
	std::vector<MessageQueue> m_inputQueues;
	std::vector<word32> m_inputChannelIds, m_outputChannelIds, m_outputToInput;
	std::vector<ByteQueue> m_outputQueues;
	std::vector<ChannelSwitch> m_channelSwitches;
	unsigned int m_threshold, m_channelsReady, m_channelsFinished;
	std::vector<SecBlock<word32> > m_v;
	SecBlock<word32> m_u, m_w, m_y;
};

/// a variant of Shamir's Secret Sharing Algorithm
class SecretSharing : public Filter
{
public:
	SecretSharing(RandomNumberGenerator &rng, unsigned int threshold, unsigned int nShares, BufferedTransformation *outQueue=NULL, bool addPadding=true);

	void Put(byte inByte)
		{SecretSharing::Put(&inByte, 1);}
	void Put(const byte *inString, unsigned int length);
	void MessageEnd(int propagation=-1);

protected:
	RandomNumberGenerator &m_rng;
	RawIDA m_ida;
	bool m_pad;
};

/// a variant of Shamir's Secret Sharing Algorithm
class SecretRecovery : public RawIDA
{
public:
	SecretRecovery(unsigned int threshold, BufferedTransformation *outQueue=NULL, bool removePadding=true);

protected:
	void FlushOutputQueues();
	void OutputMessageEnds();

	bool m_pad;
};

/// a variant of Rabin's Information Dispersal Algorithm
class InformationDispersal : public Filter
{
public:
	InformationDispersal(unsigned int threshold, unsigned int nShares, BufferedTransformation *outQueue=NULL, bool addPadding=true);

	void Put(byte inByte)
		{InformationDispersal::Put(&inByte, 1);}
	void Put(const byte *inString, unsigned int length);
	void MessageEnd(int propagation=-1);

protected:
	RawIDA m_ida;
	bool m_pad;
	unsigned int m_nextChannel;
};

/// a variant of Rabin's Information Dispersal Algorithm
class InformationRecovery : public RawIDA
{
public:
	InformationRecovery(unsigned int threshold, BufferedTransformation *outQueue=NULL, bool removePadding=true);

protected:
	void FlushOutputQueues();
	void OutputMessageEnds();

	bool m_pad;
	ByteQueue m_queue;
};

class PaddingRemover : public Filter
{
public:
	PaddingRemover(BufferedTransformation *outQueue=NULL)
		: Filter(outQueue), m_possiblePadding(false) {}

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);
	void MessageEnd(int propagation=-1);

	// GetPossiblePadding() == false at the end of a message indicates incorrect padding
	bool GetPossiblePadding() const {return m_possiblePadding;}

private:
	bool m_possiblePadding;
	unsigned long m_zeroCount;
};

NAMESPACE_END

#endif
