#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

namespace message_dump {

    std::string Trim(std::string str);

    struct indent {
        int level;
        indent(int l) : level(l) {}
        friend std::ostream& operator<<(std::ostream& os, const indent& ind); 
    };

    std::ostream& operator<<(std::ostream& os, const indent& ind);

    class MessageDump
    {
        struct DumpValue
        {
            std::string name;
            void SetName(std::string n) { name = n; }

            int16_t GetShort(char*& ptr) { int16_t res = *(int16_t*)ptr; ptr += 2; return res; }
            int32_t GetInt(char*& ptr) { int32_t res = *(int32_t*)ptr; ptr += 4; return res; }
            std::string GetString(char*& ptr, int len) {
                char buffer[2048];
                size_t actual = len;
                if (actual >= sizeof(buffer)) actual = sizeof(buffer) - 1;
                if (actual > 0) memcpy(&buffer[0], ptr, actual);
                buffer[actual] = '\0';
                ptr += len;
                return buffer;
            }

            virtual void operator()(char*& ptr, int level) = 0;

            virtual ~DumpValue() {}
        };

        struct DumpBoolean : DumpValue
        {
            void operator()(char*& ptr, int level) override
            {
                std::cout << indent(level) << name << " <boolean>: " << (*ptr ? "true" : "false") << "\n";
                ptr += 1;
            }
        };
        struct DumpVectorBoolean : DumpValue
        {
            void operator()(char*& ptr, int level) override
            {
                int len = GetInt(ptr);
                if (len == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                for (int i = 0; i < len; ++i) {
                    std::cout << indent(level) << name << "[" << i << "] <boolean>: " << (*ptr ? "true" : "false") << "\n";
                    ptr += 1;
                }
            }
        };

        template <typename T, typename P = T>
        struct DumpV : DumpValue
        {
            void operator()(char*& ptr, int level) override
            {
                std::cout << indent(level) << name << " <" << sizeof(T) << ">: " << (P)*(T*)ptr << "\n";
                ptr += sizeof(T);
            }
        };
        using DumpByte   = DumpV<int8_t, int>;
        using DumpShort  = DumpV<int16_t>;
        using DumpInt    = DumpV<int32_t>;
        using DumpLong   = DumpV<int64_t>;
        using DumpFloat  = DumpV<float>;
        using DumpDouble = DumpV<double>;

        template <typename T, typename P = T>
        struct DumpVector : DumpValue
        {
            void operator()(char*& ptr, int level) override
            {
                int len = GetInt(ptr);
                if (len == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                for (int i = 0; i < len; ++i) {
                    std::cout << indent(level) << name << "[" << i << "] <" << sizeof(T) << ">: " << (P)*(T*)ptr << "\n";
                    ptr += sizeof(T);
                }
            }
        };
        using DumpVectorByte = DumpVector<int8_t, int>;
        using DumpVectorShort = DumpVector<int16_t>;
        using DumpVectorInt = DumpVector<int32_t>;
        using DumpVectorLong = DumpVector<int64_t>;
        using DumpVectorFloat = DumpVector<float>;
        using DumpVectorDouble = DumpVector<double>;

        struct DumpString : DumpValue
        {
            void operator()(char*& ptr, int level) override
            {
                int len = GetInt(ptr);
                std::cout << indent(level) << name << " <string[" << len << "]>: " << GetString(ptr, len) << "\n";
            }
        };
        struct DumpVectorString : DumpValue
        {
            void operator()(char*& ptr, int level) override
            {
                int len1 = GetInt(ptr);
                if (len1 == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                for (int i = 0; i < len1; ++i) {
                    int len = GetInt(ptr);
                    std::cout << indent(level) << name << "[" << i << "] <string[" << len << "]>: " << GetString(ptr, len) << "\n";
                }
            }
        };

