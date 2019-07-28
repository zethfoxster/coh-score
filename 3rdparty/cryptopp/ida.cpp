// ida.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "ida.h"

#include "algebra.h"
#include "gf2_32.h"
#include "polynomi.h"

#include "polynomi.cpp"

ANONYMOUS_NAMESPACE_BEGIN
static const CryptoPP::GF2_32 field;
NAMESPACE_END

using namespace std;

NAMESPACE_BEGIN(CryptoPP)

RawIDA::RawIDA(unsigned int threshold, BufferedTransformation *outQueue)
	: Filter(outQueue), m_lastMapPosition(m_inputChannelMap.end()), m_threshold(threshold), m_channelsReady(0), m_channelsFinished(0)
	, m_w(threshold), m_y(threshold)
{
	assert(threshold > 0);
	m_inputQueues.reserve(threshold);
}

unsigned int RawIDA::InsertInputChannel(word32 channelId)
{
	if (m_lastMapPosition != m_inputChannelMap.end())
	{
		if (m_lastMapPosition->first == channelId)
			goto skipFind;
		++m_lastMapPosition;
		if (m_lastMapPosition != m_inputChannelMap.end() && m_lastMapPosition->first == channelId)
			goto skipFind;
	}
	m_lastMapPosition = m_inputChannelMap.find(channelId);

skipFind:
	if (m_lastMapPosition == m_inputChannelMap.end())
	{
		if (m_inputChannelIds.size() == m_threshold)
			return m_threshold;

		m_lastMapPosition = m_inputChannelMap.insert(pair<const unsigned long, unsigned int>(channelId, m_inputChannelIds.size())).first;
		m_inputQueues.push_back(MessageQueue());
		m_inputChannelIds.push_back(channelId);

		if (m_inputChannelIds.size() == m_threshold)
			PrepareInterpolation();
	}
	return m_lastMapPosition->second;
}

unsigned int RawIDA::LookupInputChannel(word32 channelId) const
{
	map<word32, unsigned int>::const_iterator it = m_inputChannelMap.find(channelId);
	if (it == m_inputChannelMap.end())
		return m_threshold;
	else
		return it->second;
}

void RawIDA::ChannelData(word32 channelId, const byte *inString, unsigned int length)
{
	unsigned int i = InsertInputChannel(channelId);
	if (i < m_threshold)
	{
		unsigned long size = m_inputQueues[i].MaxRetrievable();
		m_inputQueues[i].Put(inString, length);
		if (size < 4 && size + length >= 4)
		{
			m_channelsReady++;
			if (m_channelsReady == m_threshold)
				ProcessInputQueues();
		}
	}
}

unsigned int RawIDA::InputBuffered(word32 channelId) const
{
	unsigned int i = LookupInputChannel(channelId);
	return i < m_threshold ? m_inputQueues[i].MaxRetrievable() : 0;
}

void RawIDA::ChannelMessageEnd(const string &channel, int propagation)
{
	unsigned int i = InsertInputChannel(StringToWord<word32>(channel));
	if (i < m_threshold)
	{
		m_inputQueues[i].MessageEnd();
		if (m_inputQueues[i].NumberOfMessages() == 1)
		{
			m_channelsFinished++;
			if (m_channelsFinished == m_threshold)
			{
				m_channelsReady = 0;
				for (i=0; i<m_threshold; i++)
					m_channelsReady += m_inputQueues[i].AnyRetrievable();
				ProcessInputQueues();
			}
		}
	}
}

void RawIDA::ComputeV(unsigned int i)
{
	if (i >= m_v.size())
	{
		m_v.resize(i+1);
		m_outputToInput.resize(i+1);
	}

	m_outputToInput[i] = LookupInputChannel(m_outputChannelIds[i]);
	if (m_outputToInput[i] == m_threshold && i * m_threshold <= 1000*1000)
	{
		m_v[i].Resize(m_threshold);
		PrepareBulkPolynomialInterpolationAt(field, m_v[i].ptr, m_outputChannelIds[i], &m_inputChannelIds[0], m_w.ptr, m_threshold);
	}
}

void RawIDA::AddOutputChannel(word32 channelId)
{
	m_outputChannelIds.push_back(channelId);
	m_outputQueues.push_back(ByteQueue());
	m_channelSwitches.push_back(ChannelSwitch(*AttachedTransformation(), WordToString(channelId)));
	if (m_inputChannelIds.size() == m_threshold)
		ComputeV(m_outputChannelIds.size() - 1);
}

