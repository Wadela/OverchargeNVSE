#pragma once
#include <CommandTable.h>

// Declare Params with "static", so they are only built once (so this file can be included multiple times).

struct BasicType
{
	UInt8		type;
	union
	{
		double	num;
		UInt32	refID;
		char* str;
	};

	BasicType()
		:
		type(0),
		num(0) {}

	BasicType(
		double num)
		:
		type(1),
		num(num) {}

	BasicType(
		UInt32 refID)
		:
		type(2),
		refID(refID) {}

	BasicType(
		const char* str)
		: type(3),
		str(nullptr) {

		if (str) {
			this->str = new char[strlen(str) + 1];
			strcpy_s(this->str, strlen(str) + 1, str);
		}
	}

	~BasicType()
	{
		if (type == 3 && str) {
			delete[] str;
			str = nullptr;
		}
	}

};

//Had to copy these outside of their main file, since those files had NVSE-exclusive data.
enum Token_Type : UInt8
{
	kTokenType_Number = 0,
	kTokenType_Boolean,
	kTokenType_String,
	kTokenType_Form,
	kTokenType_Ref,
	kTokenType_Global,
	kTokenType_Array,
	kTokenType_ArrayElement,
	kTokenType_Slice,
	kTokenType_Command,
	kTokenType_Variable,
	kTokenType_NumericVar,
	kTokenType_RefVar,
	kTokenType_StringVar,
	kTokenType_ArrayVar,
	kTokenType_Ambiguous,
	kTokenType_Operator,
	kTokenType_ForEachContext,

	// numeric literals can optionally be encoded as one of the following
	// all are converted to _Number on evaluation
	kTokenType_Byte,
	kTokenType_Short, // 2 bytes
	kTokenType_Int,	  // 4 bytes

	kTokenType_Pair,
	kTokenType_AssignableString,
	// xNVSE 6.1.0
	kTokenType_Lambda,
	kTokenType_LambdaScriptData,

	kTokenType_Invalid,
	kTokenType_Max = kTokenType_Invalid,

	// sigil value, returned when an empty expression is parsed
	kTokenType_Empty = kTokenType_Max + 1,
};

// these are used in NVSEParamInfo to specify expected Token_Type of args to commands taking NVSE expressions as args
enum kNVSEParamType {
	kNVSEParamType_Number = (1 << kTokenType_Number) | (1 << kTokenType_Ambiguous),
	kNVSEParamType_Boolean = (1 << kTokenType_Boolean) | (1 << kTokenType_Ambiguous),
	kNVSEParamType_String = (1 << kTokenType_String) | (1 << kTokenType_Ambiguous),
	kNVSEParamType_Form = (1 << kTokenType_Form) | (1 << kTokenType_Ambiguous),
	kNVSEParamType_Array = (1 << kTokenType_Array) | (1 << kTokenType_Ambiguous),
	kNVSEParamType_ArrayElement = 1 << (kTokenType_ArrayElement) | (1 << kTokenType_Ambiguous),
	kNVSEParamType_Slice = 1 << kTokenType_Slice,
	kNVSEParamType_Command = 1 << kTokenType_Command,
	kNVSEParamType_Variable = 1 << kTokenType_Variable,
	kNVSEParamType_NumericVar = 1 << kTokenType_NumericVar,
	kNVSEParamType_RefVar = 1 << kTokenType_RefVar,
	kNVSEParamType_StringVar = 1 << kTokenType_StringVar,
	kNVSEParamType_ArrayVar = 1 << kTokenType_ArrayVar,
	kNVSEParamType_ForEachContext = 1 << kTokenType_ForEachContext,

	kNVSEParamType_Collection = kNVSEParamType_Array | kNVSEParamType_String,
	kNVSEParamType_ArrayVarOrElement = kNVSEParamType_ArrayVar | kNVSEParamType_ArrayElement,
	kNVSEParamType_ArrayIndex = kNVSEParamType_String | kNVSEParamType_Number,
	kNVSEParamType_BasicType = kNVSEParamType_Array | kNVSEParamType_String | kNVSEParamType_Number | kNVSEParamType_Form,
	kNVSEParamType_NoTypeCheck = 0,