        struct DumpObject : DumpValue
        {
            std::vector<DumpValue*> fields;
            DumpValue* base = nullptr;
            std::string extends;
            void Add(DumpValue* dmp) { fields.push_back(dmp); }
            void SetExtends(std::string n) { extends = n; }
            void operator()(char*& ptr, int level) override
            {
                if (base != nullptr) { base->operator()(ptr, level); }
                // Skip ops.OPSOBject if on top level, since it is already handled when calling Dump()
                if ((level > 0) || (name != "ops.OPSObject")) {
                    std::cout << indent(level + 1) << "<<<" << name << ">>>\n";
                    for (auto& dmp : fields) { dmp->operator()(ptr, level + 1); }
                }
            }
        };
        struct DumpEnum : DumpValue
        {
            std::vector<std::string> enumvalues;
            void AddEnum(std::string value) { enumvalues.push_back(value); }
        };
        struct DumpEnumShort : DumpEnum
        {
            void operator()(char*& ptr, int level) override
            {
                int16_t val = GetShort(ptr);
                if ((size_t)val >= enumvalues.size()) {
                    std::cout << indent(level) << name << " <2>: UNKNOWN enum value\n";
                } else {
                    std::cout << indent(level) << name << " <2>: " << enumvalues[val] << "\n";
                }
            }
        };
        struct DumpVectorEnumShort : DumpValue
        {
            DumpEnumShort* dmp;
            DumpVectorEnumShort(DumpEnumShort* d) : dmp(d) {}
            void operator()(char*& ptr, int level) override
            {
                int len = GetInt(ptr);
                if (len == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                for (int i = 0; i < len; ++i) {
                    int16_t val = GetShort(ptr);
                    if ((size_t)val >= dmp->enumvalues.size()) {
                        std::cout << indent(level) << name << "[" << i << "] <2>: UNKNOWN enum value\n";
                    }
                    else {
                        std::cout << indent(level) << name << "[" << i << "] <2>: " << dmp->enumvalues[val] << "\n";
                    }
                }
            }
        };

        struct DumpFieldObject : DumpValue
        {
            std::map<std::string, DumpObject*>& objs;
            bool isvirtual = false;
            std::string fieldtype;
            DumpFieldObject(bool isvirt, std::string fname, std::map<std::string, DumpObject*>& o) : 
                objs(o), isvirtual(isvirt), fieldtype(fname) {}
            void operator()(char*& ptr, int level) override
            {
                // Get typestring
                int len = GetInt(ptr);
                if (isvirtual) {
                    fieldtype = Trim(GetString(ptr, len));
                } else {
                    ptr += len;
                }
                
                // Dump object
                auto search = objs.find(fieldtype);
                if (search != objs.end()) {
                    DumpObject* dmp = search->second;
                    std::cout << indent(level) << name << ":\n";
                    dmp->operator()(ptr, level + 1);
                } else {
                    std::cout << "FAILED to find type: '" << fieldtype << "'\n";
                }
            }
        };
        struct DumpFieldObjectVector : DumpValue
        {
            DumpFieldObject obj;
            DumpFieldObjectVector(bool isvirt, std::string fname, std::map<std::string, DumpObject*>& o) :
                obj(isvirt, fname, o) {}
            void operator()(char*& ptr, int level) override
            {
                int len = GetInt(ptr);
                if (len == 0) {
                    std::cout << indent(level) << name << "[ size == 0 ]\n";
                }
                for (int i = 0; i < len; ++i) {
                    std::cout << indent(level) << name << "[" << i << "]";
                    obj.operator()(ptr, level + 1);
                }
            }
        };

        std::map<std::string, DumpEnumShort*> enums;
        std::map<std::string, DumpObject*> objs;