void RawIDA::PrepareInterpolation()
{
	assert(m_inputChannelIds.size() == m_threshold);
	PrepareBulkPolynomialInterpolation(field, m_w.ptr, &m_inputChannelIds[0], m_threshold);
//	polynomialRing.PrepareBulkInterpolation(m_w, m_inputChannelIds.begin(), m_threshold);
	for (unsigned int i=0; i<m_outputChannelIds.size(); i++)
		ComputeV(i);
}

void RawIDA::ProcessInputQueues()
{
	bool finished = (m_channelsFinished == m_threshold);
	unsigned int i;

	while (finished ? m_channelsReady > 0 : m_channelsReady == m_threshold)
	{
		m_channelsReady = 0;
		for (i=0; i<m_threshold; i++)
		{
			MessageQueue &queue = m_inputQueues[i];
			queue.GetWord32(m_y[i]);

			if (finished)
				m_channelsReady += queue.AnyRetrievable();
			else
				m_channelsReady += queue.NumberOfMessages() > 0 || queue.MaxRetrievable() >= 4;
		}

		for (i=0; i<m_outputChannelIds.size(); i++)
		{
			if (m_outputToInput[i] != m_threshold)
				m_outputQueues[i].PutWord32(m_y[m_outputToInput[i]]);
			else if (m_v[i].size == m_threshold)
				m_outputQueues[i].PutWord32(BulkPolynomialInterpolateAt(field, m_y.ptr, m_v[i].ptr, m_threshold));
			else
			{
				m_u.Resize(m_threshold);
				PrepareBulkPolynomialInterpolationAt(field, m_u.ptr, m_outputChannelIds[i], &m_inputChannelIds[0], m_w.ptr, m_threshold);
				m_outputQueues[i].PutWord32(BulkPolynomialInterpolateAt(field, m_y.ptr, m_u.ptr, m_threshold));
			}
		}
	}

	if (m_outputChannelIds.size() > 0 && m_outputQueues[0].AnyRetrievable())
		FlushOutputQueues();

	if (finished)
	{
		OutputMessageEnds();

		m_channelsReady = 0;
		m_channelsFinished = 0;
		m_v.clear();

		vector<MessageQueue> inputQueues;
		vector<word32> inputChannelIds;

		inputQueues.swap(m_inputQueues);
		inputChannelIds.swap(m_inputChannelIds);
		m_inputChannelMap.clear();
		m_lastMapPosition = m_inputChannelMap.end();

		for (i=0; i<m_threshold; i++)
		{
			inputQueues[i].GetNextMessage();
			ChannelSwitch channelSwitch(*this, WordToString(inputChannelIds[i]));
			inputQueues[i].TransferAllTo(channelSwitch);
		}
	}
}

void RawIDA::FlushOutputQueues()
{
	for (unsigned int i=0; i<m_outputChannelIds.size(); i++)
		m_outputQueues[i].TransferTo(m_channelSwitches[i]);
}

void RawIDA::OutputMessageEnds()
{
	if (GetAutoSignalPropagation() != 0)
	{
		for (unsigned int i=0; i<m_outputChannelIds.size(); i++)
			m_channelSwitches[i].MessageEnd(GetAutoSignalPropagation()-1);
	}
}

// ****************************************************************

SecretSharing::SecretSharing(RandomNumberGenerator &rng, unsigned int threshold, unsigned int nShares, BufferedTransformation *outQueue, bool addPadding)
	: Filter(outQueue), m_rng(rng), m_ida(threshold, new OutputProxy(*this, true)), m_pad(addPadding)
{
	for (unsigned int i=0; i<nShares; i++)
		m_ida.AddOutputChannel(i);
}

void SecretSharing::Put(const byte *inString, unsigned int length)
{
	SecByteBlock buf(STDMIN(length, 256U));
	unsigned int threshold = m_ida.GetThreshold();
	while (length > 0)
	{
		unsigned int len = STDMIN(length, buf.size);
		m_ida.ChannelData(0xffffffff, inString, len);
		for (unsigned int i=0; i<threshold-1; i++)
		{
			m_rng.GenerateBlock(buf, len);
			m_ida.ChannelData(i, buf, len);
		}
		length -= len;
		inString += len;
	}
}

void SecretSharing::MessageEnd(int propagation)
{
	m_ida.SetAutoSignalPropagation(propagation);
	if (m_pad)
	{
		SecretSharing::Put(1);
		while (m_ida.InputBuffered(0xffffffff) > 0)
			SecretSharing::Put(0);
	}
	m_ida.ChannelMessageEnd(WordToString<word32>(0xffffffff));
	for (unsigned int i=0; i<m_ida.GetThreshold()-1; i++)
		m_ida.ChannelMessageEnd(WordToString<word32>(i));
}

SecretRecovery::SecretRecovery(unsigned int threshold, BufferedTransformation *outQueue, bool removePadding)
	: RawIDA(threshold, outQueue), m_pad(removePadding)
{
	AddOutputChannel(0xffffffff);
}

