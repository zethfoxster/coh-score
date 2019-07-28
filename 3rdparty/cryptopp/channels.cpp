// channels.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "channels.h"

NAMESPACE_BEGIN(CryptoPP)
USING_NAMESPACE(std)

void ChannelSwitch::Put(byte inByte)
{
	ChannelPut(NULL_CHANNEL, inByte);
}

void ChannelSwitch::Put(const byte *inString, unsigned int length)
{
	ChannelPut(NULL_CHANNEL, inString, length);
}

void ChannelSwitch::Flush(bool completeFlush, int propagation)
{
	ChannelFlush(NULL_CHANNEL, completeFlush, propagation);
}

void ChannelSwitch::MessageEnd(int propagation)
{
	ChannelMessageEnd(NULL_CHANNEL, propagation);
}

void ChannelSwitch::MessageSeriesEnd(int propagation)
{
	ChannelMessageSeriesEnd(NULL_CHANNEL, propagation);
}

void ChannelSwitch::PutMessageEnd(const byte *inString, unsigned int length, int propagation)
{
	ChannelPutMessageEnd(NULL_CHANNEL, inString, length, propagation);
}

class RouteIterator
{
public:
	typedef ChannelSwitch::RouteMap::const_iterator MapIterator;
	typedef ChannelSwitch::DefaultRouteList::const_iterator ListIterator;

	const std::string m_channel;
	bool m_useDefault;
	MapIterator m_itMapCurrent, m_itMapEnd;
	ListIterator m_itListCurrent, m_itListEnd;

	RouteIterator(ChannelSwitch &cs, const std::string &channel)
		: m_channel(channel)
	{
		pair<MapIterator, MapIterator> range = cs.m_routeMap.equal_range(channel);
		if (range.first == range.second)
		{
			m_useDefault = true;
			m_itListCurrent = cs.m_defaultRoutes.begin();
			m_itListEnd = cs.m_defaultRoutes.end();
		}
		else
		{
			m_useDefault = false;
			m_itMapCurrent = range.first;
			m_itMapEnd = range.second;
		}
	}

	bool End() const
	{
		return m_useDefault ? m_itListCurrent == m_itListEnd : m_itMapCurrent == m_itMapEnd;
	}

	void Next()
	{
		if (m_useDefault)
			++m_itListCurrent;
		else
			++m_itMapCurrent;
	}

	BufferedTransformation & Destination()
	{
		return m_useDefault ? *m_itListCurrent->first : *m_itMapCurrent->second.first;
	}

	const std::string & Channel()
	{
		if (m_useDefault)
			return m_itListCurrent->second.get() ? *m_itListCurrent->second.get() : m_channel;
		else
			return m_itMapCurrent->second.second;
	}
};

void ChannelSwitch::ChannelPut(const std::string &channel, byte inByte)
{
	RouteIterator it(*this, channel);
	while (!it.End())
	{
		it.Destination().ChannelPut(it.Channel(), inByte);
		it.Next();
	}
}

void ChannelSwitch::ChannelPut(const std::string &channel, const byte *inString, unsigned int length)
{
	RouteIterator it(*this, channel);
	while (!it.End())
	{
		it.Destination().ChannelPut(it.Channel(), inString, length);
		it.Next();
	}
}

void ChannelSwitch::ChannelFlush(const std::string &channel, bool completeFlush, int propagation)
{
	RouteIterator it(*this, channel);
	while (!it.End())
	{
		it.Destination().ChannelFlush(it.Channel(), completeFlush, propagation);
		it.Next();
	}
}

void ChannelSwitch::ChannelMessageEnd(const std::string &channel, int propagation)
{
	RouteIterator it(*this, channel);
	while (!it.End())
	{
		it.Destination().ChannelMessageEnd(it.Channel(), propagation);
		it.Next();
	}
}

void ChannelSwitch::ChannelMessageSeriesEnd(const std::string &channel, int propagation)
{
	RouteIterator it(*this, channel);
	while (!it.End())
	{
		it.Destination().ChannelMessageSeriesEnd(it.Channel(), propagation);
		it.Next();
	}
}

void ChannelSwitch::ChannelPutMessageEnd(const std::string &channel, const byte *inString, unsigned int length, int propagation)
{
	RouteIterator it(*this, channel);
	while (!it.End())
	{
		it.Destination().ChannelPutMessageEnd(it.Channel(), inString, length, propagation);
		it.Next();
	}
}

void ChannelSwitch::AddDefaultRoute(BufferedTransformation &destination)
{
	m_defaultRoutes.push_back(DefaultRoute(&destination, value_ptr<std::string>(NULL)));
}

void ChannelSwitch::RemoveDefaultRoute(BufferedTransformation &destination)
{
	for (DefaultRouteList::iterator it = m_defaultRoutes.begin(); it != m_defaultRoutes.end(); ++it)
		if (it->first == &destination && !it->second.get())
		{
			m_defaultRoutes.erase(it);
			break;
		}
}

void ChannelSwitch::AddDefaultRoute(BufferedTransformation &destination, const std::string &outChannel)
{
	m_defaultRoutes.push_back(DefaultRoute(&destination, outChannel));
}

void ChannelSwitch::RemoveDefaultRoute(BufferedTransformation &destination, const std::string &outChannel)
{
	for (DefaultRouteList::iterator it = m_defaultRoutes.begin(); it != m_defaultRoutes.end(); ++it)
		if (it->first == &destination && (it->second.get() && *it->second == outChannel))
		{
			m_defaultRoutes.erase(it);
			break;
		}
}

void ChannelSwitch::AddRoute(const std::string &inChannel, BufferedTransformation &destination, const std::string &outChannel)
{
	m_routeMap.insert(RouteMap::value_type(inChannel, Route(&destination, outChannel)));
}

void ChannelSwitch::RemoveRoute(const std::string &inChannel, BufferedTransformation &destination, const std::string &outChannel)
{
	typedef ChannelSwitch::RouteMap::iterator MapIterator;
	pair<MapIterator, MapIterator> range = m_routeMap.equal_range(inChannel);
	
	for (MapIterator it = range.first; it != range.second; ++it)
		if (it->second.first == &destination && it->second.second == outChannel)
		{
			m_routeMap.erase(it);
			break;
		}
}

NAMESPACE_END
