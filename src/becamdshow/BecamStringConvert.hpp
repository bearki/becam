#ifndef _BECAM_STRING_CONVERT_H_
#define _BECAM_STRING_CONVERT_H_

#include <string>
#include <stringapiset.h>

/**
 * @brief 字符串转换
 * // @note 由于C++17中std::wstring和std::string之间的转换会自动转换编码，所以这里不需要再做编码转换
 */
static std::wstring CharToWchar(const char* c, size_t m_encode = CP_ACP) {
	std::wstring str;
	int len = MultiByteToWideChar(m_encode, 0, c, strlen(c), NULL, 0);
	wchar_t* m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(m_encode, 0, c, strlen(c), m_wchar, len);
	m_wchar[len] = '\0';
	str = m_wchar;
	delete m_wchar;
	return str;
}

/**
 * @brief 字符串转换
 * // @note 由于C++17中std::wstring和std::string之间的转换会自动转换编码，所以这里不需要再做编码转换
 */
static std::string WcharToChar(const wchar_t* wp, size_t m_encode = CP_UTF8) {
	std::string str;
	int len = WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
	char* m_char = new char[len + 1];
	WideCharToMultiByte(m_encode, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	str = m_char;
	delete m_char;
	return str;
}

#endif