void SecretRecovery::FlushOutputQueues()
{
	if (m_pad)
		m_outputQueues[0].TransferTo(*AttachedTransformation(), m_outputQueues[0].MaxRetrievable()-4);
	else
		m_outputQueues[0].TransferTo(*AttachedTransformation());
}

void SecretRecovery::OutputMessageEnds()
{
	if (m_pad)
	{
		PaddingRemover paddingRemover(new Redirector(*AttachedTransformation()));
		m_outputQueues[0].TransferAllTo(paddingRemover);
	}

	if (GetAutoSignalPropagation() != 0)
		AttachedTransformation()->MessageEnd(GetAutoSignalPropagation()-1);
}

// ****************************************************************

InformationDispersal::InformationDispersal(unsigned int threshold, unsigned int nShares, BufferedTransformation *outQueue, bool addPadding)
	: Filter(outQueue), m_ida(threshold, new OutputProxy(*this, true)), m_pad(addPadding), m_nextChannel(0)
{
	for (unsigned int i=0; i<nShares; i++)
		m_ida.AddOutputChannel(i);
}

void InformationDispersal::Put(const byte *inString, unsigned int length)
{
	while (length--)
	{
		m_ida.ChannelData(m_nextChannel, inString, 1);
		inString++;
		m_nextChannel++;
		if (m_nextChannel == m_ida.GetThreshold())
			m_nextChannel = 0;
	}
}

void InformationDispersal::MessageEnd(int propagation)
{
	m_ida.SetAutoSignalPropagation(propagation);
	if (m_pad)
		InformationDispersal::Put(1);
	for (unsigned int i=0; i<m_ida.GetThreshold(); i++)
		m_ida.ChannelMessageEnd(WordToString<word32>(i));
}

InformationRecovery::InformationRecovery(unsigned int threshold, BufferedTransformation *outQueue, bool removePadding)
	: RawIDA(threshold, outQueue), m_pad(removePadding)
{
	for (unsigned int i=0; i<threshold; i++)
		AddOutputChannel(i);
}

void InformationRecovery::FlushOutputQueues()
{
	while (m_outputQueues[0].AnyRetrievable())
	{
		for (unsigned int i=0; i<m_outputChannelIds.size(); i++)
			m_outputQueues[i].TransferTo(m_queue, 1);
	}

	if (m_pad)
		m_queue.TransferTo(*AttachedTransformation(), m_queue.MaxRetrievable()-4*m_threshold);
	else
		m_queue.TransferTo(*AttachedTransformation());
}

void InformationRecovery::OutputMessageEnds()
{
	if (m_pad)
	{
		PaddingRemover paddingRemover(new Redirector(*AttachedTransformation()));
		m_queue.TransferAllTo(paddingRemover);
	}

	if (GetAutoSignalPropagation() != 0)
		AttachedTransformation()->MessageEnd(GetAutoSignalPropagation()-1);
}

void PaddingRemover::Put(byte b)
{
	if (m_possiblePadding)
	{
		if (b == 0)
			m_zeroCount++;
		else
		{
			AttachedTransformation()->Put(1);
			while (m_zeroCount--)
				AttachedTransformation()->Put(0);
			AttachedTransformation()->Put(b);
			m_possiblePadding = false;
		}
	}
	else
	{
		if (b == 1)
		{
			m_possiblePadding = true;
			m_zeroCount = 0;
		}
		else
			AttachedTransformation()->Put(b);
	}
}

void PaddingRemover::Put(const byte *begin, unsigned int length)
{
	const byte *const end = begin + length;

	if (m_possiblePadding)
	{
		unsigned int len = find_if(begin, end, bind2nd(not_equal_to<byte>(), 0)) - begin;
		m_zeroCount += len;
		begin += len;
		if (begin == end)
			return;
		PaddingRemover::Put(*begin++);
	}

#if defined(_MSC_VER) && !defined(__MWERKS__)		// VC60 workaround
	typedef reverse_bidirectional_iterator<const byte *, const byte> rit;
#else
	typedef reverse_iterator<const byte *> rit;
#endif
	const byte *x = find_if(rit(end), rit(begin), bind2nd(not_equal_to<byte>(), 0)).base();
	if (x != begin && *(x-1) == 1)
	{
		AttachedTransformation()->Put(begin, x-begin-1);
		m_possiblePadding = true;
		m_zeroCount = end - x;
	}
	else
		AttachedTransformation()->Put(begin, end-begin);
}

void PaddingRemover::MessageEnd(int propagation)
{
	m_possiblePadding = false;
	Filter::MessageEnd(propagation);
}

NAMESPACE_END
