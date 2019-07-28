#ifndef _INCLUDED_SERVERID_H
#define _INCLUDED_SERVERID_H

class ServerId
{
public:
	explicit ServerId(unsigned char value);
	ServerId();
	void SetValue(unsigned char value);
	bool IsValid() const;
	void SetInvalid();
	unsigned char GetValueChar() const;
	bool operator==(const ServerId & rhs) const;
	bool operator!=(const ServerId & rhs) const;
	bool operator<(const ServerId & rhs) const;

	static ServerId s_invalid;

private:
	unsigned char m_value;
};

inline ServerId::ServerId(unsigned char value) :
	m_value(value)
{
}

inline ServerId::ServerId() :
	m_value(0)
{
}

inline void ServerId::SetValue(unsigned char value)
{
	m_value = value;
}

inline bool ServerId::IsValid() const
{
	return (m_value != 0);
}

inline void ServerId::SetInvalid()
{
	m_value = 0;
}

inline unsigned char ServerId::GetValueChar() const
{
	return m_value;
}

inline bool ServerId::operator==(const ServerId & rhs) const
{
	return m_value == rhs.m_value;
}

inline bool ServerId::operator!=(const ServerId & rhs) const
{
	return m_value != rhs.m_value;
}

inline bool ServerId::operator<(const ServerId & rhs) const
{
	return m_value < rhs.m_value;
}



#endif