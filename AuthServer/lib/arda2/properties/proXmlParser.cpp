#include "arda2/core/corFirst.h"
#include "proXmlParser.h"

#include "proStringUtils.h"

using namespace std;

XMLDocument::XMLDocument():
	root(XMLNode::ROOT), errorPos(0)
{
}

void XMLDocument::ParseDocument(const char* p)
{
    root.ParseDocument(p, *this);

	if (errorPos) 
    {
        // find line/column location in file
        int offset = (int) (errorPos - file);
        int line = 1, col = 1;
        for (int i = 0; i < offset; ++i) {
	        char ch = file[i];
	        if (ch == '\r') {
		        if (file[i+1] == '\n') {
			        ++i;
		        }
		        ++line;
		        col = 1;
	        } else if (ch == '\n') {
		        ++line;
		        col = 1;
	        } else {
		        ++col;
	        }
        }

        // construct error message
        char loc[16];
        sprintf(loc, "(%d,%d) : ", line, col);
        error = "text";
        error += loc;
        error += errorMsg;
    }
}

bool XMLDocument::Parse(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		unsigned size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		file = new char[size+1];
		fread(file, 1, size,fp);
		fclose(fp);
		file[size] = '\0';
		
		root.ParseDocument(file, *this);

		if (errorPos) {
            // find line/column location in file
			int offset = (int)(errorPos - file);
			int line = 1, col = 1;
			for (int i = 0; i < offset; ++i) {
				char ch = file[i];
				if (ch == '\r') {
					if (file[i+1] == '\n') {
						++i;
					}
					++line;
					col = 1;
				} else if (ch == '\n') {
					++line;
					col = 1;
				} else {
					++col;
				}
			}

			// construct error message
			char loc[16];
			sprintf(loc, "(%d,%d) : ", line, col);
			error = filename;
			error += loc;
			error += errorMsg;
		}
		delete [] file;
	}

	return errorPos == 0;
}

void XMLDocument::SetError(const char* pos, const char* msg)
{
	errorPos = pos;
	errorMsg = msg;
}

XMLNode::XMLNode(NodeType type):
	type(type), m_loaded(false), parent(0), first(0), last(0), prev(0), next(0)
{
	attributes.clear();
}

XMLNode::~XMLNode()
{
	XMLNode* node = first;
	XMLNode* temp = 0;

	while (node) {
		temp = node;
		node = node->next;
		temp->DecRef();
	}
}

XMLNode* XMLNode::Clone(bool deep) const
{
	XMLNode* clone = new XMLNode(type);
	clone->value = value;

	clone->SetAttributes(attributes);

	for (XMLNode* node = FirstChild(); node; node = node->NextSibling()) {
		clone->AddChild(deep ? node->Clone(deep) : node);
	}
	return clone;
}


bool XMLNode::HasAttribute(const char* name) const
{
	AttributeMap::const_iterator it = attributes.find(name);
	if (it == attributes.end()) {
		return false;
	} else {
		return true;
	}
}

const char* XMLNode::Attribute(const char* name) const
{
	AttributeMap::const_iterator it = attributes.find(name);
	if (it == attributes.end()) {
		return "";
	} else {
		return it->second.c_str();
	}
}
void XMLNode::SetAttribute(const char* name, const char* value)
{
	attributes.insert(make_pair(name, value));
}

void XMLNode::SetAttributes(const AttributeMap& newAttributes)
{
	attributes.insert(newAttributes.begin(), newAttributes.end());
}

void XMLNode::RemoveAttribute(const char* name)
{
	attributes.erase(name);
}


XMLNode* XMLNode::FirstChildElement() const
{
	for (XMLNode* node = FirstChild(); node; node = node->NextSibling()) {
		if (node->Type() == ELEMENT) {
			return node;
		}
	}
	return 0;
}

XMLNode* XMLNode::NextSiblingElement() const
{
	for (XMLNode* node = NextSibling(); node; node = node->NextSibling()) {
		if (node->Type() == ELEMENT) {
			return node;
		}
	}
	return 0;
}

void XMLNode::Ungrip() const
{
	delete this;
}

void XMLNode::AddChild(XMLNode* child)
{
	child->IncRef();
	child->parent = this;
	child->prev = last;
	if (last) {
		last->next = child;
	} else {
		first = child;
	}
	last = child;
}

static const char* SkipWhiteSpace(const char* p)
{
	while (p && *p && (isspace((uchar)*p) || *p == '\n' || *p == '\r'))
		++p;
	return p;
}

static bool ReadName(const char*& p, string* name)
{
	*name = "";
	const char* start = p;

	if (p && (isalpha((uchar)*p) || *p == '_')) {
		p++;
		while(p && *p && (isalnum((uchar)*p) || *p == '_' || *p == '-' || *p == ':'))
			p++;
		name->append(start, p - start);
		return true;
	}
	return false;
}