	kNVSEParamType_FormOrNumber = kNVSEParamType_Form | kNVSEParamType_Number,
	kNVSEParamType_StringOrNumber = kNVSEParamType_String | kNVSEParamType_Number,
	kNVSEParamType_Pair = 1 << kTokenType_Pair,
};

struct NVSEParamInfo
{
	const char* typeStr;
	kNVSEParamType	typeID;		// ParamType
	UInt32	isOptional;	// do other bits do things?
};

static constexpr NVSEParamInfo kNVSEParams_OneArray[1] =
{
	{	"array",	kNVSEParamType_Array,	0	},
};
static constexpr NVSEParamInfo kNVSEParams_OneArray_OneForm[2] =
{
	{	"array",	kNVSEParamType_Array,	0	},
	{	"form",	kNVSEParamType_Form,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneString[1] =
{
	{	"string",	kNVSEParamType_String,	0	},
};
static constexpr NVSEParamInfo kNVSEParams_OneOptionalString[1] =
{
	{	"string",	kNVSEParamType_String,	1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneNumber[2] =
{
	{	"string",	kNVSEParamType_String,	0	},
	{	"num",	kNVSEParamType_Number,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneString_OneStringOrNumber[2] =
{
	{	"string",	kNVSEParamType_String,	0	},
	{	"string or num",	kNVSEParamType_StringOrNumber,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalNumber[2] =
{
	{	"string",	kNVSEParamType_String,	0	},
	{	"num",		kNVSEParamType_Number,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalForm[3] =
{
	{	"string",	kNVSEParamType_String,	0	},
	{	"form",		kNVSEParamType_Form,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalNumber_OneOptionalForm[3] =
{
	{	"string",	kNVSEParamType_String,	0	},
	{	"num",		kNVSEParamType_Number,	1	},
	{	"form",		kNVSEParamType_Form,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneBasicType_OneBoolean[2] =
{
	{	"string",	kNVSEParamType_BasicType,	0	},
	{	"bool",	kNVSEParamType_Boolean,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneBasicType[1] =
{
{	"element", kNVSEParamType_BasicType, 1	}
};


static constexpr NVSEParamInfo kNVSEParams_OneBasicType_OneOptionalBoolean[2] =
{
	{	"string",	kNVSEParamType_BasicType,	0	},
	{	"bool",	kNVSEParamType_Boolean,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneBasicType_OneString_OneOptionalNumber[3] =
{
	{	"basicType",	kNVSEParamType_BasicType,	0	},
	{	"string",		kNVSEParamType_String,		0	},
	{	"num",			kNVSEParamType_Number,		1	}
};

static constexpr NVSEParamInfo kNVSEParams_OneBasicType_OneString_OneOptionalNumber_OneOptionalForm[4] =
{
	{	"basicType",	kNVSEParamType_BasicType,	0	},
	{	"string",		kNVSEParamType_String,		0	},
	{	"num",			kNVSEParamType_Number,		1	},
	{	"form",			kNVSEParamType_Form,		1	}
};

static constexpr NVSEParamInfo kNVSEParams_OneOptionalString_TwoOptionalNumbers[3] =
{
	{	"string",	kNVSEParamType_String,	1	},
	{	"num",	kNVSEParamType_Number,	1	},
	{	"num",	kNVSEParamType_Number,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_ThreeVars[3] =
{
	{	"var",	kNVSEParamType_Variable,	0	},
	{	"var",	kNVSEParamType_Variable,	0	},
	{	"var",	kNVSEParamType_Variable,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneNumber_OneOptionalString[2] =
{
	{	"num",	kNVSEParamType_Number,	0	},
	{	"string",	kNVSEParamType_String,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_ThreeNumbers[3] =
{
	{	"num",	kNVSEParamType_Number,	0	},
	{	"num",	kNVSEParamType_Number,	0	},
	{	"num",	kNVSEParamType_Number,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_ThreeForms[3] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"form",	kNVSEParamType_Form,	0	},
	{	"form",	kNVSEParamType_Form,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneForm_OneNumber[2] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"num",	kNVSEParamType_Number,	0	},
};



static constexpr NVSEParamInfo kNVSEParams_OneForm_OneOptionalNumber[2] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"num",	kNVSEParamType_Number,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneNumber_OneString_OneOptionalString[3] =
{
	{	"num",	kNVSEParamType_Number,	0	},
	{	"string",	kNVSEParamType_String,	0	},
	{	"string",	kNVSEParamType_String,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneOptionalStringOrNumber[1] =
{
	{	"string or number",	kNVSEParamType_StringOrNumber,	1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneOptionalBoolean[1] =
{
	{	"bool",	kNVSEParamType_Boolean,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneForm_OneArray_OneOptionalIndex[3] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"array",	kNVSEParamType_Array,	0	},
	{	"index",	kNVSEParamType_Number,1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneForm_OneOptionalForm_OneOptionalInt[3] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"form",	kNVSEParamType_Form,	1	},
	{	"int",	kNVSEParamType_Number,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneArray_OneString_OneArray[3] =
{
	{	"array",	kNVSEParamType_Array,	0	},
	{	"string",	kNVSEParamType_String,0	},
	{	"array",	kNVSEParamType_Array,	0	},
};
static constexpr NVSEParamInfo kNVSEParams_OneArray_OneString_OneDouble[3] =
{
	{	"array",	kNVSEParamType_Array,	0	},
	{	"string",	kNVSEParamType_String,0	},
	{	"float",	kNVSEParamType_Number,0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneForm_OneArray[2] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"array",	kNVSEParamType_Array,	0	},
};
static constexpr NVSEParamInfo kNVSEParams_OneForm_OneString[2] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"string",	kNVSEParamType_String,	0	},
};
static constexpr NVSEParamInfo kNVSEParams_OneForm_OneOptionalInt[2] =
{
	{	"form",	kNVSEParamType_Form,	0	},
	{	"string",	kNVSEParamType_Number,	1	},
};


static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_OneOptionalInt[3] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"int",	kNVSEParamType_Number,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_TwoOptionalInts[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"int",	kNVSEParamType_Number,1	},
	{	"int",	kNVSEParamType_Number,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_OneOptionalInt_OneOptionalBool[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"int",	kNVSEParamType_Number,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneElem_OneString_OneOptionalString_OneOptionalInt[4] =
{
	{	"elem",	kNVSEParamType_BasicType,0	},
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"int",	kNVSEParamType_Number,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneElem_OneString_OneOptionalString_OneOptionalInt_TwoOptionalBools[6] =
{
	{	"elem",	kNVSEParamType_BasicType,0	},
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"int",	kNVSEParamType_Number,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneString_OneElem_TwoOptionalStrings[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"elem",	kNVSEParamType_BasicType,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneFloatOrString_TwoOptionalStrings[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"float or string",	kNVSEParamType_StringOrNumber,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneInt_TwoOptionalStrings[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"int",		kNVSEParamType_Number,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneFloatOrString_TwoOptionalStrings_TwoOptionalBools[6] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"float or string",	kNVSEParamType_StringOrNumber,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneFloat_TwoOptionalStrings[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"float",	kNVSEParamType_Number,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneFloat_TwoOptionalStrings_TwoOptionalBools[6] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"float",	kNVSEParamType_Number,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString[2] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_OneOptionalBool[3] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_TwoOptionalStrings[3] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_TwoOptionalStrings_OneOptionalBool[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_OneOptionalFloat[3] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"float",	kNVSEParamType_Number,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_OneOptionalFloat_OneOptionalBool[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"float",	kNVSEParamType_Number,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_OneOptionalFloat_OneOptionalString[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"float",	kNVSEParamType_Number,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_OneOptionalString_OneOptionalFloat_OneOptionalString_TwoOptionalBools[6] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"float",	kNVSEParamType_Number,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},

};
static constexpr NVSEParamInfo kNVSEParams_OneString_ThreeOptionalStrings[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_OneString_ThreeOptionalStrings_TwoOptionalBools[6] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};
static constexpr NVSEParamInfo kNVSEParams_TwoStrings_TwoOptionalStrings[4] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
};
static constexpr NVSEParamInfo kNVSEParams_TwoStrings_TwoOptionalStrings_TwoOptionalBools[6] =
{
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,0	},
	{	"string",	kNVSEParamType_String,1	},
	{	"string",	kNVSEParamType_String,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
	{	"bool",	kNVSEParamType_Boolean,1	},
};

static constexpr NVSEParamInfo kNVSEParams_TwoNums_OneArray_OneStr_ThreeForms[7] =
{
	{	"num",	kNVSEParamType_Number,	0	},
	{	"num",	kNVSEParamType_Number,	0	},

	{	"array",	kNVSEParamType_Array,	0},
	{	"string",	kNVSEParamType_String,	0	},

	{	"form",	kNVSEParamType_Form,	0	},
	{	"form",	kNVSEParamType_Form,	0	},
	{	"form",	kNVSEParamType_Form,	0	},
};

static constexpr NVSEParamInfo kNVSEParams_OneBasicType_OneFormOrNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber[7] =
{
	{	"value",	kNVSEParamType_BasicType,		0	},
	{	"index",	kNVSEParamType_FormOrNumber,	0	},
	{	"form",		kNVSEParamType_BasicType,		0	},
	{	"string",	kNVSEParamType_BasicType,		1	},
	{	"form",		kNVSEParamType_BasicType,		1	},
	{	"string",	kNVSEParamType_BasicType,		1	},
	{	"num",		kNVSEParamType_BasicType,		1	}
};

static constexpr NVSEParamInfo kNVSEParams_OneFormOrNumber_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber[6] =
{
	{	"index",	kNVSEParamType_FormOrNumber,	0	},
	{	"form",		kNVSEParamType_BasicType,		0	},
	{	"string",	kNVSEParamType_BasicType,		1	},
	{	"form",		kNVSEParamType_BasicType,		1	},
	{	"string",	kNVSEParamType_BasicType,		1	},
	{	"num",		kNVSEParamType_BasicType,		1	}
};

static constexpr NVSEParamInfo  kNVSE_Event_OneNumber_TwoFormsF[7] =
{
	{"priority", kNVSEParamType_Number,	0},
	{"script", kNVSEParamType_Form, 0},
	{"int", kNVSEParamType_Number,	0},
	{"order1", kNVSEParamType_String, 1},
	{"form", kNVSEParamType_Form, 1},
	{"order2", kNVSEParamType_String, 1},
	{"form", kNVSEParamType_Form, 1}
};

static constexpr NVSEParamInfo  kNVSE_Event_OneFormF_OneStringF[7] =
{
	{"priority", kNVSEParamType_Number,	0},
	{"script", kNVSEParamType_Form, 0},
	{"order1", kNVSEParamType_String, 1},
	{"form", kNVSEParamType_Form, 1},
	{"order2", kNVSEParamType_String, 1},
	{"form", kNVSEParamType_String, 1}
};

static constexpr NVSEParamInfo kNVSE_OneOptionalString_OneOptionalInt_OneOptionalForm[3] =
{
	{	"string",	kNVSEParamType_String,	1	},
	{	"int",		kNVSEParamType_Number,	1	},
	{	"form",		kNVSEParamType_Form,	1	},
};

static constexpr NVSEParamInfo kNVSEParams_OneForm_OneString_OneOptionalForm_OneOptionalString_OneOptionalNumber[5] =
{
	{	"form",		kNVSEParamType_Form,		0	},
	{	"string",	kNVSEParamType_String,		0	},
	{	"form",		kNVSEParamType_Form,		1	},
	{	"string",	kNVSEParamType_String,		1	},
	{	"num",		kNVSEParamType_Number,		1	}
};

static constexpr NVSEParamInfo kNVSEParams_CallLoop[19] =
{
	{	"seconds",	kNVSEParamType_Number,	0	},
	{	"function",	kNVSEParamType_Form,	0	},

	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},

	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},

	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},

	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"element",	kNVSEParamType_BasicType,	1	},

};

static constexpr NVSEParamInfo EventParams3_MultiType[8] =
{
	{	"num",		kNVSEParamType_Number,		0	},	//Priority
	{	"form",		kNVSEParamType_Form,		0	},	//Script

	{	"string",	kNVSEParamType_String,		1	},	//Filter
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"string",	kNVSEParamType_String,		1	},	//Filters
	{	"element",	kNVSEParamType_BasicType,	1	},
	{	"string",	kNVSEParamType_String,		1	},	//Filters
	{	"element",	kNVSEParamType_BasicType,	1	},
};