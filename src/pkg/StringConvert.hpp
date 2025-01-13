#pragma once

#ifndef _BECAM_STRING_CONVERT_H_
#define _BECAM_STRING_CONVERT_H_

#include <codecvt>
#include <locale>
#include <string>
#include <stringapiset.h>

/**
 * @brief 将 std::string 转换为 std::wstring
 */
static std::wstring StringToWString(const std::string& str) {
	// 创建转换器
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	// 将 std::string 转换为 std::wstring
	std::wstring dstStr = converter.from_bytes(str);
	// 返回
	return dstStr;
}

/**
 * @brief 将 std::wstring 转换为 std::string
 */
static std::string WStringToString(const std::wstring& str) {
	// 创建转换器
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	// 将 std::wstring 转换为 std::string
	std::string dstStr = converter.to_bytes(str);
	// 返回
	return dstStr;
}

#endif