        void CreateEnums(pt::ptree& pt)
        {
            std::string objtype, basetype;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "type") {
                    objtype = pos->second.data();
                } else if (pos->first == "basetype") {
                    basetype = pos->second.data();
                } else if (pos->first == "enum_definition") {
                    //std::cout << objtype << "\n";
                    //std::cout << "  basetype: " << basetype << "\n";
                    if (basetype != "short") {
                        std::cout << "  UNKNOWN enum of size \"" << basetype << "\"\n";
                    }
                    DumpEnumShort* dmp = new DumpEnumShort();
                    enums[objtype] = dmp;
                    dmp->SetName(objtype);
                    AddEnums(pos->second, dmp);
                }
            }
        }

        void CreateDumper(pt::ptree& pt)
        {
            std::string objtype, basetype;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "type") {
                    objtype = pos->second.data();
                } else if (pos->first == "extends") {
                    basetype = pos->second.data();
                } else if (pos->first == "basetype") {
                    // Skip
                } else if (pos->first == "an_idea") {
                    // Skip
                } else if (pos->first == "ops_internals") {
                    // Skip
                } else if (pos->first == "constants") {
                    // Skip
                } else if (pos->first == "desc") {
                    // Just a comment, skip
                } else if (pos->first == "fields") {
                    //std::cout << objtype << "\n";
                    //std::cout << "  extends: " << basetype << "\n";
                    DumpObject* dmp = new DumpObject();
                    objs[objtype] = dmp;
                    dmp->SetName(objtype);
                    dmp->SetExtends(basetype);
                    AddFields(pos->second, dmp);
                } else if (pos->first == "enum") {
                    //std::cout << objtype << "\n";
                    //std::cout << "  extends: " << basetype << "\n";
                    DumpObject* dmp = new DumpObject();
                    objs[objtype] = dmp;
                    dmp->SetName(objtype);
                    dmp->SetExtends(basetype);
                    DumpValue* val = new DumpInt();
                    val->SetName("enum");
                    dmp->Add(val);
                } else if (pos->first == "enum_definition") {
                    // Skip
                } else {
                    std::cout << "  UNKNOWN \"" << pos->first << "\"\n";
                }
            }
        }

        void AddEnums(pt::ptree& pt, DumpEnumShort* dmp)
        {
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                AddEnum(pos->second, dmp);
            }
        }

