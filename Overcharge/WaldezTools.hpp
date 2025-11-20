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

inline bool CaseInsensitiveCmp(std::string_view a, std::string_view b)
{
	if (a.size() != b.size()) return false;
	return std::equal(a.begin(), a.end(), b.begin(),
		[](char c1, char c2) {
			return std::tolower(static_cast<unsigned char>(c1)) ==
				std::tolower(static_cast<unsigned char>(c2));
		});
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
	return static_cast<T>(std::clamp(interp, low, high));
}

template<typename T>
T InterpolateBase(T baseValue, float minPercent, float maxPercent, float ratio)
{
	float interpPercent = minPercent + (maxPercent - minPercent) * ratio;
	float low = (std::min)(minPercent, maxPercent);
	float high = (std::max)(minPercent, maxPercent);
	interpPercent = std::clamp(interpPercent, low, high);
	return static_cast<T>(baseValue * interpPercent);
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

inline float ParseDelimitedData(std::string_view sv, char openDelimiter = '\0', char closeDelimiter = '\0')
{
	if (!sv.empty() && openDelimiter != '\0' && sv.front() == openDelimiter)
		sv.remove_prefix(1);

	if (!sv.empty() && closeDelimiter != '\0' && sv.back() == closeDelimiter)
		sv.remove_suffix(1);

	float value = 0.0f;
	std::from_chars(sv.data(), sv.data() + sv.size(), value);
	return value;
}

inline std::vector<std::string_view> SplitByDelimiter(std::string_view str, char delimiter)
{
	std::vector<std::string_view> result;
	size_t start = 0;

	while (start < str.size())
	{
		size_t end = str.find(delimiter, start);
		if (end == std::string_view::npos) end = str.size();

		size_t tokenStart = str.find_first_not_of(" \t", start);
		size_t tokenEnd = str.find_last_not_of(" \t", end - 1);

		if (tokenStart != std::string_view::npos && tokenEnd != std::string_view::npos)
			result.push_back(str.substr(tokenStart, tokenEnd - tokenStart + 1));

		start = end + 1;
	}

	return result;
}

template<typename T, size_t N>
inline std::string FlagsToString(T flags, const std::array<std::pair<T, std::string_view>, N>& flagNames)
{
	if (flags == T(0)) return "None";
	if (flags == ~T(0)) return "All";

	std::string result;

	size_t maxSize = 0;
	for (const auto& [flag, name] : flagNames)
	{
		maxSize += name.size() + 3; 
	}
	result.reserve(maxSize);

	bool first = true;
	for (const auto& [flag, name] : flagNames)
	{
		if (flags & flag)
		{
			if (!first) result.append(" , ");
			result.append(name);
			first = false;
		}
	}
	return result;
}

inline std::string_view Trim(std::string_view sv) 
{
	auto is_space = [](unsigned char c) { return std::isspace(c); };
	while (!sv.empty() && is_space(sv.front())) {
		sv.remove_prefix(1);
	}
	while (!sv.empty() && is_space(sv.back())) {
		sv.remove_suffix(1);
	}
	return sv;
}

template<typename T, size_t N>
inline T StringToFlags(std::string_view str, char delimiter, const std::array<std::pair<T, std::string_view>, N>& flagNames)
{
	T result = 0;
	size_t start = 0;
	while (start < str.size()) {
		size_t end = str.find(delimiter, start);
		if (end == std::string_view::npos) end = str.size();
		std::string_view token = Trim(str.substr(start, end - start));
		if (!token.empty()) {
			for (const auto& [flag, name] : flagNames) {
				if (std::equal(token.begin(), token.end(),
					name.begin(), name.end(),
					[](unsigned char a, unsigned char b) 
					{
						return std::tolower(a) == std::tolower(b);
					}))
				{
					result |= flag;
					break;
				}
			}
		}
		start = end + 1;
	}
	return result;
}

template <typename T, size_t N>
std::string_view EnumToString(T value, const std::array<std::pair<T, std::string_view>, N>& table)
{
	for (const auto& [num, str] : table) {
		if (value == num) {
			return str;
		}
	}
	return "Unk";
}

template <typename Container, typename T>
inline bool ContainsValue(const Container& c, const T& value)
{
	return std::find(c.begin(), c.end(), value) != c.end();
}