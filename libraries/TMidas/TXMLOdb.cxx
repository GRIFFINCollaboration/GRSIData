#ifdef HAS_XML

#include <stdexcept>

#include "TXMLOdb.h"

#include "TList.h"
#include "TXMLAttr.h"

/// \cond CLASSIMP
ClassImp(TXMLOdb)
/// \endcond

std::array<char, 256> TXMLOdb::fTextBuffer;

TXMLOdb::TXMLOdb(char* buffer, int size)
   : fDoc(nullptr), fParser(new TDOMParser), fOdb(nullptr)
{
	/// Creator, tries to open buffer as input file and parse it, if that fails, parses size bytes of the buffer.
	
   std::ifstream input;
   input.open(buffer);
   fParser->SetValidate(false);
   if(input.is_open()) {
      fParser->ParseFile(buffer);
   } else {
      fParser->ParseBuffer(buffer, size);
   }
   fDoc = fParser->GetXMLDocument();
   if(fDoc == nullptr) {
		std::runtime_error("XmlOdb::XmlOdb: Malformed ODB dump: cannot get XML document");
   }
   fOdb = fDoc->GetRootNode();
   if(strcmp(fOdb->GetNodeName(), "odb") != 0) {
      std::runtime_error("XmlOdb::XmlOdb: Malformed ODB dump: cannot find <odb> tag");
   }
}

TXMLOdb::~TXMLOdb()
{
	/// Default destructor, deletes the parser.
   if(fParser != nullptr) {
      delete fParser;
   }
}

TXMLNode* TXMLOdb::FindNode(const char* name, TXMLNode* node)
{
	/// Finds node with name "name". If a node is provided this node will be used as a starting point.
	/// If the provided node is a null pointer fOdb is used instead. Returns a null pointer if the 
	/// search fails.
   if(node == nullptr) {
      if(fOdb == nullptr) {
         return nullptr;
      }
      node = fOdb; //->GetChildren();
   }
   if(!node->HasChildren()) {
      return nullptr;
   }
   node = node->GetChildren();
   while(node != nullptr) {
      std::string nodename = GetNodeName(node);
      if(nodename == name) {
         return node;
      }
      node = node->GetNextNode();
   }

   return nullptr;
}

TXMLNode* TXMLOdb::FindPath(const char* path, TXMLNode* node)
{
	/// Find path "path" under the provided node. If the node is a null pointer, fOdb is used instead.
   if(node == nullptr) {
      if(fOdb == nullptr) {
         return nullptr;
      }
      node = fOdb; //->GetChildren();
   }

   std::string              pathname = path;
   std::vector<std::string> elems;
   std::size_t              last  = 0;
   std::size_t              slash = pathname.find_first_of('/');
   if(slash == 0) {
      last = 1;
   }
   while(true) {
      slash = pathname.find_first_of('/', last);
      elems.push_back(pathname.substr(last, slash - last));
      last = slash + 1;
      if(slash == std::string::npos) {
         break;
      }
   }

   for(auto& elem : elems) {
      node = FindNode(elem.c_str(), node);
      if(node != nullptr) {
      } else {
         node = nullptr;
         break;
      }
   }

   return node;
}

const char* TXMLOdb::GetNodeName(TXMLNode* node)
{
	/// Returns the name of a node.
   TList* list = node->GetAttributes();
   if(list != nullptr) {
      std::string buffer = (static_cast<TXMLAttr*>(list->At(0)))->GetValue();
      strlcpy(fTextBuffer.data(), buffer.c_str(), fTextBuffer.size());
      return fTextBuffer.data();
   }
   return "";
}

int TXMLOdb::ReadInt(const char* path, int, int defaultValue)
{
	/// tries to find the path "path", returns defaultValue if that fails, otherwise returns 0.
   TXMLNode* node = FindPath(path);
   if(node == nullptr) {
      return defaultValue;
   }
   return 0;
}

