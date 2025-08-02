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
