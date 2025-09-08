#pragma once

//Extra Utilities
class case_insensitive_set : public std::set<std::string>
{
public:
	case_insensitive_set(std::initializer_list<std::string> items)
	{
		for (const auto& cur : items)
		{
			insert(cur);
		}
	}

	void insert(std::string data)
	{
		std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::tolower(c); });
		std::set<std::string>::insert(data);
	}

	bool contains(const std::string& data) const
	{
		std::string lower_data = data;
		std::transform(lower_data.begin(), lower_data.end(), lower_data.begin(), [](unsigned char c) { return std::tolower(c); });
		return std::set<std::string>::contains(lower_data);
	}
};

inline UInt8* GetParentBasePtr(void* addressOfReturnAddress, const bool lambda = false) {
	auto* basePtr = static_cast<UInt8*>(addressOfReturnAddress) - 4;
#if _DEBUG
	// in debug mode, lambdas are wrapped inside a closure wrapper function, so one more step needed
	if (lambda) {
		basePtr = *reinterpret_cast<UInt8**>(basePtr);
	}
#endif
	return *reinterpret_cast<UInt8**>(basePtr);
}

inline UInt64 MakeHashKey(UInt32 formID1, UInt32 formID2)
{
	return ((UInt64)formID1 << 32) | formID2;
}

template<typename T>
T InterpolateTowards(T minVal, T maxVal, float ratio) {
	float interp = minVal + (maxVal - minVal) * ratio;
	float low = (std::min)(minVal, maxVal);
	float high = (std::max)(minVal, maxVal);
	return static_cast<T>(std::round(std::clamp(interp, low, high)));
}

template<typename T>
T InterpolateBasePercent(T baseValue, float minPercent, float maxPercent, float ratio)
{
	float minVal = minPercent / 100.0f;
	float maxVal = maxPercent / 100.0f;
	float totalPercent = minVal + (maxVal - minVal) * ratio;
	float low = (std::min)(minVal, maxVal);
	float high = (std::max)(minVal, maxVal);
	totalPercent = std::clamp(totalPercent, low, high);

	return static_cast<T>(baseValue * totalPercent);
}

template <typename T>
T ScaleByPercentRange(T baseVal, T min, T max, float ratio)
{
	ratio = std::clamp(ratio, 0.0f, 1.0f);
	if (baseVal == static_cast<T>(0)) {
		return static_cast<T>(0);
	}
	float minPercent = static_cast<float>(min) / static_cast<float>(baseVal);
	float maxPercent = static_cast<float>(max) / static_cast<float>(baseVal);
	float totalPercent = minPercent + (maxPercent - minPercent) * ratio;
	float low = (std::min)(minPercent, maxPercent);
	float high = (std::max)(minPercent, maxPercent);
	totalPercent = std::clamp(totalPercent, low, high);

	return static_cast<T>(baseVal * totalPercent);
}

inline std::vector<std::string> SplitByDelimiter(const std::string& str, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delimiter)) 
	{
		size_t start = item.find_first_not_of(" \t");
		size_t end = item.find_last_not_of(" \t");
		if (start != std::string::npos && end != std::string::npos)
		result.push_back(item.substr(start, end - start + 1));
	}
	return result;
}

template<typename T, size_t N>
inline std::string FlagsToString(T flags, const std::array<std::pair<T, std::string_view>, N>& flagNames)
{
	if (flags == T(0)) return "None";
	if (flags == ~T(0)) return "All";

	std::ostringstream oss;
	bool first = true;

	for (const auto& [flag, name] : flagNames)
	{
		if (flags & flag)
		{
			if (!first) oss << " | ";
			oss << name;
			first = false;
		}
	}
	return oss.str();
}

template<typename T, size_t N>
inline T StringToFlags(const std::string& str, char delimter, const std::array<std::pair<T, std::string_view>, N>& flagNames)
{
	T result = 0;
	auto strings = SplitByDelimiter(str, delimter);

	for (const auto& string : strings)
	{
		for (const auto& [flag, name] : flagNames)
		{
			if (string == name)
			{
				result |= flag;
				break;
			}
		}
	}
	return result;
}
