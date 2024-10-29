#pragma once
#include "ShellNVSE.h"

#include <list>
#include <variant>

namespace FormTraits {

    std::unordered_map <UInt32, std::unordered_map<std::string, AuxVector>> formTraits;
    std::unordered_map <UInt32, std::unordered_map<std::string, AuxVector>> formTraitsSave;

    inline auto& getTraitMap(const std::string& trait) {
        return (trait.front() == '*') ? formTraitsSave : formTraits;
    }

    AuxVector* setAuxVector(UInt32 refID, const std::string& trait) {
        auto& traitsMap = getTraitMap(trait);  // Determine the correct map
        return &traitsMap[refID][trait];
    }

    AuxVector* findAuxVector(UInt32 refID, const std::string& trait) {

        auto& traitsMap = getTraitMap(trait);  // Determine the correct map

        auto traits = traitsMap.find(refID);
        if (traits != traitsMap.end()) {
            auto aux = traits->second.find(trait);
            if (aux != traits->second.end()) {
                return &(aux->second);
            }
        }
        return nullptr;
    }

    void eraseAuxFromVector(UInt32 refID, const std::string& trait, UInt32 index, bool collapse) {

        auto& refMap = getTraitMap(trait);
        auto refIDIt = refMap.find(refID);

        if (refIDIt == refMap.end()) return;  // Exit early if refID is not found

        auto& traitMap = refIDIt->second;
        auto traitIt = traitMap.find(trait);

        if (traitIt == traitMap.end()) return;  // Exit early if trait is not found

        AuxVector& aux = traitIt->second;

        if (index == static_cast<UInt32>(-1)) {
            // Erase the entire trait
            traitMap.erase(traitIt);
        }
        else {
            // Erase or clear the specific index
            if (collapse) {
                aux.eraseIndex(index);
            }
            else {
                aux.ClearIndex(index);
                aux.Cleanup(); // Clean up trailing empty values
            }

            if (aux.empty()) {
                traitMap.erase(traitIt);
            }
        }

        // Remove the refID entry if no traits are left
        if (traitMap.empty()) {
            refMap.erase(refIDIt);
        }
    }