        void AddEnum(pt::ptree& pt, DumpEnumShort* dmp)
        {
            std::string enumname, enumvalue;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "enum") {
                    enumname = pos->second.data();
                }
                else if (pos->first == "value") {
                    enumvalue = pos->second.data();
                }
                else {
                    std::cout << "  UNKNOWN \"" << pos->first << "\"\n";
                }
            }
            dmp->AddEnum(enumname);
        }

        void AddFields(pt::ptree& pt, DumpObject* dmp)
        {
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                AddField(pos->second, dmp);
            }
        }

        void AddField(pt::ptree& pt, DumpObject* dmp)
        {
            std::string fieldname, fieldtype, elementtype;
            for (pt::ptree::iterator pos = pt.begin(); pos != pt.end(); ++pos) {
                if (pos->first == "name") {
                    fieldname = pos->second.data();
                } else if (pos->first == "type") {
                    fieldtype = pos->second.data();
                } else if (pos->first == "desc") {
                    // Just a comment, skip
                } else if (pos->first == "elementtype") {
                    elementtype = pos->second.data();
                } else {
                    std::cout << "  UNKNOWN \"" << pos->first << "\"\n";
                }
            }
            
            DumpValue* val = nullptr;
            if      (fieldtype == "boolean")   { val = new DumpBoolean(); }
            else if (fieldtype == "byte")      { val = new DumpByte(); }
            else if (fieldtype == "short")     { val = new DumpShort(); }
            else if (fieldtype == "int"   )    { val = new DumpInt(); }
            else if (fieldtype == "long"  )    { val = new DumpLong(); }
            else if (fieldtype == "float")     { val = new DumpFloat(); }
            else if (fieldtype == "double")    { val = new DumpDouble(); }
            else if (fieldtype == "string")    { val = new DumpString(); }
            else if (fieldtype == "vector<T>") { 
                if      (elementtype == "boolean") { val = new DumpVectorBoolean(); }
                else if (elementtype == "byte")    { val = new DumpVectorByte(); }
                else if (elementtype == "short")   { val = new DumpVectorShort(); }
                else if (elementtype == "int")     { val = new DumpVectorInt(); }
                else if (elementtype == "long")    { val = new DumpVectorLong(); }
                else if (elementtype == "float")   { val = new DumpVectorFloat(); }
                else if (elementtype == "double")  { val = new DumpVectorDouble(); }
                else if (elementtype == "string")  { val = new DumpVectorString(); }
                else {
                    bool isvirt = false;
                    auto pos = elementtype.find("virtual");
                    if (pos != elementtype.npos) {
                        isvirt = true;
                        elementtype = elementtype.substr(pos + 8);
                    }

                    //std::cout << "  FieldObject, name: '" << fieldname << "', type: '" << elementtype << "'\n";
                    auto search = enums.find(elementtype);
                    if (search != enums.end()) {
                        DumpEnumShort* dmp = search->second;
                        val = new DumpVectorEnumShort(dmp);
                    } else {
                        val = new DumpFieldObjectVector(isvirt, elementtype, objs);
                    }
                }
            } else { 
                bool isvirt = false;
                auto pos = fieldtype.find("virtual");
                if (pos != fieldtype.npos) {
                    isvirt = true;
                    fieldtype = fieldtype.substr(pos + 8);
                }
                //std::cout << "  FieldObject, name: '" << fieldname << "', type: '" << fieldtype << "'\n";
                auto search = enums.find(fieldtype);
                if (search != enums.end()) {
                    DumpEnumShort* dmp = search->second;
                    val = new DumpEnumShort(*dmp);
                } else {
                    val = new DumpFieldObject(isvirt, fieldtype, objs);
                }
            }

            if (val) {
                val->SetName(fieldname);
                dmp->Add(val);
            }
        }

        void ResolveExtends(DumpObject* dmp)
        {
            if (dmp->extends != "") {
                // Look up base
                auto search = objs.find(dmp->extends);
                if (search != objs.end()) {
                    DumpObject* base = search->second;
                    dmp->base = base;
                    dmp->extends = "";
                }
            }
        }

        void ResolveExtends()
        {
            for (auto& o : objs) {
                ResolveExtends(o.second);
            }
        }

        void printTree(pt::ptree& pt, int level) {
            if (pt.empty()) {
                std::cerr << "\"" << pt.data() << "\"";

            } else {
                if (level) std::cerr << std::endl;

                std::cerr << indent(level) << "{" << std::endl;

                for (pt::ptree::iterator pos = pt.begin(); pos != pt.end();) {
                    std::cerr << indent(level + 1) << "\"" << pos->first << "\": ";

                    printTree(pos->second, level + 1);
                    ++pos;
                    if (pos != pt.end()) {
                        std::cerr << ",";
                    }
                    std::cerr << std::endl;
                }

                std::cerr << indent(level) << "}";
            }
        }

    public:
        MessageDump() = default;

        void AddDefinitions(std::string filename)
        {
            // Create empty property tree object
            pt::ptree tree;
            pt::read_json(filename, tree);
            //printTree(tree, 1);

            // Find all enums first
            for (pt::ptree::iterator pos = tree.begin(); pos != tree.end(); ++pos) {
                CreateEnums(pos->second);
            }
            for (pt::ptree::iterator pos = tree.begin(); pos != tree.end(); ++pos) {
                CreateDumper(pos->second);
            }

            ResolveExtends();

            std::cout << "\n\n";
        }

        bool Any() { return objs.size() > 0; }

        void Dump(std::string tname, char* ptr)
        {
            auto search = objs.find(tname);
            if (search != objs.end()) {
                DumpObject* dmp = search->second;
                dmp->operator()(ptr, 0);
            }
        }

    };

}