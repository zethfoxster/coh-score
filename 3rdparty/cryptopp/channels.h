#ifndef CRYPTOPP_CHANNELS_H
#define CRYPTOPP_CHANNELS_H

#include "cryptlib.h"
#include "smartptr.h"
#include <map>
#include <list>

NAMESPACE_BEGIN(CryptoPP)

//! .
class ChannelSwitch : public BufferedTransformation
{
public:
	ChannelSwitch() {}
	ChannelSwitch(BufferedTransformation &destination)
	{
		AddDefaultRoute(destination);
	}
	ChannelSwitch(BufferedTransformation &destination, const std::string &outChannel)
	{
		AddDefaultRoute(destination, outChannel);
	}

	void Put(byte inByte);
	void Put(const byte *inString, unsigned int length);

	void Flush(bool completeFlush, int propagation=-1);
	void MessageEnd(int propagation=-1);
	void PutMessageEnd(const byte *inString, unsigned int length, int propagation=-1);
	void MessageSeriesEnd(int propagation=-1);

	void ChannelPut(const std::string &channel, byte inByte);
	void ChannelPut(const std::string &channel, const byte *inString, unsigned int length);

	void ChannelFlush(const std::string &channel, bool completeFlush, int propagation=-1);
	void ChannelMessageEnd(const std::string &channel, int propagation=-1);
	void ChannelPutMessageEnd(const std::string &channel, const byte *inString, unsigned int length, int propagation=-1);
	void ChannelMessageSeriesEnd(const std::string &channel, int propagation=-1);

	void AddDefaultRoute(BufferedTransformation &destination);
	void RemoveDefaultRoute(BufferedTransformation &destination);
	void AddDefaultRoute(BufferedTransformation &destination, const std::string &outChannel);
	void RemoveDefaultRoute(BufferedTransformation &destination, const std::string &outChannel);
	void AddRoute(const std::string &inChannel, BufferedTransformation &destination, const std::string &outChannel);
	void RemoveRoute(const std::string &inChannel, BufferedTransformation &destination, const std::string &outChannel);

private:
	typedef std::pair<BufferedTransformation *, std::string> Route;
	typedef std::multimap<std::string, Route> RouteMap;
	RouteMap m_routeMap;

	typedef std::pair<BufferedTransformation *, value_ptr<std::string> > DefaultRoute;
	typedef std::list<DefaultRoute> DefaultRouteList;
	DefaultRouteList m_defaultRoutes;

	friend class RouteIterator;
};

NAMESPACE_END

#endif
