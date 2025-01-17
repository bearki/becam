#pragma once

#ifndef _BECAM_STRING_CONVERT_H_
#define _BECAM_STRING_CONVERT_H_

#include <codecvt>
#include <locale>
#include <string>
#include <algorithm>

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

/**
 * @brief 移除字符串左侧的空格
 *
 * @param str 原始字符串
 * @return 处理后的字符串
 */
static std::string TrimLeftSpace(const std::string& str) {
	auto it = std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); });
	return std::string(it, str.end());
}

/**
 * @brief 移除字符串右侧的空格
 *
 * @param str 原始字符串
 * @return 处理后的字符串
 */
static std::string TrimRightSpace(const std::string& str) {
	auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base();
	return std::string(str.begin(), it);
}

/**
 * @brief 移除字符串两侧的空格
 *
 * @param str 原始字符串
 * @return 处理后的字符串
 */
static std::string TrimSpace(const std::string& str) {
	// 复用处理函数
	return TrimRightSpace(TrimLeftSpace(str));
}

#endif
