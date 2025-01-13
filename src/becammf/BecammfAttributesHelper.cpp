#include "BecammfAttributesHelper.hpp"
#include <mfapi.h>
#include <pkg/SafeRelease.hpp>

/**
 * @implements 实现构造函数
 */
BecammfAttributesHelper::BecammfAttributesHelper() {}

/**
 * @implements 实现构造函数
 */
BecammfAttributesHelper::BecammfAttributesHelper(IMFAttributes* attributes) {
	// 接管实例
	this->_Attributes = attributes;
}

/**
 * @implements 实现析构函数
 */
BecammfAttributesHelper::~BecammfAttributesHelper() {
	// 释放托管实例
	SafeRelease(&this->_Attributes);
}

/**
 * @implements 实现获取属性存储器实例存储地址
 */
IMFAttributes** BecammfAttributesHelper::AttributesAddress() { return &this->_Attributes; }

/**
 * @implements 实现获取属性存储器实例
 */
IMFAttributes* BecammfAttributesHelper::Attributes() { return this->_Attributes; }