std::vector<int> TXMLOdb::ReadIntArray(TXMLNode* node)
{
	/// Reads and returns an array of integers.
	
   std::vector<int> temp;
   if(node == nullptr) {
      return temp;
   }
   if(!node->HasChildren()) {
      return temp;
   }
   TList* list = node->GetAttributes();
   if(list == nullptr) {
      return temp;
   }
   TIter iter(list);
   int   size = 0;
   while(auto* attr = static_cast<TXMLAttr*>(iter.Next())) {
      if(strcmp(attr->GetName(), "num_values") == 0) {
         size = atoi(attr->GetValue());
      }
   }
   temp.assign(size, 0);
   TXMLNode* child   = node->GetChildren();
   int       counter = 0;
   while(true) {
      if(TList* index = child->GetAttributes()) {
         int indexnum = atoi((static_cast<TXMLAttr*>(index->At(0)))->GetValue());
         int value    = atoi(child->GetText());
         temp.at(indexnum) = value;
      } else if(child->GetText() != nullptr) {
         int indexnum = counter++;
         temp.at(indexnum) = atoi(child->GetText());
      }
      child = child->GetNextNode();
      if(child == nullptr) {
         break;
      }
   }
   return temp;
}

std::vector<std::string> TXMLOdb::ReadStringArray(TXMLNode* node)
{
	/// Reads and returns an array of strings.

   std::vector<std::string> temp;
   if(node == nullptr) {
      return temp;
   }
   if(!node->HasChildren()) {
      return temp;
   }
   TList* list = node->GetAttributes();
   if(list == nullptr) {
      return temp;
   }
   TIter iter(list);
   int   size = 0;
   while(auto* attr = static_cast<TXMLAttr*>(iter.Next())) {
      if(strcmp(attr->GetName(), "num_values") == 0) {
         size = atoi(attr->GetValue());
      }
   }
   temp.assign(size, "");
   TXMLNode* child   = node->GetChildren();
   int       counter = 0;
   while(true) {
      if(TList* index = child->GetAttributes()) {
         int         indexnum = atoi((static_cast<TXMLAttr*>(index->At(0)))->GetValue());
         const char* value    = child->GetText();

         // Make sure we actually read a word
         std::string value_str;
         if(value == nullptr) {
            value_str = "";
         } else {
            value_str = value;
         }
         temp.at(indexnum) = value_str;
      } else if(child->GetText() != nullptr) {
         int indexnum = counter++;
         temp.at(indexnum).assign(child->GetText());
      }
      child = child->GetNextNode();
      if(child == nullptr) {
         break;
      }
   }
   return temp;
}

std::vector<double> TXMLOdb::ReadDoubleArray(TXMLNode* node)
{
	/// Reads and returns an array of doubles.

   std::vector<double> temp;
   if(node == nullptr) {
      return temp;
   }
   if(!node->HasChildren()) {
      return temp;
   }
   TList* list = node->GetAttributes();
   if(list == nullptr) {
      return temp;
   }
   TIter iter(list);
   int   size = 0;
   while(auto* attr = static_cast<TXMLAttr*>(iter.Next())) {
      if(strcmp(attr->GetName(), "num_values") == 0) {
         size = atoi(attr->GetValue());
      }
   }
   temp.assign(size, 0.0);
   TXMLNode* child   = node->GetChildren();
   int       counter = 0;
   while(true) {
      if(TList* index = child->GetAttributes()) {
         int    indexnum = atoi((static_cast<TXMLAttr*>(index->At(0)))->GetValue());
         double value    = atof(child->GetText());
         temp.at(indexnum) = value;
      } else if(child->GetText() != nullptr) {
         int indexnum      = counter++;
         temp.at(indexnum) = atof(child->GetText());
      }
      child = child->GetNextNode();
      if(child == nullptr) {
         break;
      }
   }
   return temp;
}
#endif
