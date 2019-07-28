#ifndef INCLUDED_proXmlParser_H_
#define INCLUDED_proXmlParser_H_

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlMap.h"
#include "arda2/util/utlRefCount.h"

class XMLDocument;

class XMLNode : public utlRefCount {
public:
    typedef std::map<std::string, std::string> AttributeMap;

    enum NodeType { ROOT, ELEMENT, TEXT };

    XMLNode(NodeType type);
    ~XMLNode();

    NodeType Type() const { return type; }
    const std::string& Value() const { return value; }

    XMLNode* Clone(bool deep) const;

    // Attributes
    const AttributeMap& Attributes() const { return attributes; }

    bool HasAttribute(const char* name) const;
    const char* Attribute(const char* name) const;

    void RemoveAttribute(const char* name);

    void SetAttribute(const char* name, const char* value);
    void SetAttributes(const AttributeMap& newAttributes);

    // Hierarchy navigation
    XMLNode* Parent() const { return parent; }
    XMLNode* FirstChild() const { return first; }
    XMLNode* FirstChildElement() const;
    XMLNode* LastChild() const { return last; }
    XMLNode* PrevSibling() const { return prev; }
    XMLNode* NextSibling() const { return next; }
    XMLNode* NextSiblingElement() const;

    void SetLoaded(bool value);
    bool GetLoaded();

protected:
    virtual void Grip() const {}
    virtual void Ungrip() const;

private:
    friend class XMLDocument;
    NodeType type;
    std::string value;
    AttributeMap attributes;
    bool m_loaded;

    XMLNode* parent;
    XMLNode* first;
    XMLNode* last;
    XMLNode* prev;
    XMLNode* next;

    void AddChild(XMLNode* child);

    // Parsing
    void ParseDocument(const char* p, XMLDocument& doc);
    const char* IdentifyAndParse(const char* p, XMLDocument& doc);
    const char* ParseElement(const char* p, XMLDocument& doc);
    const char* ParseContents(const char* p, XMLDocument& doc);
    const char* IgnoreData(const char* p, XMLDocument& doc);
};

class XMLDocument {
public:
    XMLDocument();

    bool Parse(const char* filename);
    const char* Error() { return error.c_str(); }

    XMLNode* Node() { return &root; }

    void ParseDocument(const char* p);

private:
    friend class XMLNode;

    void SetError(const char* pos, const char* msg);

    XMLNode root;
    char* file;
    std::string error;
    const char* errorPos;
    const char* errorMsg;

    // disable default copy constructor and assignment operator
    XMLDocument(const XMLDocument&);
    void operator=(const XMLDocument&);
};

#endif // INCLUDED_proXmlParser_H_