    void eraseAll(UInt32 refID) {
        auto traits = formTraits.find(refID);
        if (traits != formTraits.end()) {
            formTraits.erase(refID);
        }
        traits = formTraitsSave.find(refID);
        if (traits != formTraitsSave.end()) {
            formTraitsSave.erase(refID);
        }
    }

}

    void AuxValue::SetValue() {
        this->CleanUpUnion();
        type = -1;
        num = 0;
    }

    void AuxValue::SetValue(double value) {
        this->CleanUpUnion();
        type = kRetnType_Default;
        num = value;
    }
    void AuxValue::SetValue(Float32 value) {
        this->CleanUpUnion();
        type = kRetnType_Float;
        fNum = value;
    }
    void AuxValue::SetValue(UInt32 value) {
        this->CleanUpUnion();
        type = kRetnType_Form;
        refID = value;
    }
    void AuxValue::SetValue(const char* value) {
        this->CleanUpUnion();
        type = kRetnType_String;
        if (value) {
            str = strdup(value);
        }
        else {
            str = nullptr;
        }
    }

    void AuxValue::SetValue(const std::string& value) {
        this->CleanUpUnion();
        type = kRetnType_String;
        str = strdup(value.c_str());
    }

    void AuxValue::SetValue(std::string&& value) {
        this->CleanUpUnion();
        type = kRetnType_String;
        str = strdup(value.c_str());
    }

    void AuxValue::SetValue(NVSEArrayVarInterface::Array* value) {
        this->CleanUpUnion();
        type = kRetnType_Array;
        NVSEArray = CopyFromNVSEArray(value);
    }

    UInt32* AuxValue::bitCast(Script* script) const {

        switch (type) {
        case kRetnType_String: {
            return std::bit_cast<UInt32*>(str);
        }
        case kRetnType_Default: {
            float floatValue = static_cast<float>(num);
            return std::bit_cast<UInt32*>(floatValue);
        }
        case kRetnType_Form: {
            TESForm* value = LookupFormByRefID(refID);
            return std::bit_cast<UInt32*>(value);
        }
        case kRetnType_Array:
            return std::bit_cast<UInt32*>(CopyToNVSEArray(script));
        }

        return 0;

    }

    bool AuxValue::Compare(double value) const {
        return GetDouble() == value;
    }
    bool AuxValue::Compare(UInt32 value) const {
        return GetRef() == value;
    }
    bool AuxValue::Compare(const char* value) const {
        return (GetType() == kRetnType_String && str && strcmp(str, value) == 0);
    }
    bool AuxValue::Compare(NVSEArrayVarInterface::Array* value) const {
        //return GetArray() == value;
        return false;
    }

    double AuxValue::GetValue(double value) {
        return GetDouble();
    }
    Float32 AuxValue::GetValue(Float32 value) {
        return GetFlt();
    }

    UInt32 AuxValue::GetValue(UInt32 value) {
        return GetRef();
    }

    const char* AuxValue::GetValue(const char* value) {
        return GetStr();
    }

    AuxVector* AuxValue::GetValue(NVSEArrayVarInterface::Array* value) {
        return GetArray();
    }

    ArrayElementL AuxValue::GetAsElement() const
    {
        if (type == kRetnType_Form) return ArrayElementL(LookupFormByRefID(refID));
        if (type == kRetnType_String) return ArrayElementL(GetStr());
        return ArrayElementL(num);
    }

    AuxVector* AuxValue::CopyFromNVSEArray(NVSEArrayVarInterface::Array* ToCopy) {
        if (!ToCopy)
            return nullptr;

        const auto& arrData = ArrayData(ToCopy, true);
        int iSize = g_arrInterface->GetArraySize(ToCopy);
        if (iSize <= 0)
            return nullptr;

        AuxVector* valuesList = new AuxVector();
        valuesList->reserve(iSize);

        for (UInt32 idx = 0; idx < arrData.size; idx++) {
            switch (arrData.vals[idx].type) {
            case NVSEArrayVarInterface::kType_Numeric:
                valuesList->emplace_back(arrData.vals[idx].Number());
                break;
            case NVSEArrayVarInterface::kType_Form:
                if (auto temp = arrData.vals[idx].Form()) {
                    valuesList->emplace_back(temp->refID);
                }
                break;
            case NVSEArrayVarInterface::kType_String:
                if (auto temp = arrData.vals[idx].String()) {
                    valuesList->emplace_back(temp);
                }
                break;
            case NVSEArrayVarInterface::kType_Array:
                if (auto temp = arrData.vals[idx].Array()) {
                    valuesList->emplace_back(temp);
                }
                break;
            }
        }

        return valuesList;
    }

    NVSEArrayVarInterface::Array* AuxValue::CopyToNVSEArray(Script* script) const {

        AuxVector* valuesList = NVSEArray;
        NVSEArrayVarInterface::Array* resultArray = g_arrInterface->CreateArray(nullptr, 0, script);

        for (AuxValue& value : *valuesList) {
            switch (value.type) {
            case kRetnType_Default:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)value.num);
                break;
            case kRetnType_Form:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)LookupFormByRefID(value.refID));
                break;
            case kRetnType_String:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)value.str);
                break;
            case kRetnType_Array:
                g_arrInterface->AppendElement(resultArray, (ArrayElementL)value.CopyToNVSEArray(script));
                break;
            }
        }

        return resultArray;
    }

    void AuxValue::CleanUpUnion()
    {
        switch (type)
        {
        case kRetnType_String:
            if (str)
            {
                free(str);
                str = nullptr;
            }
            break;
        case kRetnType_Array:
            if (NVSEArray)
            {
                delete NVSEArray;
                NVSEArray = nullptr;
            }
            break;
        }
    }

    AuxValue::AuxValue()
        :
        type(-1),
        num(0) {}

    AuxValue::AuxValue(
        double num)
        :
        type(kRetnType_Default),
        num(num) {}

    AuxValue::AuxValue(
        Float32 fNum)
        :
        type(kRetnType_Float),
        num(fNum) {}

    AuxValue::AuxValue(
        UInt32 refID)
        :
        type(kRetnType_Form),
        refID(refID) {}

    AuxValue::AuxValue(
        TESForm* form)
        :
        type(kRetnType_Form),
        refID(form->refID) {}

    AuxValue::AuxValue(
        const char* value)
        : type(kRetnType_String),
        str(nullptr) {

        if (value) {
            str = strdup(value);
        }

    }

    AuxValue::AuxValue(std::string&& value)
        : type(kRetnType_String), str(nullptr)
    {
        if (!value.empty()) {
            str = strdup(value.c_str());
        }
    }

    AuxValue::AuxValue(
        NVSEArrayVarInterface::Array* ToCopy)
        :
        type(kRetnType_Array),
        NVSEArray(nullptr) {
        NVSEArray = CopyFromNVSEArray(ToCopy);
    };

    AuxValue::AuxValue(const AuxValue& other) : type(other.type) {  //Copy constructor
        switch (type) {
        case kRetnType_Default:
            num = other.num;
            break;
        case kRetnType_Form:
            refID = other.refID;
            break;
        case kRetnType_String:
            if (other.str) {
                str = strdup(other.str);
            }
            else {
                str = nullptr;
            }
            break;
        case kRetnType_Array:
            if (other.NVSEArray) {
                NVSEArray = new AuxVector(*other.NVSEArray);
            }
            else {
                NVSEArray = nullptr;
            }
            break;
        }
    }

    AuxValue& AuxValue::operator=(const AuxValue& other) {
        if (this != &other) {
            CleanUpUnion(); // Clean up existing resources
            type = other.type;
            switch (type) {
            case kRetnType_Default:
                num = other.num;
                break;
            case kRetnType_Form:
                refID = other.refID;
                break;
            case kRetnType_String:
                if (other.str) {
                    str = strdup(other.str);
                }
                else {
                    str = nullptr;
                }
                break;
            case kRetnType_Array:
                if (other.NVSEArray) {
                    NVSEArray = new AuxVector(*other.NVSEArray);
                }
                else {
                    NVSEArray = nullptr;
                }
                break;
            case kRetnType_Float:
                fNum = other.fNum;
            }
        }
        return *this;
    }

    bool AuxValue::operator==(const AuxValue& rhs) const {
        if (type != rhs.type) return false; // Different types cannot be equal

        switch (type) {
        case kRetnType_Default:
            return num == rhs.num;
        case kRetnType_Form:
            return refID == rhs.refID;
        case kRetnType_String:
            return strcmp(str, rhs.str) == 0;
        case kRetnType_Array:
            return NVSEArray == rhs.NVSEArray; // Simple pointer comparison, or deeper if needed
        case kRetnType_Float:
            return fNum == rhs.fNum;
        default:
            return false;
        }
    }