#pragma once

struct AuxVector;

struct AuxValue
{
    SInt8		type;
    union
    {
        double	num;
        UInt32	refID;
        char* str;                //Stop trying to convert these to std::string you lazy fuck.
        AuxVector* NVSEArray;
        Float32 fNum; //Used to prevent CallLoop conversions
    };

    AuxValue();
    AuxValue(double num);
    AuxValue(Float32 fNum);
    AuxValue(UInt32 refID);
    AuxValue(TESForm* form);
    AuxValue(const char* value);
    AuxValue(std::string&& value);
    AuxValue(NVSEArrayVarInterface::Array* ToCopy);

    //Copy constructor:
    AuxValue(const AuxValue& other);

    //Operator overloads
    AuxValue& operator=(const AuxValue& other);
    bool operator==(const AuxValue& rhs) const;

    ~AuxValue()
    {
        CleanUpUnion();
    }

    AuxVector* CopyFromNVSEArray(NVSEArrayVarInterface::Array* ToCopy);
    NVSEArrayVarInterface::Array* CopyToNVSEArray(Script* resultArray) const;

    bool IsEmpty() const { return type == -1; }
    UInt8 GetType() const { return type; }
    Float32 GetFlt() const { return (type == kRetnType_Float) ? fNum : 0; }
    double GetDouble() const { return (type == kRetnType_Default) ? num : 0; }
    UInt32 GetRef() const { return (type == kRetnType_Form) ? refID : 0; }
    const char* GetStr() const { return (type == kRetnType_String) ? str : nullptr; }
    AuxVector* GetArray() const { return (type == kRetnType_Array) ? NVSEArray : nullptr; }

    void SetValue();
    void SetValue(double value);
    void SetValue(Float32 value);
    void SetValue(UInt32 value);
    void SetValue(const char* value);
    void SetValue(NVSEArrayVarInterface::Array* value);
    void SetValue(const std::string& value);
    void SetValue(std::string&& value);

    UInt32* bitCast(Script* script) const; //Used for pushing unknown arguments to CallFunction
    double GetValue(double value);
    Float32 GetValue(Float32 value);
    UInt32 GetValue(UInt32 value);
    const char* GetValue(const char* value);
    AuxVector* GetValue(NVSEArrayVarInterface::Array* value);

    bool Compare(double value) const;
    bool Compare(UInt32 value) const;
    bool Compare(const char* value) const;
    bool Compare(NVSEArrayVarInterface::Array* value) const;

    ArrayElementL GetAsElement() const;

    void CleanUpUnion();

    struct Hash {
        size_t operator()(const AuxValue& value) const {
            size_t hash = 17;
            switch (value.type) {
            case kRetnType_Default:
                hash = hash * 31 + std::hash<double>()(value.num);
                break;
            case kRetnType_Float:
                hash = hash * 31 + std::hash<Float32>()(value.fNum);
                break;
            case kRetnType_Form:
                hash = hash * 31 + std::hash<UInt32>()(value.refID);
                break;
            case kRetnType_String:
                if (value.str) {
                    std::string strValue(value.str);
                    hash = hash * 31 + std::hash<std::string>()(strValue);
                }
                break;
            case kRetnType_Array:
                hash = hash * 31 + std::hash<void*>()(value.NVSEArray);
                break;
            default:
                break;
            }
            return hash;
        }
    };

};


struct AuxVector : public std::vector<AuxValue> {

    using std::vector<AuxValue>::vector;

    template<class T>
    void AddValue(int index, T value) {
        if (index < 0) {
            push_back(AuxValue{ value });
        }
        else {
            if (index >= size()) {
                resize(index + 1);
            }
            (*this)[index].SetValue(value);
        }
    }

    void AddValue(int index, std::string&& value) {
        if (index < 0) {
            push_back(AuxValue{ std::move(value) });
        }
        else {
            if (index >= size()) {
                resize(index + 1);
            }
            (*this)[index].SetValue(std::move(value));
        }
    }


    void AddEmptyValue(int index) {
        if (index < 0) {
            push_back(AuxValue{});
        }
        else {
            if (index >= size()) {
                resize(index + 1);
            }
            (*this)[index].SetValue();
        }
    }

    void eraseIndex(int index) {
        if (index >= 0 && index < this->size()) {
            this->erase(this->begin() + index);
        }
    }

    void ClearIndex(int index) {
        if (index >= 0 && index < size()) {
            if (index == size() - 1) {
                pop_back();
            }
            else {
                (*this)[index].SetValue(); //Sets to empty
            }
        }
    }

    void Cleanup() {
        while (!empty() && back().IsEmpty()) {
            pop_back();
        }
    }

    template<class T>
    SInt32 Find(T value) {

        for (size_t index = 0; index < this->size(); ++index) {
            if ((*this)[index].Compare(value)) {
                return index;
            }
        }
        return -1; // Return a special value to indicate that the element was not found
    }

    void Dump() const {
        for (size_t index = 0; index < this->size(); ++index) {

            switch ((*this)[index].GetType()) {
            case kRetnType_Default:
            {
                Console_Print("[%d] %d", index, (*this)[index].GetDouble());
                break;
            }
            case kRetnType_Form:
            {
                if (TESForm* form = LookupFormByRefID((*this)[index].GetRef())) {
                    Console_Print("[%d] %s", index, form->GetTheName());
                }
                break;
            }
            case kRetnType_String:
            {
                Console_Print("[%d] %s", index, (*this)[index].GetStr());
                break;
            }
            case kRetnType_Array:
            {
                //Console_Print("[%d] %d", index, (*this)[index].GetArray());
                break;
            }
            }

        }
    }

    bool operator==(const AuxVector& rhs) const {
        if (this->size() != rhs.size()) return false;
        for (size_t i = 0; i < this->size(); ++i) {
            if (!((*this)[i] == rhs[i])) return false;
        }
        return true;
    }

    size_t hash() const {
        size_t hash = 17;
        AuxValue::Hash valueHasher;
        for (const AuxValue& value : *this) {
            hash = hash * 31 + valueHasher(value);
        }
        return hash;
    }

};