void XMLNode::ParseDocument(const char* p, XMLDocument& doc)
{
 	p = SkipWhiteSpace(p);
	if (!*p) {
		doc.SetError(p, "Document empty");
	}

	while (p && *p)	{	
		if (*p != '<') {
			doc.SetError(p, "The '<' symbol that starts a tag was not found.");
			break;
		} else {
			p = SkipWhiteSpace(p+1);
            
			if (isalpha((uchar)*p) || *p == '_') {
				p = ParseElement(p, doc);
			} else {
				p = IgnoreData(p, doc);
			}
		}
		p = SkipWhiteSpace(p);
	}
}

const char* XMLNode::IdentifyAndParse(const char* p, XMLDocument& doc)
{
	p = SkipWhiteSpace(p+1);

	if (isalpha((uchar)*p) || *p == '_') {
		// Element
		XMLNode* child = new XMLNode(ELEMENT);
		AddChild(child);

		p = child->ParseElement(p, doc);
	} else {
		p = IgnoreData(p, doc);
	}
	return p;
}

const char* XMLNode::ParseElement(const char* p, XMLDocument& doc)
{
	p = SkipWhiteSpace(p);
	if (!*p) {
		doc.SetError(p, "Error parsing element");
		return 0;
	}

	if (!ReadName(p, &value)) {
		doc.SetError(p, "Error reading element name");
		return 0;
	}

	string endTag = "</";
	endTag += value;
	endTag += ">";

	while (p && *p)	{
		p = SkipWhiteSpace(p);
		if (!*p) {
			doc.SetError(p, "Error reading attributes");
			return 0;
		}
		if (*p == '/') {
			// End of empty tag
			if (*(p+1) != '>') {
				doc.SetError(p, "Error parsing empty tag");
				return 0;
			}
			return p+2;
		} else if (*p == '>') {
			// End of start tag
			p = ParseContents(p+1, doc);
			if (!p) {
				return 0;
			}

			string buf(p, endTag.size());
			if (endTag == buf) {
				return p+endTag.size();
			} else {
				doc.SetError(p, "Error reading end tag");
				return 0;
			}
		} else {
			// Attribute
			string attribName, attribValue;

			if (!ReadName(p, &attribName)) {
				doc.SetError(p, "Error reading attributes");
				return 0;
			}
			p = SkipWhiteSpace(p);
			if (*p != '=') {
				doc.SetError(p, "Error reading attributes");
				return 0;
			}
			p = SkipWhiteSpace(p+1);
			if (!*p) {
				doc.SetError(p, "Error reading attributes");
				return 0;
			}
	
			const char* end = 0;
			const char* start = p+1;
			const char* past = 0;

			if (*p == '\'') {
				end = strchr(start, '\'');
				past = end+1;
			} else if (*p == '"') {
				end = strchr(start, '"');
				past = end+1;
			} else {
				// No quotes - try to parse anyway
				start--;
				for (end = start; *end; end++) {
					if (isspace((uchar)*end) || *end == '/' || *end == '>' || *p == '\n' || *p == '\r') {
						break;
					}
				}
				past = end;
			}
			if (!end) {
				doc.SetError(p, "Error reading attributes");
				return 0;
			}
			attribValue = string(start, end-start);
			p = past;

			attributes.insert(make_pair(attribName, attribValue));
		}
	}
	return 0;
}

const char* XMLNode::ParseContents(const char* p, XMLDocument& doc)
{
	while (p && *p) {
		p = SkipWhiteSpace(p);

		// Save any character data, collapsing sequential whitespace
		string cdata;
		bool whitespace = false;
		while (*p && *p != '<') {
			if (isspace((uchar)*p) || *p == '\n' || *p == '\r') {
				whitespace = true;
			} else {
				if (whitespace) {
					cdata += ' ';
					whitespace = false;
				}
				cdata += *p;
			}
			p++;
		}

		if (!*p) {
			doc.SetError(p, "Error reading element value");
			return 0;
		}

		if (cdata != "") {
			XMLNode *child = new XMLNode(TEXT);
			child->value = cdata;
			AddChild(child);
		} else {
			if (*(p+1) == '/') {
				return p;
			} else {
				p = IdentifyAndParse(p, doc);
			}
		}
	}
	return 0;
}

const char* XMLNode::IgnoreData(const char* p, XMLDocument& doc)
{
	if (p[0] == '!' && p[1] == '-' && p[2] == '-') {
		// Ignore comment
		p += 3;
		while (*p) {
			if (p[0] == '-' && p[1] == '-' && p[2] == '>') {
				p += 3;
				return p;
			}
			++p;
		}
	} else {
		// Ignore declaration or processing instruction
        const char* end = strchr(p, '>');
		if (end) {
			return end + 1;
		}
	}
	doc.SetError(p, "Could not find > while skipping data");
	return 0;
}


void XMLNode::SetLoaded(bool value)
{
    m_loaded = value;
}

bool XMLNode::GetLoaded()
{
    return m_loaded;
}
